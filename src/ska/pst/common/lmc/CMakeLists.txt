set(public_headers
  LmcService.h
  LmcServiceHandler.h
  LmcServiceException.h
)

set(sources
  src/LmcService.cpp
  src/LmcServiceException.cpp
)

set(private_headers
)

add_library(ska_pst_lmc
    SHARED
    ${sources} ${optional_sources}
    ${private_headers} ${optional_private_headers}
    ${public_headers}
    $<TARGET_OBJECTS:ska_pst_lmc_generated>
)

target_include_directories(
  ska_pst_lmc
  PUBLIC
  $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/src>
  $<BUILD_INTERFACE:${PROJECT_BINARY_DIR}/src>
  $<INSTALL_INTERFACE:include>
  ${PSRDADA_INCLUDE_DIR}
)

target_link_libraries(
  ska_pst_lmc
  PUBLIC
  ska_pst_lmc_generated
  spdlog
  protobuf::libprotobuf
  gRPC::grpc
  gRPC::grpc++
  ${PSRDADA_LIBRARIES}
)

if (BUILD_TESTING)
  add_subdirectory(tests)
endif()

install(
  TARGETS
    ska_pst_lmc
  EXPORT
    ska_pst_lmc-targets
  LIBRARY DESTINATION
    lib
  ARCHIVE DESTINATION
    lib
)

install(
  FILES
    ${public_headers}
  DESTINATION
    include/ska/pst/common/lmc
)

set(export_dest_dir "${CMAKE_INSTALL_PREFIX}/cmake/ska-pst-lmc")

# Install cmake config + version + target files
include(CMakePackageConfigHelpers)
configure_package_config_file(
  SkaPstLmcConfig.cmake.in
  "${CMAKE_CURRENT_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/SkaPstLmcConfig.cmake"
  INSTALL_DESTINATION
    share/ska-pst-common/cmake
)
write_basic_package_version_file(
  "${CMAKE_CURRENT_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/SkaPstLmcVersion.cmake"
  COMPATIBILITY
    AnyNewerVersion
)

install(
  FILES
    ${CMAKE_CURRENT_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/SkaPstLmcConfig.cmake
    ${CMAKE_CURRENT_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/SkaPstLmcVersion.cmake
  DESTINATION
    "${export_dest_dir}"
)
