#include "counting.hpp"

#include <iostream>

int main() {
  auto p1 = indirections_template<int, 42>();
  std::cout << meta::name_of(meta::type_of(^p1)) << '\n';

  constexpr meta::info type = indirections_refl(^int, 42);
  std::cout << meta::name_of(meta::type_of(type)) << '\n';

  typename [:type:] p2 = {};
  std::cout << meta::name_of(meta::type_of(^p2)) << '\n';

  auto p3 = indirections_fast<int, 42>();
  std::cout << meta::name_of(meta::type_of(^p3)) << '\n';
}

