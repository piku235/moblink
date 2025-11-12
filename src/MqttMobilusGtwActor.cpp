#include "MqttMobilusGtwActor.h"
#include "TargetMqttActor.h"

#include <jungi/mobilus_gtw_client/Platform.h>
#include <jungi/mobilus_gtw_client/EventNumber.h>
#include <jungi/mobilus_gtw_client/MessageType.h>
#include <jungi/mobilus_gtw_client/proto/CallEvents.pb.h>

#include <iostream>
#include <format>

using namespace jungi::mobilus_gtw_client;
using std::chrono::steady_clock;

namespace moblink {

MqttMobilusGtwActor::MqttMobilusGtwActor(mobgtw::MqttMobilusGtwClient::Builder& builder)
    : mClient(builder.attachTo(&mLoop).build())
{
    start();
}

MqttMobilusGtwActor::~MqttMobilusGtwActor()
{
    shutdown();
}

void MqttMobilusGtwActor::run()
{
    if (auto r = mClient->connect(); !r) {
        std::cerr << std::format("Connection failed: {}\n",  r.error().message());
        return;
    }

    std::cout << "Connected to mobilus\n";
    loop();
}

void MqttMobilusGtwActor::pushEventsTo(TargetMqttActor* actor)
{
    enqueue(PushEventsToActorCommand { actor });
}

void MqttMobilusGtwActor::sendCommandToDevice(long deviceId, std::string command)
{
    enqueue(SendCommandToDeviceCommand{ deviceId, std::move(command) });
}

void MqttMobilusGtwActor::handle(const PushEventsToActorCommand& cmd)
{
    mClient->messageBus().subscribe<proto::CallEvents>(MessageType::CallEvents, [actor = cmd.actor](const auto& callEvents) {
        if (callEvents.events_size() < 1) {
            return;
        }

        auto& event = callEvents.events(0);

        switch (event.event_number()) {
        case EventNumber::Sent:
            actor->publishDevicePendingCommand(event.device_id(), event.value());
            break;
        case EventNumber::Reached:
            actor->publishDeviceState(event.device_id(), event.value());
            break;
        case EventNumber::Error:
            actor->publishDeviceError(event.device_id(), event.value());
            break;
        }
    });
}

void MqttMobilusGtwActor::handle(const SendCommandToDeviceCommand& cmd)
{
    proto::CallEvents callEvents;

    auto* event = callEvents.add_events();
    event->set_device_id(cmd.deviceId);
    event->set_value(cmd.command);
    event->set_event_number(EventNumber::Triggered);
    event->set_platform(Platform::Web);

    (void)mClient->send(callEvents);
    std::cout << std::format("Sent command: {} for device id: {}\n", cmd.command, cmd.deviceId);
}

}
