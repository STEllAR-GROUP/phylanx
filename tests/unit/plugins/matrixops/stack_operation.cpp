// Copyright (c) 2019 Shahrzad Shirzad
// Copyright (c) 2019 Hartmut Kaiser
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/phylanx.hpp>

#include <hpx/hpx_main.hpp>
#include <hpx/testing.hpp>

#include <cstdint>
#include <string>
#include <utility>

///////////////////////////////////////////////////////////////////////////////
phylanx::execution_tree::primitive_argument_type compile_and_run(
    std::string const& codestr)
{
    phylanx::execution_tree::compiler::function_list snippets;
    phylanx::execution_tree::compiler::environment env =
        phylanx::execution_tree::compiler::default_environment();

    auto const& code = phylanx::execution_tree::compile(codestr, snippets, env);
    return code.run().arg_;
}

///////////////////////////////////////////////////////////////////////////////
void test_stack_operation_operation(std::string const& code,
    std::string const& expected_str)
{
    HPX_TEST_EQ(compile_and_run(code), compile_and_run(expected_str));
}

///////////////////////////////////////////////////////////////////////////////
int main(int argc, char* argv[])
{
    test_stack_operation_operation("stack(list([1., 2.]))", "[1., 2.]");
    test_stack_operation_operation("stack(list([1., 2.]), 0)", "[1., 2.]");
    test_stack_operation_operation("stack(list([1., 2.]), -1)", "[1., 2.]");
    test_stack_operation_operation(
            "stack(list([[1., 2., 3.], [4., 5., 6.]]))",
            "[[1., 2., 3.],[4., 5., 6.]]");
    test_stack_operation_operation(
            "stack(list([[1., 2., 3.], [4., 5., 6.]]), __arg(axis, 0))",
            "[[1., 2., 3.],[4., 5., 6.]]");
    test_stack_operation_operation(
            "stack(list([[1., 2., 3.], [4., 5., 6.]]), __arg(axis, 1))",
            "[[1., 4.], [2., 5.], [3., 6.]]");
    test_stack_operation_operation(
            "stack(list([[1., 2., 3.], [4., 5., 6.]]), __arg(axis, -1))",
            "[[1., 4.], [2., 5.], [3., 6.]]");
    test_stack_operation_operation("stack(list(1., 2.))", "[1., 2.]");
    test_stack_operation_operation("stack(list(1., 2.), 0)", "[1., 2.]");
    test_stack_operation_operation(
        "stack(list([1., 2., 3.], [4., 5., 6.]))",
        "[[1., 2., 3.],[4., 5., 6.]]");
    test_stack_operation_operation(
        "stack(list([1., 2., 3.], [4., 5., 6.]), __arg(axis, 0))",
        "[[1., 2., 3.],[4., 5., 6.]]");
    test_stack_operation_operation(
        "stack(list([1., 2., 3.], [4., 5., 6.]), __arg(axis, 1))",
        "[[1., 4.], [2., 5.],[3., 6.]]");

    test_stack_operation_operation(
        "stack(list([[[1., 2., 3.]], [[4., 5., 6.]]]))",
        "[[[1., 2., 3.]], [[4., 5., 6.]]]");
    test_stack_operation_operation(
        "stack(list([[[1., 2., 3.]], [[4., 5., 6.]]]), 0)",
        "[[[1., 2., 3.]], [[4., 5., 6.]]]");
    test_stack_operation_operation(
        "stack(list([[[1., 2., 3.]], [[4., 5., 6.]]]), -1)",
        "[[[1., 4.], [2., 5.],[3., 6.]]]");
    test_stack_operation_operation(
        "stack(list([[[1., 2., 3.]], [[4., 5., 6.]]]), 2)",
        "[[[1., 4.], [2., 5.],[3., 6.]]]");
    test_stack_operation_operation(
        "stack(list([[[1., 2., 3.]], [[4., 5., 6.]]]), 1)",
        "[[[1., 2., 3.], [4., 5., 6.]]]");
    test_stack_operation_operation(
        "stack(list([[[1., 2., 3.], [4., 5., 6.]], "
            "[[7., 8., 9.], [10., 11., 12.]]]))",
        "[[[1., 2., 3.], [4., 5., 6.]], [[7., 8., 9.], [10., 11., 12.]]]");
    test_stack_operation_operation(
        "stack(list([[[1., 2., 3.], [4., 5., 6.]], "
            "[[7., 8., 9.], [10., 11., 12.]]]), 0)",
        "[[[1., 2., 3.], [4., 5., 6.]], [[7., 8., 9.], [10., 11., 12.]]]");
    test_stack_operation_operation(
        "stack(list([[[1., 2., 3.], [4., 5., 6.]], "
            "[[7., 8., 9.], [10., 11., 12.]]]), -1)",
        "[[[1., 7.], [2., 8.], [3., 9.]], [[4., 10.], [5., 11.], [6., 12.]]]");
    test_stack_operation_operation(
        "stack(list([[[1., 2., 3.], [4., 5., 6.]], "
            "[[7., 8., 9.], [10., 11., 12.]]]), 2)",
        "[[[1., 7.], [2., 8.], [3., 9.]], [[4., 10.], [5., 11.], [6., 12.]]]");
    test_stack_operation_operation(
        "stack(list([[[1., 2., 3.], [4., 5., 6.]], "
            "[[7., 8., 9.], [10., 11., 12.]]]), 1)",
        "[[[1., 2., 3.], [7., 8., 9.]], [[4., 5., 6.], [10., 11., 12.]]]");
    test_stack_operation_operation(
        "stack(list([[1., 2., 3.]], [[4., 5., 6.]]))",
        "[[[1., 2., 3.]], [[4., 5., 6.]]]");
    test_stack_operation_operation(
        "stack(list([[1., 2., 3.]], [[4., 5., 6.]]), 0)",
        "[[[1., 2., 3.]], [[4., 5., 6.]]]");
    test_stack_operation_operation(
        "stack(list([[1., 2., 3.]], [[4., 5., 6.]]), 1)",
        "[[[1., 2., 3.], [4., 5., 6.]]]");
    test_stack_operation_operation(
        "stack(list([[1., 2.], [3., 4.]], [[5., 6.], [7., 8.]]))",
        "[[[1., 2.], [3., 4.]], [[5., 6.], [7., 8.]]]");
    test_stack_operation_operation(
        "stack(list([[1., 2.], [3., 4.]], [[5., 6.], [7., 8.]]), 0)",
        "[[[1., 2.], [3., 4.]], [[5., 6.], [7., 8.]]]");
    test_stack_operation_operation(
        "stack(list([[1., 2.], [3., 4.]], [[5., 6.], [7., 8.]]), 1)",
        "[[[1., 2.], [5., 6.]], [[3., 4.], [7., 8.]]]");
    test_stack_operation_operation(
        "stack(list([[1., 2.], [3., 4.], [5., 6.]], "
            "[[7., 8.], [9., 10.], [11., 12.]]))",
        "[[[1., 2.], [3., 4.], [5., 6.]], [[7., 8.], [9., 10.], [11., 12.]]]");
    test_stack_operation_operation(
        "stack(list([[1., 2.], [3., 4.], [5., 6.]], "
            "[[7., 8.], [9., 10.], [11., 12.]]), __arg(axis, 0))",
        "[[[1., 2.], [3., 4.], [5., 6.]], [[7., 8.], [9., 10.], [11., 12.]]]");
    test_stack_operation_operation(
        "stack(list([[1., 2.], [3., 4.], [5., 6.]], "
            "[[7., 8.], [9., 10.], [11., 12.]]), __arg(axis, 1))",
        "[[[1., 2.], [7., 8.]], [[3., 4.], [9., 10.]], "
            "[[5., 6.], [11., 12.]]]");

    return hpx::util::report_errors();
}
