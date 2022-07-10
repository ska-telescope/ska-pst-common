cmake_minimum_required(VERSION 3.5)

message(STATUS "Building ...")

# Project configuration, specifying version, languages,
# and the C++ standard to use for the whole project
set(PACKAGE_NAME SkaPstCommon)
set(DESCRIPTION "Shared memory ring buffer for the SKA Pulsar Timing instrument")
file(STRINGS version.txt SkaPstCommon_VERSION)

project(${PACKAGE_NAME} VERSION ${SkaPstCommon_VERSION} DESCRIPTION ${DESCRIPTION} LANGUAGES CXX )
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib")

if (BUILD_TESTING)
  add_subdirectory(tests/protobuf)
endif()
add_subdirectory(protobuf)
