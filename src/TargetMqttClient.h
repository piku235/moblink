#pragma once

#include <jungi/mobgtw/MqttDsn.h>
#include <jungi/mobgtw/io/SocketEvents.h>
#include <jungi/mobgtw/io/SocketEventHandler.h>
#include <jungi/mobgtw/io/EventLoop.h>

#include <mosquitto.h>

#include <string>
#include <functional>

namespace moblink {

class TargetMqttClient final : public jungi::mobgtw::io::SocketEventHandler {
public:
    using DeviceCommandSubscriber = std::function<void(long, const std::string&)>;

    TargetMqttClient(jungi::mobgtw::MqttDsn dsn, jungi::mobgtw::io::EventLoop& loop);
    ~TargetMqttClient();

    TargetMqttClient(const TargetMqttClient& other) = delete;
    TargetMqttClient& operator=(const TargetMqttClient& other) = delete;

    bool connect();
    void disconnect();
    void setRootTopic(std::string rootTopic);
    void publishDeviceState(long deviceId, const std::string& state);
    void publishDeviceError(long deviceId, const std::string& error);
    void publishDevicePendingCommand(long deviceId, const std::string& command);
    void subscribeDeviceCommands(DeviceCommandSubscriber subscriber);

    jungi::mobgtw::io::SocketEvents socketEvents() override;
    void handleSocketEvents(jungi::mobgtw::io::SocketEvents revents) override;

private:
    mosquitto* mMosq = nullptr;
    bool mConnected = false;
    jungi::mobgtw::MqttDsn mDsn;
    jungi::mobgtw::io::EventLoop& mLoop;
    jungi::mobgtw::io::EventLoop::TimerId mMiscTimerId = jungi::mobgtw::io::EventLoop::kInvalidTimerId;
    std::string mRootTopic = "mobilus";
    DeviceCommandSubscriber mDeviceCommandSubscriber = [](long, const std::string&) {};

    static void onMessageCallback(mosquitto* mosq, void* obj, const mosquitto_message* message) { reinterpret_cast<TargetMqttClient*>(obj)->onMessage(message); }
    static void miscTimerCallback(void* callbackData) { reinterpret_cast<TargetMqttClient*>(callbackData)->handleMisc(); }

    void onMessage(const mosquitto_message* message);
    void handleMisc();
    std::string buildTopic(const char* topic);
    std::string formatTopic(const char* format, ...);
};

}
