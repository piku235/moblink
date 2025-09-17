#pragma once

#include <jungi/mobilus_gtw_client/MqttDsn.h>
#include <jungi/mobilus_gtw_client/io/SocketEvents.h>
#include <jungi/mobilus_gtw_client/io/SocketEventHandler.h>
#include <jungi/mobilus_gtw_client/io/EventLoop.h>

#include <mosquitto.h>

#include <string>
#include <functional>

namespace moblink {

namespace mobgtw = jungi::mobilus_gtw_client;

class TargetMqttClient final : public mobgtw::io::SocketEventHandler {
public:
    using DeviceCommandSubscriber = std::function<void(long, const std::string&)>;

    TargetMqttClient(mobgtw::MqttDsn dsn, mobgtw::io::EventLoop& loop);
    ~TargetMqttClient();

    TargetMqttClient(const TargetMqttClient& other) = delete;
    TargetMqttClient& operator=(const TargetMqttClient& other) = delete;

    bool connect();
    void disconnect();
    void publishDeviceState(long deviceId, const std::string& state);
    void publishDeviceError(long deviceId, const std::string& error);
    void publishDeviceRequestedCommand(long deviceId, const std::string& command);
    void subscribeDeviceCommands(DeviceCommandSubscriber subscriber);

    mobgtw::io::SocketEvents socketEvents() override;
    void handleSocketEvents(mobgtw::io::SocketEvents revents) override;

private:
    mosquitto* mMosq = nullptr;
    bool mConnected = false;
    mobgtw::MqttDsn mDsn;
    mobgtw::io::EventLoop& mLoop;
    mobgtw::io::EventLoop::TimerId mMiscTimerId = mobgtw::io::EventLoop::kInvalidTimerId;
    DeviceCommandSubscriber mDeviceCommandSubscriber = [](long, const std::string&) {};

    static void onMessageCallback(mosquitto* mosq, void* obj, const mosquitto_message* message) { reinterpret_cast<TargetMqttClient*>(obj)->onMessage(message); }
    static void miscTimerCallback(void* callbackData) { reinterpret_cast<TargetMqttClient*>(callbackData)->handleMisc(); }

    void onMessage(const mosquitto_message* message);
    void handleMisc();
};

}
