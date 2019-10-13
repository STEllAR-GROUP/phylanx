// Copyright (c) 2018 Hartmut Kaiser
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/phylanx.hpp>

#include <hpx/hpx_main.hpp>
#include <hpx/include/lcos.hpp>
#include <hpx/testing.hpp>

#include <cstddef>
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
    return code.run().arg_;
}

///////////////////////////////////////////////////////////////////////////////
void test_cumsum(std::string const& code, std::string const& expected_str)
{
    HPX_TEST_EQ(compile_and_run(code), compile_and_run(expected_str));
}

///////////////////////////////////////////////////////////////////////////////
void test_cumsum_0d()
{
    test_cumsum("cumsum(42)", "[42]");
    test_cumsum("cumsum(42, 0)", "[42]");

    test_cumsum("cumsum(42.0)", "[42.0]");
    test_cumsum("cumsum(42.0, 0)", "[42.0]");

    test_cumsum(R"(cumsum(true, __arg(dtype, "bool")))", "hstack(list(true))");
    test_cumsum(R"(cumsum(42, __arg(dtype, "int")))", "[42]");
    test_cumsum(R"(cumsum(42, __arg(dtype, "float")))", "[42.0]");
}

void test_cumsum_1d()
{
    test_cumsum("cumsum([42])", "[42]");
    test_cumsum("cumsum([42], 0)", "[42]");

    test_cumsum(
        "cumsum([1, 2, 3, 4, 5, 6])", "[1, 3, 6, 10, 15, 21]");
    test_cumsum(
        "cumsum([1, 2, 3, 4, 5, 6], 0)", "[1, 3, 6, 10, 15, 21]");

    test_cumsum("cumsum([42.0])", "[42.0]");
    test_cumsum("cumsum([42.0], 0)", "[42.0]");

    test_cumsum(
        "cumsum([1.0, 2.0, 3.0, 4.0, 5.0, 6.0])",
        "[1.0, 3.0, 6.0, 10.0, 15.0, 21.0]");
    test_cumsum(
        "cumsum([1.0, 2.0, 3.0, 4.0, 5.0, 6.0], 0)",
        "[1.0, 3.0, 6.0, 10.0, 15.0, 21.0]");

    test_cumsum(R"(cumsum([42], __arg(dtype, "int")))", "[42]");
    test_cumsum(R"(cumsum([42], __arg(dtype, "int"), __arg(axis, 0)))", "[42]");

    test_cumsum(
        R"(cumsum([1, 2, 3, 4, 5, 6], __arg(dtype, "int")))",
        "[1, 3, 6, 10, 15, 21]");
    test_cumsum(
        R"(cumsum([1, 2, 3, 4, 5, 6], __arg(dtype, "int"), __arg(axis, 0)))",
        "[1, 3, 6, 10, 15, 21]");

    test_cumsum(R"(cumsum([42], __arg(dtype, "float")))", "[42.0]");
    test_cumsum(R"(cumsum([42], __arg(dtype, "float"), __arg(axis, 0)))",
        "[42.0]");

    test_cumsum(
        R"(cumsum([1, 2, 3, 4, 5, 6], __arg(dtype, "float")))",
        "[1.0, 3.0, 6.0, 10.0, 15.0, 21.0]");
    test_cumsum(
        R"(cumsum([1, 2, 3, 4, 5, 6], __arg(dtype, "float"), __arg(axis, 0)))",
        "[1.0, 3.0, 6.0, 10.0, 15.0, 21.0]");
}

void test_cumsum_2d()
{
    test_cumsum("cumsum([[42]])", "[42]");
    test_cumsum("cumsum([[42]], 0)", "[[42]]");
    test_cumsum("cumsum([[42]], 1)", "[[42]]");

    test_cumsum(
        "cumsum([[1, 2, 3], [4, 5, 6]])",
        "[1, 3, 6, 10, 15, 21]");
    test_cumsum(
        "cumsum([[1, 2, 3], [4, 5, 6]], 0)",
        "[[1, 2, 3], [5, 7, 9]]");
    test_cumsum(
        "cumsum([[1, 2, 3], [4, 5, 6]], 1)",
        "[[1, 3, 6], [4, 9, 15]]");

    test_cumsum("cumsum([[42.0]])", "[42.0]");
    test_cumsum("cumsum([[42.0]], 0)", "[[42.0]]");
    test_cumsum("cumsum([[42.0]], 1)", "[[42.0]]");

    test_cumsum(
        "cumsum([[1.0, 2.0, 3.0], [4.0, 5.0, 6.0]])",
        "[1.0, 3.0, 6.0, 10.0, 15.0, 21.0]");
    test_cumsum(
        "cumsum([[1.0, 2.0, 3.0], [4.0, 5.0, 6.0]], 0)",
        "[[1.0, 2.0, 3.0], [5.0, 7.0, 9.0]]");
    test_cumsum(
        "cumsum([[1.0, 2.0, 3.0], [4.0, 5.0, 6.0]], 1)",
        "[[1.0, 3.0, 6.0], [4.0, 9.0, 15.0]]");

    test_cumsum(R"(cumsum([[42]], __arg(dtype, "int")))", "[42]");
    test_cumsum(R"(cumsum([[42]], 0, __arg(dtype, "int")))", "[[42]]");
    test_cumsum(R"(cumsum([[42]], 1, __arg(dtype, "int")))", "[[42]]");

    test_cumsum(
        R"(cumsum([[1, 2, 3], [4, 5, 6]], __arg(dtype, "int")))",
        "[1, 3, 6, 10, 15, 21]");
    test_cumsum(
        R"(cumsum([[1, 2, 3], [4, 5, 6]], 0, __arg(dtype, "int")))",
        "[[1, 2, 3], [5, 7, 9]]");
    test_cumsum(
        R"(cumsum([[1, 2, 3], [4, 5, 6]], 1, __arg(dtype, "int")))",
        "[[1, 3, 6], [4, 9, 15]]");

    test_cumsum(R"(cumsum([[42]], __arg(dtype, "float")))", "[42.0]");
    test_cumsum(R"(cumsum([[42]], 0, __arg(dtype, "float")))", "[[42.0]]");
    test_cumsum(R"(cumsum([[42]], 1, __arg(dtype, "float")))", "[[42.0]]");

    test_cumsum(
        R"(cumsum([[1, 2, 3], [4, 5, 6]], __arg(dtype, "float")))",
        "[1.0, 3.0, 6.0, 10.0, 15.0, 21.0]");
    test_cumsum(
        R"(cumsum([[1, 2, 3], [4, 5, 6]], 0, __arg(dtype, "float")))",
        "[[1.0, 2.0, 3.0], [5.0, 7.0, 9.0]]");
    test_cumsum(
        R"(cumsum([[1, 2, 3], [4, 5, 6]], 1, __arg(dtype, "float")))",
        "[[1.0, 3.0, 6.0], [4.0, 9.0, 15.0]]");
}

void test_cumsum_3d()
{
    test_cumsum("cumsum([[[42]]])", "[42]");
    test_cumsum("cumsum([[[42]]], 0)", "[[[42]]]");
    test_cumsum("cumsum([[[42]]], 1)", "[[[42]]]");
    test_cumsum("cumsum([[[42]]], 2)", "[[[42]]]");

    test_cumsum(
        "cumsum([[[1, 2, 3], [4, 5, 6]]])", "[1, 3, 6, 10, 15, 21]");
    test_cumsum(
        "cumsum([[[1, 2, 3], [4, 5, 6]]], 0)", "[[[1, 2, 3], [4, 5, 6]]]");
    test_cumsum(
        "cumsum([[[1, 2, 3], [4, 5, 6]]], 1)", "[[[1, 2, 3], [5, 7, 9]]]");
    test_cumsum(
        "cumsum([[[1, 2, 3], [4, 5, 6]]], 2)", "[[[1, 3, 6], [4, 9, 15]]]");

    test_cumsum("cumsum([[[42.]]])", "[42.]");
    test_cumsum("cumsum([[[42.]]], 0)", "[[[42.]]]");
    test_cumsum("cumsum([[[42.]]], 1)", "[[[42.]]]");
    test_cumsum("cumsum([[[42.]]], 2)", "[[[42.]]]");

    test_cumsum("cumsum([[[1., 2., 3.], [4., 5., 6.]]])",
        "[1., 3., 6., 10., 15., 21.]");
    test_cumsum("cumsum([[[1., 2., 3.], [4., 5., 6.]]], 0)",
        "[[[1., 2., 3.], [4., 5., 6.]]]");
    test_cumsum("cumsum([[[1., 2., 3.], [4., 5., 6.]]], 1)",
        "[[[1., 2., 3.], [5., 7., 9.]]]");
    test_cumsum("cumsum([[[1., 2., 3.], [4., 5., 6.]]], 2)",
        "[[[1., 3., 6.], [4., 9., 15.]]]");

    test_cumsum(R"(cumsum([[[42.]]], __arg(dtype, "int")))", "[42]");
    test_cumsum(R"(cumsum([[[42.]]], 0, __arg(dtype, "int")))", "[[[42]]]");
    test_cumsum(R"(cumsum([[[42.]]], 1, __arg(dtype, "int")))", "[[[42]]]");
    test_cumsum(R"(cumsum([[[42.]]], 2, __arg(dtype, "int")))", "[[[42]]]");

    test_cumsum(
        R"(cumsum([[[1., 2., 3.], [4., 5., 6.]]], __arg(dtype, "int")))",
        "[1, 3, 6, 10, 15, 21]");
    test_cumsum(
        R"(cumsum([[[1., 2., 3.], [4., 5., 6.]]], 0, __arg(dtype, "int")))",
        "[[[1, 2, 3], [4, 5, 6]]]");
    test_cumsum(
        R"(cumsum([[[1., 2., 3.], [4., 5., 6.]]], 1, __arg(dtype, "int")))",
        "[[[1, 2, 3], [5, 7, 9]]]");
    test_cumsum(
        R"(cumsum([[[1., 2., 3.], [4., 5., 6.]]], 2, __arg(dtype, "int")))",
        "[[[1, 3, 6], [4, 9, 15]]]");

    test_cumsum(R"(cumsum([[[42]]], __arg(dtype, "float")))", "[42.]");
    test_cumsum(R"(cumsum([[[42]]], 0, __arg(dtype, "float")))", "[[[42.]]]");
    test_cumsum(R"(cumsum([[[42]]], 1, __arg(dtype, "float")))", "[[[42.]]]");
    test_cumsum(R"(cumsum([[[42]]], 2, __arg(dtype, "float")))", "[[[42.]]]");

    test_cumsum(
        R"(cumsum([[[1, 2, 3], [4, 5, 6]]], __arg(dtype, "float")))",
        "[1., 3., 6., 10., 15., 21.]");
    test_cumsum(
        R"(cumsum([[[1, 2, 3], [4, 5, 6]]], 0, __arg(dtype, "float")))",
        "[[[1., 2., 3.], [4., 5., 6.]]]");
    test_cumsum(
        R"(cumsum([[[1, 2, 3], [4, 5, 6]]], 1, __arg(dtype, "float")))",
        "[[[1., 2., 3.], [5., 7., 9.]]]");
    test_cumsum(
        R"(cumsum([[[1, 2, 3], [4, 5, 6]]], 2, __arg(dtype, "float")))",
        "[[[1., 3., 6.], [4., 9., 15.]]]");
}

int main(int argc, char* argv[])
{
    test_cumsum_0d();
    test_cumsum_1d();
    test_cumsum_2d();
    test_cumsum_3d();

    return hpx::util::report_errors();
}
