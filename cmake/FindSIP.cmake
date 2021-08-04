# Find SIP
# ~~~~~~~~
#
# SIP website: http://www.riverbankcomputing.co.uk/sip/index.php
#
# Find the installed version of SIP. FindSIP should be called after Python
# has been found.
#
# This file defines the following variables:
#
# SIP_VERSION - SIP version.
#
# SIP_EXECUTABLE - Path to the SIP executable.
#
# SIP_INCLUDE_DIRS - The SIP include directories.
#

# Copyright (c) 2007, Simon Edwards <simon@simonzone.com>
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

if(APPLE)
    # Workaround for broken FindPythonLibs. It will always find Python 2.7 libs on OSX
    set(CMAKE_FIND_FRAMEWORK LAST)
endif()

if(NOT Python_VERSION)
    set(Python_VERSION
            3.8
            CACHE STRING "Python Version" FORCE)
    message(STATUS "Setting Python version to ${Python_VERSION}. Set Python_VERSION if you want to compile against an other version.")
endif()
find_package(Python3 ${Python_VERSION} EXACT REQUIRED COMPONENTS Interpreter Development)

execute_process(
   COMMAND ${Python3_EXECUTABLE} -c
           "import distutils.sysconfig; print(distutils.sysconfig.get_python_lib(plat_specific=False,standard_lib=False))"
   RESULT_VARIABLE _process_status
   OUTPUT_VARIABLE _process_output
   OUTPUT_STRIP_TRAILING_WHITESPACE
)
if(${_process_status} EQUAL 0)
   string(STRIP ${_process_output} Python3_SITELIB)
else()
   message(FATAL_ERROR "Failed to get Python3_SITELIB. Error: ${_process_output}")
endif()

execute_process(
   COMMAND ${Python3_EXECUTABLE} -c
           "import distutils.sysconfig; print(distutils.sysconfig.get_python_lib(plat_specific=True,standard_lib=False))"
   RESULT_VARIABLE _process_status
   OUTPUT_VARIABLE _process_output
   OUTPUT_STRIP_TRAILING_WHITESPACE
)
if(${_process_status} EQUAL 0)
   string(STRIP ${_process_output} Python3_SITEARCH)
else()
   message(FATAL_ERROR "Failed to get Python3_SITEARCH. Error: ${_process_output}")
endif()

get_filename_component(_python_binary_path ${Python3_EXECUTABLE} DIRECTORY)

find_program(SIP_EXECUTABLE sip
    HINTS ${CMAKE_PREFIX_PATH}/bin ${CMAKE_INSTALL_PATH}/bin ${_python_binary_path} ${Python3_SITELIB}/PyQt5
)

find_path(SIP_INCLUDE_DIRS sip.h
    HINTS ${CMAKE_PREFIX_PATH}/include ${CMAKE_INSTALL_PATH}/include ${Python3_INCLUDE_DIRS} ${Python3_SITELIB}/PyQt5
)

execute_process(
    COMMAND ${Python3_EXECUTABLE} -c "import sip; print(sip.SIP_VERSION_STR)"
    RESULT_VARIABLE _process_status
    OUTPUT_VARIABLE _process_output
    OUTPUT_STRIP_TRAILING_WHITESPACE
)

if(${_process_status} EQUAL 0)
    string(STRIP ${_process_output} SIP_VERSION)
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(SIP REQUIRED_VARS SIP_EXECUTABLE SIP_INCLUDE_DIRS VERSION_VAR SIP_VERSION)

if(SIP_FOUND)
    include(${CMAKE_CURRENT_LIST_DIR}/SIPMacros.cmake)
endif()

add_library(SIP::SIP INTERFACE IMPORTED)
set_property(TARGET SIP::SIP
        PROPERTY INTERFACE_INCLUDE_DIRECTORIES
        ${SIP_INCLUDE_DIRS} APPEND)

mark_as_advanced(SIP_EXECUTABLE SIP_INCLUDE_DIRS SIP_VERSION SIP::SIP)
