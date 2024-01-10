cmake_minimum_required(VERSION 3.28)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

project(async CXX)

add_compile_options(-g -Wall -Werror -pedantic)

add_library(${PROJECT_NAME})

target_sources(${PROJECT_NAME}
PUBLIC FILE_SET modules TYPE CXX_MODULES FILES
  async.cpp

  src/sync/waitgroup.cpp
  src/sync/queue.cpp
  src/sync/threadpool.cpp

  src/io/ioobject.cpp
  src/io/pipe.cpp
)

include(CTest)
if (BUILD_TESTING)
  add_subdirectory(tests)
endif()
