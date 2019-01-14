# Copyright (c) 2018 Hartmut Kaiser
#
# Distributed under the Boost Software License, Version 1.0. (See accompanying
# file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

set(tests
    cumsum_operation
    max_operation
    mean_operation
    min_operation
    prod_operation
    std_operation
    sum_operation
    var_operation
   )

foreach(test ${tests})
  set(sources ${test}.cpp)

  source_group("Source Files" FILES ${sources})

  # add executable
  add_phylanx_executable(${test}_test
    SOURCES ${sources}
    ${${test}_FLAGS}
    EXCLUDE_FROM_ALL
    FOLDER "Tests/Unit/Plugins/Statistics")

  add_phylanx_unit_test("plugins.statistics" ${test} ${${test}_PARAMETERS})

  add_phylanx_pseudo_target(tests.unit.plugins.statistics.${test})
  add_phylanx_pseudo_dependencies(tests.unit.plugins.statistics
    tests.unit.plugins.statistics.${test})
  add_phylanx_pseudo_dependencies(tests.unit.plugins.statistics.${test}
    ${test}_test_exe)

endforeach()

