#ifndef LOCK3_EXPAND_HPP
#define LOCK3_EXPAND_HPP

#include <vector>
#include <tuple>
#include <experimental/meta>

namespace lock3
{
  namespace meta = std::experimental::meta;

  // template<typename T>
  // consteval std::vector<meta::info> expand(T const& t)
  // {
  //   std::vector<meta::info> vec;
  //   constexpr auto members = meta::subobjects_of(^T);
  //   template for (constexpr int I : size(members)) {
  //     constexpr meta::info expr = ^(std::get<I>(t));
  //   }
  //   return vec;
  // }

  template<typename T>
  consteval meta::info expand(T const& t)
  {
    return ^(std::get<0>(t));
  }

} // namespace lock3

#endif
