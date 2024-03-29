cmake_minimum_required(VERSION 3.5)

file(READ ".release" ver)
string(REGEX MATCH "release=([0-9]+.[0-9]+.[0-9]+)" _ ${ver})
set(SkaPstCommon_VERSION ${CMAKE_MATCH_1})

message(STATUS "Building SkaPstCommon version ${SkaPstCommon_VERSION}")

# Project configuration, specifying version, languages,
# and the C++ standard to use for the whole project
set(PACKAGE_NAME SkaPstCommon)
set(DESCRIPTION "Common C++ code for the SKA Pulsar Timing instrument")

project(${PACKAGE_NAME} VERSION ${SkaPstCommon_VERSION} DESCRIPTION ${DESCRIPTION} LANGUAGES CXX )
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib")

set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/resources/cmake-modules ${PROJECT_SOURCE_DIR}/cmake)
include(dependencies)
include(lint)
include(CTest)

# External projects
if (BUILD_TESTING)
  find_package(GTest REQUIRED)
endif()

find_package(Protobuf)
find_package(gRPC)
find_package(spdlog)

find_dependencies()
add_subdirectory(src/ska/pst/common)
add_subdirectory(protobuf)
add_subdirectory(src/apps)

