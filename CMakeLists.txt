cmake_policy(SET CMP0091 NEW)
project(libarcus)
cmake_minimum_required(VERSION 3.20)
include(cmake/StandardProjectSettings.cmake)

find_package(protobuf 3.17.1 REQUIRED)

set(arcus_SRCS
    src/Socket.cpp
    src/SocketListener.cpp
    src/MessageTypeStore.cpp
    src/PlatformSocket.cpp
    src/Error.cpp
)

if(BUILD_SHARED_LIBS)
    add_library(libArcus SHARED ${arcus_SRCS})
else()
    add_library(libArcus STATIC ${arcus_SRCS})
endif()

target_include_directories(libArcus
        PUBLIC
            $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include/>
            $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
        PRIVATE
            $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>
)
target_link_libraries(libArcus PUBLIC protobuf::libprotobuf)

if(WIN32)
    target_compile_definitions(libArcus PRIVATE -D_WIN32_WINNT=0x0600)
    # Declare we require Vista or higher, this allows us to use IPv6 functions.

    target_link_libraries(libArcus PUBLIC Ws2_32)
    set_target_properties(libArcus PROPERTIES WINDOWS_EXPORT_ALL_SYMBOLS ON)
endif()

if(${CMAKE_BUILD_TYPE} STREQUAL "Debug" OR ${CMAKE_BUILD_TYPE} STREQUAL "RelWithDebInfo")
    target_compile_definitions(libArcus PRIVATE -DARCUS_DEBUG)
endif()
