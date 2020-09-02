# Copyright (c) 2017-2018 Hartmut Kaiser
#
# Distributed under the Boost Software License, Version 1.0. (See accompanying
# file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

set(tests
    blaze_benchmarks
    simple_loop
    conv1d_dist_instrumented
   )

foreach(test ${tests})
  set(sources ${test}.cpp)

  source_group("Source Files" FILES ${sources})

  # add executable
  add_phylanx_executable(${test}_test
    SOURCES ${sources}
    ${${test}_FLAGS}
    EXCLUDE_FROM_ALL
    FOLDER "Tests/Performance/")

  add_phylanx_pseudo_target(tests.performance.${test})
  add_phylanx_pseudo_dependencies(tests.performance tests.performance.${test})
  add_phylanx_pseudo_dependencies(tests.performance.${test} ${test}_test_exe)

endforeach()

set(args
    4
    9
    16
    )
	
foreach(param ${args})
  set(dist_cannon_${param}_PARAMETERS LOCALITIES ${param})
  set(sources dist_cannon.cpp)

  source_group("Source Files" FILES ${sources})

  # add executable
  add_phylanx_executable(dist_cannon_${param}_test
    SOURCES ${sources}
    ${dist_cannon_${param}_FLAGS}
    EXCLUDE_FROM_ALL
    FOLDER "Tests/Performance/")

  # add_phylanx_unit_test("performance" dist_cannon_${param} ${dist_cannon_${param}_PARAMETERS})
  add_phylanx_pseudo_target(tests.performance.dist_cannon_${param})
  add_phylanx_pseudo_dependencies(tests.performance tests.performance.dist_cannon_${param})
  add_phylanx_pseudo_dependencies(tests.performance.dist_cannon_${param} dist_cannon_${param}_test_exe)
endforeach()
