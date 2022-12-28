
set(public_headers
    StateModel.h
)

set(sources
    src/StateModel.cpp
)

set(private_headers
)

set(public_headers
    StateModel.h
    ApplicationManager.h
)

add_library(
    ska-pst-common-statemodel
    OBJECT
    ${sources}
    ${private_headers}
    ${public_headers}
)

target_include_directories(
    ska-pst-common-statemodel
    PUBLIC
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/src>
    $<BUILD_INTERFACE:${PROJECT_BINARY_DIR}/src>
    $<INSTALL_INTERFACE:include>
)

target_link_libraries(
    ska-pst-common-statemodel
    PUBLIC
    spdlog
)

if (BUILD_TESTING)
  add_subdirectory(tests)
endif()

install(
    FILES
        ${public_headers}
    DESTINATION
        include/ska/pst/common
)
