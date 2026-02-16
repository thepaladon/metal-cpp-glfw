#include "core/aa_file.hpp"

namespace aa
{

AAFile::~AAFile()
{
    Close();
}

AAFile::AAFile(AAFile &&other) noexcept
{
    fd_ = other.fd_;
    cursor_ = other.cursor_;
    appendMode_ = other.appendMode_;

    other.fd_ = -1;
    other.cursor_ = 0;
    other.appendMode_ = false;
}

AAFile &AAFile::operator=(AAFile &&other) noexcept
{
    if (this == &other)
    {
        return *this;
    }

    Close();

    fd_ = other.fd_;
    cursor_ = other.cursor_;
    appendMode_ = other.appendMode_;

    other.fd_ = -1;
    other.cursor_ = 0;
    other.appendMode_ = false;

    return *this;
}

b8 AAReadAllBytes(const AAPath &path, AAVector<u8> &out)
{
    out.clear();

    AAFile file;
    if (!file.Open(path, AAFileMode::Read))
    {
        return false;
    }

    const u64 fileSize = file.Size();
    if (fileSize > static_cast<u64>(static_cast<usize>(-1)))
    {
        return false;
    }

    const usize bytes = static_cast<usize>(fileSize);
    out.resize(bytes);

    usize offset = 0;
    while (offset < bytes)
    {
        const usize chunk = file.Read(out.data() + offset, bytes - offset);
        if (chunk == 0)
        {
            out.clear();
            return false;
        }
        offset += chunk;
    }

    return true;
}

b8 AAReadAllText(const AAPath &path, AAString &out)
{
    AAVector<u8> bytes;
    if (!AAReadAllBytes(path, bytes))
    {
        out.clear();
        return false;
    }

    out.assign(reinterpret_cast<const char *>(bytes.data()), bytes.size());
    return true;
}

b8 AAWriteAllBytes(const AAPath &path, const void *data, usize bytes)
{
    if (data == nullptr && bytes != 0)
    {
        return false;
    }

    AAFile file;
    if (!file.Open(path, AAFileMode::WriteTruncate))
    {
        return false;
    }

    usize offset = 0;
    const u8 *source = static_cast<const u8 *>(data);
    while (offset < bytes)
    {
        const usize chunk = file.Write(source + offset, bytes - offset);
        if (chunk == 0)
        {
            return false;
        }
        offset += chunk;
    }

    return true;
}

b8 AAWriteAllText(const AAPath &path, const char *text)
{
    if (text == nullptr)
    {
        return AAWriteAllBytes(path, nullptr, 0);
    }

    const usize bytes = AAString(text).size();
    return AAWriteAllBytes(path, text, bytes);
}

b8 AAAppendAllBytes(const AAPath &path, const void *data, usize bytes)
{
    if (data == nullptr && bytes != 0)
    {
        return false;
    }

    AAFile file;
    if (!file.Open(path, AAFileMode::WriteAppend))
    {
        return false;
    }

    usize offset = 0;
    const u8 *source = static_cast<const u8 *>(data);
    while (offset < bytes)
    {
        const usize chunk = file.Write(source + offset, bytes - offset);
        if (chunk == 0)
        {
            return false;
        }
        offset += chunk;
    }

    return true;
}

b8 AAAppendAllText(const AAPath &path, const char *text)
{
    if (text == nullptr)
    {
        return AAAppendAllBytes(path, nullptr, 0);
    }

    const usize bytes = AAString(text).size();
    return AAAppendAllBytes(path, text, bytes);
}

} // namespace aa
