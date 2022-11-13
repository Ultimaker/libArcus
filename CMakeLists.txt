cmake_policy(SET CMP0091 NEW)
project(libarcus)
cmake_minimum_required(VERSION 3.20)
find_package(standardprojectsettings REQUIRED)

find_package(protobuf 3.17.1 REQUIRED)

set(arcus_SRCS
    src/Socket.cpp
    src/SocketListener.cpp
    src/MessageTypeStore.cpp
    src/PlatformSocket.cpp
    src/Error.cpp
)

if(BUILD_SHARED_LIBS)
    add_library(Arcus SHARED ${arcus_SRCS})
else()
    add_library(Arcus STATIC ${arcus_SRCS})
endif()

use_threads(Arcus)

target_include_directories(Arcus
        PUBLIC
            $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include/>
            $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
        PRIVATE
            $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>
)
target_link_libraries(Arcus PUBLIC protobuf::libprotobuf)

if(WIN32)
    target_compile_definitions(Arcus PRIVATE -D_WIN32_WINNT=0x0600)
    # Declare we require Vista or higher, this allows us to use IPv6 functions.

    target_link_libraries(Arcus PUBLIC Ws2_32)
    set_target_properties(Arcus PROPERTIES WINDOWS_EXPORT_ALL_SYMBOLS ON)
endif()
