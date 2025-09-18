#include "MqttMobilusGtwActor.h"
#include "TargetMqttActor.h"

#include <jungi/mobilus_gtw_client/Platform.h>
#include <jungi/mobilus_gtw_client/EventNumber.h>
#include <jungi/mobilus_gtw_client/MessageType.h>

#include <iostream>

using namespace jungi::mobilus_gtw_client;
using std::chrono::steady_clock;

namespace moblink {

MqttMobilusGtwActor::MqttMobilusGtwActor(mobgtw::MqttDsn dsn, mobgtw::MobilusCredentials mobilusCreds, std::promise<void>& onFinished)
    : BaseActor(onFinished)
    , mDsn(std::move(dsn))
    , mMobilusCreds(std::move(mobilusCreds))
{
}

void MqttMobilusGtwActor::pushEventsTo(TargetMqttActor* targetMqttActor)
{
    mTargetMqttActor = targetMqttActor;
}

void MqttMobilusGtwActor::sendCommandToDevice(long deviceId, std::string command)
{
    post([deviceId, command = std::move(command)](io::EventLoop&, MqttMobilusGtwClient& client) {
        proto::CallEvents callEvents;
        
        auto* event = callEvents.add_events();
        event->set_device_id(deviceId);
        event->set_value(command);
        event->set_event_number(EventNumber::Triggered);
        event->set_platform(Platform::Web);

        (void)client.send(callEvents);
    });
}

void MqttMobilusGtwActor::run()
{
    io::SelectEventLoop loop;
    auto client = MqttMobilusGtwClient::builder()
        .dsn(std::move(mDsn))
        .login(std::move(mMobilusCreds))
        .attachTo(&loop)
        .build();

    client->messageBus().subscribe<proto::CallEvents>(MessageType::CallEvents, [this](const auto& callEvents) { push(callEvents); });

    if (auto r = client->connect(); !r) {
        std::cerr << r.error().message() << std::endl;
        return;
    }

    std::cout << "Connected to mobilus" << std::endl;
    runLoop(loop, *client);
}

void MqttMobilusGtwActor::push(const proto::CallEvents& callEvents)
{
    if (callEvents.events_size() < 1 || nullptr == mTargetMqttActor) {
        return;
    }

    auto& event = callEvents.events(0);


    switch (event.event_number()) {
    case EventNumber::Sent:
        mTargetMqttActor->publishDevicePendingCommand(event.device_id(), event.value());
        break;
    case EventNumber::Reached:
        mTargetMqttActor->publishDeviceState(event.device_id(), event.value());
        break;
    case EventNumber::Error:   
        mTargetMqttActor->publishDeviceError(event.device_id(), event.value());
        break; 
    }
}

}
