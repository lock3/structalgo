#ifndef LOCK3_COMPARISON_HPP
#define LOCK3_COMPARISON_HPP

#include "concepts.hpp"

#include <cstddef>
#include <cstdint>
#include <concepts>
#include <ranges>
#include <compare>

#include <iostream> // FIXME: Remove this.

namespace lock3
{
  // compare

  struct compare_fn
  {
    /// Compare integral types.
    template<std::integral T>
    std::strong_ordering operator()(T a, T b) const noexcept
    {
      if (a < b)
        return std::strong_ordering::less;
      if (b < a)
        return std::strong_ordering::greater;
      return std::strong_ordering::equal;
    }

    /// Compare floating point types.
    template<std::floating_point T>
    std::partial_ordering operator()(T a, T b) const noexcept
    {
      if (a == b)
        return std::partial_ordering::equivalent;
      if (a < b)
        return std::partial_ordering::less;
      if (b < a)
        return std::partial_ordering::greater;
      return std::partial_ordering::unordered;
    }

    /// Compare enumeration types.
    template<enumeral T>
    std::strong_ordering operator()(T a, T b) const noexcept
    {
      using Z = std::underlying_type_t<T>;
      return operator()(static_cast<Z>(a), static_cast<Z>(b));
    }

    /// Compare two pointers. In the case where pointers are non-equal, but but
    /// point not distinctly less or greater is unspecified.
    template<typename T>
    std::strong_ordering operator()(T* p, T* q) const noexcept
    {
      if (p < q)
        return std::strong_ordering::less;
      if (q < p)
        return std::strong_ordering::greater;
      return std::strong_ordering::equal;
    }
  };

  constexpr compare_fn compare;


  // FIXME: The comparison of a and b must yield a strong order for a and
  // b to be equal. This is kind of weird since that's defined in terms of
  // an order that's kind of implicit in the type.
  template<typename T>
  bool equal(T a, T b) noexcept
  {
    return compare(a, b) == std::strong_ordering::equal;
  }

} // namespace lock3

#endif
