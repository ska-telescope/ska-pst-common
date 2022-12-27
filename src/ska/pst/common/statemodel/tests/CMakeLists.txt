
include_directories(../..)

add_executable(StateModelTest src/StateModelTest.cpp)

set(TEST_LINK_LIBS ${SkaPstCommon_LIBRARIES} ${PSRDADA_LIBRARIES} ska-pst-common-statemodel ska-pst-common-testutils)

target_link_libraries(StateModelTest gtest_main ${TEST_LINK_LIBS})

add_test(StateModelTest StateModelTest --test_data "${CMAKE_CURRENT_LIST_DIR}/data")
