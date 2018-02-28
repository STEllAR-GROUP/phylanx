# Copyright (c) 2017 Hartmut Kaiser
#
# Distributed under the Boost Software License, Version 1.0. (See accompanying
# file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

set(tests
    matrix_iterators
    performance_data
    serialization_variant
   )

foreach(test ${tests})
  set(sources ${test}.cpp)

  source_group("Source Files" FILES ${sources})

  # add executable
  add_phylanx_executable(${test}_test
    SOURCES ${sources}
    ${${test}_FLAGS}
    EXCLUDE_FROM_ALL
    FOLDER "Tests/Unit/Util/")

  add_phylanx_unit_test("util" ${test} ${${test}_PARAMETERS})

  add_phylanx_pseudo_target(tests.unit.util.${test})
  add_phylanx_pseudo_dependencies(tests.unit.util tests.unit.util.${test})
  add_phylanx_pseudo_dependencies(tests.unit.util.${test} ${test}_test_exe)

endforeach()

