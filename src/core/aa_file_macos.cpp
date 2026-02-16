#include "core/aa_file.hpp"

#include <cerrno>
#include <climits>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

namespace aa
{
namespace
{

int AAFlagsForMode(AAFileMode mode)
{
    switch (mode)
    {
    case AAFileMode::Read:
        return O_RDONLY;
    case AAFileMode::WriteTruncate:
        return O_WRONLY | O_CREAT | O_TRUNC;
    case AAFileMode::WriteAppend:
        return O_WRONLY | O_CREAT | O_APPEND;
    case AAFileMode::ReadWrite:
        return O_RDWR;
    case AAFileMode::ReadWriteCreate:
        return O_RDWR | O_CREAT;
    case AAFileMode::ReadWriteTruncate:
        return O_RDWR | O_CREAT | O_TRUNC;
    }

    return O_RDONLY;
}

usize AAMaxIo(usize bytes)
{
    const usize limit = static_cast<usize>(SSIZE_MAX);
    return bytes < limit ? bytes : limit;
}

} // namespace

b8 AAFile::Open(const AAPath &path, AAFileMode mode)
{
    Close();

    const int fd = open(path.CStr(), AAFlagsForMode(mode), 0666);
    if (fd < 0)
    {
        return false;
    }

    fd_ = fd;
    appendMode_ = mode == AAFileMode::WriteAppend;
    cursor_ = appendMode_ ? Size() : 0;
    return true;
}

void AAFile::Close()
{
    if (fd_ >= 0)
    {
        close(fd_);
        fd_ = -1;
    }

    cursor_ = 0;
    appendMode_ = false;
}

b8 AAFile::IsOpen() const
{
    return fd_ >= 0;
}

u64 AAFile::Size() const
{
    if (fd_ < 0)
    {
        return 0;
    }

    struct stat s = {};
    if (fstat(fd_, &s) != 0 || s.st_size < 0)
    {
        return 0;
    }

    return static_cast<u64>(s.st_size);
}

u64 AAFile::Tell() const
{
    return IsOpen() ? cursor_ : 0;
}

b8 AAFile::Seek(u64 offset)
{
    if (!IsOpen())
    {
        return false;
    }

    cursor_ = offset;
    return true;
}

usize AAFile::Read(void *dst, usize bytes)
{
    if (!IsOpen() || dst == nullptr || bytes == 0)
    {
        return 0;
    }

    if (cursor_ > static_cast<u64>(INT64_MAX))
    {
        return 0;
    }

    ssize_t result = -1;
    do
    {
        result = pread(fd_, dst, AAMaxIo(bytes), static_cast<off_t>(cursor_));
    } while (result < 0 && errno == EINTR);

    if (result <= 0)
    {
        return 0;
    }

    cursor_ += static_cast<u64>(result);
    return static_cast<usize>(result);
}

usize AAFile::Write(const void *src, usize bytes)
{
    if (!IsOpen() || src == nullptr || bytes == 0)
    {
        return 0;
    }

    if (appendMode_)
    {
        cursor_ = Size();
    }

    if (cursor_ > static_cast<u64>(INT64_MAX))
    {
        return 0;
    }

    ssize_t result = -1;
    do
    {
        result = pwrite(fd_, src, AAMaxIo(bytes), static_cast<off_t>(cursor_));
    } while (result < 0 && errno == EINTR);

    if (result <= 0)
    {
        return 0;
    }

    cursor_ += static_cast<u64>(result);
    return static_cast<usize>(result);
}

b8 AAFile::Flush()
{
    if (!IsOpen())
    {
        return false;
    }

    return fsync(fd_) == 0;
}

} // namespace aa
