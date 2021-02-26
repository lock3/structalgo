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

  namespace detail
  {
    template<hash_algorithm H, sa::class_type T>
    void hash_append_class(H& hash, T const& obj)
    {
      namespace meta = std::experimental::meta;
      std::size_t count = 0;
      constexpr auto subobjects = meta::subobjects_of(^T, meta::is_data_member);
      template for (constexpr meta::info sub : subobjects) {
        hash_append(hash, obj.[:sub:]);
        ++count;
      }
      hash_append(hash, count);
    }

    template<hash_algorithm H, sa::destructurable T>
    void hash_append_destructurable(H& hash, T const& obj)
    {
      std::size_t count = 0;
      template for (auto member : obj) {
        hash_append(hash, member);
        ++count;
      }
      hash_append(hash, count);
    }
  } // namespace detail

  // Hash classes or destructurable types.
  //
  // These constraints define overlapping sets of types. Some destructurable
  // types are also classes.
  //
  // FIXME: The class_type constraint is REALLY too loose. We probably want
  // similar guarantees of no unnamed unions.
  template<hash_algorithm H, typename T>
    requires sa::class_type<T> || sa::destructurable<T>
  void hash_append(H& hash, T const& obj)
  {
    if constexpr (sa::destructurable<T>)
      detail::hash_append_destructurable(hash, obj);
    else
      detail::hash_append_class(hash, obj);
  }

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

} // namespace hashing

#endif
