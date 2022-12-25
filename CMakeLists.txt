cmake_minimum_required(VERSION 3.14)
project(Async)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

add_library(${PROJECT_NAME})

target_sources(${PROJECT_NAME}
PUBLIC
    src/thread_pool/pool.cpp
)

target_include_directories(${PROJECT_NAME} PUBLIC include)

# Testing with GTEST
include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip
)

set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

enable_testing()

SET(TESTS_GROUP async_tests)

add_executable(${TESTS_GROUP} tests/test_thread_pool.cpp)
target_link_libraries(${TESTS_GROUP} GTest::gtest_main ${PROJECT_NAME})
target_include_directories(${TESTS_GROUP} PRIVATE include)

include(GoogleTest)
gtest_discover_tests(${TESTS_GROUP})