
include_directories(../..)

add_executable(LmcServiceTest src/LmcServiceTest.cpp)

target_link_libraries(LmcServiceTest gtest_main ska-pst-common-lmc ska-pst-common-testutils)

add_test(LmcServiceTest LmcServiceTest)
