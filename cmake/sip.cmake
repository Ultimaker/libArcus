# set(SIP_BUILD_EXECUTABLE /home/peer23peer/.conan/data/sip/6.5.0/python/stable/package/919a80bca9eb6a39029fa3ff69bbb78ddf84bbb8/bin/sip-build)
set(PRE_BUILD_CMD)
#set(PRE_BUILD_CMD ". ${CMAKE_BINARY_DIR}/conan/conanbuild.sh && ")


macro(get_library_name lib_var)
    set(lib "${${lib_var}}")
    if(lib)
        cmake_path(GET lib STEM LAST_ONLY lib)
        string(REGEX REPLACE "^lib" "" lib ${lib})
        list(APPEND _sip_link_libraries ${lib})
    endif()
endmacro()

macro(get_library_directory lib_dir_var)
    set(lib_dir "${${lib_dir_var}}")
    if(lib_dir)
        cmake_path(SET lib_dir NORMALIZE "${lib_dir}/")
        list(APPEND _sip_link_libraries_dirs "${lib_dir}")
    endif()
endmacro()

macro(get_library_include_directory lib_dir_var)
    set(lib_dir "${${lib_dir_var}}")
    if(lib_dir)
        cmake_path(SET lib_dir NORMALIZE "${lib_dir}/")
        list(APPEND _sip_include_dirs "${lib_dir}")
    endif()
endmacro()

macro(sanitize_list var_list)
    list(REMOVE_DUPLICATES ${var_list})
    list(REMOVE_ITEM ${var_list} "")
    list(TRANSFORM ${var_list} PREPEND "\"")
    list(TRANSFORM ${var_list} APPEND "\"")
    list(JOIN ${var_list} ", " ${var_list})
endmacro()

function(add_sip_module MODULE_TARGET MODULE_SIP SIP_FILES HDR_FILES SRC_FILES)
    message(STATUS "SIP: Generating files")
    string(TOUPPER ${CMAKE_BUILD_TYPE} _upper_case_build_type)

    # Get the sip modules copy the files to the build and set the tobe generated path.
    # Because that files needs to be in the same folder as the pyproject.toml
    cmake_path(GET MODULE_SIP FILENAME _module_sip_filename)
    cmake_path(GET MODULE_SIP PARENT_PATH _module_sip_path)
    cmake_path(SET _module_sip_path NORMALIZE "${_module_sip_path}/")

    list(PREPEND SIP_FILES ${MODULE_SIP})
    set(_sip_extra_include_dirs)
    foreach (_sip_include ${SIP_FILES})
        cmake_path(GET _sip_include PARENT_PATH _sip_include_path)
        cmake_path(SET _sip_include_path NORMALIZE "${_sip_include_path}/")
        list(APPEND _sip_extra_include_dirs ${_sip_include_path})
    endforeach ()
    sanitize_list(_sip_extra_include_dirs)

    message(STATUS "SIP: Generating pyproject.toml")
    # Get the module specified include locations
    set(_sip_include_dirs)
    get_target_property(_own_sip_include_dirs ${MODULE_TARGET} INTERFACE_INCLUDE_DIRECTORIES)
    if(_own_sip_include_dirs)
        list(APPEND _sip_include_dirs "${_own_sip_include_dirs}")
    endif()

    # Get the module compiler options
    set(_compile_options)
    get_target_property(_own_compile_options ${MODULE_TARGET} INTERFACE_COMPILE_OPTIONS)
    if(_own_compile_options)
        list(APPEND _compile_options "${_own_compile_options}")
    endif()

    set(_compile_features)
    get_target_property(_own_compile_features ${MODULE_TARGET} INTERFACE_COMPILE_FEATURES)
    if(_own_compile_features)
        foreach(feature ${_own_compile_features})
            if(feature STREQUAL "cxx_std_17")
                list(APPEND _compile_features "-std=c++17")
            elseif(feature STREQUAL "cxx_std_20")
                list(APPEND _compile_features "-std=c++20")
            elseif(feature STREQUAL "cxx_std_23")
                list(APPEND _compile_features "-std=c++23")
            endif()
        endforeach()
    endif()

    set(_compile_definitions)
    get_target_property(_own_compile_definitions ${MODULE_TARGET} INTERFACE_COMPILE_DEFINITIONS)
    if(_own_compile_definitions)
        list(APPEND _compile_definitions "${_own_compile_definitions}")
    endif()

    # Get the linked dependency include locations and lib names
    set(_sip_link_libraries)
    set(_sip_link_libraries_dirs)

    get_target_property(_link_libraries ${MODULE_TARGET} INTERFACE_LINK_LIBRARIES)
    if(_link_libraries)
        foreach(_link_lib ${_link_libraries})

            # Get the name(-space) of the dependency
            get_target_property(_lib_name ${_link_lib} NAME)
            string(REPLACE "::" ";" _x "${_lib_name}")
            list(GET _x 0 _name)

            # Set the property names depending on the dependency type (interface targets need to be named INTERFACE_<>
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

            get_target_property(_lib_compile_features ${_link_lib} ${_compile_features_prop_var})
            if(_lib_compile_features)
                foreach(feature ${_lib_compile_features})
                    if(feature STREQUAL "cxx_std_17")
                        list(APPEND _compile_features "-std=c++17")
                    elseif(feature STREQUAL "cxx_std_20")
                        list(APPEND _compile_features "-std=c++20")
                    elseif(feature STREQUAL "cxx_std_23")
                        list(APPEND _compile_features "-std=c++23")
                    endif()
                endforeach()
            endif()

            get_target_property(_lib_compile_options ${_link_lib} ${_compile_options_prop_var})
            if(_lib_compile_options)
                list(APPEND _compile_options "${_lib_compile_options}")
            endif()

            get_target_property(_lib_compile_definitions ${_link_lib} ${_compile_definitions_prop_var})
            if(_lib_compile_definitions)
                list(APPEND _compile_definitions "${_lib_compile_definitions}")
            endif()

            # Get Include directories of dependencies
            get_target_property(_link_include_dirs ${_link_lib} ${_include_directories_prop_var})
            if(_link_include_dirs)
                list(APPEND _sip_include_dirs ${_link_include_dirs})
            endif()
            get_library_include_directory("${_name}_INCLUDE_DIRS")
            get_library_include_directory("${_name}_INCLUDE_DIR")

            # Get library directories
            get_library_directory("${_name}_LIB_DIRS_${_upper_case_build_type}")
            get_library_directory("${_name}_LIBRARY_DIRS")

            # Get library
            get_library_name("${_name}_LIBS_${_upper_case_build_type}")
            get_library_name("${_name}_LIBRARY")
            get_library_name("${_name}_LIBRARIES")
            get_library_name("${_name}_SYSTEM_LIBS_${_upper_case_build_type}")
        endforeach()
    endif()

    # Sanitize the lists
    set(_compile_args)
    list(APPEND _compile_args ${_compile_features} ${_compile_options})
    sanitize_list(_compile_args)
    sanitize_list(_compile_definitions)
    message(STATUS "BLAAAAAAAAAAAAAAT ${_compile_definitions}")
    sanitize_list(_sip_include_dirs)
    sanitize_list(_sip_link_libraries)
    sanitize_list(_sip_link_libraries_dirs)
    sanitize_list(SRC_FILES)
    sanitize_list(HDR_FILES)

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
sip-include-dirs = [${_sip_extra_include_dirs}]
build-dir = \"${CMAKE_CURRENT_BINARY_DIR}/${MODULE_TARGET}/\"
verbose = true

[tool.sip.bindings.${MODULE_TARGET}]
headers = [${HDR_FILES}]
include-dirs = [${_sip_include_dirs}]
libraries = [${_sip_link_libraries}]
library-dirs = [${_sip_link_libraries_dirs}]
sources = [${SRC_FILES}]
extra-compile-args = [${_compile_args}]
define-macros = [${_compile_definitions}]
exceptions = true
release-gil = true
$<$<CONFIG:Debug>:debug = true>
$<$<BOOL:${BUILD_STATIC}>:static = true>
")

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

    # TODO: Installer, for the Python module don't forget about the pyi file for code completion
endfunction()
