# Copyright (c) 2018 Hartmut Kaiser
#
# Distributed under the Boost Software License, Version 1.0. (See accompanying
# file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

set(tests
    arange
    argmin
    argmax
    clip
    column_slicing
    concatenate
    constant
    count_nonzero
    cross_operation
    determinant
    diag_operation
    dot_operation
    dstack_operation
    expand_dims
    extract_shape
    eye_operation
    flip_operation
    gauss_inverse
    gradient_operation
    hsplit_operation
    hstack_operation
    identity
    insert
    inverse_operation
    linearmatrix
    linspace
    list_slicing_operation
    ndim
    norm
    pad
    power_operation
    random
    random_distributions
    repeat_operation
    reshape_operation
    row_slicing
    shuffle_operation
    size
    slicing_operation
    sort
    squeeze_operation
    stack_operation
    tile_operation
    transpose_operation
    triu_operation
    unique
    vsplit_operation
    vstack_operation
   )

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

