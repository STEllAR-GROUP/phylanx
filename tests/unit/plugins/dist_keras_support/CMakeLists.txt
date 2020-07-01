# Copyright (c) 2020 Hartmut Kaiser
#
# Distributed under the Boost Software License, Version 1.0. (See accompanying
# file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

set(tests
    dist_conv1d_2_loc
    dist_conv1d_3_loc
    dist_conv1d_4_loc
   )

set(dist_conv1d_2_loc_PARAMETERS LOCALITIES 2)
set(dist_conv1d_3_loc_PARAMETERS LOCALITIES 3)
set(dist_conv1d_4_loc_PARAMETERS LOCALITIES 4)


foreach(test ${tests})
  set(sources ${test}.cpp)

  source_group("Source Files" FILES ${sources})

  # add executable
  add_phylanx_executable(${test}_test
    SOURCES ${sources}
    ${${test}_FLAGS}
    EXCLUDE_FROM_ALL
    FOLDER "Tests/Unit/Plugins/DistKerasSupport")

  add_phylanx_unit_test("plugins.dist_keras_support" ${test} ${${test}_PARAMETERS})

  add_phylanx_pseudo_target(tests.unit.plugins.dist_keras_support.${test})
  add_phylanx_pseudo_dependencies(tests.unit.plugins.dist_keras_support
    tests.unit.plugins.dist_keras_support.${test})
  add_phylanx_pseudo_dependencies(tests.unit.plugins.dist_keras_support.${test}
    ${test}_test_exe)

endforeach()

