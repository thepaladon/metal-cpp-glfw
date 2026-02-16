#pragma once

#include <memory>

namespace aa
{

template <typename T>
using AAPtr = std::unique_ptr<T>;

} // namespace aa

#define AA_NEW new(__FILE__, __LINE__)
#define AA_NEW_TAG(tag) new((tag), __FILE__, __LINE__)
