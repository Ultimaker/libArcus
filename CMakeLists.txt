project(arcus)
cmake_minimum_required(VERSION 2.8.12)

include(CMakePackageConfigHelpers)

option(BUILD_PYTHON "Build " ON)
option(BUILD_EXAMPLES "Build the example programs" ON)
option(BUILD_STATIC "Build as a static library" OFF)

option(MSVC_STATIC_RUNTIME "Link the MSVC runtime statically" OFF)

set(REQUIRED_PROTOBUF_VERSION 3.0.0)
find_package(Protobuf ${REQUIRED_PROTOBUF_VERSION} REQUIRED)

if(BUILD_PYTHON)
    set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake)
    if(APPLE OR WIN32)
        set(PYTHON_SITE_PACKAGES_DIR lib/python3.4/site-packages CACHE STRING "Directory to install Python bindings to")
    else()
        set(PYTHON_SITE_PACKAGES_DIR lib/python3/dist-packages CACHE STRING "Directory to install Python bindings to")
    endif()
    find_package(SIP REQUIRED)
    include_directories(python/ src/ ${PYTHON_INCLUDE_DIR} ${SIP_INCLUDE_DIR})
endif()

if(NOT ${CMAKE_VERSION} VERSION_LESS 3.1)
    set(CMAKE_CXX_STANDARD 11)
else()
    set(CMAKE_CXX_FLAGS "-std=c++11")
endif()

if(APPLE AND CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++")
endif()

if(NOT APPLE AND NOT WIN32)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -static-libstdc++")
endif()

set(arcus_SRCS
    src/Socket.cpp
    src/SocketListener.cpp
    src/MessageTypeStore.cpp
    src/PlatformSocket.cpp
    src/Error.cpp
)

set(arcus_HDRS
    src/Socket.h
    src/SocketListener.h
    src/Types.h
    src/ArcusExport.h
    src/MessageTypeStore.h
    src/Error.h
)

set(ARCUS_VERSION 1.0.0)
set(ARCUS_SOVERSION 2)

set(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib")

if(BUILD_STATIC)
    add_library(Arcus STATIC ${arcus_SRCS})
    if(NOT WIN32 OR CMAKE_COMPILER_IS_GNUCXX)
        target_link_libraries(Arcus PRIVATE pthread)
        set_target_properties(Arcus PROPERTIES COMPILE_FLAGS -fPIC)
    endif()
else()
    add_library(Arcus SHARED ${arcus_SRCS})
endif()

if(MSVC_STATIC_RUNTIME)
    foreach(flag_var
            CMAKE_CXX_FLAGS CMAKE_CXX_FLAGS_DEBUG CMAKE_CXX_FLAGS_RELEASE
            CMAKE_CXX_FLAGS_MINSIZEREL CMAKE_CXX_FLAGS_RELWITHDEBINFO)
    if(${flag_var} MATCHES "/MD")
        string(REGEX REPLACE "/MD" "/MT" ${flag_var} "${${flag_var}}")
    endif(${flag_var} MATCHES "/MD")
    endforeach(flag_var)
endif()

if(BUILD_PYTHON)
    set(SIP_EXTRA_FILES_DEPEND python/SocketListener.sip python/Types.sip python/PythonMessage.sip python/Error.sip)
    set(SIP_EXTRA_SOURCE_FILES python/PythonMessage.cpp)
    set(SIP_EXTRA_OPTIONS -g) # -g means always release the GIL before calling C++ methods.
    add_sip_python_module(Arcus python/Socket.sip Arcus)
endif()

target_include_directories(Arcus PUBLIC
    ${PROTOBUF_INCLUDE_DIR}
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>
    $<INSTALL_INTERFACE:include>
)
target_link_libraries(Arcus PUBLIC
    ${PROTOBUF_LIBRARY}
)

if(WIN32)
    add_definitions(-D_WIN32_WINNT=0x0600) # Declare we require Vista or higher, this allows us to use IPv6 functions.
    target_link_libraries(Arcus PUBLIC Ws2_32)
endif()

if(${CMAKE_BUILD_TYPE})
    if(${CMAKE_BUILD_TYPE} STREQUAL "Debug" OR ${CMAKE_BUILD_TYPE} STREQUAL "RelWithDebInfo")
        add_definitions(-DARCUS_DEBUG)
    endif()
endif()

set_target_properties(Arcus PROPERTIES
    FRAMEWORK FALSE
    VERSION ${ARCUS_VERSION}
    SOVERSION ${ARCUS_SOVERSION}
    PUBLIC_HEADER "${arcus_HDRS}"
    DEFINE_SYMBOL MAKE_ARCUS_LIB
)

if(BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()

install(TARGETS Arcus
    EXPORT Arcus-targets
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    PUBLIC_HEADER DESTINATION include/Arcus
)

install(EXPORT Arcus-targets
    DESTINATION lib/cmake/Arcus
)

configure_package_config_file(ArcusConfig.cmake.in ${CMAKE_BINARY_DIR}/ArcusConfig.cmake INSTALL_DESTINATION lib/cmake/Arcus)
write_basic_package_version_file(${CMAKE_BINARY_DIR}/ArcusConfigVersion.cmake VERSION ${ARCUS_VERSION} COMPATIBILITY SameMajorVersion)

install(FILES
    ${CMAKE_BINARY_DIR}/ArcusConfig.cmake
    ${CMAKE_BINARY_DIR}/ArcusConfigVersion.cmake
    DESTINATION lib/cmake/Arcus
)

include(CPackConfig.cmake)
