cmake_policy(SET CMP0091 NEW)
cmake_minimum_required(VERSION 3.20)
project(PackageTest CXX)

find_package(libarcus REQUIRED)
find_package(protobuf 3.17.1 REQUIRED)
protobuf_generate_cpp(generated_PROTOBUF_SOURCES generated_PROTOBUF_HEADERS test.proto)

add_executable(test "src/test.cpp" ${generated_PROTOBUF_SOURCES})
target_link_libraries(test PUBLIC libarcus::libarcus protobuf::libprotobuf)
target_include_directories(test PUBLIC ${CMAKE_CURRENT_BINARY_DIR} "include")

set_target_properties(test PROPERTIES PUBLIC_HEADER "include/test.h")
