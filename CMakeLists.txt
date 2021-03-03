
project(structalgo CXX)

cmake_minimum_required(VERSION 3.15)

# Set build flags.
set(CMAKE_CXX_FLAGS "-std=c++20 -freflection")

# FIXME: This is a hack because the testing version of
# libc++ doesn't define any concepts.
if(CMAKE_SYSTEM_NAME STREQUAL Darwin)
  set(CMAKE_CXX_FLAGS
    "${CMAKE_CXX_FLAGS} \
    -stdlib=libstdc++ \
    -stdlib++-isystem=$ENV{HOME}/opt/include/c++/11.0.0 \
    -I$ENV{HOME}/opt/include/c++/11.0.0 \
    -I$ENV{HOME}/opt/include/c++/11.0.0/x86_64-apple-darwin18.7.0"
    )
  set(CMAKE_CXX_LINK_FLAGS "-L$ENV{HOME}/opt/lib")
endif()

include_directories($ENV{HOME}/opt/include)

add_library(sa
  concepts.cpp
  integers.cpp
)

add_executable(hash
  hash.cpp)
add_executable(compare
  compare.cpp)
add_executable(tuple
  tuple.cpp)
add_executable(json-write
  json-write.cpp)
add_executable(json-read
  json-read.cpp)
add_executable(universal
  universal.cpp)
