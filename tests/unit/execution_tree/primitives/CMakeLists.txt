# Copyright (c) 2017 Hartmut Kaiser
#
# Distributed under the Boost Software License, Version 1.0. (See accompanying
# file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

set(tests
    add_operation
    and_operation
    equal_operation
    exponential_operation
    file_primitives
    greater_operation
    greater_equal_operation
    less_operation
    less_equal_operation
    literal_value
    mul_operation
    not_equal_operation
    or_operation
    sub_operation
   )

foreach(test ${tests})
  set(sources ${test}.cpp)

  source_group("Source Files" FILES ${sources})

  # add executable
  add_phylanx_executable(${test}_test
    SOURCES ${sources}
    ${${test}_FLAGS}
    EXCLUDE_FROM_ALL
    FOLDER "Tests/Unit/ExecutionTree/Primitives")

  add_phylanx_unit_test("primitives" ${test} ${${test}_PARAMETERS})

  add_phylanx_pseudo_target(tests.unit.execution_tree.primitives.${test})
  add_phylanx_pseudo_dependencies(tests.unit.execution_tree.primitives
    tests.unit.execution_tree.primitives.${test})
  add_phylanx_pseudo_dependencies(tests.unit.execution_tree.primitives.${test}
    ${test}_test_exe)

endforeach()

