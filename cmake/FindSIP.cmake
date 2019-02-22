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
# SIP_VERSION_STR - The version of SIP found as a human readable string.
#
# SIP_BINARY_PATH - Path and filename of the SIP command line executable.
#
# SIP_INCLUDE_DIR - Directory holding the SIP C++ header file.
#

# Copyright (c) 2007, Simon Edwards <simon@simonzone.com>
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

if(APPLE)
    # Workaround for broken FindPythonLibs. It will always find Python 2.7 libs on OSX
    set(CMAKE_FIND_FRAMEWORK LAST)
endif()

get_filename_component(PYTHON_BINARY_PATH ${PYTHON_EXECUTABLE} DIRECTORY)

find_program(SIP_BINARY_PATH sip
    HINTS ${CMAKE_PREFIX_PATH}/bin ${CMAKE_INSTALL_PREFIX}/bin ${PYTHON_BINARY_PATH} ${PYTHON_BINARY_PATH}/site-packages/PyQt5
)

find_path(SIP_INCLUDE_DIR sip.h
    HINTS ${CMAKE_PREFIX_PATH}/include ${CMAKE_INSTALL_PREFIX}/include ${PYTHON_INCLUDE_DIRS} ${PYTHON_BINARY_PATH}/site-packages/PyQt5
)

execute_process(
    COMMAND ${PYTHON_EXECUTABLE} -c "import sip; print(sip.SIP_VERSION_STR)"
    RESULT_VARIABLE _process_status
    OUTPUT_VARIABLE _process_output
    OUTPUT_STRIP_TRAILING_WHITESPACE
)

if(${_process_status} EQUAL 0)
    string(STRIP ${_process_output} SIP_VERSION_STR)
else()
    unset(SIP_VERSION_STR)
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(SIP REQUIRED_VARS SIP_BINARY_PATH SIP_INCLUDE_DIR SIP_VERSION_STR VERSION_VAR SIP_VERSION_STR)

if(SIP_FOUND)
    include(${CMAKE_CURRENT_LIST_DIR}/SIPMacros.cmake)
endif()

mark_as_advanced(SIP_BINARY_PATH SIP_INCLUDE_DIR SIP_VERSION_STR)

