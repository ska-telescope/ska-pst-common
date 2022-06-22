enable_testing()

find_package(Protobuf REQUIRED)
find_package(gRPC REQUIRED)
include_directories(${Protobuf_INCLUDE_DIRS})
include_directories(${gRPC_INCLUDE_DIRS})

include_directories(${CMAKE_CURRENT_BINARY_DIR})
include_directories(${CMAKE_SOURCE_DIR})

set(PROTO_FILES
    helloworld.proto
)

add_library(test_proto_lib ${PROTO_FILES} ${GRPC_FILES})
target_link_libraries(test_proto_lib
    PUBLIC
        protobuf::libprotobuf
        gRPC::grpc
        gRPC::grpc++
)
target_include_directories(test_proto_lib PUBLIC ${CMAKE_CURRENT_BINARY_DIR})

get_target_property(protoc_location protobuf::protoc LOCATION)
get_target_property(grpc_cpp_plugin_location gRPC::grpc_cpp_plugin LOCATION)

message("protoc is at ${protoc_location}") #for sanity check
message("grpc plugin is at ${grpc_cpp_plugin_location}") #for sanity check

protobuf_generate(TARGET test_proto_lib LANGUAGE cpp)
protobuf_generate(TARGET test_proto_lib LANGUAGE grpc GENERATE_EXTENSIONS .grpc.pb.h .grpc.pb.cc PLUGIN "protoc-gen-grpc=${grpc_cpp_plugin_location}")

add_executable(test_proto
    test_proto.cpp
)
target_link_libraries(test_proto
    test_proto_lib
)

add_test(NAME test_proto COMMAND test_proto)
