#pragma once

#include <EASTL/list.h>
#include <EASTL/map.h>
#include <EASTL/string.h>
#include <EASTL/vector.h>

namespace aa
{

template <typename T>
using AAVector = eastl::vector<T>;

template <typename K, typename V>
using AAMap = eastl::map<K, V>;

template <typename T>
using AAList = eastl::list<T>;

using AAString = eastl::string;

} // namespace aa
