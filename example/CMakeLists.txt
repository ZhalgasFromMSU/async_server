cmake_minimum_required(VERSION 3.28)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

project(example CXX)

add_subdirectory(../ async)

add_executable(${PROJECT_NAME} main.cpp)
target_link_libraries(${PROJECT_NAME} async)
