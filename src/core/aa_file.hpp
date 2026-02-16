#pragma once

#include "core/aa_containers.hpp"
#include "core/aa_path.hpp"
#include "core/aa_types.hpp"

namespace aa
{

enum class AAFileMode
{
    Read,
    WriteTruncate,
    WriteAppend,
    ReadWrite,
    ReadWriteCreate,
    ReadWriteTruncate,
};

class AAFile
{
  public:
    AAFile() = default;
    ~AAFile();

    AAFile(const AAFile &) = delete;
    AAFile &operator=(const AAFile &) = delete;

    AAFile(AAFile &&other) noexcept;
    AAFile &operator=(AAFile &&other) noexcept;

    b8 Open(const AAPath &path, AAFileMode mode);
    void Close();

    b8 IsOpen() const;

    u64 Size() const;
    u64 Tell() const;
    b8 Seek(u64 offset);

    usize Read(void *dst, usize bytes);
    usize Write(const void *src, usize bytes);
    b8 Flush();

  private:
    i32 fd_ = -1;
    u64 cursor_ = 0;
    b8 appendMode_ = false;
};

b8 AAReadAllBytes(const AAPath &path, AAVector<u8> &out);
b8 AAReadAllText(const AAPath &path, AAString &out);

b8 AAWriteAllBytes(const AAPath &path, const void *data, usize bytes);
b8 AAWriteAllText(const AAPath &path, const char *text);

b8 AAAppendAllBytes(const AAPath &path, const void *data, usize bytes);
b8 AAAppendAllText(const AAPath &path, const char *text);

} // namespace aa
