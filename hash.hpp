#ifndef HASHING_UTILITY_HASH_HPP
#define HASHING_UTILITY_HASH_HPP

#include "concepts.hpp"

#include <cstddef>
#include <cstdint>
#include <concepts>
#include <ranges>

#include <iostream> // FIXME: Remove this.

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
    void operator()(const void* p, std::size_t n)
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

  // Generic, structurally recursive hash append.
  //
  // NOTE: Like all recursive algorithms on composite tree structures, we either
  // need to declare the generic version before its various implementations,
  // or we declare all the various implementations before this function's
  // definition. This uses the former.
  template<hash_algorithm H, typename T>
  void hash_append(H& hash, T const& obj);

  namespace detail
  {
    // Satisfied if `T` provides a hash-append member for `H`.
    template<typename T, typename H>
    concept custom_member_hashable =
      requires (T const& t, H& hash)
      {
        t.hash_append(hash);
      };
    
    // Hash append for classes that customize hash append as a member.
    template<hash_algorithm H, custom_member_hashable<H> T>
    void hash_append_custom_member(H& hash, T const& obj)
    {
      obj.hash_append(hash);
    }

    // FIXME: This doesn't work because the lookup for hash_append can find
    // the wrong thing, resulting in infinite recursion.
    template<typename T, typename H>
    concept custom_adl_hashable =
      requires (T const& t, H& hash)
      {
        hash_append(hash, t);
      };

    // Hash append for classes that customize hash append as a non-member or
    // friend definition.
    template<hash_algorithm H, custom_adl_hashable<H> T>
    void hash_append_custom_adl(H& hash, T const& obj)
    {
      hash_append(hash, obj);
    }

    // Hash append for ranges.
    template<hash_algorithm H, std::ranges::range T>
    void hash_append_range(H& hash, T const& range)
    {
      std::size_t count = 0;
      for (auto const& elem : range) {
        hash_append(hash, elem);
        ++count;
      }
      hash_append(hash, count);
    }

    // Hash append for arrays, tuples, and other simple classes.
    template<hash_algorithm H, sa::destructurable T>
    void hash_append_destructurable(H& hash, T const& obj)
    {
      std::size_t count = 0;
      template for (auto const& member : obj) {
        hash_append(hash, member);
        ++count;
      }
      hash_append(hash, count);
    }

    // Hash append for basic data types in an application domain.
    template<hash_algorithm H, sa::basic_data_type T>
    void hash_append_data_type(H& hash, T const& obj)
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
  } // namespace detail

  // Hash classes or destructurable types.
  //
  // These constraints define overlapping sets of types. Some destructurable
  // types are also classes.
  //
  // NOTE: Concepts are great when alternatives operate on either disjoint sets
  // of types, or when refinement is clearly involved. When that isn't the
  // case---when the sets of types overlap---we can only choose behaviors based
  // on some preference. That's where constexpr if comes into play.
  //
  // NOTE: The ADL case is *bad* if you put it in the wrong place. The ADL
  // lookup can find the main `hash_append` function, causing lookup to succeed
  // but generate infinite recursion. Here, we make that the fallback case,
  // which forces a late error.
  template<hash_algorithm H, typename T>
  void hash_append(H& hash, T const& obj)
  {
    if constexpr (detail::custom_member_hashable<T, H>)
      detail::hash_append_custom_member(hash, obj);
    else if constexpr (std::ranges::range<T>)
      detail::hash_append_range(hash, obj);
    else if constexpr (sa::destructurable<T>)
      detail::hash_append_destructurable(hash, obj);
    else if constexpr(sa::basic_data_type<T>)
      detail::hash_append_data_type(hash, obj);
    // else if constexpr (detail::custom_adl_hashable<T, H>)
    //   detail::hash_append_custom_adl(hash, obj);
    else
      static_assert(sa::dependent_false<T>(), "no matching overload");
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
