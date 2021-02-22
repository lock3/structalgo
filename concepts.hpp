#ifndef STRUCTALGO_CONCEPTS_HPP
#define STRUCTALGO_CONCEPTS_HPP

#include <type_traits>

#include <experimental/meta>

namespace sa
{
  /// Satisfied if `T` is an enumeration type.
  template<typename T>
  concept enumeral = std::is_enum_v<T>;

  // TODO: This is almost certainly wrong.
  template<typename T>
  concept class_type = std::is_class_v<T>;

} // namespace sa

#endif
