#pragma once

#include "Actor.h"
#include "Messages.h"

#include <jungi/mobgtw/MqttMobilusGtwClient.h>

#include <memory>

namespace moblink {

class TargetMqttActor;

class MqttMobilusGtwActor final : public Actor<MqttMobilusGtwActor, MobilusGtwCommandVariant> {
public:
    using Command = MobilusGtwCommandVariant;
    using Actor<MqttMobilusGtwActor, Command>::handle;

    MqttMobilusGtwActor(jungi::mobgtw::MqttMobilusGtwClient::Builder& builder);
    ~MqttMobilusGtwActor();

    void pushEventsTo(TargetMqttActor* actor);
    void sendCommandToDevice(long deviceId, std::string command);

    void handle(const PushEventsToActorCommand& cmd);
    void handle(const SendCommandToDeviceCommand& cmd);

protected:
    void run() override;

private:
    std::unique_ptr<jungi::mobgtw::MqttMobilusGtwClient> mClient;

};

}
