enable_testing()

find_package(Protobuf REQUIRED)
include_directories(${Protobuf_INCLUDE_DIRS})

include_directories(${CMAKE_CURRENT_BINARY_DIR})
include_directories(${CMAKE_SOURCE_DIR})

protobuf_generate_cpp(PROTO_SRCS PROTO_HDRS helloworld.proto)

add_executable(test_proto
    test_proto.cpp
    ${PROTO_SRCS}
    ${PROTO_HDRS}
)
target_link_libraries(test_proto
    ${Protobuf_LIBRARIES}
)

add_test(NAME test_proto COMMAND test_proto)
