# Copyright (c) 2018 Hartmut Kaiser
#
# Distributed under the Boost Software License, Version 1.0. (See accompanying
# file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

set(tests
    arange
    argmin
    argmax
    column_slicing
    constant
    count_nonzero
    cross_operation
    determinant
    diag_operation
    dot_operation
    expand_dims
    extract_shape
    eye_operation
    flatten
    gradient_operation
    hstack_operation
    identity
    inverse_operation
    linearmatrix
    linspace
    list_slicing_operation
    power_operation
    random
    random_distributions
    reshape_operation
    row_slicing
    shuffle_operation
    slicing_operation
    softmax_operation
    squeeze_operation
    tile_operation
    transpose_operation
    unique
    vstack_operation
   )

if(PHYLANX_WITH_BLAZE_TENSOR)
  set(tests ${tests} dstack_operation)
endif()

foreach(test ${tests})
  set(sources ${test}.cpp)

  source_group("Source Files" FILES ${sources})

  # add executable
  add_phylanx_executable(${test}_test
    SOURCES ${sources}
    ${${test}_FLAGS}
    EXCLUDE_FROM_ALL
    FOLDER "Tests/Unit/Plugins/MatrixOps")

  add_phylanx_unit_test("plugins.matrixops" ${test} ${${test}_PARAMETERS})

  add_phylanx_pseudo_target(tests.unit.plugins.matrixops.${test})
  add_phylanx_pseudo_dependencies(tests.unit.plugins.matrixops
    tests.unit.plugins.matrixops.${test})
  add_phylanx_pseudo_dependencies(tests.unit.plugins.matrixops.${test}
    ${test}_test_exe)

endforeach()

