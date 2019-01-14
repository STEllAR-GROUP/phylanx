# Copyright (c) 2018 Hartmut Kaiser
#
# Distributed under the Boost Software License, Version 1.0. (See accompanying
# file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

set(tests
    add_operation
    add_lists
    div_operation
    generic_operation
    mul_operation
    sub_operation
    unary_minus_operation
   )

foreach(test ${tests})
  set(sources ${test}.cpp)

  source_group("Source Files" FILES ${sources})

  # add executable
  add_phylanx_executable(${test}_test
    SOURCES ${sources}
    ${${test}_FLAGS}
    EXCLUDE_FROM_ALL
    FOLDER "Tests/Unit/Plugins/Arithmetics")

  add_phylanx_unit_test("plugins.arithmetics" ${test} ${${test}_PARAMETERS})

  add_phylanx_pseudo_target(tests.unit.plugins.arithmetics.${test})
  add_phylanx_pseudo_dependencies(tests.unit.plugins.arithmetics
    tests.unit.plugins.arithmetics.${test})
  add_phylanx_pseudo_dependencies(tests.unit.plugins.arithmetics.${test}
    ${test}_test_exe)

endforeach()

