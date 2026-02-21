#include "core/aa_assert_internal.hpp"

#if defined(_WIN32)

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#include <commctrl.h>

namespace aa
{
namespace
{

using TaskDialogIndirectFn = HRESULT(WINAPI *)(const TASKDIALOGCONFIG *, i32 *, i32 *, BOOL *);

} // namespace

b8 AAPlatformShowAssertDialog(const char *titleText, const char *bodyText, AAAssertDialogAction &outAction)
{
    HMODULE comctl = LoadLibraryA("comctl32.dll");
    TaskDialogIndirectFn taskDialogIndirect = nullptr;

    if (comctl != nullptr)
    {
        taskDialogIndirect = reinterpret_cast<TaskDialogIndirectFn>(GetProcAddress(comctl, "TaskDialogIndirect"));
    }

    if (taskDialogIndirect != nullptr)
    {
        TASKDIALOG_BUTTON buttons[3] = {};
        buttons[0].nButtonID = 1001;
        buttons[0].pszButtonText = L"Ignore Once";
        buttons[1].nButtonID = 1002;
        buttons[1].pszButtonText = L"Mute Assert";
        buttons[2].nButtonID = 1003;
        buttons[2].pszButtonText = L"Break";

        i32 selectedButtonId = 0;
        TASKDIALOGCONFIG dialogConfig = {};
        dialogConfig.cbSize = sizeof(dialogConfig);
        dialogConfig.hwndParent = nullptr;
        dialogConfig.dwFlags = TDF_ALLOW_DIALOG_CANCELLATION | TDF_POSITION_RELATIVE_TO_WINDOW;
        dialogConfig.pszWindowTitle = titleText;
        dialogConfig.pszContent = bodyText;
        dialogConfig.cButtons = 3;
        dialogConfig.pButtons = buttons;
        dialogConfig.nDefaultButton = 1003;

        const HRESULT result = taskDialogIndirect(&dialogConfig, &selectedButtonId, nullptr, nullptr);

        if (comctl != nullptr)
        {
            FreeLibrary(comctl);
        }

        if (FAILED(result))
        {
            outAction = AAAssertDialogAction::BreakNow;
            return false;
        }

        if (selectedButtonId == 1001)
        {
            outAction = AAAssertDialogAction::IgnoreOnce;
        }
        else if (selectedButtonId == 1002)
        {
            outAction = AAAssertDialogAction::MuteAssert;
        }
        else
        {
            outAction = AAAssertDialogAction::BreakNow;
        }

        return true;
    }

    if (comctl != nullptr)
    {
        FreeLibrary(comctl);
    }

    const i32 fallbackResult = static_cast<i32>(
        MessageBoxA(nullptr,
                    bodyText != nullptr ? bodyText : "Assertion failed.",
                    titleText != nullptr ? titleText : "Assertion Failed",
                    MB_YESNOCANCEL | MB_ICONERROR | MB_TOPMOST));

    if (fallbackResult == IDYES)
    {
        outAction = AAAssertDialogAction::IgnoreOnce;
    }
    else if (fallbackResult == IDNO)
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
