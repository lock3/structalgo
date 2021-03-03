#ifndef LOCK3_TUPLE_HPP
#define LOCK3_TUPLE_HPP

#include "concepts.hpp"

#include <type_traits>
#include <tuple>
#include <experimental/meta>
#include <experimental/compiler>

namespace lock3
{
  namespace detail
  {
    // Satsified if T can be made to work like a tuple.
    //
    // FIXME: This is a terrible name.
    template<typename T>
    concept tuple_like =
      detail::std_product_type<T> ||
      basic_data_type<T>;

    template<std::size_t N, basic_data_type T>
    decltype(auto) get_data_type_member(T const& t)
    {
      namespace meta = std::experimental::meta;
      constexpr auto members = meta::subobjects_of(^T);
      static_assert(N < size(members));
      constexpr meta::info nth = *std::next(members.begin(), N);
      return t.[:nth:];
    }
  } // namespace detail

  /// Get the Nth element of the class T.
  ///
  /// NOTE: std::get is not a customization point, but lock3::get could be.
  /// We haven't defined it as such for now.
  template<std::size_t N, detail::tuple_like T>
  decltype(auto) get(T const& t)
  {
    if constexpr (detail::std_product_type<T>)
      return std::get<N>(t);
    else
      return detail::get_data_type_member<N>(t);
  }

  /// FIXME: Add some kind of tuple_size (call size() and make it constexpr)?

} // namespace lock3

#endif