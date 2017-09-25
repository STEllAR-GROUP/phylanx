# Copyright (c) 2017 Hartmut Kaiser
#
# Distributed under the Boost Software License, Version 1.0. (See accompanying
# file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

macro(add_phylanx_python_test category name)
  set(options FAILURE_EXPECTED)
  set(one_value_args SCRIPT FOLDER WORKING_DIRECTORY ENVIRONMENT)
  set(multi_value_args ARGS DEPENDS)
  cmake_parse_arguments(${name} "${options}" "${one_value_args}" "${multi_value_args}" ${ARGN})

  if(NOT ${name}_SCRIPT)
    set(${name}_SCRIPT ${name})
  endif()

  set(_depends)
  if(${name}_DEPENDS)
    set(_depends "${${name}_DEPENDS}")
  endif()

  set(_working_directory)
  if(${name}_WORKING_DIRECTORY)
    set(_working_directory WORKING_DIRECTORY ${${name}_WORKING_DIRECTORY})
  endif()

  set(_environment)
  if(${name}_ENVIRONMENT)
    if(MSVC)
      set(_environment ${CMAKE_COMMAND} -E env ${${name}_ENVIRONMENT})
    else()
      set(_environment ENVIRONMENT ${${name}_ENVIRONMENT})
    endif()
  endif()

  set(script ${CMAKE_CURRENT_SOURCE_DIR}/${${name}_SCRIPT})

  phylanx_debug("add_phylanx_python_test" "target name: ${${name}_SCRIPT}_test_py, script: ${script}")
  add_custom_target(
    ${name}_test_py
    SOURCES ${script})

  phylanx_debug("add_phylanx_python_test" "FOLDER: ${${name}_FOLDER}")
  if(${name}_FOLDER)
    set_target_properties(
      ${name}_test_py
      PROPERTIES FOLDER ${${name}_FOLDER})
  endif()

  source_group("Scripts" FILES ${script})
#  set_source_files_properties(${script} PROPERTIES HEADER_FILE_ONLY TRUE)

  set(expected "0")
  if(${name}_FAILURE_EXPECTED)
    set(expected "1")
  endif()

#  set(args)
#  foreach(arg ${${name}_UNPARSED_ARGUMENTS})
#    set(args ${args} "${arg}")
#  endforeach()
#  if(args)
#    set(args "-v" "--" ${args})
#  endif()

  set(cmd "${PYTHON_EXECUTABLE}" ${script} ${args})
  if(MSVC)
    set(OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/build/timestamp.${name}")
    add_custom_command(OUTPUT ${OUTPUT} ${script}
      COMMAND ${_environment} ${cmd}
      COMMAND ${CMAKE_COMMAND} -E touch ${OUTPUT}
      DEPENDS ${name}_test_py ${_depends}
      ${_working_directory})
  else()
    add_test(
      NAME "${category}.${name}"
      COMMAND ${cmd})

    set_tests_properties("${category}.${name}"
      PROPERTIES ${_environment} ${_working_directory})
  endif()

endmacro()

macro(add_phylanx_python_unit_test category name)
  add_phylanx_python_test("tests.unit.python.${category}" ${name} ${ARGN})
endmacro()

macro(add_phylanx_python_regression_test category name)
  add_phylanx_python_test("tests.regressions.python.${category}" ${name} ${ARGN})
endmacro()

