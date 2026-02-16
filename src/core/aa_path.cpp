#include "core/aa_path.hpp"

namespace aa
{

namespace
{

#if defined(_WIN32)
constexpr char kPathSep = '\\';
#else
constexpr char kPathSep = '/';
#endif

} // namespace

AAPath::AAPath(const char *path)
{
    if (path != nullptr)
    {
        value_ = path;
    }
}

AAPath::AAPath(const AAString &path)
    : value_(path)
{
}

const char *AAPath::CStr() const
{
    return value_.c_str();
}

const AAString &AAPath::Str() const
{
    return value_;
}

b8 AAPath::Empty() const
{
    return value_.empty();
}

void AAPath::Clear()
{
    value_.clear();
}

b8 AAPath::EndsWith(const char *suffix) const
{
    if (suffix == nullptr)
    {
        return false;
    }

    const AAString suffixString(suffix);
    if (suffixString.size() > value_.size())
    {
        return false;
    }

    const usize start = value_.size() - suffixString.size();
    for (usize i = 0; i < suffixString.size(); ++i)
    {
        if (value_[start + i] != suffixString[i])
        {
            return false;
        }
    }

    return true;
}

AAPath AAPath::Join(const char *segment) const
{
    if (segment == nullptr || segment[0] == '\0')
    {
        return AAPath(value_);
    }

    if (value_.empty())
    {
        return AAPath(segment);
    }

    AAString joined = value_;
    if (joined.back() != kPathSep)
    {
        joined.push_back(kPathSep);
    }

    if (segment[0] == kPathSep)
    {
        joined.append(segment + 1);
    }
    else
    {
        joined.append(segment);
    }

    return AAPath(joined);
}

} // namespace aa
