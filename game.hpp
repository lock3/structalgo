#ifndef GAME_HPP
#define GAME_HPP

#include "hash.hpp"

namespace game
{
  /// A simple ratio between the max value of a stat and its current level
  /// (e.g. health and magic).
  struct ratio
  {
    int max;
    int current;
  };

  struct player
  {
    std::string name;
    ratio health;
    ratio magic;

    template<lock3::hash_algorithm H>
    void hash_append(H& hash) const
    {
      lock3::hash_append(hash, name);
      lock3::hash_append(hash, health);
      lock3::hash_append(hash, magic);
    }
  };

  struct monster
  {
    int id;

    template<lock3::hash_algorithm H>
    friend void hash_append(H& hash, monster const& m)
    {
      lock3::hash_append(hash, m.id);
    }

    // Suppress generic customization.
    union { int x; };
  };

  struct item
  {
    int id;

    friend bool operator==(item a, item b)
    {
      return a.id == b.id;
    }

    friend bool operator!=(item a, item b)
    {
      return a.id != b.id;
    }

    // Suppress generic customization.
    union { int x; };
  };

  template<lock3::hash_algorithm H>
  void hash_append(H& hash, item const& i)
  {
    lock3::hash_append(hash, i.id);
  }

} // namespace game

#endif
