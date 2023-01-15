
include_directories(../..)

add_executable(AsciiHeaderTest src/AsciiHeaderTest.cpp)
add_executable(TimeTest src/TimeTest.cpp)
add_executable(TimerTest src/TimerTest.cpp)
add_executable(RandomSequenceTest src/RandomSequenceTest.cpp)

target_link_libraries(AsciiHeaderTest gtest_main ska_pst_common-utils ska-pst-common-testutils)
target_link_libraries(TimeTest gtest_main ska_pst_common-utils ska-pst-common-testutils)
target_link_libraries(TimerTest gtest_main ska_pst_common-utils ska-pst-common-testutils)
target_link_libraries(RandomSequenceTest gtest_main ska_pst_common-utils ska-pst-common-testutils)

add_test(AsciiHeaderTest AsciiHeaderTest --test_data "${CMAKE_CURRENT_LIST_DIR}/data")
add_test(TimeTest TimeTest --test_data "${CMAKE_CURRENT_LIST_DIR}/data")
add_test(TimerTest TimerTest)
add_test(RandomSequenceTest RandomSequenceTest)

