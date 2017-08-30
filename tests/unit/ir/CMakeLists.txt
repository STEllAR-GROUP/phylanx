# Copyright (c) 2017 Hartmut Kaiser
#
# Distributed under the Boost Software License, Version 1.0. (See accompanying
# file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

set(tests
    node_data
   )

foreach(test ${tests})
  set(sources ${test}.cpp)

  source_group("Source Files" FILES ${sources})

  # add executable
  add_phylanx_executable(${test}_test
    SOURCES ${sources}
    ${${test}_FLAGS}
    EXCLUDE_FROM_ALL
    FOLDER "Tests/Unit/IR/")

  add_phylanx_unit_test("ir" ${test} ${${test}_PARAMETERS})

  add_hpx_pseudo_target(tests.unit.ir.${test})
  add_hpx_pseudo_dependencies(tests.unit.ir tests.unit.ir.${test})
  add_hpx_pseudo_dependencies(tests.unit.ir.${test} ${test}_test_exe)

endforeach()

