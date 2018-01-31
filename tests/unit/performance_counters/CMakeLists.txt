# Copyright (c) 2018 Parsa Amini
# Copyright (c) 2018 Hartmut Kaiser
#
# Distributed under the Boost Software License, Version 1.0. (See accompanying
# file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

set(tests
    primitive_counter
   )

foreach(test ${tests})
  set(sources ${test}.cpp)

  source_group("Source Files" FILES ${sources})

  # add executable
  add_phylanx_executable(${test}_test
    SOURCES ${sources}
    ${${test}_FLAGS}
    EXCLUDE_FROM_ALL
    FOLDER "Tests/Unit/PerformanceCounters/")

  add_phylanx_unit_test("performance_counters" ${test} ${${test}_PARAMETERS})

  add_phylanx_pseudo_target(tests.unit.performance_counters.${test})
  add_phylanx_pseudo_dependencies(tests.unit.performance_counters tests.unit.performance_counters.${test})
  add_phylanx_pseudo_dependencies(tests.unit.performance_counters.${test} ${test}_test_exe)

endforeach()

