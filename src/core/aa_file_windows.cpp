#include "core/aa_file.hpp"

#include <climits>
#include <fcntl.h>
#include <io.h>
#include <share.h>
#include <sys/stat.h>

namespace aa
{
namespace
{

int AAFlagsForMode(AAFileMode mode)
{
    switch (mode)
    {
    case AAFileMode::Read:
        return _O_RDONLY | _O_BINARY;
    case AAFileMode::WriteTruncate:
        return _O_WRONLY | _O_CREAT | _O_TRUNC | _O_BINARY;
    case AAFileMode::WriteAppend:
        return _O_WRONLY | _O_CREAT | _O_APPEND | _O_BINARY;
    case AAFileMode::ReadWrite:
        return _O_RDWR | _O_BINARY;
    case AAFileMode::ReadWriteCreate:
        return _O_RDWR | _O_CREAT | _O_BINARY;
    case AAFileMode::ReadWriteTruncate:
        return _O_RDWR | _O_CREAT | _O_TRUNC | _O_BINARY;
    }

    return _O_RDONLY | _O_BINARY;
}

unsigned int AAMaxIo(usize bytes)
{
    const usize limit = static_cast<usize>(INT_MAX);
    return static_cast<unsigned int>(bytes < limit ? bytes : limit);
}

} // namespace

b8 AAFile::Open(const AAPath &path, AAFileMode mode)
{
    Close();

    int fd = -1;
    if (_sopen_s(&fd, path.CStr(), AAFlagsForMode(mode), _SH_DENYNO, _S_IREAD | _S_IWRITE) != 0)
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
        _close(fd_);
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

    struct _stat64 s = {};
    if (_fstat64(fd_, &s) != 0 || s.st_size < 0)
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

    if (cursor_ > static_cast<u64>(LLONG_MAX))
    {
        return 0;
    }

    if (_lseeki64(fd_, static_cast<__int64>(cursor_), SEEK_SET) < 0)
    {
        return 0;
    }

    const int result = _read(fd_, dst, AAMaxIo(bytes));
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

    if (cursor_ > static_cast<u64>(LLONG_MAX))
    {
        return 0;
    }

    if (!appendMode_)
    {
        if (_lseeki64(fd_, static_cast<__int64>(cursor_), SEEK_SET) < 0)
        {
            return 0;
        }
    }

    const int result = _write(fd_, src, AAMaxIo(bytes));
    if (result <= 0)
    {
        return 0;
    }

    if (appendMode_)
    {
        cursor_ = Size();
    }
    else
    {
        cursor_ += static_cast<u64>(result);
    }

    return static_cast<usize>(result);
}

b8 AAFile::Flush()
{
    if (!IsOpen())
    {
        return false;
    }

    return _commit(fd_) == 0;
}

} // namespace aa
