# Copyright (c) 2017-2018 Hartmut Kaiser
#
# Distributed under the Boost Software License, Version 1.0. (See accompanying
# file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

set(tests
    advanced_boolean_slicing
    advanced_integer_slicing
    assert_condition
    broadcast
    define_operation
    dictionary
    format_string
    invoke_operation
    literal_value
    store_operation
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

  add_phylanx_pseudo_target(tests.unit.execution_tree.primitives_.${test})
  add_phylanx_pseudo_dependencies(tests.unit.execution_tree.primitives_
    tests.unit.execution_tree.primitives_.${test})
  add_phylanx_pseudo_dependencies(tests.unit.execution_tree.primitives_.${test}
    ${test}_test_exe)

endforeach()

