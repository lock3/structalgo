#include "json.hpp"
#include "game.hpp"

#include <iostream>
#include <fstream>
#include <vector>

int main(int argc, char* argv[])
{
  std::ifstream is(argv[1]);

  lock3::json::reader reader(is);
  game::player p1;
  reader.read(p1);

  lock3::json::writer writer(std::cout);
  writer.write(p1);
  std::cout << '\n';
}
