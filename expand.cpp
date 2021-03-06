#include "expand.hpp"

int main() {
  auto x = std::make_tuple(42, 'a', 3.14);

  std::cout << [:lock3::expand(x):] << '\n';

  return 0;
}
