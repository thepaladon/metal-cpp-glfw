#pragma once

#include <cstddef>
#include <memory>

namespace aa
{

template <typename T>
using AAPtr = std::unique_ptr<T>;

} // namespace aa

void *operator new(std::size_t size, const char *file, int line);
void operator delete(void *ptr, const char *file, int line) noexcept;

void *operator new[](std::size_t size, const char *file, int line);
void operator delete[](void *ptr, const char *file, int line) noexcept;

void *operator new(std::size_t size, const char *tag, const char *file, int line);
void operator delete(void *ptr, const char *tag, const char *file, int line) noexcept;

void *operator new[](std::size_t size, const char *tag, const char *file, int line);
void operator delete[](void *ptr, const char *tag, const char *file, int line) noexcept;

#define AA_NEW new(__FILE__, __LINE__)
#define AA_NEW_TAG(tag) new((tag), __FILE__, __LINE__)
