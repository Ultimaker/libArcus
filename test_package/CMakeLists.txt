project(PackageTest)
cmake_minimum_required(VERSION 3.13)

find_package(Arcus REQUIRED)
find_package(Threads)
find_package(Protobuf 3.9.2 REQUIRED)

protobuf_generate_cpp(test_PB_SRC test_PB_HDR "test.proto")

add_executable(test test.cpp ${test_PB_SRC} ${test_PB_HDR})

target_link_libraries(test
        PRIVATE
            Arcus::Arcus
            protobuf::libprotobuf
            Threads::Threads
        )

target_include_directories(test PRIVATE ${CMAKE_CURRENT_BINARY_DIR})