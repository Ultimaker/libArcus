find_package(Threads)
find_package(Protobuf 3.9.2 REQUIRED)

set(example_SRCS
        example.cpp
        example_pb2.py
        )

protobuf_generate_cpp(example_PB_SRCS example_PB_HDRS "example.proto")

add_executable(example ${example_SRCS} ${example_PB_SRCS})
target_link_libraries(example
        PRIVATE
            Arcus
            protobuf::libprotobuf
            Threads::Threads
        )
target_include_directories(example PRIVATE ${CMAKE_CURRENT_BINARY_DIR})

if (WIN32 AND NOT BUILD_STATIC)
    add_custom_command(TARGET example POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${CMAKE_BINARY_DIR}/source/Arcus.dll"
            "${CMAKE_CURRENT_BINARY_DIR}")
endif()

add_custom_command(
        OUTPUT example_pb2.py
        COMMAND  ${PROTOBUF_PROTOC_EXECUTABLE}
        ARGS --python_out ${CMAKE_CURRENT_BINARY_DIR} --proto_path=${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/example.proto
        COMMENT "Running Python protocol buffer compiler on example.proto"
        VERBATIM )

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/example_py.sh ${CMAKE_CURRENT_BINARY_DIR}/example_py.sh)