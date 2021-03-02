#include "json.hpp"
#include "game.hpp"

#include <iostream>
#include <fstream>
#include <vector>

int main(int argc, char* argv[])
{
  game::player p1 {"andrew", {100, 100}, {50, 50}};
  lock3::json::writer writer(std::cout);
  writer.write(p1);
}
