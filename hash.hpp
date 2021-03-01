#ifndef HASHING_HPP
#define HASHING_HPP

#include "concepts.hpp"

#include <cstddef>
#include <cstdint>
#include <concepts>
#include <ranges>

namespace lock3
{
  /// The generic fnv1a hash algorithm.
  template<typename T, T Prime, T Offset>
  struct fn1va_hash
  {
    using result_type = T;

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
    explicit operator result_type() const
    {
      return code;
    }

    /// The accumulator.
    T code;
  };

  /// The fnv1a hash algorithm for 64-bit integers.
  struct fn1va32_hasher : fn1va_hash<std::uint32_t, 16777619u, 2166136261u>
  {
  };

  /// The fnv1a hash algorithm for 64-bit integers.
  struct fn1va64_hasher : fn1va_hash<std::uint64_t, 1099511628211ul, 14695981039346656037ul>
  {
  };

  // hash_algorithm

  /// Satisfied when `H` is a hash algorithm.
  template<typename H>
  concept hash_algorithm =
    requires (H& hash, void const* p, std::size_t n) {
      typename H::result_type;
      requires std::unsigned_integral<typename H::result_type>;
      hash(p, n);
      { (typename H::result_type)hash } -> std::same_as<typename H::result_type>;
    };

  // hash_result_t

  // The result type of a hash.
  template<hash_algorithm H>
  using hash_result_t = typename H::result_type;

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

    // Satisfied if T is a compound type that can be hashed.
    template<typename T, typename H>
    concept compound_hashable =
      member_hashable<T, H> ||
      adl_hashable<T, H> ||
      std::ranges::range<T> ||
      destructurable<T> ||
      basic_data_type<T>;

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
    template<hash_algorithm H, enumeral T>
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
    ///
    /// TODO: Define requirements for this algorithm.
    ///
    /// TODO: Provide a specialization for composite data types with unique
    /// object representation. Those can be applied separately for the range and
    /// destructuring cases, and possibly data type cases. We might also want to
    /// emit a warning for customizations that could be optimized, although
    /// those warnings are spurious if not all members are hashed by the
    /// customizing type.
    ///
    /// TODO: Some ranges (e.g., filter_view) are not const-iterable. To
    /// facilitatee those, we probably need to forward `obj` and rethink all
    /// our concepts.
    template<hash_algorithm H, detail::compound_hashable<H> T>
    void operator()(H& hash, T const& obj) const noexcept
    {
      if constexpr (detail::member_hashable<T, H>)
        append_using_member(hash, obj);
      else if constexpr (detail::adl_hashable<T, H>)
        append_using_adl(hash, obj);
      else if constexpr (std::ranges::range<T>)
        append_range(hash, obj);
      else if constexpr (destructurable<T>)
        append_destructurable(hash, obj);
      else // basic_data_type<T>
        append_data_type(hash, obj);
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
    template<hash_algorithm H, destructurable T>
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
    template<hash_algorithm H, basic_data_type T>
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

  // hashable

  /// Satisfied when T can be hashed with `H`.
  template<typename T, typename H>
  concept hashable_with =
    hash_algorithm<H> &&
    requires (T const& t, H& hash) {
      hash_append(hash, t);
    };
  
  // hash object

  /// A hash function that can (presumably) work
  template<hash_algorithm H>
  struct hash
  {
    template<hashable_with<H> T>
    hash_result_t<H> operator()(const T& obj) const noexcept
    {
      H hash;
      hash_append(hash, obj);
      return (hash_result_t<H>)hash;
    };
  };

} // namespace lock3

#endif
