#pragma once

#include "Messages.h"

#include <concurrentqueue.h>
#include <jungi/mobilus_gtw_client/io/SocketEventHandler.h>
#include <jungi/mobilus_gtw_client/io/SelectEventLoop.h>

#include <thread>
#include <variant>
#include <latch>
#include <unistd.h>

namespace moblink {

namespace mobgtw = jungi::mobilus_gtw_client;

template <typename T>
concept Variant = requires {
    typename std::variant_size<T>::type;
};

template <typename Derived, Variant Message>
class Actor : public mobgtw::io::SocketEventHandler {
public:
    virtual ~Actor()
    {
        if (kInvalidFd != mWakeFd[0]) {
            (void)::close(mWakeFd[0]);
        }
        if (kInvalidFd != mWakeFd[1]) {
            (void)::close(mWakeFd[1]);
        }
    }

    mobgtw::io::SocketEvents socketEvents() override
    {
        mobgtw::io::SocketEvents events;
        events.set(mobgtw::io::SocketEvents::Read);

        return events;
    }

    void handleSocketEvents(mobgtw::io::SocketEvents revents) override
    {
        if (!revents.has(mobgtw::io::SocketEvents::Read)) {
            return;
        }

        uint8_t buf[64];
        (void)::read(mWakeFd[0], &buf, sizeof(buf));

        Message message;
        while (mMailbox.try_dequeue(message)) {
            std::visit([self = static_cast<Derived*>(this)](auto&& msg) { self->handle(std::forward<decltype(msg)>(msg)); }, message);
        }
    }

    void stop()
    {
        enqueue(StopActorCommand {});
    }

    void handle(const StopActorCommand& cmd)
    {
        mLoop.stop();
    }

    void latch(std::latch* latch)
    {
        mLatch = latch;
    }

protected:
    mobgtw::io::SelectEventLoop mLoop;

    void start()
    {
        if (-1 == pipe(mWakeFd)) {
            return;
        }

        mSelf = std::thread([this] {
            run();
            if (nullptr != mLatch) {
                mLatch->count_down();
            }
        });
    }

    void enqueue(Message message)
    {
        uint8_t byte = 1;

        mMailbox.enqueue(std::move(message));
        (void)::write(mWakeFd[1], &byte, 1);
    }

    void loop()
    {
        mLoop.watchSocket(mWakeFd[0], this);
        mLoop.run();
        mLoop.unwatchSocket(mWakeFd[0]);
    }

    void shutdown()
    {
        stop();

        if (mSelf.joinable()) {
            mSelf.join();
        }
    }

    virtual void run() = 0;

private:
    static constexpr int kInvalidFd = -1;

    std::thread mSelf;
    moodycamel::ConcurrentQueue<Message> mMailbox;
    std::latch* mLatch = nullptr;
    int mWakeFd[2] = { kInvalidFd, kInvalidFd };
};

}
