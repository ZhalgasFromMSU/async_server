cmake_minimum_required(VERSION 3.28)

project(async_tests CXX)

include(FetchContent)

FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG f8d7d77c06936315286eb55f8de22cd23c188571 # release-1.14.0
)
FetchContent_MakeAvailable(googletest)

add_executable(${PROJECT_NAME}
  waitgroup.cpp
  queue.cpp
  threadpool.cpp
)

target_link_libraries(${PROJECT_NAME} ${CMAKE_PROJECT_NAME} GTest::gtest_main)

