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

add_library(ska-pst-common-lmc
    ${sources} ${optional_sources}
    ${private_headers} ${optional_private_headers}
    ${public_headers}
)

target_include_directories(
  ska-pst-common-lmc
  PUBLIC
  $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/src>
  $<BUILD_INTERFACE:${PROJECT_BINARY_DIR}/src>
  $<INSTALL_INTERFACE:include>
  ${PSRDADA_INCLUDE_DIR}
)

target_link_libraries(
  ska-pst-common-lmc
  PUBLIC
  ska_pst_lmc
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
  FILES
    ${public_headers}
  DESTINATION
    include/ska/pst/common/lmc
)
