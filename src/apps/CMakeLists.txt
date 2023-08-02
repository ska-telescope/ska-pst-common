
target_include_directories(
  ska_pst_common-utils
  PUBLIC
  $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/src>
  $<BUILD_INTERFACE:${PROJECT_BINARY_DIR}/src>
  $<INSTALL_INTERFACE:include>
)

add_executable(ska_pst_generate_file ska_pst_generate_file.cpp)

target_link_libraries(ska_pst_generate_file PUBLIC ska_pst_common)

install (
  TARGETS
    ska_pst_generate_file
  DESTINATION
    bin
)

add_test(NAME ska_pst_generate_file COMMAND ska_pst_generate_file -h)

