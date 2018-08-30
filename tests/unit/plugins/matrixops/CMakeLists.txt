# Copyright (c) 2018 Hartmut Kaiser
#
# Distributed under the Boost Software License, Version 1.0. (See accompanying
# file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

set(tests
    add_dimension
    argmin
    argmax
    column_set
    column_slicing
    constant
    count_nonzero
    determinant
    diag_operation
    dot_operation
    cross_operation
    extract_shape
    generic_operation
    gradient_operation
    hstack_operation
    identity
    inverse_operation
    linearmatrix
    linspace
    list_slicing_operation
    mean_operation
    power_operation
    random
    random_distributions
    row_set
    row_slicing
    set_operation
    shuffle_operation
    slicing_operation
    square_root_operation
    sum_operation
    transpose_operation
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

