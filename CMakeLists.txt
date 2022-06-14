cmake_policy(SET CMP0091 NEW)
project(arcus)
cmake_minimum_required(VERSION 3.20)
include(cmake/StandardProjectSettings.cmake)

option(BUILD_PYTHON "Build Python bindings for this library" ON)

find_package(Protobuf 3.17.1 REQUIRED)

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

target_include_directories(Arcus
        PUBLIC
            $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/arcus_include/>
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

if(${CMAKE_BUILD_TYPE} STREQUAL "Debug" OR ${CMAKE_BUILD_TYPE} STREQUAL "RelWithDebInfo")
    target_compile_definitions(Arcus PRIVATE -DARCUS_DEBUG)
endif()

# Create the Python bindings
if(BUILD_PYTHON)
    list(APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake)
    if(DEFINED ENV{Python_ROOT_DIR})
        set(Python_ROOT_DIR $ENV{Python_ROOT_DIR})
    endif()
    if(NOT DEFINED Python_VERSION)
        set(Python_VERSION
                3.10
                CACHE STRING "Python Version" FORCE)
        message(STATUS "Setting Python version to ${Python_VERSION}. Set Python_VERSION if you want to compile against an other version.")
    endif()
    if(APPLE)
        set(Python_FIND_FRAMEWORK NEVER)
    endif()
    find_package(Python ${Python_VERSION} EXACT REQUIRED COMPONENTS Interpreter Development)
    message(STATUS "Linking and building ${project_name} against Python ${Python_VERSION}")

    find_package(SIP REQUIRED 6.5.0)

    add_library(pyArcus INTERFACE ${CMAKE_SOURCE_DIR}/python/PythonMessage.cpp)
    use_threads(pyArcus)

    target_include_directories(pyArcus
            INTERFACE
                $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
                $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/pyarcus_include/>
            )

    target_link_libraries(pyArcus INTERFACE Arcus protobuf::libprotobuf Python::Python)

    add_sip_module(pyArcus)
    if (DEFINED Python_SITELIB_LOCAL)
        install_sip_module(pyArcus ${Python_SITELIB_LOCAL})
    else()
        install_sip_module(pyArcus)
    endif ()
endif()