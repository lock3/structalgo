#include "compare.hpp"

#include <iostream>
#include <iomanip>
#include <vector>
#include <tuple>

std::ostream& operator<<(std::ostream& os, std::strong_ordering ord)
{
  if (ord == std::strong_ordering::less)
    return os << "less";
  if (ord == std::strong_ordering::greater)
    return os << "greater";
  return os << "equal";
}

int main()
{
  namespace cmp = comparison;

  std::cout << cmp::compare(0, 42) << '\n';
}
