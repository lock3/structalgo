#include "hash.hpp"

#include <iostream>
#include <iomanip>
#include <vector>
#include <tuple>

struct debug_hasher
{
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

namespace game
{
  /// A simple ratio between the max value of a stat and its current level
  /// (e.g. health and magic).
  struct ratio
  {
    int max;
    int now;
  };

  struct player
  {
    std::string name;
    ratio health;
    ratio magic;

    template<hashing::hash_algorithm H>
    void hash_append(H& hash) const
    {
      hashing::hash_append(hash, name);
      hashing::hash_append(hash, health);
      hashing::hash_append(hash, magic);
    }
  };

  struct monster
  {
    int id;

    template<hashing::hash_algorithm H>
    friend void hash_append(H& hash, monster const& m)
    {
      hashing::hash_append(hash, m.id);
    }

    // Suppress customization.
    union { int x; };
  };

  struct item
  {
    int id;

    // Suppress customization.
    union { int x; };
  };

  template<hashing::hash_algorithm H>
  void hash_append(H& hash, item const& i)
  {
    hashing::hash_append(hash, i.id);
  }

} // namespace game

int main()
{
  using namespace hashing;
  
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

  // player andrew {"andrew", {100, 100}, {50, 50}};
  // hash_append(h, andrew);

  game::monster dragon {-1};
  hash_append(h, dragon);

  game::item sword {42};
  hash_append(h, sword);

  h.dump(std::cout);
}
