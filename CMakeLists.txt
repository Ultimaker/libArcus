project(arcus)
cmake_minimum_required(VERSION 3.18)

include(GNUInstallDirs)
include(CMakePackageConfigHelpers)
include(GenerateExportHeader)

option(BUILD_PYTHON "Build " ON)
option(BUILD_EXAMPLES "Build the example programs" ON)
option(BUILD_STATIC "Build as a static library" OFF)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(WIN32)
    option(MSVC_STATIC_RUNTIME "Link the MSVC runtime statically" OFF)
endif()

# We want to have access to protobuf_generate_cpp and other FindProtobuf features.
# However, if ProtobufConfig is used instead, there is a CMake option that controls
# this, which defaults to OFF. We need to force this option to ON instead.
set(protobuf_MODULE_COMPATIBLE ON CACHE INTERNAL "" FORCE)
find_package(Protobuf 3.0.0 REQUIRED)

set(CMAKE_POSITION_INDEPENDENT_CODE ON) #Required if a patch to libArcus needs to be made via templates.

set(CMAKE_CXX_STANDARD 17)

if(APPLE AND CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++")
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
    src/MessageTypeStore.h
    src/Error.h
    ${CMAKE_CURRENT_BINARY_DIR}/src/ArcusExport.h
)

set(ARCUS_VERSION 1.1.0)
set(ARCUS_SOVERSION 3)

OPTION(SET_RPATH ON)

if(SET_RPATH)
    set(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_FULL_LIBDIR}")
endif()

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

target_include_directories(Arcus PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
    ${PROTOBUF_INCLUDE_DIR}
)
target_link_libraries(Arcus PUBLIC protobuf::libprotobuf)

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
    CXX_VISIBILITY_PRESET hidden
    VISIBILITY_INLINES_HIDDEN 1
)

generate_export_header(Arcus
    EXPORT_FILE_NAME src/ArcusExport.h
)
# This is required when building out-of-tree.
# The compiler won't find the generated header otherwise.
include_directories(${CMAKE_BINARY_DIR}/src)

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

# Create the Python bindings
if(BUILD_PYTHON)
    list(APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake)

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

    target_include_directories(pyArcus
            INTERFACE
            $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
            $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/python/>
            )

    target_compile_features(pyArcus INTERFACE cxx_std_17)
    target_link_libraries(pyArcus INTERFACE Arcus protobuf::libprotobuf Python::Python)

    add_sip_module(pyArcus)
    install_sip_module(pyArcus)
endif()

include(CPackConfig.cmake)