
include_directories(../..)

add_executable(versionTest src/versionTest.cpp)

target_link_libraries(versionTest gtest_main ska_pst_common ska-pst-common-testutils)

add_test(versionTest versionTest --test_data "${PROJECT_SOURCE_DIR}")
