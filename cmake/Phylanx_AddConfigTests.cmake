# Copyright (c) 2011 Bryce Lelbach
# Copyright (c) 2014 Thomas Heller
# Copyright (c) 2020 Hartmut Kaiser
#
# SPDX-License-Identifier: BSL-1.0
# Distributed under the Boost Software License, Version 1.0. (See accompanying
# file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

set(PHYLANX_ADDCONFIGTEST_LOADED TRUE)

include(CheckLibraryExists)

function(add_phylanx_config_test variable)
  set(options FILE EXECUTE)
  set(one_value_args SOURCE ROOT CMAKECXXFEATURE)
  set(multi_value_args
      INCLUDE_DIRECTORIES
      LINK_DIRECTORIES
      COMPILE_DEFINITIONS
      LIBRARIES
      ARGS
      DEFINITIONS
      REQUIRED
  )
  cmake_parse_arguments(
    ${variable} "${options}" "${one_value_args}" "${multi_value_args}" ${ARGN}
  )

  set(_run_msg)
  # Check CMake feature tests if the user didn't override the value of this
  # variable:
  if(NOT DEFINED ${variable})
    if(${variable}_CMAKECXXFEATURE)
      # We don't have to run our own feature test if there is a corresponding
      # cmake feature test and cmake reports the feature is supported on this
      # platform.
      list(FIND CMAKE_CXX_COMPILE_FEATURES ${${variable}_CMAKECXXFEATURE} __pos)
      if(NOT ${__pos} EQUAL -1)
        set(${variable}
            TRUE
            CACHE INTERNAL ""
        )
        set(_run_msg "Success (cmake feature test)")
      endif()
    endif()
  endif()

  if(NOT DEFINED ${variable})
    file(MAKE_DIRECTORY
         "${PROJECT_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/config_tests"
    )

    string(TOLOWER "${variable}" variable_lc)
    if(${variable}_FILE)
      if(${variable}_ROOT)
        set(test_source "${${variable}_ROOT}/share/phylanx/${${variable}_SOURCE}")
      else()
        set(test_source "${PROJECT_SOURCE_DIR}/${${variable}_SOURCE}")
      endif()
    else()
      set(test_source
          "${PROJECT_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/config_tests/${variable_lc}.cpp"
      )
      file(WRITE "${test_source}" "${${variable}_SOURCE}\n")
    endif()
    set(test_binary
        ${PROJECT_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/config_tests/${variable_lc}
    )

    get_directory_property(CONFIG_TEST_INCLUDE_DIRS INCLUDE_DIRECTORIES)
    get_directory_property(CONFIG_TEST_LINK_DIRS LINK_DIRECTORIES)
    set(COMPILE_DEFINITIONS_TMP)
    set(CONFIG_TEST_COMPILE_DEFINITIONS)
    get_directory_property(COMPILE_DEFINITIONS_TMP COMPILE_DEFINITIONS)
    foreach(def IN LISTS COMPILE_DEFINITIONS_TMP
                         ${variable}_COMPILE_DEFINITIONS
    )
      set(CONFIG_TEST_COMPILE_DEFINITIONS
          "${CONFIG_TEST_COMPILE_DEFINITIONS} -D${def}"
      )
    endforeach()
    get_property(
      PHYLANX_TARGET_COMPILE_OPTIONS_PUBLIC_VAR GLOBAL
      PROPERTY phylanx_TARGET_COMPILE_OPTIONS_PUBLIC
    )
    get_property(
      PHYLANX_TARGET_COMPILE_OPTIONS_PRIVATE_VAR GLOBAL
      PROPERTY phylanx_TARGET_COMPILE_OPTIONS_PRIVATE
    )
    set(PHYLANX_TARGET_COMPILE_OPTIONS_VAR
        ${PHYLANX_TARGET_COMPILE_OPTIONS_PUBLIC_VAR}
        ${PHYLANX_TARGET_COMPILE_OPTIONS_PRIVATE_VAR}
    )
    foreach(_flag ${PHYLANX_TARGET_COMPILE_OPTIONS_VAR})
      if(NOT "${_flag}" MATCHES "^\\$.*")
        set(CONFIG_TEST_COMPILE_DEFINITIONS
            "${CONFIG_TEST_COMPILE_DEFINITIONS} ${_flag}"
        )
      endif()
    endforeach()

    set(CONFIG_TEST_INCLUDE_DIRS ${CONFIG_TEST_INCLUDE_DIRS}
                                 ${${variable}_INCLUDE_DIRECTORIES}
    )
    set(CONFIG_TEST_LINK_DIRS ${CONFIG_TEST_LINK_DIRS}
                              ${${variable}_LINK_DIRECTORIES}
    )

    if(${variable}_EXECUTE)
      if(NOT CMAKE_CROSSCOMPILING)
        # cmake-format: off
        try_run(
          ${variable}_RUN_RESULT ${variable}_COMPILE_RESULT
          ${PROJECT_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/config_tests
          ${test_source}
          CMAKE_FLAGS
            "-DINCLUDE_DIRECTORIES=${CONFIG_TEST_INCLUDE_DIRS}"
            "-DLINK_DIRECTORIES=${CONFIG_TEST_LINK_DIRS}"
            "-DLINK_LIBRARIES=${CONFIG_TEST_LINK_LIBRARIES}"
            "-DCOMPILE_DEFINITIONS=${CONFIG_TEST_COMPILE_DEFINITIONS}"
          CXX_STANDARD ${phylanx_CXX_STANDARD}
          CXX_STANDARD_REQUIRED ON
          CXX_EXTENSIONS FALSE
          RUN_OUTPUT_VARIABLE ${variable}_OUTPUT
          ARGS ${${variable}_ARGS}
        )
        # cmake-format: on
        if(${variable}_COMPILE_RESULT AND NOT ${variable}_RUN_RESULT)
          set(${variable}_RESULT TRUE)
        else()
          set(${variable}_RESULT FALSE)
        endif()
      else()
        set(${variable}_RESULT FALSE)
      endif()
    else()
      # cmake-format: off
      try_compile(
        ${variable}_RESULT
        ${PROJECT_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/config_tests
        ${test_source}
        CMAKE_FLAGS
          "-DINCLUDE_DIRECTORIES=${CONFIG_TEST_INCLUDE_DIRS}"
          "-DLINK_DIRECTORIES=${CONFIG_TEST_LINK_DIRS}"
          "-DLINK_LIBRARIES=${CONFIG_TEST_LINK_LIBRARIES}"
          "-DCOMPILE_DEFINITIONS=${CONFIG_TEST_COMPILE_DEFINITIONS}"
        OUTPUT_VARIABLE ${variable}_OUTPUT
        CXX_STANDARD ${phylanx_CXX_STANDARD}
        CXX_STANDARD_REQUIRED ON
        CXX_EXTENSIONS FALSE
        COPY_FILE ${test_binary}
      )
      # cmake-format: on
      phylanx_debug("Compile test: ${variable}")
      phylanx_debug("Compilation output: ${${variable}_OUTPUT}")
    endif()

    set(_run_msg "Success")
  else()
    set(${variable}_RESULT ${${variable}})
    if(NOT _run_msg)
      set(_run_msg "pre-set to ${${variable}}")
    endif()
  endif()

  string(TOUPPER "${variable}" variable_uc)
  set(_msg "Performing Test ${variable_uc}")

  if(${variable}_RESULT)
    set(_msg "${_msg} - ${_run_msg}")
  else()
    set(_msg "${_msg} - Failed")
  endif()

  set(${variable}
      ${${variable}_RESULT}
      CACHE INTERNAL ""
  )
  phylanx_info(${_msg})

  if(${variable}_RESULT)
    foreach(definition ${${variable}_DEFINITIONS})
      phylanx_add_config_define(${definition})
    endforeach()
  elseif(${variable}_REQUIRED)
    phylanx_warn("Test failed, detailed output:\n\n${${variable}_OUTPUT}")
    phylanx_error(${${variable}_REQUIRED})
  endif()
endfunction()

# Makes it possible to provide a feature test that is able to test the compiler
# to build parts of phylanx directly when the given definition is defined.
function(add_phylanx_in_framework_config_test variable)
  # Generate the config only if the test wasn't executed yet
  if(NOT DEFINED ${variable})
    # Location to generate the config headers to
    set(${variable}_GENERATED_DIR
        "${PROJECT_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/config_tests/header-${variable}"
    )
    generate_config_defines_header(${${variable}_GENERATED_DIR})
  endif()

  set(options)
  set(one_value_args)
  set(multi_value_args DEFINITIONS INCLUDE_DIRECTORIES COMPILE_DEFINITIONS)
  cmake_parse_arguments(
    ${variable} "${options}" "${one_value_args}" "${multi_value_args}" ${ARGN}
  )

  # We call the generic feature test method while modifying some existing parsed
  # arguments in order to alter the INCLUDE_DIRECTORIES and the
  # COMPILE_DEFINITIONS. It's important here not to link the config test against
  # an executable because otherwise this will result in unresolved references to
  # the phylanx library, that wasn't built as of now.
  add_phylanx_config_test(
    ${variable} ${${variable}_UNPARSED_ARGUMENTS}
    DEFINITIONS ${${variable}_DEFINITIONS}
    COMPILE_DEFINITIONS
      ${${variable}_COMPILE_DEFINITIONS}
      # We add the definitions we test to the existing compile definitions.
      ${${variable}_DEFINITIONS}
      # Add PHYLANX_NO_VERSION_CHECK to make header only parts of phylanx available
      # without requiring to link against the phylanx sources. We can remove this
      # workaround as soon as CMake 3.6 is the minimal required version and
      # supports: CMAKE_TRY_COMPILE_TARGET_TYPE = STATIC_LIBRARY when using
      # try_compile to not to throw errors on unresolved symbols.
      PHYLANX_NO_VERSION_CHECK
    INCLUDE_DIRECTORIES
      ${${variable}_INCLUDE_DIRECTORIES}
      # We add the generated headers to the include dirs
      ${${variable}_GENERATED_DIR}
  )

  if(DEFINED ${variable}_GENERATED_DIR)
    # Cleanup the generated header
    file(REMOVE_RECURSE "${${variable}_GENERATED_DIR}")
  endif()
endfunction()

# ##############################################################################
function(phylanx_check_for_cxx17_shared_ptr_array)
  add_phylanx_config_test(
    PHYLANX_WITH_CXX17_SHARED_PTR_ARRAY
    SOURCE cmake/tests/cxx17_shared_ptr_array.cpp FILE ${ARGN}
  )
endfunction()
