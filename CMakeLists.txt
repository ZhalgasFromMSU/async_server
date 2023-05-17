cmake_minimum_required(VERSION 3.24)
project(Async)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_EXPORT_COMPILE_COMMANDS 1)

add_library(${PROJECT_NAME})

target_sources(${PROJECT_NAME}
PUBLIC
    src/thread/wait_group.cpp
    src/thread/pool.cpp
    src/polling/epoll.cpp
    src/io/io_object.cpp
    src/io/well_known_structs.cpp
    src/net/address/ip.cpp
    src/net/socket.cpp
    src/util/result.cpp
    src/coro/coro.cpp
    src/http/message.cpp
)

target_include_directories(${PROJECT_NAME} PUBLIC include)

include(FetchContent)

# Dependencies for Dependencies
FetchContent_Declare(
    Boost
    URL https://boostorg.jfrog.io/artifactory/main/release/1.82.0/source/boost_1_82_0.tar.gz
    URL_HASH SHA256=66a469b6e608a51f8347236f4912e27dc5c60c60d7d53ae9bfe4683316c6f04c
    OVERRIDE_FIND_PACKAGE
)

FetchContent_MakeAvailable(Boost)

# Dependepncies
FetchContent_Declare(
    httpparser
    GIT_REPOSITORY https://github.com/nekipelov/httpparser.git
    GIT_TAG ac5e383805a1a84ede1e164b9a543b3ab92c84ff
)
FetchContent_MakeAvailable(httpparser)
target_link_libraries(${PROJECT_NAME} PRIVATE httpparser)

# Compiler options
add_compile_options(-Werror -Wall -pedantic -g)

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
    tests/test_socket.cpp
    tests/test_address.cpp
    tests/test_http.cpp
)

target_link_libraries(${TESTS_GROUP} GTest::gtest_main ${PROJECT_NAME})
target_include_directories(${TESTS_GROUP} PRIVATE include tests)

# Discover tests
include(GoogleTest)
gtest_discover_tests(${TESTS_GROUP})
