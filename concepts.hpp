#ifndef STRUCTALGO_CONCEPTS_HPP
#define STRUCTALGO_CONCEPTS_HPP

#include "integers.hpp"

#include <type_traits>
#include <tuple>
#include <experimental/meta>
#include <experimental/compiler>

namespace sa
{
  /// Satisfied if `T` is an enumeration type.
  template<typename T>
  concept enumeral = std::is_enum_v<T>;

  /// Satisfied if `T` is a class type.
  template<typename T>
  concept class_type = std::is_class_v<T>;

  // destructurable

  namespace detail
  {
    namespace meta = std::experimental::meta;

    template<typename T, std::size_t I>
    consteval bool check_tuple_element()
    {
      return false;
    }

    template<typename T, std::size_t I>
      requires requires (T const& t) { std::get<I>(t); }
    consteval bool check_tuple_element()
    {
      return true;
    }

    // Check for `std::get<I>(t)` for all I in [0, N), with `t` an invented
    // tuple of type T.
    //
    // TODO: I should be able to inline the requires-expression in the condition
    // of the if statement, but when that fails, it effectively elides the
    // entire loop, so there's clearly a compiler bug here.
    template<typename T, std::size_t N>
    consteval bool check_tuple_get()
    {
      template for (constexpr std::size_t I : ints(N)) {
        if constexpr (!check_tuple_element<T, I>())
          return false;
      }
      return true;
    }

    // Satisfied if T can be queried with std::tuple_size and std::get.
    //
    // NOTE: We can't use tuple_size_v because that's wrongly satisfied for
    // non-tuple types (or maybe rightly). The specialization may be valid,
    // but the initializer is not. Something is a little off.
    template<typename T>
    concept tuple_type =
      class_type<T> &&
      requires {
        { std::tuple_size<T>::value } -> std::integral;
        // requires check_tuple_get<T, std::tuple_size_v<T>>();
      };

    // Returns true if the first data member of T is public or if T is empty.
    template<typename T>
    consteval bool is_first_member_accessible()
    {
      auto members = meta::members_of(^T, meta::is_data_member);
      if (members.begin() == members.end())
        return true;
      return meta::is_public(*members.begin());
    }

    // A helper function for checking if something is unnamed.
    constexpr std::size_t constexpr_length(char const* str)
    {
      return __builtin_strlen(str);
    }

    // Returns true if `T` contains a data member that is an anonymous union.
    template<typename T>
    consteval bool has_anonymous_union()
    {
      auto members = meta::members_of(^T, meta::is_data_member);
      for (meta::info member : members) {
        if (meta::is_union_type(meta::type_of(member)) && 
            constexpr_length(meta::name_of(member)) == 0)
          return true;
      }
      return false;
    }

    // Returns true if `T` contains no data members that are anonymous unions.
    template<typename T>
    consteval bool no_anonymous_union()
    {
      return !has_anonymous_union<T>();
    }

    // A destructurable class.
    //
    // We can't write requirements for statements, so there's no direct
    // way of expressing the requirement as a statement (sadly), but we
    // can generally express the rules in the standard for destructuring
    // an argument of type T.
    //
    // Note that the standard layout requirement guarantees the same access
    // for all data members. We can just check if the first is accessible
    // rather than all of them. Technically the rules for destructuring are
    // context dependent, meaning we could destructure private members in
    // contexts where we have access. However, for the purpose of generic
    // libraries, it seems reasonable to assume that we never have access,
    // either as a member of a class or befriended by a class.
    //
    // FIXME: This needs to be adjusted to accommodate inheritance.
    template<typename T>
    concept destructurable_class_type =
      class_type<T> &&
      std::is_standard_layout_v<T> &&
      is_first_member_accessible<T>() &&
      no_anonymous_union<T>();

    // Returns true if `T` can be used as the initializer of a structured
    // binding.
    //
    // NOTE: This isn't a concept because I don't want the disjunction to
    // participate in ordering overloads.
    //
    // TODO: Do we also care about destructuring constexpr ranges? Is there
    // overlap with the expansion statements, or is destrucurable necessarily
    // a more refined concept. I think these should be the same. Note that an
    // array is both a constexpr range and destructurable class.
    //
    // TODO: Re-enable destructurable classes. There's currently a bug
    // causing the compiler to crash on expanding plan classes.
    template<typename T>
    concept is_destructurable =
      tuple_type<T> ||
      destructurable_class_type<T> ||
      std::is_array_v<T>;

  } // namespace detail

  // Specifically a destructurable class.
  template<typename T>
  concept destructurable = detail::is_destructurable<T>;

  namespace detail
  {
    // FIXME: This is is practically the same as has_anonymous_union except
    // that it operates on all subobjects, not just direct members.
    template<typename T>
    consteval bool no_anonymous_union_subobjects()
    {
      auto members = meta::subobjects_of(^T);
      for (meta::info member : members) {
        if (meta::is_union_type(meta::type_of(member)) && 
            constexpr_length(meta::name_of(member)) == 0)
          return false;
      }
      return true;
    }
  } // namespace detail

  template<typename T>
  concept basic_data_type = detail::no_anonymous_union_subobjects<T>();


} // namespace sa

#endif
