#include "core/aa_assert_internal.hpp"

#if defined(__APPLE__)

#include <CoreFoundation/CoreFoundation.h>

namespace aa
{

b8 AAPlatformShowAssertDialog(const char *titleText, const char *bodyText, AAAssertDialogAction &outAction)
{
    CFStringRef titleString = CFStringCreateWithCString(kCFAllocatorDefault,
                                                         titleText != nullptr ? titleText : "Assertion Failed",
                                                         kCFStringEncodingUTF8);
    CFStringRef bodyString = CFStringCreateWithCString(kCFAllocatorDefault,
                                                        bodyText != nullptr ? bodyText : "Assertion failed.",
                                                        kCFStringEncodingUTF8);
    CFStringRef ignoreButton = CFStringCreateWithCString(kCFAllocatorDefault, "Ignore Once", kCFStringEncodingUTF8);
    CFStringRef muteButton = CFStringCreateWithCString(kCFAllocatorDefault, "Mute Assert", kCFStringEncodingUTF8);
    CFStringRef breakButton = CFStringCreateWithCString(kCFAllocatorDefault, "Break", kCFStringEncodingUTF8);

    if (titleString == nullptr || bodyString == nullptr || ignoreButton == nullptr || muteButton == nullptr || breakButton == nullptr)
    {
        if (titleString != nullptr)
        {
            CFRelease(titleString);
        }
        if (bodyString != nullptr)
        {
            CFRelease(bodyString);
        }
        if (ignoreButton != nullptr)
        {
            CFRelease(ignoreButton);
        }
        if (muteButton != nullptr)
        {
            CFRelease(muteButton);
        }
        if (breakButton != nullptr)
        {
            CFRelease(breakButton);
        }

        outAction = AAAssertDialogAction::BreakNow;
        return false;
    }

    CFOptionFlags responseFlags = 0;
    const SInt32 result = CFUserNotificationDisplayAlert(0,
                                                          kCFUserNotificationStopAlertLevel,
                                                          nullptr,
                                                          nullptr,
                                                          nullptr,
                                                          titleString,
                                                          bodyString,
                                                          ignoreButton,
                                                          muteButton,
                                                          breakButton,
                                                          &responseFlags);

    CFRelease(titleString);
    CFRelease(bodyString);
    CFRelease(ignoreButton);
    CFRelease(muteButton);
    CFRelease(breakButton);

    if (result != 0)
    {
        outAction = AAAssertDialogAction::BreakNow;
        return false;
    }

    if ((responseFlags & 0x3) == kCFUserNotificationDefaultResponse)
    {
        outAction = AAAssertDialogAction::IgnoreOnce;
    }
    else if ((responseFlags & 0x3) == kCFUserNotificationAlternateResponse)
    {
        outAction = AAAssertDialogAction::MuteAssert;
    }
    else
    {
        outAction = AAAssertDialogAction::BreakNow;
    }

    return true;
}

} // namespace aa

#endif
