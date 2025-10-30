#include "mobilus_cert.h"
#include "MqttMobilusGtwActor.h"
#include "TargetMqttActor.h"

#include <filesystem/TempFile.h>

#include <future>
#include <iostream>
#include <cstdlib>
#include <cstdio>
#include <string>
#include <csignal>

#ifndef DEFAULT_MOBILUS_DSN
#define DEFAULT_MOBILUS_DSN "mqtts://mobilus:8883"
#endif

using namespace moblink;
using namespace jungi::mobilus_gtw_client;
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

std::promise<void> gStop;

void handleStopSignal(int)
{
    try {
        gStop.set_value();
    } catch (const std::future_error& err) {
        // ignore
    }
}

int main()
{
    auto mobilusDsn = mqttDsnFrom("MOBILUS_DSN").value_or(kDefaultMobilusDsn);
    auto targetDsn = mqttDsnFrom("TARGET_DSN").value_or(kDefaultMobilusDsn);
    auto mobilusUsername = envFrom("MOBILUS_USERNAME").value_or("admin");
    auto mobilusPassword = envFrom("MOBILUS_PASSWORD").value_or("admin");
    auto rootTopic = envFrom("ROOT_TOPIC");

    applyMobilusCaCert(mobilusDsn);

    MqttMobilusGtwActor mobilusGtwActor(std::move(mobilusDsn), { std::move(mobilusUsername), std::move(mobilusPassword) }, gStop);
    TargetMqttActor targetMqttActor(std::move(targetDsn), std::move(rootTopic), gStop);

    mobilusGtwActor.pushEventsTo(&targetMqttActor);
    targetMqttActor.pushCommandsTo(&mobilusGtwActor);

    signal(SIGINT, handleStopSignal);
    signal(SIGTERM, handleStopSignal);

    mobilusGtwActor.start();
    targetMqttActor.start();

    auto stopFut = gStop.get_future();
    stopFut.wait();

    return 0;
}
