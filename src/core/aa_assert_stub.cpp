#include "core/aa_assert_internal.hpp"

#if !defined(_WIN32) && !defined(__APPLE__)

namespace aa
{

b8 AAPlatformShowAssertDialog(const char * /*titleText*/, const char * /*bodyText*/, AAAssertDialogAction &outAction)
{
    outAction = AAAssertDialogAction::BreakNow;
    return false;
}

} // namespace aa

#endif
