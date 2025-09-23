#pragma once

#include "Actor.h"
#include "TargetMqttClient.h"

#include <jungi/mobilus_gtw_client/MqttDsn.h>

#include <string>
#include <thread>
#include <queue>
#include <mutex>
#include <memory>
#include <optional>

namespace moblink {

class MqttMobilusGtwActor;

class TargetMqttActor final : public Actor<TargetMqttClient> {
public:
    using BaseActor = Actor<TargetMqttClient>;
    
    TargetMqttActor(jungi::mobilus_gtw_client::MqttDsn dsn, std::optional<std::string> rootTopic = std::nullopt, std::promise<void>& onFinished = BaseActor::defaultFinishedPromise);

    void pushCommandsTo(MqttMobilusGtwActor* mobGtwMqttActor);
    void publishDeviceState(long deviceId, std::string state);
    void publishDeviceError(long deviceId, std::string error);
    void publishDevicePendingCommand(long deviceId, std::string command);

protected:
    void run() override;

private:
    jungi::mobilus_gtw_client::MqttDsn mDsn;
    std::optional<std::string> mRootTopic;
    MqttMobilusGtwActor* mMobGtwMqttActor = nullptr;

    void push(long deviceId, std::string command);
};

}
