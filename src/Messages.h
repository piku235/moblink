#pragma once

#include <string>
#include <variant>

namespace moblink {

class TargetMqttActor;
class MqttMobilusGtwActor;

struct SendCommandToDeviceCommand final {
    long deviceId;
    std::string command;
};

struct PushEventsToActorCommand final {
    TargetMqttActor* actor;
};

struct PublishDeviceStateCommand final {
    long deviceId;
    std::string state;
};

struct PublishDeviceErrorCommand final {
    long deviceId;
    std::string error;
};

struct PublishDevicePendingCommand final {
    long deviceId;
    std::string command;
};

struct PushCommandsToActorCommand final {
    MqttMobilusGtwActor* actor;
};

struct StopActorCommand final {};

using MobilusGtwCommandVariant = std::variant<
    PushEventsToActorCommand,
    SendCommandToDeviceCommand,
    StopActorCommand
>;
using TargetMqttCommandVariant = std::variant<
    PushCommandsToActorCommand,
    PublishDeviceStateCommand,
    PublishDeviceErrorCommand,
    PublishDevicePendingCommand,
    StopActorCommand
>;

}
