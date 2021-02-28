
project(structalgo CXX)

cmake_minimum_required(VERSION 3.15)

set(CMAKE_CXX_FLAGS "-std=c++20 -freflection")

include_directories($ENV{HOME}/opt/include)

add_library(sa
  concepts.cpp
  integers.cpp
)

add_executable(hash
  hash.cpp)

add_executable(compare
  compare.cpp)

