project(arcus)
cmake_minimum_required(VERSION 2.8.12)

include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

option(INSTALL_PYTHON_PACKAGE "Install the Python parts of Arcus" ON)
option(BUILD_EXAMPLES "Build the example programs" ON)

set(REQUIRED_PROTOBUF_VERSION 3.0.0)
find_package(Protobuf ${REQUIRED_PROTOBUF_VERSION} REQUIRED)

if(INSTALL_PYTHON_PACKAGE)
    find_package(PythonInterp 3.0.0 REQUIRED)
    set(PYTHON_SITE_PACKAGES_DIR ${CMAKE_INSTALL_LIBDIR}/python${PYTHON_VERSION_MAJOR}.${PYTHON_VERSION_MINOR}/dist-packages CACHE PATH "Install location of Python package")
endif()

if(NOT ${CMAKE_VERSION} VERSION_LESS 3.1)
    set(CMAKE_CXX_STANDARD 11)
else()
    set(CMAKE_CXX_FLAGS "-std=c++11")
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

set(ARCUS_VERSION 0.1.0)
set(ARCUS_SOVERSION 0)

add_library(Arcus SHARED ${arcus_SRCS})
target_include_directories(Arcus PUBLIC
    ${PROTOBUF_INCLUDE_DIR}
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
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

add_subdirectory(examples)

install(TARGETS Arcus
    EXPORT Arcus-targets
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/Arcus
)

install(EXPORT Arcus-targets
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/Arcus
)

configure_package_config_file(ArcusConfig.cmake.in ${CMAKE_BINARY_DIR}/ArcusConfig.cmake INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/Arcus)
write_basic_package_version_file(${CMAKE_BINARY_DIR}/ArcusConfigVersion.cmake VERSION ${ARCUS_VERSION} COMPATIBILITY SameMajorVersion)

install(FILES
    ${CMAKE_BINARY_DIR}/ArcusConfig.cmake
    ${CMAKE_BINARY_DIR}/ArcusConfigVersion.cmake
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/Arcus
)

if(INSTALL_PYTHON_PACKAGE)
    install(FILES python/Arcus/__init__.py DESTINATION ${PYTHON_SITE_PACKAGES_DIR}/Arcus)
endif()

include(CPackConfig.cmake)
