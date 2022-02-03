project(arcus)
cmake_minimum_required(VERSION 3.20)

include(CMakePackageConfigHelpers)
include(GenerateExportHeader)

option(BUILD_PYTHON "Should Python bindings be created" ON)
option(BUILD_EXAMPLES "Build the example programs" OFF)
option(BUILD_STATIC "Build as a static library" OFF)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake)
include(cmake/StandardProjectSettings.cmake)

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
    src/MessageTypeStore.h
    src/Error.h
    python/PythonMessage.h
    ${CMAKE_CURRENT_BINARY_DIR}/src/ArcusExport.h
)

set(ARCUS_VERSION 1.1.0)
set(ARCUS_SOVERSION 3)

if(${BUILD_SHARED_LIBS})
    add_library(Arcus SHARED ${arcus_SRCS})
else()
    add_library(Arcus STATIC ${arcus_SRCS})
endif()

use_threads(Arcus)

find_package(Protobuf 3.17.1 REQUIRED)
target_link_libraries(Arcus PRIVATE protobuf::libprotobuf)

set_project_standards(Arcus)
set_project_warnings(Arcus)
enable_sanitizers(Arcus)

if(NOT MSVC)
    if(${CMAKE_BUILD_TYPE} STREQUAL "Debug" OR ${CMAKE_BUILD_TYPE} STREQUAL "RelWithDebInfo")
        target_compile_definitions(Arcus PUBLIC -DARCUS_DEBUG)
    endif()
endif()

if(WIN32)
    # Declare we require Vista or higher, this allows us to use IPv6 functions.
    target_compile_definitions(Arcus PUBLIC -D_WIN32_WINNT=0x0600)
    target_link_libraries(Arcus PUBLIC Ws2_32)
endif()

generate_export_header(Arcus EXPORT_FILE_NAME src/ArcusExport.h)
target_include_directories(Arcus
        PUBLIC
            $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/>
        PRIVATE
            $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include/>
            $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/src/>
            $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/src/>
        )

set_target_properties(Arcus PROPERTIES
    FRAMEWORK FALSE
    VERSION ${ARCUS_VERSION}
    SOVERSION ${ARCUS_SOVERSION}
    PUBLIC_HEADER "${arcus_HDRS}"
    DEFINE_SYMBOL MAKE_ARCUS_LIB
    CXX_VISIBILITY_PRESET hidden
    VISIBILITY_INLINES_HIDDEN 1
)

if(BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()

if(BUILD_PYTHON)
    if(NOT TARGET arcus::arcus)  # Make Arcus appear as an external namespaced project (lower case is Conan best practices)
        add_library(arcus::arcus ALIAS Arcus)
    endif()

    if(NOT DEFINED Python_VERSION)
        set(Python_VERSION
                3.10
                CACHE STRING "Python Version" FORCE)
        message(STATUS "Setting Python version to ${Python_VERSION}. Set Python_VERSION if you want to compile against an other version.")
    endif()
    find_package(Python ${Python_VERSION} EXACT REQUIRED COMPONENTS Interpreter Development)
    message(STATUS "Linking and building ${project_name} against Python ${Python_VERSION}")

    find_package(SIP REQUIRED 6.5.0)
    include(cmake/sip.cmake)

    add_library(pyArcus INTERFACE)  # Create an interface target which will be our Python Arcus

    target_link_libraries(pyArcus
            INTERFACE
                Python::Python
                arcus::arcus
            )  # Link the external (from the Python-side) libraries to our Python module

    # Work around for not finding libs and lib location of target_link_libraries
    set(Arcus_LIBRARY Arcus)
    set(Arcus_LIBRARY_DIRS ${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR}/)

    target_include_directories(pyArcus
            INTERFACE
                $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/python/>
                $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/src/>
            )  # Add the included directories to our Python module

    set(SIP_MODULE "${CMAKE_SOURCE_DIR}/python/Arcus.sip")  # Specify the main sip module
    set(EXTRA_SIP_FILES
            "${CMAKE_SOURCE_DIR}/python/Error.sip"
            "${CMAKE_SOURCE_DIR}/python/PythonMessage.sip"
            "${CMAKE_SOURCE_DIR}/python/SocketListener.sip"
            "${CMAKE_SOURCE_DIR}/python/Types.sip"
            )  # Specify the extra sip definition files
    set(HDR_FILES "PythonMessage.h")  # Specify the extra C++ header to be used
    set(SRC_FILES "${CMAKE_SOURCE_DIR}/python/PythonMessage.cpp")  # Specify the extra C++ source file

    target_compile_features(pyArcus INTERFACE cxx_std_17)  # Set extra compile features (not yet implemented in sip.cmake)
    set_project_warnings(pyArcus)

    add_sip_module(pyArcus ${SIP_MODULE} "${EXTRA_SIP_FILES}" "${HDR_FILES}" "${SRC_FILES}")
endif()

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

include(CPackConfig.cmake)
