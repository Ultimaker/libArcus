project(arcus)
cmake_minimum_required(VERSION 2.8.12)

include(CMakePackageConfigHelpers)

option(INSTALL_PYTHON_PACKAGE "Install the Python parts of Arcus" ON)
option(BUILD_EXAMPLES "Build the example programs" ON)
option(BUILD_SHARED_LIBS "Build with shared libraries" ON)

set(REQUIRED_PROTOBUF_VERSION 3.0.0)
find_package(Protobuf ${REQUIRED_PROTOBUF_VERSION} REQUIRED)

if(INSTALL_PYTHON_PACKAGE)
    find_package(PythonInterp 3.4.0 REQUIRED)
endif()

if(NOT ${CMAKE_VERSION} VERSION_LESS 3.1)
    set(CMAKE_CXX_STANDARD 11)
else()
    set(CMAKE_CXX_FLAGS "-std=c++11")
endif()

if(APPLE AND CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++")
endif()

set(arcus_SRCS
    src/Socket.cpp
    src/SocketListener.cpp
)

set(arcus_HDRS
    src/Socket.h
    src/SocketListener.h
    src/Types.h
    src/ArcusExport.h
)

set(ARCUS_VERSION 15.09.81)
set(ARCUS_SOVERSION 1)

add_library(Arcus ${arcus_SRCS})
target_link_libraries(Arcus PRIVATE pthread)

set(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_FULL_LIBDIR}")

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

if(INSTALL_PYTHON_PACKAGE)
    if(APPLE OR WIN32)
        install(FILES python/Arcus/__init__.py DESTINATION lib/python${PYTHON_VERSION_MAJOR}.${PYTHON_VERSION_MINOR}/site-packages/Arcus)
    else()
        install(FILES python/Arcus/__init__.py DESTINATION lib/python${PYTHON_VERSION_MAJOR}/dist-packages/Arcus)
    endif()
endif()

include(CPackConfig.cmake)
