#pragma once

#include "core/aa_types.hpp"

#if defined(__clang__) || defined(__GNUC__)
#include <signal.h>
#endif

namespace aa
{

class AAAssert
{
  public:
    static b8 HandleFailure(const char *conditionText, const char *messageText, const char *filePath, i32 lineNumber);
};

#if defined(_MSC_VER)
#define AADebugBreak() __debugbreak()
#elif defined(__clang__) || defined(__GNUC__)
#define AADebugBreak() raise(SIGTRAP)
#else
#define AADebugBreak() ((void)0)
#endif

#define AAAssertMessage(conditionExpr, messageText)                                                          \
    do                                                                                                       \
    {                                                                                                        \
        if (!(conditionExpr))                                                                                \
        {                                                                                                    \
            if (::aa::AAAssert::HandleFailure(#conditionExpr, messageText, __FILE__, static_cast<::aa::i32>(__LINE__))) \
            {                                                                                                \
                AADebugBreak();                                                                              \
            }                                                                                                \
        }                                                                                                    \
    } while (0)

#define AAAssert(conditionExpr) AAAssertMessage(conditionExpr, nullptr)

} // namespace aa
