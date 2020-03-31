# Copyright (c) 2017-2018 Hartmut Kaiser
#
# Distributed under the Boost Software License, Version 1.0. (See accompanying
# file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

set(tests
    blaze_benchmarks
    dist_cannon_4_loc
    dist_cannon_9_loc
    dist_cannon_16_loc
    simple_loop
   )

set(dist_cannon_4_loc_PARAMETERS LOCALITIES 4)
set(dist_cannon_9_loc_PARAMETERS LOCALITIES 9)
set(dist_cannon_16_loc_PARAMETERS LOCALITIES 16)

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
