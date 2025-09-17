#pragma once

#include <jungi/mobilus_gtw_client/io/SocketEventHandler.h>
#include <jungi/mobilus_gtw_client/io/SocketEvents.h>
#include <jungi/mobilus_gtw_client/io/SelectEventLoop.h>

#include <future>
#include <atomic>
#include <thread>
#include <queue>
#include <mutex>
#include <functional>
#include <unistd.h>

namespace moblink {

namespace mobgtw = jungi::mobilus_gtw_client;

template <typename T>
class Actor : public mobgtw::io::SocketEventHandler {
public:
    virtual ~Actor()
    {
        stop();
    }

    explicit Actor(std::promise<void>& onFinished = defaultFinishedPromise)
        : mFinishedPromise(onFinished)
    {
    }

    void start()
    {
        if (isRunning()) {
            return;
        }
        if (pipe(mWakeFd) != 0) {
            return;
        }

        std::promise<void> ready;
        auto readyFut = ready.get_future();

        mSelf = std::thread([this, ready = std::move(ready)]() mutable {
            mRunning = true;
            ready.set_value();

            run();

            mRunning = false;

            try {
                mFinishedPromise.set_value();
            } catch (const std::future_error&) {
                // ignore
            }
        });

        readyFut.wait();
    }

    void stop()
    {
        post([](mobgtw::io::SelectEventLoop& loop, T&) { loop.stop(); });
        join();

        if (kInvalidFd != mWakeFd[0]) {
            close(mWakeFd[0]);
            mWakeFd[0] = kInvalidFd;
        }
        if (kInvalidFd != mWakeFd[1]) {
            close(mWakeFd[1]);
            mWakeFd[1] = kInvalidFd;
        }
    }

    void join()
    {
        if (mSelf.joinable()) {
            mSelf.join();
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
        ::read(mWakeFd[0], &buf, sizeof(buf));

        std::lock_guard<std::mutex> lock(mMutex);

        while (!mTasks.empty()) {
            auto& task = mTasks.front();
            mTaskHandler(std::move(task));
            mTasks.pop();
        }
    }

    bool isRunning() const
    {
        return mRunning.load();
    }

protected:
    using Task = std::function<void(mobgtw::io::SelectEventLoop&, T&)>;
    using TaskHandler = std::function<void(Task)>;
    
    static std::promise<void> defaultFinishedPromise;

    void runLoop(mobgtw::io::SelectEventLoop& loop, T& dependency)
    {
        mTaskHandler = [&](Task task) { task(loop, dependency); };

        loop.watchSocket(mWakeFd[0], this);
        loop.run();

        mTaskHandler = [](Task) {};
    }

    void post(Task task)
    {
        if (!isRunning()) {
            return;
        }

        std::lock_guard<std::mutex> lock(mMutex);
        mTasks.push(std::move(task));
        wakeUp();
    }

    virtual void run() = 0;

private:
    static constexpr int kInvalidFd = -1;

    std::thread mSelf;
    std::mutex mMutex;
    std::queue<Task> mTasks;
    TaskHandler mTaskHandler;
    int mWakeFd[2] = { kInvalidFd, kInvalidFd };
    std::atomic<bool> mRunning = false;
    std::promise<void>& mFinishedPromise;
    
    void wakeUp()
    {
        uint8_t byte = 1;
        ::write(mWakeFd[1], &byte, 1);
    }
};

}
