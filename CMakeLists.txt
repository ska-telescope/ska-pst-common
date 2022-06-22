project(ska-pst-common)

cmake_minimum_required(VERSION 3.5)

include(CTest)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

if (BUILD_TESTING)
  add_subdirectory(tests/protobuf)
endif()
