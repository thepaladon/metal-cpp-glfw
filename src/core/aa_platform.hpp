#pragma once

#include "core/aa_path.hpp"

namespace aa
{

AAPath AAGetCurrentWorkingDirectory();
b8 AACreateDirectories(const AAPath &path);

} // namespace aa
