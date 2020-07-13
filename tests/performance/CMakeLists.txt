# Copyright (c) 2017-2018 Hartmut Kaiser
#
# Distributed under the Boost Software License, Version 1.0. (See accompanying
# file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

set(tests
    blaze_benchmarks
    simple_loop
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

foreach(param ${args})
  set(dist_dot_${param}_PARAMETERS LOCALITIES ${param})
  set(sources dist_dot.cpp)

  source_group("Source Files" FILES ${sources})

  # add executable
  add_phylanx_executable(dist_dot_${param}_test
    SOURCES ${sources}
    ${dist_dot_${param}_FLAGS}
    EXCLUDE_FROM_ALL
    FOLDER "Tests/Performance/")

  # add_phylanx_unit_test("performance" dist_dot_${param} ${dist_dot_${param}_PARAMETERS})
  add_phylanx_pseudo_target(tests.performance.dist_dot_${param})
  add_phylanx_pseudo_dependencies(tests.performance tests.performance.dist_dot_${param})
  add_phylanx_pseudo_dependencies(tests.performance.dist_dot_${param} dist_dot_${param}_test_exe)
endforeach()

set(conv_args
    2
    4
    6
    8
    10
    12
    14
    16
    )
	
foreach(param ${conv_args})
  set(dist_conv1d_${param}_PARAMETERS LOCALITIES ${param})
  set(sources dist_conv1d.cpp)

  source_group("Source Files" FILES ${sources})

  # add executable
  add_phylanx_executable(dist_conv1d_${param}_test
    SOURCES ${sources}
    ${dist_conv1d_${param}_FLAGS}
    EXCLUDE_FROM_ALL
    FOLDER "Tests/Performance/")

  add_phylanx_unit_test("performance" dist_conv1d_${param} ${dist_conv1d_${param}_PARAMETERS})
  add_phylanx_pseudo_target(tests.performance.dist_conv1d_${param})
  add_phylanx_pseudo_dependencies(tests.performance tests.performance.dist_conv1d_${param})
  add_phylanx_pseudo_dependencies(tests.performance.dist_conv1d_${param} dist_conv1d_${param}_test_exe)
endforeach()