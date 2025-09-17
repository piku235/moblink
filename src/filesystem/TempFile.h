#pragma once

#include <string>

static constexpr int kInvalidFd = -1;

namespace moblink::filesystem {

class TempFile final {
public:
    static TempFile unique(std::string pattern);
    ~TempFile();

    TempFile(TempFile&& other);
    TempFile& operator=(TempFile&& other) = delete;

    void write(const char* buf, size_t size);
    const std::string path() const { return mPath; }
private:
    int mFd;
    std::string mPath;

    TempFile(int fd, std::string path);
};

}
