#ifndef LOCK3_INTEGERS_HPP
#define LOCK3_INTEGERS_HPP

#include <cassert>
#include <cstddef>
#include <compare>
#include <concepts>
#include <iterator>
#include <type_traits>

namespace lock3
{
  /// A contexpr iterator over a range of integers.
  ///
  /// TODO: What's the minimal boilerplate needed to define a C++20 iterator?
  template<std::integral T>
  struct integer_iterator
  {
    using value_type = T;
    using reference = value_type;
    using difference_type = std::make_signed_t<value_type>;
    using iterator_category = std::random_access_iterator_tag;

    constexpr explicit integer_iterator(value_type n)
      : m_value(n)
    { }

    constexpr reference operator*() const
    {
      return m_value;
    }

    constexpr reference operator[](difference_type n) const
    {
      return m_value + n;
    }

    constexpr integer_iterator& operator++()
    {
      ++m_value;
      return *this;
    }

    constexpr integer_iterator& operator++(int)
    {
      integer_iterator tmp = *this;
      ++m_value;
      return tmp;
    }

    constexpr integer_iterator operator+=(difference_type n)
    {
      m_value += n;
      return *this;
    }

    constexpr integer_iterator& operator--()
    {
      --m_value;
      return *this;
    }

    constexpr integer_iterator& operator--(int)
    {
      integer_iterator tmp = *this;
      --m_value;
      return tmp;
    }

    constexpr integer_iterator operator-=(difference_type n)
    {
      m_value -= n;
      return *this;
    }

    constexpr friend integer_iterator operator+(integer_iterator i, difference_type n)
    {
      return i += n;
    }

    constexpr friend integer_iterator operator+(difference_type n, integer_iterator i)
    {
      return i += n;
    }

    constexpr friend integer_iterator operator-(integer_iterator i, difference_type n)
    {
      return i -= n;
    }

    constexpr friend difference_type operator-(integer_iterator i, integer_iterator j)
    {
      return i.m_value - j.m_value;
    }

    constexpr friend std::strong_ordering operator<=>(integer_iterator i, integer_iterator j)
    {
      return i.m_value <=> j.m_value;
    }

    value_type m_value;
  };

  /// Defines a destructur
  template<std::integral T>
  struct integer_range
  {
    using value_type = T;
    using iterator = integer_iterator<T>;

    /// Constructs an integer range over `[0, last)`.
    constexpr integer_range(T last)
      : integer_range(T(0), last)
    {
    }

    /// Constructs an integer range over `[first, last)`.
    constexpr integer_range(T first, T last)
      : m_first(first), m_last(last)
    {
      assert(first <= last);
    }

    constexpr iterator begin() const
    {
      return iterator(m_first);
    }

    constexpr iterator end() const
    {
      return iterator(m_last);
    }

    value_type m_first;
    value_type m_last;
  };

  /// Returns an expandable sequence of integers.
  template<std::integral T>
  constexpr integer_range<T> ints(T last)
  {
    return integer_range<T>(last);
  }

  /// Returns an expandable sequence of integers.
  template<std::integral T>
  constexpr integer_range<T> ints(T first, T last)
  {
    return integer_range<T>(first, last);
  }

} // namespace lock3

#endif
