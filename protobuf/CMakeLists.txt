find_package(Protobuf REQUIRED)
find_package(gRPC REQUIRED)
include_directories(${Protobuf_INCLUDE_DIRS})
include_directories(${gRPC_INCLUDE_DIRS})

include_directories(${CMAKE_CURRENT_BINARY_DIR})
include_directories(${CMAKE_SOURCE_DIR})

set(PROTO_FILES
    ska/pst/lmc/ska_pst_lmc.proto
)

set(HDRS
    ${CMAKE_CURRENT_BINARY_DIR}/ska/pst/lmc/ska_pst_lmc.pb.h
    ${CMAKE_CURRENT_BINARY_DIR}/ska/pst/lmc/ska_pst_lmc.grpc.pb.h
)

add_library(
    ska_pst_lmc_generated
)
target_include_directories(ska_pst_lmc_generated PUBLIC ${CMAKE_CURRENT_BINARY_DIR})

get_target_property(protoc_location protobuf::protoc LOCATION)
get_target_property(grpc_cpp_plugin_location gRPC::grpc_cpp_plugin LOCATION)

message("protoc is at ${protoc_location}") #for sanity check
message("grpc plugin is at ${grpc_cpp_plugin_location}") #for sanity check

protobuf_generate(TARGET ska_pst_lmc_generated LANGUAGE cpp PROTOS ${PROTO_FILES})
protobuf_generate(TARGET ska_pst_lmc_generated LANGUAGE grpc PROTOS ${PROTO_FILES} GENERATE_EXTENSIONS .grpc.pb.h .grpc.pb.cc PLUGIN "protoc-gen-grpc=${grpc_cpp_plugin_location}")

install( DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/protobuf DESTINATION include FILES_MATCHING PATTERN "*.h")

install(
  FILES
    ${HDRS}
  DESTINATION
    include/ska/pst/lmc
)
