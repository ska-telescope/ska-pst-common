#include <iostream>
#include "helloworld.pb.h"

using test::proto::HelloWorldRequest;
using test::proto::HelloWorldResponse;

int main(int argc, char** argv) {
    HelloWorldRequest request;
    request.set_name("World");

    std::cout << request.DebugString() << std::endl;

    HelloWorldResponse response;
    response.set_text("Hello World!");

    std::cout << response.DebugString() << std::endl;
}
