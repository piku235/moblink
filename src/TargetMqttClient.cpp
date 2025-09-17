#include "TargetMqttClient.h"

#include <chrono>
#include <cstdio>

using namespace jungi::mobilus_gtw_client;

static constexpr char kDeviceStateTopic[] = "devices/%ld/state";
static constexpr char kDeviceErrorTopic[] = "devices/%ld/error";
static constexpr char kDeviceCommandTopic[] = "devices/%ld/command";
static constexpr char kDeviceRequestedCommandTopic[] = "devices/%ld/requested-command";
static constexpr char kDeviceCommandSubTopic[] = "devices/+/command";
static constexpr std::chrono::seconds kDelay(1);

namespace moblink {

TargetMqttClient::TargetMqttClient(mobgtw::MqttDsn dsn, mobgtw::io::EventLoop& loop)
    : mDsn(std::move(dsn))
    , mLoop(loop)
{
}

TargetMqttClient::~TargetMqttClient()
{
    disconnect();

    if (nullptr != mMosq) {
        mosquitto_destroy(mMosq);
        mMosq = nullptr;
    }
}

bool TargetMqttClient::connect()
{
    if (mConnected) {
        return true;
    }

    mMosq = mosquitto_new(nullptr, true, this);

    if (nullptr == mMosq) {
        return false;
    }

    mosquitto_message_callback_set(mMosq, onMessageCallback);

    if (mDsn.secure) {
        if (MOSQ_ERR_SUCCESS != mosquitto_tls_set(mMosq, mDsn.cacert ? mDsn.cacert->c_str() : nullptr, nullptr, nullptr, nullptr, nullptr)) {
            return false;
        }

        mosquitto_tls_insecure_set(mMosq, mDsn.verify.value_or(true) ? false : true);
    }

    if (MOSQ_ERR_SUCCESS != mosquitto_connect(mMosq, mDsn.host.c_str(), *mDsn.port, 60)) {
        return false;
    }

    if (MOSQ_ERR_SUCCESS != mosquitto_subscribe(mMosq, nullptr, kDeviceCommandSubTopic, 0)) {
        mosquitto_disconnect(mMosq);
        return false;
    }

    mConnected = true;
    mLoop.watchSocket(mosquitto_socket(mMosq), this);
    mMiscTimerId = mLoop.startTimer(kDelay, miscTimerCallback, this);

    return true;
}

void TargetMqttClient::disconnect()
{
    if (mConnected) {
        mLoop.unwatchSocket(mosquitto_socket(mMosq));
        mLoop.stopTimer(mMiscTimerId);

        mosquitto_disconnect(mMosq);
        mConnected = false;
    }
}

void TargetMqttClient::publishDeviceState(long deviceId, const std::string& state)
{
    if (!mMosq) {
        return;
    }
    
    char topic[64];
    snprintf(topic, sizeof(topic), kDeviceStateTopic, deviceId);

    mosquitto_publish(mMosq, nullptr, topic, state.size(), state.data(), 0, false);
}

void TargetMqttClient::publishDeviceError(long deviceId, const std::string& error)
{
    if (!mMosq) {
        return;
    }
    
    char topic[64];
    snprintf(topic, sizeof(topic), kDeviceErrorTopic, deviceId);

    mosquitto_publish(mMosq, nullptr, topic, error.size(), error.data(), 0, false);
}

void TargetMqttClient::publishDeviceRequestedCommand(long deviceId, const std::string& command)
{
    if (!mMosq) {
        return;
    }

    char topic[64];
    snprintf(topic, sizeof(topic), kDeviceRequestedCommandTopic, deviceId);

    mosquitto_publish(mMosq, nullptr, topic, command.size(), command.data(), 0, false);
}

void TargetMqttClient::subscribeDeviceCommands(DeviceCommandSubscriber subscriber)
{
    mDeviceCommandSubscriber = std::move(subscriber);
}

mobgtw::io::SocketEvents TargetMqttClient::socketEvents()
{
    if (!mMosq) {
        return {};
    }

    io::SocketEvents socketEvents;
    socketEvents.set(io::SocketEvents::Read);
    
    if (mosquitto_want_write(mMosq)) {
        socketEvents.set(io::SocketEvents::Write);
    }

    return socketEvents;
}

void TargetMqttClient::handleSocketEvents(mobgtw::io::SocketEvents revents)
{
    if (!mMosq) {
        return;
    }

    int rc;

    if (revents.has(io::SocketEvents::Read)) {
        rc = mosquitto_loop_read(mMosq, 1);
        if (MOSQ_ERR_SUCCESS != rc) {
            disconnect();
            return;
        }
    }
    if (revents.has(io::SocketEvents::Write)) {
        rc = mosquitto_loop_write(mMosq, 1);
        if (MOSQ_ERR_SUCCESS != rc) {
            disconnect();
            return;
        }
    }
}

void TargetMqttClient::handleMisc() 
{
    if (!mMosq) {
        return;
    }

    int rc = mosquitto_loop_misc(mMosq);
    if (MOSQ_ERR_SUCCESS != rc) {
        disconnect();
        return;
    }

    mMiscTimerId = mLoop.startTimer(kDelay, miscTimerCallback, this);
}

void TargetMqttClient::onMessage(const mosquitto_message* message)
{
    long deviceId;
        
    if (sscanf(message->topic, kDeviceCommandTopic, &deviceId) > 0) {
        mDeviceCommandSubscriber(deviceId, std::string(reinterpret_cast<const char*>(message->payload), message->payloadlen));
    }
}

}
