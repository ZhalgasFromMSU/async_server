cmake_minimum_required(VERSION 3.28)

project(async_tests CXX)

include(GoogleTest)

FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG f8d7d77c06936315286eb55f8de22cd23c188571 # release-1.14.0
)
FetchContent_MakeAvailable(googletest)

set(TEST_SOURCES
  waitgroup.cpp
  queue.cpp
  threadpool.cpp
  optional.cpp

  pipe.cpp
  eventfd.cpp

  ioprovider.cpp
  operation.cpp

  coro.cpp
)

# Plain tests
add_executable(${PROJECT_NAME}
  ${TEST_SOURCES}
)
target_link_libraries(${PROJECT_NAME} ${CMAKE_PROJECT_NAME} GTest::gtest_main)
gtest_discover_tests(${PROJECT_NAME})

# ASAN tests
add_executable(${PROJECT_NAME}_asan
  ${TEST_SOURCES}
)
target_compile_options(${PROJECT_NAME}_asan
PUBLIC
  -fsanitize=address
)
target_link_options(${PROJECT_NAME}_asan
PUBLIC
  -fsanitize=address
)
target_link_libraries(${PROJECT_NAME}_asan ${CMAKE_PROJECT_NAME} GTest::gtest_main)
gtest_discover_tests(${PROJECT_NAME}_asan
  TEST_SUFFIX _asan
)

# TSAN tests
add_executable(${PROJECT_NAME}_tsan
  ${TEST_SOURCES}
)
target_compile_options(${PROJECT_NAME}_tsan
PUBLIC
  -fsanitize=thread
)
target_link_options(${PROJECT_NAME}_tsan
PUBLIC
  -fsanitize=thread
)
target_link_libraries(${PROJECT_NAME}_tsan ${CMAKE_PROJECT_NAME} GTest::gtest_main)
gtest_discover_tests(${PROJECT_NAME}_tsan
  TEST_SUFFIX _tsan
)




