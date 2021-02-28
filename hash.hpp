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

  namespace detail
  {
    // Satisfied if `T` provides a hash-append member for `H`.
    template<typename T, typename H>
    concept member_hashable =
      requires (T const& t, H& hash)
      {
        t.hash_append(hash);
      };
    
    // Satisfied if `T` can be hashed.
    template<typename T, typename H>
    concept adl_hashable =
      requires (T const& t, H& hash)
      {
        hash_append(hash, t);
      };

  } // namespace detail

  struct hash_append_fn
  {
    /// Append the bits of integral types.
    template<hash_algorithm H, std::integral T>
    void operator()(H& hash, T t) const noexcept
    {
      hash(&t, sizeof(t));
    }

    /// Append the bits of enumeration types.
    template<hash_algorithm H, sa::enumeral T>
    void operator()(H& hash, T t) const noexcept
    {
      hash(&t, sizeof(t));
    }

    /// Append the bits of floating point types, but guarantee that 0 and -0
    /// have the same hash code since 0 == -0.
    template<hash_algorithm H, std::floating_point T>
    void operator()(H& hash, T t) const noexcept
    {
      if (t == 0)
        t = 0;
      hash(&t, sizeof(t));
    }

    /// Append the bits of a pointer by hashing its representation, not the
    /// bits of the object pointed at (i.e., no indirection is performed).
    template<hash_algorithm H, typename T>
    void operator()(H& hash, T* p) const noexcept
    {
      hash(&p, sizeof(p));
    }

    /// Hash the bits of the nullptr constant.
    template<hash_algorithm H>
    inline void operator()(H& hash, std::nullptr_t p) const noexcept
    {
      hash(&p, sizeof(p));
    }

    /// Hash the contents of almost everything else.
    ///
    /// NOTE: Concepts are great when alternatives operate on either disjoint
    /// sets of types, or when refinement is clearly involved. When that isn't
    /// the case---when the sets of types overlap---we can only choose behaviors
    /// based on some preference. That's where constexpr if comes into play.
    template<hash_algorithm H, typename T>
    void operator()(H& hash, T const& obj) const noexcept
    {
      if constexpr (detail::member_hashable<T, H>)
        append_using_member(hash, obj);
      else if constexpr (std::ranges::range<T>)
        append_range(hash, obj);
      else if constexpr (sa::destructurable<T>)
        append_destructurable(hash, obj);
      else if constexpr (sa::basic_data_type<T>)
        append_data_type(hash, obj);
      else if constexpr (detail::adl_hashable<T, H>)
        append_using_adl(hash, obj);
      else
        static_assert(sa::dependent_false<T>(), "no matching overload");
    }

    // Append for member customization.
    template<hash_algorithm H, detail::member_hashable<H> T>
    void append_using_member(H& hash, T const& obj) const noexcept
    {
      obj.hash_append(hash);
    }

    // Append using ADL lookup to find non-member definitions or friend function
    // definitions.
    //
    // NOTE: This is somewhat fragile. If this wasn't a customization point
    // object, the "most generic" version of `hash_append` would need to be
    // declared before the "dispatcher". In that case, calls to `hash_append`
    // could bind to that name for non-matching cases. The result is infinite
    // recursion.
    //
    // The CPO approach works (presumably) because the hash_append has not
    // yet been declared in this scope.
    template<hash_algorithm H, detail::adl_hashable<H> T>
    void append_using_adl(H& hash, T const& obj) const noexcept
    {
      hash_append(hash, obj);
    }

    // Hash append for ranges.
    template<hash_algorithm H, std::ranges::range T>
    void append_range(H& hash, T const& range) const noexcept
    {
      std::size_t count = 0;
      for (auto const& elem : range) {
        operator()(hash, elem);
        ++count;
      }
      operator()(hash, count);
    }

    // Hash append for arrays, tuples, and other simple classes.
    template<hash_algorithm H, sa::destructurable T>
    void append_destructurable(H& hash, T const& obj) const noexcept
    {
      std::size_t count = 0;
      template for (auto const& member : obj) {
        operator()(hash, member);
        ++count;
      }
      operator()(hash, count);
    }

    // Hash append for basic data types in an application domain.
    template<hash_algorithm H, sa::basic_data_type T>
    void append_data_type(H& hash, T const& obj) const noexcept
    {
      namespace meta = std::experimental::meta;
      std::size_t count = 0;
      constexpr auto subobjects = meta::subobjects_of(^T, meta::is_data_member);
      template for (constexpr meta::info sub : subobjects) {
        operator()(hash, obj.[:sub:]);
        ++count;
      }
      operator()(hash, count);
    }
  };

  constexpr hash_append_fn hash_append;

  // hash object

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
