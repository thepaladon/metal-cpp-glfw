#include "core/aa_platform.hpp"

#include <cerrno>
#include <cstring>

#if defined(_WIN32)
#include <direct.h>
#else
#include <limits.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#endif

namespace aa
{

namespace
{

#if defined(_WIN32)
constexpr char kPathSep = '\\';
#else
constexpr char kPathSep = '/';
#endif

b8 AADirectoryExists(const char *path)
{
#if defined(_WIN32)
    struct _stat info = {};
    if (_stat(path, &info) != 0)
    {
        return false;
    }
    return (info.st_mode & _S_IFDIR) != 0;
#else
    struct stat info = {};
    if (stat(path, &info) != 0)
    {
        return false;
    }
    return S_ISDIR(info.st_mode);
#endif
}

b8 AAMakeSingleDirectory(const char *path)
{
#if defined(_WIN32)
    if (_mkdir(path) == 0)
    {
        return true;
    }
#else
    if (mkdir(path, 0755) == 0)
    {
        return true;
    }
#endif

    return errno == EEXIST && AADirectoryExists(path);
}

} // namespace

AAPath AAGetCurrentWorkingDirectory()
{
#if defined(_WIN32)
    char buffer[4096] = {};
    if (_getcwd(buffer, sizeof(buffer)) == nullptr)
    {
        return AAPath();
    }
#else
    char buffer[PATH_MAX] = {};
    if (getcwd(buffer, sizeof(buffer)) == nullptr)
    {
        return AAPath();
    }
#endif

    return AAPath(buffer);
}

b8 AACreateDirectories(const AAPath &path)
{
    if (path.Empty())
    {
        return false;
    }

    AAString mutablePath = path.Str();
    if (mutablePath.empty())
    {
        return false;
    }

    for (usize i = 1; i < mutablePath.size(); ++i)
    {
        if (mutablePath[i] == '/' || mutablePath[i] == '\\')
        {
            const char backup = mutablePath[i];
            mutablePath[i] = '\0';

            if (mutablePath[0] != '\0' && !AAMakeSingleDirectory(mutablePath.c_str()))
            {
                mutablePath[i] = backup;
                return false;
            }

            mutablePath[i] = backup;
        }
    }

    return AAMakeSingleDirectory(mutablePath.c_str());
}

} // namespace aa
