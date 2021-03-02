#ifndef LOCK3_ENUM_HPP
#define LOCK3_ENUM_HPP

#include "concepts.hpp"

#include <experimental/meta>

namespace lock3
{
  /// Returns the string representation of `e`.
  template<enumeral T>
  char const* to_string(T value)
  {
    template for (constexpr meta::info e : meta::members_of(^T))
      if (e == value)
        return meta::name_of(e);
    return "<unknown>";
  }

} // namespace lock3

#endif