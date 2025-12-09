#include "mobilus_cert.h"
#include "MqttMobilusGtwActor.h"
#include "TargetMqttClient.h"
#include "TargetMqttActor.h"

#include <jungi/mobgtw/MqttMobilusGtwClient.h>
#include <filesystem/TempFile.h>

#include <latch>
#include <cstdlib>
#include <cstdio>
#include <string>
#include <csignal>

#ifndef DEFAULT_MOBILUS_DSN
#define DEFAULT_MOBILUS_DSN "mqtts://mobilus:8883"
#endif

using namespace moblink;
using namespace jungi::mobgtw;
using filesystem::TempFile;

TempFile loadMobilusCaCert()
{
    auto tempFile = TempFile::unique("/tmp/mobilus_ca_XXXXXX");
    tempFile.write(kMobilusCaCert, sizeof(kMobilusCaCert));

    return tempFile;
}

auto applyMobilusCaCert(MqttDsn& dsn)
{
    if (dsn.secure && !dsn.cacert) {
        static auto cacert = loadMobilusCaCert();
        dsn.cacert = cacert.path();
    }

    return dsn;
}

std::optional<std::string> envFrom(const char* env)
{
    const auto value = getenv(env);

    return nullptr != value ? std::optional(value) : std::nullopt;
}

std::optional<MqttDsn> mqttDsnFrom(const char* env)
{
    auto r = envFrom(env);

    return r ? MqttDsn::from(*r) : std::nullopt;
}

const MqttDsn kDefaultMobilusDsn = []() {
    auto dsn = MqttDsn::from(DEFAULT_MOBILUS_DSN).value();
    applyMobilusCaCert(dsn);

    return dsn;
}();

std::latch gExit(1);

void handleSignal(int signal)
{
    gExit.count_down();
}

int main()
{
    auto mobilusDsn = mqttDsnFrom("MOBILUS_DSN").value_or(kDefaultMobilusDsn);
    auto targetDsn = mqttDsnFrom("TARGET_DSN").value_or(kDefaultMobilusDsn);
    auto mobilusUsername = envFrom("MOBILUS_USERNAME").value_or("admin");
    auto mobilusPassword = envFrom("MOBILUS_PASSWORD").value_or("admin");
    auto rootTopic = envFrom("ROOT_TOPIC");

    applyMobilusCaCert(mobilusDsn);

    MqttMobilusGtwActor mobilusGtwActor(MqttMobilusGtwClient::builder().dsn(mobilusDsn).login({ mobilusUsername, mobilusPassword }));
    TargetMqttActor targetMqttActor(std::move(targetDsn), std::move(rootTopic));
    
    mobilusGtwActor.latch(&gExit);
    targetMqttActor.latch(&gExit);

    mobilusGtwActor.pushEventsTo(&targetMqttActor);
    targetMqttActor.pushCommandsTo(&mobilusGtwActor);

    signal(SIGINT, handleSignal);
    signal(SIGTERM, handleSignal);

    gExit.wait();

    return 0;
}
