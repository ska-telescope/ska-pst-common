
include_directories(../..)

add_executable(LmcServiceTest src/LmcServiceTest.cpp)

target_link_libraries(LmcServiceTest gtest_main ska_pst_common ska-pst-common-testutils)

add_test(LmcServiceTest LmcServiceTest)
