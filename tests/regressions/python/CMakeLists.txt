# Copyright (c) 2017-2019 Hartmut Kaiser
#
# Distributed under the Boost Software License, Version 1.0. (See accompanying
# file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

set(tests
    364_local_variables
    466_compiler_problems
    508_unknown_named_argument
    522_multidimension_np_arrays
    530_augmented_assignment_indexed_lhs
    596_list_arrays
    624_advanced_indexing
    635_arange_args
    636_boolean_array
    640_function_argument
    651_vstack_dtype
    794_3d_array
    805_lazy_variable
    805_two_variables
    808_print_twice
    815_plain_return
    826_default_args
    827_random_0dim
    832_performance_data
    837_none_default_argument
    846_if_expression
    852_concatenate_list
    862_var_std_two_axis
    866_dynamic_dtype
    874_0d_shape
    886_notnone
    896_return_none
    903_variable_conversion
    903_variable_dtype
    904_pass_decorator
    905_call_python_snippet
    911_list_comprehension
    912_zip
    925_none_dtype
    933_arange
    933_eval_wrapper
    936_arithmetic_operations
    942_lazy_fmap
    956_named_argument_underscore
    962_broadcast_logical_operations
    966_named_arguments
    979_lazy_compilation
    979_lazy_compilation_with_init
    1052_variable_set_dtype
    1054_variable_shape
    1056_client_base
    1104_no_arg_lambda
    1170_no_error
    1206_improper_defaults
    1224_dictionary_len
    1225_iterate_dictionary
    1230_assign_argument_dictionary
    1235_print
    1236_store_failure
    1242_list_comparison
    1244_function_call
    1258_randn_missing
    1255_assign_string
    1266_sum_axis_argument
    1267_random_integers
    1267_random_sample
    1279_global_keyword
    1280_hstack_problem
    1281_hostname
    1301_return_tuple
    1303_iterate_over_array
    array_len_494
    array_shape_486
    array_subscript_403
    broadcasting_rules_410
    create_list_409
    empty_list_509
    empty_list_510
    exception_swallowed_369
    for_map_516
    lambda_492
    list_iteration_524
    list_iter_space_429
    list_slice_assign_528
    name_constants_411
    no_parens_decorator_407
    np_sum_489
    passing_compiler_state_453
    reassign_512
    zero_dimensional_array_502
   )

foreach(test ${tests})
  set(script ${test}.py)

  add_phylanx_python_regression_test("python" ${test}
    SCRIPT ${script}
    FOLDER "Tests/Python/Regression"
    DEPENDS phylanx_py python_setup
    WORKING_DIRECTORY ${PHYLANX_PYTHON_EXTENSION_LOCATION}
    ENVIRONMENT "PYTHONPATH=${PHYLANX_PYTHON_EXTENSION_LOCATION}:$ENV{PYTHONPATH}")

  add_phylanx_pseudo_target(tests.regressions.python_dir.${test}_py)
  add_phylanx_pseudo_dependencies(
    tests.regressions.python_dir tests.regressions.python_dir.${test}_py)
  add_phylanx_pseudo_dependencies(
    tests.regressions.python_dir.${test}_py ${test}_test_py)

endforeach()
