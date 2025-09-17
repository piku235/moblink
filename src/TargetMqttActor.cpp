#include "TargetMqttActor.h"
#include "MqttMobilusGtwActor.h"

#include <jungi/mobilus_gtw_client/io/SelectEventLoop.h>

#include <iostream>

using namespace jungi::mobilus_gtw_client;

namespace moblink {

TargetMqttActor::TargetMqttActor(MqttDsn dsn, std::promise<void>& onFinished)
    : BaseActor(onFinished)
    , mDsn(std::move(dsn))
{
}

void TargetMqttActor::pushCommandsTo(MqttMobilusGtwActor* mobGtwMqttActor)
{
    mMobGtwMqttActor = mobGtwMqttActor;
}

void TargetMqttActor::publishDeviceState(long deviceId, std::string state)
{
    post([deviceId, state = std::move(state)](io::EventLoop&, TargetMqttClient& client) {
        client.publishDeviceState(deviceId, state);
        std::cout << "Published state: " << state << " for device id: " << deviceId << std::endl;
    });
}

void TargetMqttActor::publishDeviceError(long deviceId, std::string error)
{
    post([deviceId, error = std::move(error)](io::EventLoop&, TargetMqttClient& client) {
        client.publishDeviceError(deviceId, error);
        std::cout << "Published error: " << error << " for device id: " << deviceId << std::endl;
    });
}

void TargetMqttActor::publishDeviceRequestedCommand(long deviceId, std::string command)
{
    post([deviceId, command = std::move(command)](io::EventLoop&, TargetMqttClient& client) {
        client.publishDeviceRequestedCommand(deviceId, command);
        std::cout << "Published requested command: " << command << " for device id: " << deviceId << std::endl;
    });
}

void TargetMqttActor::run()
{
    io::SelectEventLoop loop;
    TargetMqttClient client(std::move(mDsn), loop);

    client.subscribeDeviceCommands([this](long deviceId, std::string command) { push(deviceId, std::move(command)); });

    if (!client.connect()) {
        std::cerr << "Unable to connect to target MQTT broker" << std::endl;
        return;
    }

    std::cout << "Connected to target MQTT broker" << std::endl;
    runLoop(loop, client);
}

void TargetMqttActor::push(long deviceId, std::string command)
{
    if (nullptr != mMobGtwMqttActor) {
        mMobGtwMqttActor->sendCommandToDevice(deviceId, command);
        std::cout << "Sent command: " << command << " to device id: " << deviceId << std::endl;
    }
}

}
