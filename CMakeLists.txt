cmake_minimum_required(VERSION 3.14)
project(Async)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

add_library(${PROJECT_NAME})

target_sources(${PROJECT_NAME}
PUBLIC
    src/thread/wait_group.cpp
    src/thread/pool.cpp
    src/polling/epoll.cpp
    src/io/io_object.cpp
    src/io/read.cpp
    src/io/write.cpp
    src/io/well_known_structs.cpp
    src/socket/address.cpp
    src/util/result.cpp
)

target_include_directories(${PROJECT_NAME} PUBLIC include)

# Compiler options
add_compile_options(-Werror -Wall -pedantic)

# For debugging
option(USE_THREAD_SANITIZER "Compile with thread sanitizer to detect data races")
option(USE_ADDRESS_SANITIZER "Compile with address sanitizer to detect memory errors" ON)

if (USE_THREAD_SANITIZER)
    add_compile_options(-fsanitize=thread)
    add_link_options(-fsanitize=thread)
endif()

if (USE_ADDRESS_SANITIZER)
    add_compile_options(-fsanitize=address)
    add_link_options(-fsanitize=address)
endif()


# Testing with GTEST
include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip
)

set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

enable_testing()

set(TESTS_GROUP async_tests)

add_executable(${TESTS_GROUP}
    tests/test_thread.cpp
    tests/test_polling.cpp
    tests/test_utils.cpp
    tests/test_io.cpp
    tests/test_coro.cpp
    tests/test_net.cpp
)

target_link_libraries(${TESTS_GROUP} GTest::gtest_main ${PROJECT_NAME})
target_include_directories(${TESTS_GROUP} PRIVATE include tests)

# Discover tests
include(GoogleTest)
gtest_discover_tests(${TESTS_GROUP})
