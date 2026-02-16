#pragma once

#include "core/aa_containers.hpp"
#include "core/aa_types.hpp"

namespace aa
{

class AAPath
{
  public:
    AAPath() = default;
    explicit AAPath(const char *path);
    explicit AAPath(const AAString &path);

    const char *CStr() const;
    const AAString &Str() const;

    b8 Empty() const;
    void Clear();

    b8 EndsWith(const char *suffix) const;
    AAPath Join(const char *segment) const;

  private:
    AAString value_;
};

} // namespace aa
