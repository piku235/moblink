#include "MqttMobilusGtwActor.h"
#include "TargetMqttActor.h"
#include "filesystem/TempFile.h"
#include "MobilusCacert.h"

#include <future>
#include <iostream>
#include <cstdlib>
#include <cstdio>
#include <string>
#include <csignal>

using namespace moblink;
using namespace moblink::filesystem;
using namespace jungi::mobilus_gtw_client;

TempFile loadMobilusCa()
{
    auto tempFile = TempFile::unique("/tmp/mobilus_ca_XXXXXX");
    tempFile.write(kMobilusCacert, sizeof(kMobilusCacert));

    return tempFile;
}

auto applyMobilusCacert(MqttDsn& dsn)
{
    if (dsn.secure && !dsn.cacert) {
        static auto cacert = loadMobilusCa();
        dsn.cacert = cacert.path();
    }

    return dsn;
}

std::optional<MqttDsn> mqttDsnFrom(const char* env)
{
    const auto value = getenv(env);

    if (nullptr == value) {
        return std::nullopt;
    }

    const auto dsn = MqttDsn::from(value);

    if (!dsn) {
        return std::nullopt;
    }

    return dsn;
}

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
    auto mobilusDsn = mqttDsnFrom("MOBILUS_DSN");
    auto targetDsn = mqttDsnFrom("TARGET_DSN");
    auto mobilusUsername = getenv("MOBILUS_USERNAME");
    auto mobilusPassword = getenv("MOBILUS_PASSWORD");
    auto rootTopic = getenv("ROOT_TOPIC");

    if (!mobilusDsn) {
        std::cerr << "MOBILUS_DSN is missing or malformed" << std::endl;
        return 1;
    }

    if (!targetDsn) {
        std::cerr << "TARGET_DSN is missing or malformed" << std::endl;
        return 1;
    }

    applyMobilusCacert(*mobilusDsn);

    MqttMobilusGtwActor mobilusGtwActor(*mobilusDsn, { nullptr != mobilusUsername ? mobilusUsername : "admin", nullptr != mobilusPassword ? mobilusPassword : "admin" }, gStop);
    TargetMqttActor targetMqttActor(*targetDsn, nullptr != rootTopic ? std::optional(rootTopic) : std::nullopt, gStop);

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
