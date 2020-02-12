# Copyright (c) 2019 Hartmut Kaiser
#
# Distributed under the Boost Software License, Version 1.0. (See accompanying
# file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

set(tests
    dist_dot_operation
    dist_transpose_operation
    dist_cannon_product
    dist_cannon_product_9_loc
   )

set(dist_dot_operation_PARAMETERS LOCALITIES 2)
set(dist_cannon_product_PARAMETERS LOCALITIES 4)
set(dist_cannon_product_9_loc_PARAMETERS LOCALITIES 9)

foreach(test ${tests})
  set(sources ${test}.cpp)

  source_group("Source Files" FILES ${sources})

  # add executable
  add_phylanx_executable(${test}_test
    SOURCES ${sources}
    ${${test}_FLAGS}
    EXCLUDE_FROM_ALL
    FOLDER "Tests/Unit/Plugins/DistMatrixOps")

  add_phylanx_unit_test("plugins.dist_matrixops" ${test} ${${test}_PARAMETERS})

  add_phylanx_pseudo_target(tests.unit.plugins.dist_matrixops.${test})
  add_phylanx_pseudo_dependencies(tests.unit.plugins.dist_matrixops
    tests.unit.plugins.dist_matrixops.${test})
  add_phylanx_pseudo_dependencies(tests.unit.plugins.dist_matrixops.${test}
    ${test}_test_exe)

endforeach()

