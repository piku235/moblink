#include "TempFile.h"

#include <cstdio>
#include <unistd.h>
#include <utility>

namespace moblink::filesystem {

TempFile TempFile::unique(std::string pattern)
{
    return { ::mkstemp(pattern.data()), std::move(pattern) };
}

TempFile::TempFile(int fd, std::string path)
    : mFd(fd)
    , mPath(std::move(path))
{
}

TempFile::TempFile(TempFile&& other)
    : mFd(std::exchange(other.mFd, kInvalidFd))
{
}

TempFile::~TempFile()
{
    if (kInvalidFd == mFd) {
        return;
    }

    ::close(mFd);
    ::unlink(mPath.c_str());
}

void TempFile::write(const char* buf, size_t size)
{
    ::write(mFd, buf, size);
}

}
