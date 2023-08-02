
include_directories(
  ..
  ${SkaPstCommon_INCLUDE_DIR}
  $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/src>
  $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/proto>
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

