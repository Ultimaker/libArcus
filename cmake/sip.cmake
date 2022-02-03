# set(SIP_BUILD_EXECUTABLE /home/peer23peer/.conan/data/sip/6.5.0/python/stable/package/919a80bca9eb6a39029fa3ff69bbb78ddf84bbb8/bin/sip-build)
set(PRE_BUILD_CMD)
#set(PRE_BUILD_CMD ". ${CMAKE_BINARY_DIR}/conan/conanbuild.sh && ")


function(add_sip_module MODULE_TARGET MODULE_SIP SIP_FILES HDR_FILES SRC_FILES)
    message(STATUS "SIP: Generating files")

    # Get the sip modules copy the files to the build and set the tobe generated path.
    # Because that files needs to be in the same folder as the pyproject.toml
    cmake_path(GET MODULE_SIP FILENAME _module_sip_filename)
    cmake_path(GET MODULE_SIP PARENT_PATH _module_sip_path)

    list(PREPEND SIP_FILES ${MODULE_SIP})

    message(STATUS "SIP: Generating pyproject.toml")
    # Get the module specified include locations
    get_target_property(_sip_inlcude_dirs ${MODULE_TARGET} INTERFACE_INCLUDE_DIRECTORIES)

    # Get the module compiler options
    get_target_property(_compile_features ${MODULE_TARGET} INTERFACE_COMPILE_FEATURES)
    get_target_property(_compile_options ${MODULE_TARGET} INTERFACE_COMPILE_OPTIONS)
    get_target_property(_compile_definitions ${MODULE_TARGET} INTERFACE_COMPILE_DEFINITIONS)

    # Get the linked dependency include locations and lib names
    get_target_property(_link_libraries ${MODULE_TARGET} INTERFACE_LINK_LIBRARIES)
    set(_sip_link_libraries Arcus)
    set(_sip_link_libraries_DIRS)
    foreach(_link_lib ${_link_libraries})

        get_target_property(_type ${_link_lib} TYPE)
        if(_type STREQUAL "INTERFACE_LIBRARY")
            set(_include_directories_prop_var "INTERFACE_INCLUDE_DIRECTORIES")
            set(_compile_features_prop_var "INTERFACE_COMPILE_FEATURES")
            set(_compile_options_prop_var "INTERFACE_COMPILE_OPTIONS")
            set(_compile_definitions_prop_var "INTERFACE_COMPILE_DEFINITIONS")
        else()
            set(_include_directories_prop_var "INCLUDE_DIRECTORIES")
            set(_compile_features_prop_var "COMPILE_FEATURES")
            set(_compile_options_prop_var "COMPILE_OPTIONS")
            set(_compile_definitions_prop_var "COMPILE_DEFINITIONS")
        endif()

        get_target_property(_link_include_dirs ${_link_lib} ${_include_directories_prop_var})
        if(_link_include_dirs)
            list(APPEND _sip_inlcude_dirs ${_link_include_dirs})
        endif()

        get_target_property(_lib_compile_features ${_link_lib} ${_compile_features_prop_var})
        if(_lib_compile_features)
            list(APPEND _compile_features "${_lib_compile_features}")
        endif()

        get_target_property(_lib_compile_options ${_link_lib} ${_compile_options_prop_var})
        if(_lib_compile_options)
            list(APPEND _compile_options "${_lib_compile_options}")
        endif()

        get_target_property(_lib_compile_definitions ${_link_lib} ${_compile_definitions_prop_var})
        if(_lib_compile_definitions)
            list(APPEND _compile_definitions "${_lib_compile_definitions}")
        endif()

        get_target_property(_lib_name ${_link_lib} NAME)
        string(REPLACE "::" ";" _x "${_lib_name}")
        list(GET _x 0 _name)

        string(TOUPPER ${CMAKE_BUILD_TYPE} _upper_case_build_type)

        set(_lib_dir_var "${_name}_LIB_DIRS_${_upper_case_build_type}")
        set(_lib_dir "${${_lib_dir_var}}")
        if(_lib_dir)
            list(APPEND _sip_link_libraries_DIRS "${_lib_dir}")
        endif()

        set(_lib_dir_var "${_name}_LIBRARY_DIRS")
        set(_lib_dir "${${_lib_dir_var}}")
        if(_lib_dir)
            list(APPEND _sip_link_libraries_DIRS "${_lib_dir}")
        endif()

        set(_lib_var "${_name}_LIBS_${_upper_case_build_type}")
        set(_lib "${${_lib_var}}")
        if(_lib)
            cmake_path(GET _lib FILENAME _lib)
            list(APPEND _sip_link_libraries "${_lib}")
        endif()

        set(_lib_var "${_name}_LIBRARY")
        set(_lib "${${_lib_var}}")
        if(_lib)
            cmake_path(GET _lib FILENAME _lib)
            list(APPEND _sip_link_libraries "${_lib}")
        endif()

        set(_lib_var "${_name}_LIBRARIES")
        set(_lib "${${_lib_var}}")
        if(_lib)
            cmake_path(GET _lib STEM LAST_ONLY _lib)
            string(REGEX REPLACE "^lib" "" _lib ${_lib})
            list(APPEND _sip_link_libraries ${_lib})
        endif()

        set(_lib_var "${_name}_SYSTEM_LIBS_${_upper_case_build_type}")
        set(_lib "${${_lib_var}}")
        if(_lib)
            cmake_path(GET _lib FILENAME _lib)
            list(APPEND _sip_link_libraries "${_lib}")
        endif()
    endforeach()

    # Sanitize the _compile_features
    list(REMOVE_DUPLICATES _compile_features)
    string(STRIP "${_compile_features}" _compile_features)
    string(REPLACE ";" "\", \"" _compile_features "${_compile_features}")

    # Sanitize the _compile_options
    list(REMOVE_DUPLICATES _compile_options)
    string(STRIP "${_compile_options}" _compile_options)
    string(REPLACE ";" "\", \"" _compile_options "${_compile_options}")

    # Sanitize the _compile_definitions
    list(REMOVE_DUPLICATES _compile_definitions)
    string(STRIP "${_compile_definitions}" _compile_definitions)
    string(REPLACE ";" "\", \"" _compile_definitions "${_compile_definitions}")

    # Sanitize the _sip_inlcude_dirs
    list(REMOVE_DUPLICATES _sip_inlcude_dirs)
    string(STRIP "${_sip_inlcude_dirs}" _sip_inlcude_dirs)
    string(REPLACE ";" "\", \"" _sip_inlcude_dirs "${_sip_inlcude_dirs}")

    # Sanitize the _sip_link_libraries
    list(REMOVE_DUPLICATES _sip_link_libraries)
    string(STRIP "${_sip_link_libraries}" _sip_link_libraries)
    string(REPLACE ";" "\", \"" _sip_link_libraries "${_sip_link_libraries}")

    # Sanitize the _sip_link_libraries_DIRS
    list(REMOVE_DUPLICATES _sip_link_libraries_DIRS)
    string(STRIP "${_sip_link_libraries_DIRS}" _sip_link_libraries_DIRS)
    string(REPLACE ";" "\", \"" _sip_link_libraries_DIRS "${_sip_link_libraries_DIRS}")

    # Sanitize the SRC_FILES
    list(REMOVE_DUPLICATES SRC_FILES)
    string(STRIP "${SRC_FILES}" SRC_FILES)
    string(REPLACE ";" "\", \"" SRC_FILES "${SRC_FILES}")

    # Sanitize the HDR_FILES
    list(REMOVE_DUPLICATES HDR_FILES)
    string(STRIP "${HDR_FILES}" HDR_FILES)
    string(REPLACE ";" "\", \"" HDR_FILES "${HDR_FILES}")

    file(GENERATE
            OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/pyproject.toml"
            CONTENT "[build-system]
requires = [\"sip >=6, <7\"]
build-backend = \"sipbuild.api\"

[tool.sip.metadata]
name = \"${MODULE_TARGET}\"

[tool.sip.project]
sip-module = \"${_module_sip_filename}\"
sip-files-dir = \"${_module_sip_path}\"
sip-include-dirs = [\"${_module_sip_path}\"]
build-dir = \"${CMAKE_CURRENT_BINARY_DIR}/${MODULE_TARGET}/\"
verbose = true

[tool.sip.bindings.${MODULE_TARGET}]
headers = [\"${HDR_FILES}\"]
include-dirs = [\"${_sip_inlcude_dirs}\"]
libraries = [\"${_sip_link_libraries}\"]
library-dirs = [\"${_sip_link_libraries_DIRS}\"]
sources = [\"${SRC_FILES}\"]
extra-compile-args = [\"${_compile_options}\"]
define-macros = []
exceptions = true
release-gil = true"
            )

    # Generating the CPP code using sip-build
    set(_sip_output_files)
    foreach(_sip_file "${SIP_FILES}")
        cmake_path(GET _sip_file STEM _sip_stem)
        list(APPEND _sip_output_files "${CMAKE_CURRENT_BINARY_DIR}/${MODULE_TARGET}/sip${MODULE_TARGET}${_sip_stem}.cpp")
    endforeach ()

    set(SIPCMD ${PRE_BUILD_CMD} cd ${CMAKE_CURRENT_BINARY_DIR} && ${SIP_BUILD_EXECUTABLE} --no-protected-is-public --pep484-pyi ${SIP_BUILD_EXTRA_OPTIONS})

    add_custom_command(
            OUTPUT ${_sip_output_files}
            COMMAND ${CMAKE_COMMAND} -E echo ${message}
            COMMAND ${SIPCMD}
            COMMAND ${CMAKE_COMMAND} -E touch "${_sip_output_files}"
            WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/
            MAIN_DEPENDENCY ${MODULE_SIP}
            DEPENDS ${SIP_FILES}
            VERBATIM
    )

    add_custom_target("sip_${MODULE_TARGET}" ALL DEPENDS ${_sip_output_files})
endfunction()
