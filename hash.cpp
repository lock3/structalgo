#include "hash.hpp"
#include "game.hpp"

#include <iostream>
#include <iomanip>
#include <vector>
#include <tuple>
#include <unordered_map>

struct debug_hasher
{
  using result_type = std::size_t;

  void operator()(void const* key, std::size_t len) noexcept
  {
    unsigned char const* p = static_cast<unsigned char const*>(key);
    unsigned char const* const e = p + len;
    for (; p < e; ++p)
      buf.push_back(*p);
  }

  explicit operator std::size_t() noexcept
  {
    return buf.size();
  }

  void dump(std::ostream& os)
  {
    os << std::hex;
    os << std::setfill('0');
    unsigned int n = 0;
    for (auto c : buf) {
      os << std::setw(2) << (unsigned)c << ' ';
      if (++n == 16) {
        os << '\n';
        n = 0;
      }
    }
    os << '\n';
    os << std::dec;
    os << std::setfill(' ');
  }

  std::vector<unsigned char> buf;
};

void test_unordered_map()
{
  using hash = lock3::hash<lock3::fn1va64_hasher>;
  std::unordered_map<game::item, int, hash> prices;
  prices.emplace(game::item {0}, 100);
  prices.emplace(game::item {1}, 200);

  std::cout << prices[game::item {0}] << '\n';
  std::cout << prices[game::item {1}] << '\n';
  assert(prices.find(game::item {42}) == prices.end());
}

int main()
{
  using namespace lock3;
  
  debug_hasher h;

  // Base types
  hash_append(h, char(42));
  hash_append(h, 42);
  hash_append(h, 42ull);

  enum E0 { A, B, C };
  enum class E1 { A, B, C };
  hash_append(h, C);
  hash_append(h, E1::C);

  int n = 42;
  hash_append(h, &n);
  hash_append(h, nullptr);

  // User-defined types
  class S0 { };
  class S1 { int x = 42; };
  struct S2 { int x = 42; private: int y = 42; };
  struct S { union { int x; }; };

  hash_append(h, S0());
  hash_append(h, S1());
  hash_append(h, S2());
  // hash_append(h, S()); // error: anonymous union
  hash_append(h, std::make_pair(42, 'a'));
  hash_append(h, std::make_tuple(42, 'a', 32.0));

  game::player andrew {"andrew", {100, 100}, {50, 50}};
  hash_append(h, andrew);

  game::monster dragon {-1};
  hash_append(h, dragon);

  game::item sword {42};
  hash_append(h, sword);

  h.dump(std::cout);

  test_unordered_map();

  // FIXME: Test bitwise hashable things.
}
