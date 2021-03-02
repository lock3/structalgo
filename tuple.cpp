#include "tuple.hpp"

#include <iostream>
#include <iomanip>
#include <vector>
#include <tuple>

struct s1
{
  int a = 42, b = -1, c = 12;
};

struct s2 : s1
{
  int z = 51;
};

struct s3 : private s1
{
};

int main()
{
  auto t1 = std::make_tuple(1, 2, 3);
  std::cout << lock3::get<0>(t1) << ' ';
  std::cout << lock3::get<1>(t1) << ' ';
  std::cout << lock3::get<2>(t1) << '\n';

  std::array<int, 2> a1 = { 4, 5 };
  std::cout << lock3::get<0>(a1) << ' ';
  std::cout << lock3::get<1>(a1) << '\n';
  
  s1 x1;
  std::cout << lock3::get<0>(x1) << ' ';
  std::cout << lock3::get<1>(x1) << ' ';
  std::cout << lock3::get<2>(x1) << '\n';

  s2 x2;
  std::cout << lock3::get<0>(x2) << ' ';
  std::cout << lock3::get<3>(x2) << '\n';

  s3 x3;
  std::cout << lock3::get<0>(x3) << '\n';
}
