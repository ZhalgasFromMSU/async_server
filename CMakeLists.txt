cmake_minimum_required(VERSION 3.13)
project(AsyncServer)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

add_subdirectory(src)

add_executable(runner main.cpp)

target_link_libraries(runner PUBLIC src)
target_include_directories(runner PUBLIC src)