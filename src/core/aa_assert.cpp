#include "core/aa_assert.hpp"

#include "core/aa_assert_internal.hpp"
#include "core/aa_containers.hpp"

#include <cstdio>

namespace aa
{
namespace
{

AAMap<AAString, b8> gMutedAsserts;

AAString AABuildAssertKey(const char *conditionText, const char *filePath, i32 lineNumber)
{
    AAString key;
    key.append(filePath != nullptr ? filePath : "<unknown>");
    key.append(":");

    char lineBuffer[32] = {};
    snprintf(lineBuffer, sizeof(lineBuffer), "%d", lineNumber);
    key.append(lineBuffer);

    key.append(":");
    key.append(conditionText != nullptr ? conditionText : "<unknown condition>");
    return key;
}

AAString AABuildAssertBody(const char *conditionText, const char *messageText, const char *filePath, i32 lineNumber)
{
    AAString body;

    body.append("Condition:\n");
    body.append(conditionText != nullptr ? conditionText : "<unknown condition>");
    body.append("\n\n");

    if (messageText != nullptr && messageText[0] != '\0')
    {
        body.append("Message:\n");
        body.append(messageText);
        body.append("\n\n");
    }

    body.append("Location:\n");
    body.append(filePath != nullptr ? filePath : "<unknown>");
    body.append(":");

    char lineBuffer[32] = {};
    snprintf(lineBuffer, sizeof(lineBuffer), "%d", lineNumber);
    body.append(lineBuffer);
    body.append("\n\n");

    body.append("Ignore Once: skip this hit only\n");
    body.append("Mute Assert: never stop on this assert again\n");
    body.append("Break: stop now");
    return body;
}

} // namespace

b8 AAAssert::HandleFailure(const char *conditionText, const char *messageText, const char *filePath, i32 lineNumber)
{
    const AAString assertKey = AABuildAssertKey(conditionText, filePath, lineNumber);

    const auto mutedIt = gMutedAsserts.find(assertKey);
    if (mutedIt != gMutedAsserts.end() && mutedIt->second)
    {
        return false;
    }

    const AAString dialogBody = AABuildAssertBody(conditionText, messageText, filePath, lineNumber);
    AAAssertDialogAction action = AAAssertDialogAction::BreakNow;

    if (!AAPlatformShowAssertDialog("Assertion Failed", dialogBody.c_str(), action))
    {
        return true;
    }

    if (action == AAAssertDialogAction::MuteAssert)
    {
        gMutedAsserts[assertKey] = true;
        return false;
    }

    return action == AAAssertDialogAction::BreakNow;
}

} // namespace aa
