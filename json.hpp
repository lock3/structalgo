#ifndef LOCK3_JSON_HPP
#define LOCK3_JSON_HPP

#include "concepts.hpp"

#include <string>
#include <sstream>
#include <variant>
#include <experimental/meta>
#include <experimental/compiler>

#include <iostream> // FIXME: Remove this

namespace lock3::json
{
  /// Writes JSON-formatted values to an output stream. This is a CRTP class,
  /// meaning it is parameterized by its derived class. Doing so means that
  /// the derived class can provide additional overrides of write_value()
  /// for application-specific types.
  template<typename Derived, typename Out>
  struct basic_writer
  {
    basic_writer(Out& out)
      : out(out)
    { }

    /// Returns this cast as the derived class.
    Derived const& derived() const
    {
      return static_cast<Derived const&>(*this);
    }

    /// Returns this cast as the derived class.
    Derived& derived()
    {
      return static_cast<Derived&>(*this);
    }

    void write_value(bool b)
    {
      out << (b ? "true" : "false");
    }

    template<std::integral T>
    void write_value(T n)
    {
      out << n;
    }

    template<std::floating_point T>
    void write_value(T n)
    {
      out << n;
    }

    void write_value(std::string const& str)
    {
      out << '"' << str << '"';
    }

    template<std::ranges::range R>
    void write_array(R const& range)
    {
      out << '[';
      auto first = std::begin(range);
      auto last = std::end(range);
      for (auto iter = first; iter != last; ++iter) {
        derived().write(*iter);
        if (std::next(iter) != last)
          out << ',';
      }
      out << ']';
    }

    /// Write the members of a simple class.
    ///
    /// TODO: This does not handle base classes.
    template<basic_data_type T>
    void write_class(T const& obj)
    {
      namespace meta = std::experimental::meta;
      out << '{';
      constexpr auto members = meta::members_of(^T, meta::is_data_member);
      constexpr std::size_t num = size(members);
      std::size_t count = 0;
      template for (constexpr meta::info member : members) {
        out << '"' << meta::name_of(member) << '"' << ':';
        derived().write(obj.[:member:]);
        if (++count != num)
          out << ',';
      }
      out << '}';
    }

    /// Write the value of user-defined types (and arrays).
    template<typename T>
    void write_value(T const& t)
    {
      if constexpr (std::ranges::range<T>)
        return write_array(t);
      if constexpr (basic_data_type<T>)
        return write_class(t);
      else
        static_assert(dependent_false<T>(), "unreachable");
    }

    /// Write the JSON-formatted version of `t` to the output stream.
    template<typename T>
    void write(T const& t)
    {
      derived().write_value(t);
    }

    Out& out;
  };

  /// A simple JSON writer that handles classes without indirection.
  template<typename Out>
  struct writer : basic_writer<writer<Out>, Out>
  {
    writer(Out& out)
      : basic_writer<writer<Out>, Out>(out)
    { }
  };

  /// Reads JSON-formatted values from an input stream. This is a CRTP class,
  /// meaning it is parameterized by its derived class. Doing so means that
  /// the derived class can provide additional overrides of read_value()
  /// for application-specific types.
  ///
  /// This is a type-directed parser. That is, the type of object provided to
  /// `read()` will determine how the input is parsed.
  ///
  /// NOTE: This is not an efficient parser.
  template<typename Derived, typename In>
  struct basic_reader
  {
    basic_reader(In& in)
      : in(in)
    { }

    /// Returns this cast as the derived class.
    Derived const& derived() const
    {
      return static_cast<Derived const&>(*this);
    }

    /// Returns this cast as the derived class.
    Derived& derived()
    {
      return static_cast<Derived&>(*this);
    }

    [[noreturn]]
    void error(std::string const& str)
    {
      std::stringstream ss;
      ss << "error @ " << line << ':' << column << ": " << str;
      throw std::runtime_error(ss.str());
    }

    char get_char()
    {
      char c = in.get();
      if (c == '\n') {
        ++line;
        column = 1;
      }
      else {
        ++column;
      }
      return c;
    }

    char expect_char(char c)
    {
      if (in.peek() != c) {
        std::stringstream ss;
        ss << "expected '" << c << "'";
        error(ss.str());
      }
      get_char();
      return c;
    }

    char expect_punctuation(char c)
    {
      skip_space();
      char r = expect_char(c);
      skip_space();
      return r;
    }

    void skip_space()
    {
      while (char c = in.peek()) {
        if (!std::isspace(c))
          break;
        get_char();
      }      
    }

    std::string scan_word()
    {
      std::string s;
      skip_space();
      while (char c = in.peek()) {
        if (!std::isalpha(c))
          break;
        s += get_char();
      }
      skip_space();
      return s;
    }

    void scan_number(std::string& s)
    {
      while (char c = in.peek()) {
        if (!std::isdigit(c))
          break;
        s += get_char();
      }
    }

    // TODO: Support hex numbers?
    std::string scan_integer()
    {
      std::string s;
      skip_space();
      scan_number(s);
      if (s.empty())
        error("expected integer value");
      skip_space();
      return s;
    }

    // FIXME: Make this conform to the floating point input.
    std::string scan_float()
    {
      std::string s;
      skip_space();
      scan_number(s);
        error("expected floating point value");
      if (in.peek() == '.')
        s += get_char();
      scan_number(s);
      skip_space();
      return s;
    }

    // FIXME: Do a better job with escape characters.
    std::string scan_string()
    {
      std::string s;
      expect_char('"');
      while (char c = in.peek()) {
        if (c == '"')
          break;
        if (c == '\\')
          get_char();
        s += get_char();
      }
      expect_char('"');
      return s;
    }

    void read_value(bool& b)
    {
      std::string s = scan_word();
      if (s == "true")
        b = true;
      else if (s == "false")
        b = false;
      else
        error("expected 'true' or 'false'");
    }

    template<std::integral T>
    void read_value(T& n)
    {
      std::string num = scan_integer();
      n = std::stoll(num);
    }

    template<std::floating_point T>
    void read_value(T& n)
    {
      std::string num = scan_float();
      n = std::stod(num);
    }

    void read_value(std::string& str)
    {
      str = scan_string();
    }

    // template<std::ranges::range R>
    // void write_array(R const& range)
    // {
    //   out << '[';
    //   auto first = std::begin(range);
    //   auto last = std::end(range);
    //   for (auto iter = first; iter != last; ++iter) {
    //     derived().write(*iter);
    //     if (std::next(iter) != last)
    //       out << ',';
    //   }
    //   out << ']';
    // }

    template<typename T>
    void read_member(T& obj, std::string const& name)
    {
      namespace meta = std::experimental::meta;
      constexpr auto members = meta::members_of(^T, meta::is_data_member);
      template for (constexpr meta::info member : members) {
        if (meta::name_of(member) == name)
          return derived().read(obj.[:member:]);
      }
      std::stringstream ss;
      ss << "no member named '" << name << "' in '" << meta::name_of(^T) << "'";
      error(ss.str());
    }

    /// Read the members of a simple class.
    ///
    /// TODO: This does not handle base classes.
    template<basic_data_type T>
    void read_class(T& obj)
    {
      namespace meta = std::experimental::meta;

      constexpr auto members = meta::members_of(^T, meta::is_data_member);
      constexpr std::size_t num = size(members);
      std::size_t count = 0;

      expect_punctuation('{');
      while (true) {
        std::string key = scan_string();
        expect_punctuation(':');
        read_member(obj, key);
        ++count;
        if (in.peek() == '}')
          break;
        expect_punctuation(',');
      }
      expect_punctuation('}');

      if (count != num)
        error("incomplete initialization of object");
      }

    /// Write the value of user-defined types (and arrays).
    template<typename T>
    void read_value(T& t)
    {
      // if constexpr (std::ranges::range<T>)
      //   return write_array(t);
      if constexpr (basic_data_type<T>)
        return read_class(t);
      else
        static_assert(dependent_false<T>(), "unreachable");
    }

    /// Write the JSON-formatted version of `t` to the output stream.
    template<typename T>
    void read(T& t)
    {
      derived().read_value(t);
    }

    In& in;
    int line;
    int column;
  };

  /// A simple JSON reader that handles classes without indirection.
  template<typename In>
  struct reader : basic_reader<reader<In>, In>
  {
    reader(In& in)
      : basic_reader<reader<In>, In>(in)
    { }
  };

} // namespace lock3

#endif