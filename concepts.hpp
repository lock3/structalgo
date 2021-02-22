#ifndef STRUCTALGO_CONCEPTS_HPP
#define STRUCTALGO_CONCEPTS_HPP

#include <type_traits>
#include <tuple>

#include <experimental/meta>

namespace sa
{
  /// Satisfied if `T` is an enumeration type.
  template<typename T>
  concept enumeral = std::is_enum_v<T>;

  // Any old class.
  template<typename T>
  concept class_type = std::is_class_v<T>;

  // A standard layout class.
  template<typename T>
  concept standard_layout_class =
    std::is_class_v<T> &&
    std::is_standard_layout_v<T>;

  // Specifically a destructurable class.
  template<typename T>
  concept destructurable_class =
    std::is_class_v<T> &&
    requires {
      { std::tuple_size_v<T> } -> std::integral;
    };

} // namespace sa

#endif
