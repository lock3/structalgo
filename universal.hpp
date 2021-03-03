#ifndef LOCK3_UNIVERSAL_HPP
#define LOCK3_UNIVERSAL_HPP

#include <experimental/meta>

namespace lock3::mpl
{
  // Metaprogramming facilities

  namespace meta = std::experimental::meta;

  /// Apply `Args...` to the reflected `Template`.
  template<meta::info Template, meta::info... Args>
  using apply = typename [:Template:]<...[:Args:]...>;

} // namespace lock3

#endif