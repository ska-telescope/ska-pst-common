
configure_file(config.h.in config.h)

add_subdirectory(lmc)
add_subdirectory(utils)
add_subdirectory(statemodel)

set(public_headers
  version.h
  definitions.h
  ${CMAKE_CURRENT_BINARY_DIR}/config.h
)

set(sources
  src/version.cpp
)

set(private_headers
)

if (BUILD_TESTING)
  add_subdirectory(testutils)
  add_subdirectory(tests)
endif()

add_library(ska_pst_common
    SHARED
    ${sources} ${optional_sources}
    ${private_headers} ${optional_private_headers}
    ${public_headers}
    $<TARGET_OBJECTS:ska_pst_common-utils>
    $<TARGET_OBJECTS:ska_pst_common-statemodel>
)

target_include_directories(
  ska_pst_common
  PUBLIC
  $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/src>
  $<BUILD_INTERFACE:${PROJECT_BINARY_DIR}/src>
  $<INSTALL_INTERFACE:include>
  ${PSRDADA_INCLUDE_DIR}
)

target_link_libraries(
  ska_pst_common
  PUBLIC
  spdlog::spdlog
  ${PSRDADA_LIBRARIES}
)

install(
  TARGETS
    ska_pst_common
  EXPORT
    ska_pst_common-targets
  LIBRARY DESTINATION
    lib
  ARCHIVE DESTINATION
    lib
)

install(
  FILES
    ${public_headers}
  DESTINATION
    include/ska/pst/common
)

set(export_dest_dir "${CMAKE_INSTALL_PREFIX}/cmake/ska-pst-common")

include(CMakePackageConfigHelpers)
configure_package_config_file(
  SkaPstCommonConfig.cmake.in
  "${CMAKE_CURRENT_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/SkaPstCommonConfig.cmake"
  INSTALL_DESTINATION
    share/ska-pst-common/cmake
)
write_basic_package_version_file(
  "${CMAKE_CURRENT_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/SkaPstCommonVersion.cmake"
  COMPATIBILITY
    AnyNewerVersion
)

install(
  FILES
    ${CMAKE_CURRENT_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/SkaPstCommonConfig.cmake
    ${CMAKE_CURRENT_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/SkaPstCommonVersion.cmake
  DESTINATION
    ${export_dest_dir}
)
