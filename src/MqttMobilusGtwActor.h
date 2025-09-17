#pragma once

#include "Actor.h"

#include <jungi/mobilus_gtw_client/MqttDsn.h>
#include <jungi/mobilus_gtw_client/MobilusCredentials.h>
#include <jungi/mobilus_gtw_client/MqttMobilusGtwClient.h>
#include <jungi/mobilus_gtw_client/proto/CallEvents.pb.h>

#include <string>
#include <memory>
#include <future>

namespace moblink {

class TargetMqttActor;

class MqttMobilusGtwActor final : public Actor<jungi::mobilus_gtw_client::MqttMobilusGtwClient> {
public:
    using BaseActor = Actor<jungi::mobilus_gtw_client::MqttMobilusGtwClient>;

    MqttMobilusGtwActor(jungi::mobilus_gtw_client::MqttDsn dsn, jungi::mobilus_gtw_client::MobilusCredentials mobilusCreds, std::promise<void>& onFinished = BaseActor::defaultFinishedPromise);

    void pushEventsTo(TargetMqttActor* targetMqttActor);
    void sendCommandToDevice(long deviceId, std::string command);

protected:
    void run() override;

private:
    jungi::mobilus_gtw_client::MqttDsn mDsn;
    jungi::mobilus_gtw_client::MobilusCredentials mMobilusCreds;
    TargetMqttActor* mTargetMqttActor = nullptr;

    void push(const jungi::mobilus_gtw_client::proto::CallEvents& callEvents);
};

}
