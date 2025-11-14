#include "TargetMqttClient.h"

#include <chrono>
#include <cstdio>
#include <cstdarg>

using namespace jungi::mobilus_gtw_client;

static constexpr char kDeviceStateTopic[] = "devices/%ld/state";
static constexpr char kDeviceErrorTopic[] = "devices/%ld/error";
static constexpr char kDeviceCommandTopic[] = "devices/%ld/command";
static constexpr char kDevicePendingCommandTopic[] = "devices/%ld/pending-command";
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

    mMosq = mosquitto_new(nullptr, true, nullptr);

    if (nullptr == mMosq) {
        return false;
    }

    if (mDsn.secure) {
        if (MOSQ_ERR_SUCCESS != mosquitto_tls_set(mMosq, mDsn.cacert ? mDsn.cacert->c_str() : nullptr, nullptr, nullptr, nullptr, nullptr)) {
            return false;
        }

        mosquitto_tls_insecure_set(mMosq, mDsn.verify.value_or(true) ? false : true);
    }

    if (MOSQ_ERR_SUCCESS != mosquitto_username_pw_set(mMosq, mDsn.username ? (*mDsn.username).c_str() : nullptr, mDsn.password ? (*mDsn.password).c_str() : nullptr)) {
        return false;
    }

    int connectRc = -1;
    mosquitto_user_data_set(mMosq, &connectRc);

    mosquitto_message_callback_set(mMosq, onMessageCallback);
    mosquitto_connect_callback_set(mMosq, [](mosquitto*, void* obj, int rc) {
        auto connectRc = reinterpret_cast<int*>(obj);
        *connectRc = rc;
    });

    if (MOSQ_ERR_SUCCESS != mosquitto_connect(mMosq, mDsn.host.c_str(), *mDsn.port, 60)) {
        return false;
    }

    while (connectRc < MOSQ_ERR_SUCCESS) {
        if (MOSQ_ERR_SUCCESS != mosquitto_loop(mMosq, 1000, 1)) {
            break;
        }
    }

    if (MOSQ_ERR_SUCCESS != connectRc) {
        return false;
    }

    mosquitto_user_data_set(mMosq, this);

    if (MOSQ_ERR_SUCCESS != mosquitto_subscribe(mMosq, nullptr, buildTopic(kDeviceCommandSubTopic).c_str(), 0)) {
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

void TargetMqttClient::setRootTopic(std::string rootTopic)
{
    if (nullptr != mMosq) {
        return;
    }

    mRootTopic = std::move(rootTopic);
}

void TargetMqttClient::publishDeviceState(long deviceId, const std::string& state)
{
    if (!mConnected) {
        return;
    }

    mosquitto_publish(mMosq, nullptr, formatTopic(kDeviceStateTopic, deviceId).c_str(), state.size(), state.data(), 0, false);
}

void TargetMqttClient::publishDeviceError(long deviceId, const std::string& error)
{
    if (!mConnected) {
        return;
    }

    mosquitto_publish(mMosq, nullptr, formatTopic(kDeviceErrorTopic, deviceId).c_str(), error.size(), error.data(), 0, false);
}

void TargetMqttClient::publishDevicePendingCommand(long deviceId, const std::string& command)
{
    if (!mConnected) {
        return;
    }

    mosquitto_publish(mMosq, nullptr, formatTopic(kDevicePendingCommandTopic, deviceId).c_str(), command.size(), command.data(), 0, false);
}

void TargetMqttClient::subscribeDeviceCommands(DeviceCommandSubscriber subscriber)
{
    mDeviceCommandSubscriber = std::move(subscriber);
}

mobgtw::io::SocketEvents TargetMqttClient::socketEvents()
{
    if (!mConnected) {
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
    if (!mConnected) {
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
    if (!mConnected) {
        return;
    }

    int rc = mosquitto_loop_misc(mMosq);
    if (MOSQ_ERR_SUCCESS != rc) {
        disconnect();
        return;
    }

    mLoop.stopTimer(mMiscTimerId);
    mMiscTimerId = mLoop.startTimer(kDelay, miscTimerCallback, this);
}

void TargetMqttClient::onMessage(const mosquitto_message* message)
{
    long deviceId;

    if (sscanf(message->topic, buildTopic(kDeviceCommandTopic).c_str(), &deviceId) > 0) {
        mDeviceCommandSubscriber(deviceId, std::string(reinterpret_cast<const char*>(message->payload), message->payloadlen));
    }
}

std::string TargetMqttClient::buildTopic(const char* topic)
{
    std::string builtTopic;

    builtTopic.append(mRootTopic);
    builtTopic.append("/");
    builtTopic.append(topic);

    return builtTopic;
}

std::string TargetMqttClient::formatTopic(const char* format, ...)
{
    char formattedTopic[64];

    va_list args;
    va_start(args, format);
    vsnprintf(formattedTopic, sizeof(formattedTopic), format, args);
    va_end(args);

    return buildTopic(formattedTopic);
}

}
