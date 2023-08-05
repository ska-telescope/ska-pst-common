
add_executable(ska_pst_generate_file ska_pst_generate_file.cpp)
add_executable(ska_pst_common_info ska_pst_common_info.cpp)

include_directories(
  ..
  $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/src>
  $<BUILD_INTERFACE:${PROJECT_BINARY_DIR}/src>
  $<INSTALL_INTERFACE:include>
)

target_link_libraries(ska_pst_generate_file PUBLIC ska_pst_common-utils spdlog::spdlog)
target_link_libraries(ska_pst_common_info PUBLIC ska_pst_common)

install (
  TARGETS
    ska_pst_generate_file
    ska_pst_common_info
  DESTINATION
    bin
)

add_test(NAME ska_pst_generate_file COMMAND ska_pst_generate_file -h)
add_test(NAME ska_pst_common_info COMMAND ska_pst_common_info -h)
