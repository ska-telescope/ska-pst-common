
include_directories(../..)

add_executable(StateModelTest src/StateModelTest.cpp)
add_executable(ApplicationManagerTest src/ApplicationManagerTest.cpp)

# AsciiHeader is needed from ska_pst_common-utils
set(TEST_LINK_LIBS ${SkaPstCommon_LIBRARIES} ${PSRDADA_LIBRARIES} ska_pst_common-statemodel ska_pst_common-utils ska-pst-common-testutils)

target_link_libraries(StateModelTest gtest_main ${TEST_LINK_LIBS})
target_link_libraries(ApplicationManagerTest gtest_main ${TEST_LINK_LIBS})

add_test(StateModelTest StateModelTest --test_data "${CMAKE_CURRENT_LIST_DIR}/data")
add_test(ApplicationManagerTest ApplicationManagerTest --test_data "${CMAKE_CURRENT_LIST_DIR}/data")
