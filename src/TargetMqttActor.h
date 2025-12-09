#pragma once

#include "Actor.h"
#include "Messages.h"
#include "TargetMqttClient.h"

#include <jungi/mobgtw/MqttDsn.h>

#include <string>
#include <optional>

namespace moblink {

class MqttMobilusGtwActor;

class TargetMqttActor final : public Actor<TargetMqttActor, TargetMqttCommandVariant> {
public:
    using Command = TargetMqttCommandVariant;
    using Actor<TargetMqttActor, Command>::handle;

    TargetMqttActor(jungi::mobgtw::MqttDsn dsn, std::optional<std::string> rootTopic = std::nullopt);
    ~TargetMqttActor();

    void pushCommandsTo(MqttMobilusGtwActor* actor);
    void publishDeviceState(long deviceId, std::string state);
    void publishDeviceError(long deviceId, std::string error);
    void publishDevicePendingCommand(long deviceId, std::string command);

    void handle(const PushCommandsToActorCommand& cmd);
    void handle(const PublishDeviceStateCommand& cmd);
    void handle(const PublishDeviceErrorCommand& cmd);
    void handle(const PublishDevicePendingCommand& cmd);

protected:
    void run() override;

private:
    TargetMqttClient mClient;

};

}
