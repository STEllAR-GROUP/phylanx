// Copyright (c) 2017-2018 Hartmut Kaiser
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/phylanx.hpp>

#include <hpx/hpx_main.hpp>
#include <hpx/include/lcos.hpp>
#include <hpx/testing.hpp>

#include <iostream>
#include <string>
#include <utility>
#include <vector>

///////////////////////////////////////////////////////////////////////////////
phylanx::execution_tree::primitive_argument_type compile_and_run(
    std::string const& codestr)
{
    phylanx::execution_tree::compiler::function_list snippets;
    phylanx::execution_tree::compiler::environment env =
        phylanx::execution_tree::compiler::default_environment();

    auto const& code = phylanx::execution_tree::compile(codestr, snippets, env);
    return code.run();
}

void test_count_std_operation(std::string const& code,
    std::string const& expected_str)
{
    HPX_TEST_EQ(compile_and_run(code), compile_and_run(expected_str));
}

///////////////////////////////////////////////////////////////////////////////
int main(int argc, char* argv[])
{
    // scalars
    test_count_std_operation("std(1.0)", "0.0");
    test_count_std_operation("std(1)", "0.0");
    test_count_std_operation("std__float(1)", "0.0");

    // vectors
    test_count_std_operation("std([1.0, 2.0, 3.0, 4.0])", "sqrt(1.25)");
    test_count_std_operation("std__float([1, 2, 3, 4])", "sqrt(1.25)");

    test_count_std_operation("std([1.0, 2.0, 3.0, 4.0], 0)", "sqrt(1.25)");
    test_count_std_operation("std([1.0, 2.0, 3.0, 4.0], -1)", "sqrt(1.25)");

    test_count_std_operation(
        "std([1.0, 2.0, 3.0, 4.0], nil, false)", "sqrt(1.25)");
    test_count_std_operation(
        "std([1.0, 2.0, 3.0, 4.0], nil, true)", "hstack(list(sqrt(1.25)))");
    test_count_std_operation(
        "std([1.0, 2.0, 3.0, 4.0], 0, false)", "sqrt(1.25)");
    test_count_std_operation(
        "std([1.0, 2.0, 3.0, 4.0], 0, true)", "hstack(list(sqrt(1.25)))");
    test_count_std_operation(
        "std([1.0, 2.0, 3.0, 4.0], -1, false)", "sqrt(1.25)");
    test_count_std_operation(
        "std([1.0, 2.0, 3.0, 4.0], -1, true)", "hstack(list(sqrt(1.25)))");

    // matrices
    test_count_std_operation(
        "std([[1.0, 2.0], [3.0, 4.0]])", "sqrt(1.25)");

    test_count_std_operation(
        "std([[1.0, 2.0], [3.0, 4.0]], nil, false)",
        "sqrt(1.25)");
    test_count_std_operation(
        "std([[1.0, 2.0], [3.0, 4.0]], nil, true)",
        "vstack(list(hstack(list(sqrt(1.25)))))");

    test_count_std_operation(
        "std([[1.0, 2.0], [3.0, 4.0]], 0)",
        "[1.0, 1.0]");
    test_count_std_operation(
        "std([[1.0, 2.0], [3.0, 4.0]], 1)",
        "[0.5, 0.5]");

    test_count_std_operation(
        "std([[1.0, 2.0], [3.0, 4.0]], 0, false)",
        "[1.0, 1.0]");
    test_count_std_operation(
        "std([[1.0, 2.0], [3.0, 4.0]], 1, false)",
        "[0.5, 0.5]");

    test_count_std_operation(
        "std([[1.0, 2.0], [3.0, 4.0]], 0, true)",
        "[[1.0, 1.0]]");
    test_count_std_operation(
        "std([[1.0, 2.0], [3.0, 4.0]], 1, true)",
        "[[0.5], [0.5]]");

    // tensors
    test_count_std_operation(
        "std([[[1.0, 2.0], [3.0, 4.0]], [[4.0, 3.0], [2.0, 1.0]]])",
        "sqrt(1.25)");

    test_count_std_operation(
        "std([[[1.0, 2.0], [3.0, 4.0]], [[4.0, 3.0], [2.0, 1.0]]], nil, false)",
        "sqrt(1.25)");
    test_count_std_operation(
        "std([[[1.0, 2.0], [3.0, 4.0]], [[4.0, 3.0], [2.0, 1.0]]], nil, true)",
        "dstack(list(vstack(list(hstack(list(sqrt(1.25)))))))");

    test_count_std_operation(
        "std([[[1.0, 2.0], [3.0, 4.0]], [[4.0, 3.0], [2.0, 1.0]]], 0)",
        "[[1.5, 0.5], [0.5, 1.5]]");
    test_count_std_operation(
        "std([[[1.0, 2.0], [3.0, 4.0]], [[4.0, 3.0], [2.0, 1.0]]], 1)",
        "[[1.0, 1.0], [1.0, 1.0]]");
    test_count_std_operation(
        "std([[[1.0, 2.0], [3.0, 4.0]], [[4.0, 3.0], [2.0, 1.0]]], 2)",
        "[[0.5, 0.5], [0.5, 0.5]]");

    test_count_std_operation(
        "std([[[1.0, 2.0], [3.0, 4.0]], [[4.0, 3.0], [2.0, 1.0]]], 0, false)",
        "[[1.5, 0.5], [0.5, 1.5]]");
    test_count_std_operation(
        "std([[[1.0, 2.0], [3.0, 4.0]], [[4.0, 3.0], [2.0, 1.0]]], 1, false)",
        "[[1.0, 1.0], [1.0, 1.0]]");
    test_count_std_operation(
        "std([[[1.0, 2.0], [3.0, 4.0]], [[4.0, 3.0], [2.0, 1.0]]], 2, false)",
        "[[0.5, 0.5], [0.5, 0.5]]");

    test_count_std_operation(
        "std([[[1.0, 2.0], [3.0, 4.0]], [[4.0, 3.0], [2.0, 1.0]]], 0, true)",
        "[[[1.5, 0.5], [0.5, 1.5]]]");
    test_count_std_operation(
        "std([[[1.0, 2.0], [3.0, 4.0]], [[4.0, 3.0], [2.0, 1.0]]], 1, true)",
        "[[[1.0, 1.0]], [[1.0, 1.0]]]");
    test_count_std_operation(
        "std([[[1.0, 2.0], [3.0, 4.0]], [[4.0, 3.0], [2.0, 1.0]]], 2, true)",
        "[[[0.5], [0.5]], [[0.5], [0.5]]]");

    return hpx::util::report_errors();
}
