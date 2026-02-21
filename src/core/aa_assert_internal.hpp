#pragma once

#include "core/aa_types.hpp"

namespace aa
{

enum class AAAssertDialogAction
{
    IgnoreOnce,
    MuteAssert,
    BreakNow,
};

b8 AAPlatformShowAssertDialog(const char *titleText, const char *bodyText, AAAssertDialogAction &outAction);

} // namespace aa
