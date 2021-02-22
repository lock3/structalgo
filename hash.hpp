#ifndef HASHING_UTILITY_HASH_HPP
#define HASHING_UTILITY_HASH_HPP

#include "concepts.hpp"

#include <cstddef>
#include <cstdint>
#include <concepts>
#include <ranges>

namespace hashing
{
  /// The generic fnv1a hash algorithm.
  template<typename T, T Prime, T Offset>
  struct fn1va_hash
  {
    fn1va_hash()
      : code(Offset)
    {
    }

    /// Hash bytes into the code.
    void operator()(const void* p, int n)
    {
      unsigned char const* first = static_cast<unsigned char const*>(p);
      unsigned char const* limit = first + n;
      for (; first != limit; ++first)
        code = (code ^ *first) * Prime;
    }

    /// Converts to the computed hash code.
    explicit operator T() const
    {
      return code;
    }

    /// The accumulator.
    T code;
  };

  /// The fnv1a hash algorithm for 64-bit integers.
  ///
  /// We use inheritance so the prime and Offset aren't exposed in diagnostics.
  struct fn1va32_hasher : fn1va_hash<std::uint32_t, 16777619u, 2166136261u>
  {
  };

  /// The fnv1a hash algorithm for 64-bit integers.
  ///
  /// We use inheritance so the prime and Offset aren't exposed in diagnostics.
  struct fn1va64_hasher : fn1va_hash<std::uint64_t, 1099511628211ul, 14695981039346656037ul>
  {
  };

  // hasher

  template<typename H>
  concept hash_algorithm =
    requires (H& hash, void const* p, std::size_t n) {
      hash(p, n);
      { (std::uintmax_t)hash } -> std::integral;
    };

  // hash_append

  /// Hash for trivially comparable T.
  template<hash_algorithm H, std::integral T>
  void hash_append(H& hash, T t)
  {
    hash(&t, sizeof(t));
  }

  template<hash_algorithm H, sa::enumeral T>
  void hash_append(H& hash, T t)
  {
    hash(&t, sizeof(t));
  }

  /// Hash for floating point T. Guarantee that 0 and -0 have the same
  /// hash code since 0 == -0.
  template<hash_algorithm H, std::floating_point T>
  void hash_append(H& hash, T t)
  {
    if (t == 0)
      t = 0;
    hash(&t, sizeof(t));
  }

  /// Hash for pointers. This just hashes the bits of the address, not the
  /// indirect object. To hash the indirect object, provide an overload of
  /// hash_append for that specific type.
  template<hash_algorithm H, typename T>
  void hash_append(H& hash, T* p)
  {
    hash(&p, sizeof(p));
  }

  /// Hash append for nullptr.
  template<hash_algorithm H>
  inline void hash_append(H& hash, std::nullptr_t p)
  {
    hash(&p, sizeof(p));
  }

  /// Hash a plain class.
  template<hash_algorithm H, sa::plain_struct T>
  void hash_append(H& hash, T const& obj)
  {
    namespace meta = std::experimental::meta;
    constexpr auto members = meta::members_of(^T, meta::is_data_member);
    template for (constexpr meta::info member : members)
      hash_append(hash, obj.[:member:]);
    hash_append(hash, std::distance(members.begin(), members.end()));
  }

#if 0
  /// Hash a range of elements.
  template<std::ranges::range R, hash_algorithm<T> H>
  void hash_append(H& hash, R const& range)
  {
    std::size_t count = 0;
    for (auto const& elem : range) {
      hash_append(hash, elem);
      ++count;
    }
    hash_append(hash, count);
  }

  /// Hash a sized range of elements.
  template<std::ranges::sized_range R, hash_algorithm<T> H>
  void hash_append(fn1va64_hasher& h, R const& range)
  {
    for (auto const& elem : range)
      hash_append(h, elem);
    hash_append(h, std::ranges::size(range));
  }
#endif

  // hash

  /// Computes the hash values of objects.
  template<typename T>
  struct hash
  {
    std::size_t operator()(const T& obj) const noexcept
    {
      fn1va64_hasher h;
      hash_append(h, obj);
      return (std::size_t)h;
    };
  };

  // /// Hashes a pointer to an object.
  // template<typename T>
  // struct Indirect_hash
  // {
  //   std::size_t operator()(const T* obj) const noexcept
  //   {
  //     fn1va64_hasher h;
  //     hash_append(h, *obj);
  //     return (std::size_t)h;
  //   };
  // };

} // namespace hashing

#endif
