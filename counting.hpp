#ifndef LOCK3_COUNTING_EXAMPLE_HPP
#define LOCK3_COUNTING_EXAMPLE_HPP

#include <experimental/meta>

namespace meta = std::experimental::meta;

template<typename T, int N>
auto indirections_template()
{
  if constexpr (N == 0)
    return T{};
  else
    return indirections_template<T*, N - 1>();
}

consteval meta::info indirections_refl(meta::info type, int n)
{
  for (int i = 0; i < n; ++i)
    type = meta::add_pointer(type);
  return type;
}

// template<typename T, int N>
// auto indirections_fast()
// {
//   return typename [:indirections_refl(^T, N):]{};
// }

template<typename T, int N>
auto indirections_fast()
{
  constexpr auto type = []() consteval {
    auto type = ^T;
    for (int i = 0; i < N; ++i) {
      type = meta::add_pointer(type);
    }
    return type;
  }();

  return typename [:type:]{};
}

#endif
