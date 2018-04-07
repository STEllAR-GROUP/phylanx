# Copyright (c) 2017 Hartmut Kaiser
#
# Distributed under the Boost Software License, Version 1.0. (See accompanying
# file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

set(tests
    all_operation
    any_operation
    argmin
    argmax
    car_cdr_operation
    column_set
    column_slicing
    constant
    define_operation
    determinant
    diag_operation
    dot_operation
    cross_operation
    exponential_operation
    extract_shape
    file_primitives
    file_csv_primitives
    gradient_operation
    add_dimension
    hstack_operation
    identity
    inverse_operation
    invoke_operation
    literal_value
    linearmatrix
    linspace
    mean_operation
    power_operation
    random
    random_distributions
    range_operation
    row_set
    row_slicing
    set_operation
    shuffle_operation
    slicing_operation
    square_root_operation
    store_operation
    sum_operation
    transpose_operation
    vstack_operation
   )

if(PHYLANX_WITH_HIGHFIVE)
  set(tests ${tests}
        file_hdf5_primitives
     )
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

