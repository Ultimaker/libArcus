project(arcus)
cmake_minimum_required(VERSION 3.0)

include(GNUInstallDirs)

option(INSTALL_PYTHON_PACKAGE "Install the Python parts of Arcus" ON)
option(BUILD_EXAMPLES "Build the example programs" ON)

find_package(Protobuf 3.0.0 REQUIRED)

if(INSTALL_PYTHON_PACKAGE)
    find_package(PythonInterp 3.0.0 REQUIRED)
    set(PYTHON_SITE_PACKAGES_DIR ${CMAKE_INSTALL_LIBDIR}/python${PYTHON_VERSION_MAJOR}.${PYTHON_VERSION_MINOR}/site-packages CACHE PATH "Install location of Python package")
endif()

set(arcus_SRCS
    src/Socket.cpp
    src/SocketListener.cpp
)

set(arcus_HDRS
    src/Socket.h
    src/SocketListener.h
    src/Types.h
)

set(ARCUS_SOVERSION 0)

add_library(Arcus SHARED ${arcus_SRCS})
target_include_directories(Arcus PUBLIC
    ${PROTOBUF_INCLUDE_DIR}
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/Arcus>
)
target_link_libraries(Arcus PUBLIC
    ${PROTOBUF_LIBRARY}
)
set_target_properties(Arcus PROPERTIES
    FRAMEWORK TRUE
    SOVERSION ${ARCUS_SOVERSION}
    COMPILE_FLAGS "-std=c++11"
    PUBLIC_HEADER "${arcus_HDRS}"
)

add_subdirectory(examples)

install(TARGETS Arcus
    EXPORT Arcus
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/Arcus
)

install(EXPORT Arcus
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake
)

if(INSTALL_PYTHON_PACKAGE)
    install(DIRECTORY python/Arcus DESTINATION ${PYTHON_SITE_PACKAGES_DIR})
endif()
