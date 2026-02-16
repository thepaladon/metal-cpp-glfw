#pragma once

#include <memory>

namespace aa
{

template <typename T>
using AAPtr = std::unique_ptr<T>;

} // namespace aa
