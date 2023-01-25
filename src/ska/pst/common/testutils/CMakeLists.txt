
enable_testing()

add_library(
    ska-pst-common-testutils
    STATIC
    src/GtestMain.cpp)

target_include_directories(
    ska-pst-common-testutils
    PUBLIC
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/src>
    $<BUILD_INTERFACE:${PROJECT_BINARY_DIR}/src>
    $<INSTALL_INTERFACE:include>)

target_link_libraries(ska-pst-common-testutils GTest::gtest_main gmock spdlog::spdlog ska_pst_common-utils)

include_directories(..)
