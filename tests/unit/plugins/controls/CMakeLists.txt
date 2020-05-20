# Copyright (c) 2018-2020 Hartmut Kaiser
#
# Distributed under the Boost Software License, Version 1.0. (See accompanying
# file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

set(tests
    async_operation
    block_operation
    filter_operation
    fold_left_operation
    fold_right_operation
    for_operation
    if_conditional
    fmap_operation
    parallel_block_operation
    parallel_map_operation
    range_operation
    while_operation
   )

foreach(test ${tests})
  set(sources ${test}.cpp)

  source_group("Source Files" FILES ${sources})

  # add executable
  add_phylanx_executable(${test}_test
    SOURCES ${sources}
    ${${test}_FLAGS}
    EXCLUDE_FROM_ALL
    FOLDER "Tests/Unit/Plugins/Controls")

  add_phylanx_unit_test("plugins.controls" ${test} ${${test}_PARAMETERS})

  add_phylanx_pseudo_target(tests.unit.plugins.controls.${test})
  add_phylanx_pseudo_dependencies(tests.unit.plugins.controls
    tests.unit.plugins.controls.${test})
  add_phylanx_pseudo_dependencies(tests.unit.plugins.controls.${test}
    ${test}_test_exe)

endforeach()

