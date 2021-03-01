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

template<typename T>
void test(T a, T b)
{
  using namespace lock3;
  std::cout << "compare(" << a << ", " << b << ") == " << compare(a, b) << '\n';
}

int main()
{
  test(0, 0);
  test(0, 42);
  test(42, 0);
}
