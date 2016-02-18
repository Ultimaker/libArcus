
include_directories(${CMAKE_SOURCE_DIR}/src)

set(testserver_SRCS
    main.cpp
    TestServer.cpp
)

add_executable(TestServer ${testserver_SRCS})
target_link_libraries(TestServer Arcus)
