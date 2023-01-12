
set(public_headers
    StateModel.h
    StateModelException.h
    ApplicationManager.h
)

set(sources
    src/StateModel.cpp
    src/ApplicationManager.cpp
)

set(private_headers
)

add_library(
    ska_pst_common-statemodel
    OBJECT
    ${sources}
    ${private_headers}
    ${public_headers}
)

target_include_directories(
    ska_pst_common-statemodel
    PUBLIC
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/src>
    $<BUILD_INTERFACE:${PROJECT_BINARY_DIR}/src>
    $<INSTALL_INTERFACE:include>
)

target_link_libraries(
    ska_pst_common-statemodel
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
        include/ska/pst/common/statemodel
)
