#pragma once

#include <cstdio>
#include <string>
#include <unistd.h>
#include <utility>

namespace filesystem {

class TempFile final {
public:
    static TempFile unique(std::string pattern)
    {
        return { ::mkstemp(pattern.data()), std::move(pattern) };
    }

    ~TempFile()
    {
        if (kInvalidFd == mFd) {
            return;
        }

        ::close(mFd);
        ::unlink(mPath.c_str());
    }

    TempFile(const TempFile&) = delete;
    TempFile& operator=(const TempFile&) = delete;

    TempFile(TempFile&& other): mFd(std::exchange(other.mFd, kInvalidFd)) {}
    TempFile& operator=(TempFile&& other) = delete;

    void write(const char* buf, size_t size) { ::write(mFd, buf, size); }
    const std::string& path() const { return mPath; }

private:
    static constexpr int kInvalidFd = -1;

    int mFd;
    std::string mPath;

    TempFile(int fd, std::string path): mFd(fd), mPath(std::move(path)) {}
};

}
