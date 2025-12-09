#include "TargetMqttActor.h"
#include "TargetMqttClient.h"
#include "MqttMobilusGtwActor.h"

#include <jungi/mobgtw/io/SelectEventLoop.h>

#include <iostream>
#include <format>

using namespace jungi::mobgtw;

namespace moblink {

TargetMqttActor::TargetMqttActor(MqttDsn dsn, std::optional<std::string> rootTopic)
    : mClient(std::move(dsn), mLoop)
{
    if (rootTopic) {
        mClient.setRootTopic(std::move(*rootTopic));
    }

    start();
}

TargetMqttActor::~TargetMqttActor()
{
    shutdown();
}

void TargetMqttActor::run()
{
    if (!mClient.connect()) {
        std::cout << "Unable to connect to target MQTT broker\n";
        return;
    }

    std::cout << "Connected to target MQTT broker\n";
    loop();
}

void TargetMqttActor::pushCommandsTo(MqttMobilusGtwActor* actor)
{
    enqueue(PushCommandsToActorCommand { actor });
}

void TargetMqttActor::publishDeviceState(long deviceId, std::string state)
{
    enqueue(PublishDeviceStateCommand { deviceId, std::move(state) });
}

void TargetMqttActor::publishDeviceError(long deviceId, std::string error)
{
    enqueue(PublishDeviceErrorCommand { deviceId, std::move(error) });
}

void TargetMqttActor::publishDevicePendingCommand(long deviceId, std::string command)
{
    enqueue(PublishDevicePendingCommand { deviceId, std::move(command) });
}

void TargetMqttActor::handle(const PushCommandsToActorCommand& cmd)
{
    mClient.subscribeDeviceCommands([actor = cmd.actor](long deviceId, std::string command) {
        actor->sendCommandToDevice(deviceId, command);
    });
}

void TargetMqttActor::handle(const PublishDeviceStateCommand& cmd)
{
    mClient.publishDeviceState(cmd.deviceId, cmd.state);
    std::cout << std::format("Published state: {} for device id: {}\n", cmd.state, cmd.deviceId);
}

void TargetMqttActor::handle(const PublishDeviceErrorCommand& cmd)
{
    mClient.publishDeviceError(cmd.deviceId, cmd.error);
    std::cout << std::format("Published error: {} for device id: {}\n", cmd.error, cmd.deviceId);
}

void TargetMqttActor::handle(const PublishDevicePendingCommand& cmd)
{
    mClient.publishDevicePendingCommand(cmd.deviceId, cmd.command);
    std::cout << std::format("Published pending command: {} for device id: {}\n", cmd.command, cmd.deviceId);
}

}
