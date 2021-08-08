project(arcus)
cmake_minimum_required(VERSION 3.13)

# Basic projects options
option(BUILD_PYTHON "Build Python module" ON)
option(BUILD_EXAMPLES "Build the example programs" ON)
option(BUILD_STATIC "Build as a static library" OFF)

include(cmake/StandardProjectSettings.cmake)
include(CMakePackageConfigHelpers)
include(GenerateExportHeader)

set(ARCUS_VERSION 1.1.0)
set(ARCUS_SOVERSION 3)

set(arcus_SRCS
    src/Socket.cpp
    src/SocketListener.cpp
    src/MessageTypeStore.cpp
    src/PlatformSocket.cpp
    src/Error.cpp)

set(arcus_HDRS
    src/Socket.h
    src/SocketListener.h
    src/Types.h
    src/MessageTypeStore.h
    src/Error.h
    src/ArcusExport.h
)

if(${BUILD_SHARED_LIBS})
    add_library(Arcus SHARED ${arcus_SRCS})
else()
    add_library(Arcus STATIC ${arcus_SRCS})
endif()

use_threads(Arcus)

find_package(Protobuf 3.9.2 REQUIRED)
target_link_libraries(Arcus PUBLIC protobuf::libprotobuf)

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

generate_export_header(Arcus EXPORT_FILE_NAME ${CMAKE_SOURCE_DIR}/src/ArcusExport.h)
target_include_directories(Arcus
        PUBLIC
            $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>
            $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
        )

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
    CXX_VISIBILITY_PRESET hidden
    VISIBILITY_INLINES_HIDDEN 1
)

if(BUILD_PYTHON)
    list(APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake)

    find_package(SIP REQUIRED)
    if(NOT DEFINED LIB_SUFFIX)
        set(LIB_SUFFIX "")
    endif()

    set(SIP_EXTRA_FILES_DEPEND python/SocketListener.sip python/Types.sip python/PythonMessage.sip python/Error.sip)
    set(SIP_EXTRA_SOURCE_FILES python/PythonMessage.cpp)
    set(SIP_EXTRA_OPTIONS -y Arcus.pyi -o -g -n PyQt5.sip) # -g means always release the GIL before calling C++ methods. -n PyQt5.sip is required to not get the PyCapsule error
    add_sip_python_module(Arcus python/Socket.sip Arcus)
    use_python(python_module_Arcus Interpreter Development)

    target_link_libraries(python_module_Arcus
            PRIVATE
                SIP::SIP
                protobuf::libprotobuf
                Arcus
            )
    target_include_directories(python_module_Arcus
            PRIVATE
                $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>
                $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/python>
            )
    set_project_standards(python_module_Arcus)
    set_project_warnings(python_module_Arcus)
endif()

if(BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()

install(TARGETS Arcus
    EXPORT Arcus-targets
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/Arcus
)

install(EXPORT Arcus-targets DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/Arcus)

configure_package_config_file(ArcusConfig.cmake.in ${CMAKE_BINARY_DIR}/ArcusConfig.cmake INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/Arcus)
write_basic_package_version_file(${CMAKE_BINARY_DIR}/ArcusConfigVersion.cmake VERSION ${ARCUS_VERSION} COMPATIBILITY SameMajorVersion)

install(FILES
    ${CMAKE_BINARY_DIR}/ArcusConfig.cmake
    ${CMAKE_BINARY_DIR}/ArcusConfigVersion.cmake
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/Arcus
)

include(CPackConfig.cmake)
