# Copyright (c) 2018-2019 Hartmut Kaiser
#
# Distributed under the Boost Software License, Version 1.0. (See accompanying
# file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

set(tests
    and_operation
    equal_operation
    greater_equal_operation
    greater_operation
    less_equal_operation
    less_operation
    not_equal_operation
    nonzero_operation
    or_operation
    unary_not_operation
    where_operation
   )

foreach(test ${tests})
  set(sources ${test}.cpp)

  source_group("Source Files" FILES ${sources})

  # add executable
  add_phylanx_executable(${test}_test
    SOURCES ${sources}
    ${${test}_FLAGS}
    EXCLUDE_FROM_ALL
    FOLDER "Tests/Unit/Plugins/Booleans")

  add_phylanx_unit_test("plugins.booleans" ${test} ${${test}_PARAMETERS})

  add_phylanx_pseudo_target(tests.unit.plugins.booleans.${test})
  add_phylanx_pseudo_dependencies(tests.unit.plugins.booleans
    tests.unit.plugins.booleans.${test})
  add_phylanx_pseudo_dependencies(tests.unit.plugins.booleans.${test}
    ${test}_test_exe)

endforeach()

