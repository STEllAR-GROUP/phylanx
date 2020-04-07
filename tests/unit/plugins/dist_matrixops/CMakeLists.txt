# Copyright (c) 2019 Hartmut Kaiser
#
# Distributed under the Boost Software License, Version 1.0. (See accompanying
# file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

set(tests
    dist_cannon_product_4_loc
    dist_cannon_product_9_loc
    dist_constant_2_loc
    dist_constant_3_loc
    dist_constant_4_loc
    dist_constant_6_loc
    dist_dot_operation_2_loc
    dist_identity_2_loc
    dist_identity_4_loc
    dist_identity_6_loc
    dist_transpose_operation
    dist_random_2_loc
    dist_random_4_loc
    dist_random_5_loc
    retile_2_loc
    retile_3_loc
    retile_6_loc
   )


set(dist_cannon_product_4_loc_PARAMETERS LOCALITIES 4)
set(dist_cannon_product_9_loc_PARAMETERS LOCALITIES 9)
set(dist_constant_2_loc_PARAMETERS LOCALITIES 2)
set(dist_constant_3_loc_PARAMETERS LOCALITIES 3)
set(dist_constant_4_loc_PARAMETERS LOCALITIES 4)
set(dist_constant_6_loc_PARAMETERS LOCALITIES 6)
set(dist_dot_operation_2_loc_PARAMETERS LOCALITIES 2)
set(dist_identity_2_loc_PARAMETERS LOCALITIES 2)
set(dist_identity_4_loc_PARAMETERS LOCALITIES 4)
set(dist_identity_6_loc_PARAMETERS LOCALITIES 6)
set(dist_random_2_loc_PARAMETERS LOCALITIES 2)
set(dist_random_4_loc_PARAMETERS LOCALITIES 4)
set(dist_random_5_loc_PARAMETERS LOCALITIES 5)
set(retile_2_loc_PARAMETERS LOCALITIES 2)
set(retile_3_loc_PARAMETERS LOCALITIES 3)
set(retile_6_loc_PARAMETERS LOCALITIES 6)


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

