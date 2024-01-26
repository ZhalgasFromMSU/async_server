cmake_minimum_required(VERSION 3.28)

FetchContent_Declare(
  expected
  GIT_REPOSITORY https://github.com/TartanLlama/expected.git
  GIT_TAG 292eff8bd8ee230a7df1d6a1c00c4ea0eb2f0362 # release-1.1.0
)

FetchContent_MakeAvailable(expected)

set_target_properties(tl-expected-tests PROPERTIES EXCLUDE_FROM_ALL ON)
target_link_libraries(${CMAKE_PROJECT_NAME} expected)
