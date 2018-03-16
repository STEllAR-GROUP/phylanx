# Copyright (c) 2017 Hartmut Kaiser
#
# Distributed under the Boost Software License, Version 1.0. (See accompanying
# file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

set(tests
    add_operation
    all_operation
    and_operation
    any_operation
    argmin
    argmax
    block_operation
    column_set
    column_slicing
    constant
    define_operation
    determinant
    diag_operation
    div_operation
    dot_operation
    cross_operation
    equal_operation
    exponential_operation
    extract_shape
    file_primitives
    file_csv_primitives
    filter_operation
    fold_left_operation
    fold_right_operation
    for_operation
    greater_operation
    greater_equal_operation
    hstack_operation
    identity
    if_conditional
    inverse_operation
    invoke_operation
    less_operation
    less_equal_operation
    literal_value
    linearmatrix
    linspace
    map_operation
    mul_operation
    not_equal_operation
    or_operation
    parallel_block_operation
    power_operation
    random
    random_distributions
    row_set
    row_slicing
    set_operation
    shuffle_operation
    slicing_operation
    square_root_operation
    store_operation
    sub_operation
    transpose_operation
    unary_minus_operation
    unary_not_operation
    vstack_operation
    while_operation
   )

if(PHYLANX_WITH_HIGHFIVE)
  set(tests ${tests}
        file_hdf5_primitives
     )
  set(file_hdf5_primitives_FLAGS COMPILE_FLAGS -DPHYLANX_HAVE_HIGHFIVE)
endif()

foreach(test ${tests})
  set(sources ${test}.cpp)

  source_group("Source Files" FILES ${sources})

  # add executable
  add_phylanx_executable(${test}_test
    SOURCES ${sources}
    ${${test}_FLAGS}
    EXCLUDE_FROM_ALL
    FOLDER "Tests/Unit/ExecutionTree/Primitives")

  add_phylanx_unit_test("primitives" ${test} ${${test}_PARAMETERS})

  add_phylanx_pseudo_target(tests.unit.execution_tree.primitives.${test})
  add_phylanx_pseudo_dependencies(tests.unit.execution_tree.primitives
    tests.unit.execution_tree.primitives.${test})
  add_phylanx_pseudo_dependencies(tests.unit.execution_tree.primitives.${test}
    ${test}_test_exe)

endforeach()

