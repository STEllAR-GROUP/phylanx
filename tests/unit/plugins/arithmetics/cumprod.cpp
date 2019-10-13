// Copyright (c) 2018 Hartmut Kaiser
// Copyright (c) 2019 Bibek Wagle
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
void test_cumprod(std::string const& code, std::string const& expected_str)
{
    HPX_TEST_EQ(compile_and_run(code), compile_and_run(expected_str));
}

///////////////////////////////////////////////////////////////////////////////
void test_cumprod_0d()
{
    test_cumprod("cumprod(42)", "[42]");
    test_cumprod("cumprod(42, 0)", "[42]");

    test_cumprod("cumprod(42.0)", "[42.0]");
    test_cumprod("cumprod(42.0, 0)", "[42.0]");

    test_cumprod(R"(cumprod(true, __arg(dtype, "bool")))", "hstack(list(true))");
    test_cumprod(R"(cumprod(42, __arg(dtype, "int")))", "[42]");
    test_cumprod(R"(cumprod(42, __arg(dtype, "float")))", "[42.0]");
}

void test_cumprod_1d()
{
    test_cumprod("cumprod([42])", "[42]");
    test_cumprod("cumprod([42], 0)", "[42]");

    test_cumprod(
        "cumprod([1, 2, 3, 4, 5, 6])", "[1, 2, 6, 24, 120, 720]");
    test_cumprod(
        "cumprod([1, 2, 3, 4, 5, 6], 0)", "[1, 2, 6, 24, 120, 720]");

    test_cumprod("cumprod([42.0])", "[42.0]");
    test_cumprod("cumprod([42.0], 0)", "[42.0]");

    test_cumprod(
        "cumprod([1.0, 2.0, 3.0, 4.0, 5.0, 6.0])",
        "[1.0, 2.0, 6.0, 24.0, 120.0, 720.0]");
    test_cumprod(
        "cumprod([1.0, 2.0, 3.0, 4.0, 5.0, 6.0], 0)",
        "[1.0, 2.0, 6.0, 24.0, 120.0, 720.0]");

    test_cumprod(R"(cumprod([42], __arg(dtype, "int")))", "[42]");
    test_cumprod(R"(cumprod([42], 0, __arg(dtype, "int")))", "[42]");

    test_cumprod(R"(cumprod([1, 2, 3, 4, 5, 6], __arg(dtype, "int")))",
        "[1, 2, 6, 24, 120, 720]");
    test_cumprod(R"(cumprod([1, 2, 3, 4, 5, 6], 0, __arg(dtype, "int")))",
        "[1, 2, 6, 24, 120, 720]");

    test_cumprod(R"(cumprod([42], __arg(dtype, "float")))", "[42.0]");
    test_cumprod(R"(cumprod([42], 0, __arg(dtype, "float")))", "[42.0]");

    test_cumprod(R"(cumprod([1, 2, 3, 4, 5, 6], __arg(dtype, "float")))",
        "[1.0, 2.0, 6.0, 24.0, 120.0, 720.0]");
    test_cumprod(R"(cumprod([1, 2, 3, 4, 5, 6], 0, __arg(dtype, "float")))",
        "[1.0, 2.0, 6.0, 24.0, 120.0, 720.0]");
}

void test_cumprod_2d()
{
    test_cumprod("cumprod([[42]])", "[42]");
    test_cumprod("cumprod([[42]], 0)", "[[42]]");
    test_cumprod("cumprod([[42]], 1)", "[[42]]");

    test_cumprod(
        "cumprod([[1, 2, 3], [4, 5, 6]])",
        "[1, 2, 6, 24, 120, 720]");
    test_cumprod(
        "cumprod([[1, 2, 3], [4, 5, 6]], 0)",
        "[[1, 2, 3], [4, 10, 18]]");
    test_cumprod(
        "cumprod([[1, 2, 3], [4, 5, 6]], 1)",
        "[[1, 2, 6], [4, 20, 120]]");

    test_cumprod("cumprod([[42.0]])", "[42.0]");
    test_cumprod("cumprod([[42.0]], 0)", "[[42.0]]");
    test_cumprod("cumprod([[42.0]], 1)", "[[42.0]]");

    test_cumprod(
        "cumprod([[1.0, 2.0, 3.0], [4.0, 5.0, 6.0]])",
        "[1.0, 2.0, 6.0, 24.0, 120.0, 720.0]");
    test_cumprod(
        "cumprod([[1.0, 2.0, 3.0], [4.0, 5.0, 6.0]], 0)",
        "[[1.0, 2.0, 3.0], [4.0, 10.0, 18.0]]");
    test_cumprod(
        "cumprod([[1.0, 2.0, 3.0], [4.0, 5.0, 6.0]], 1)",
        "[[1.0, 2.0, 6.0], [4.0, 20.0, 120.0]]");

    test_cumprod(R"(cumprod([[42]], __arg(dtype, "int")))", "[42]");
    test_cumprod(R"(cumprod([[42]], 0, __arg(dtype, "int")))", "[[42]]");
    test_cumprod(R"(cumprod([[42]], 1, __arg(dtype, "int")))", "[[42]]");

    test_cumprod(
        R"(cumprod([[1, 2, 3], [4, 5, 6]], __arg(dtype, "int")))",
        "[1, 2, 6, 24, 120, 720]");
    test_cumprod(
        R"(cumprod([[1, 2, 3], [4, 5, 6]], 0, __arg(dtype, "int")))",
        "[[1, 2, 3], [4, 10, 18]]");
    test_cumprod(
        R"(cumprod([[1, 2, 3], [4, 5, 6]], 1, __arg(dtype, "int")))",
        "[[1, 2, 6], [4, 20, 120]]");

    test_cumprod(R"(cumprod([[42]], __arg(dtype, "float")))", "[42.0]");
    test_cumprod(R"(cumprod([[42]], 0, __arg(dtype, "float")))", "[[42.0]]");
    test_cumprod(R"(cumprod([[42]], 1, __arg(dtype, "float")))", "[[42.0]]");

    test_cumprod(
        R"(cumprod([[1, 2, 3], [4, 5, 6]], __arg(dtype, "float")))",
        "[1.0, 2.0, 6.0, 24.0, 120.0, 720.0]");
    test_cumprod(
        R"(cumprod([[1, 2, 3], [4, 5, 6]], 0, __arg(dtype, "float")))",
        "[[1.0, 2.0, 3.0], [4.0, 10.0, 18.0]]");
    test_cumprod(
        R"(cumprod([[1, 2, 3], [4, 5, 6]], 1, __arg(dtype, "float")))",
        "[[1.0, 2.0, 6.0], [4.0, 20.0, 120.0]]");
}

void test_cumprod_3d()
{
    test_cumprod("cumprod([[[42]]])", "[42]");
    test_cumprod("cumprod([[[42]]], 0)", "[[[42]]]");
    test_cumprod("cumprod([[[42]]], 1)", "[[[42]]]");
    test_cumprod("cumprod([[[42]]], 2)", "[[[42]]]");

    test_cumprod(
        "cumprod([[[1, 2, 3], [4, 5, 6]]])", "[1, 2, 6, 24, 120, 720]");
    test_cumprod(
        "cumprod([[[1, 2, 3], [4, 5, 6]]], 0)", "[[[1, 2, 3], [4, 5, 6]]]");
    test_cumprod(
        "cumprod([[[1, 2, 3], [4, 5, 6]]], 1)", "[[[1, 2, 3], [4, 10, 18]]]");
    test_cumprod(
        "cumprod([[[1, 2, 3], [4, 5, 6]]], 2)", "[[[1, 2, 6], [4, 20, 120]]]");

    test_cumprod("cumprod([[[42.]]])", "[42.]");
    test_cumprod("cumprod([[[42.]]], 0)", "[[[42.]]]");
    test_cumprod("cumprod([[[42.]]], 1)", "[[[42.]]]");
    test_cumprod("cumprod([[[42.]]], 2)", "[[[42.]]]");

    test_cumprod("cumprod([[[1., 2., 3.], [4., 5., 6.]]])",
        "[1., 2., 6., 24., 120., 720.]");
    test_cumprod("cumprod([[[1., 2., 3.], [4., 5., 6.]]], 0)",
        "[[[1., 2., 3.], [4., 5., 6.]]]");
    test_cumprod("cumprod([[[1., 2., 3.], [4., 5., 6.]]], 1)",
        "[[[1., 2., 3.], [4., 10., 18.]]]");
    test_cumprod("cumprod([[[1., 2., 3.], [4., 5., 6.]]], 2)",
        "[[[1., 2., 6.], [4., 20., 120.]]]");

    test_cumprod(R"(cumprod([[[42.]]], __arg(dtype, "int")))", "[42]");
    test_cumprod(R"(cumprod([[[42.]]], 0, __arg(dtype, "int")))", "[[[42]]]");
    test_cumprod(R"(cumprod([[[42.]]], 1, __arg(dtype, "int")))", "[[[42]]]");
    test_cumprod(R"(cumprod([[[42.]]], 2, __arg(dtype, "int")))", "[[[42]]]");

    test_cumprod(
        R"(cumprod([[[1., 2., 3.], [4., 5., 6.]]], __arg(dtype, "int")))",
        "[1, 2, 6, 24, 120, 720]");
    test_cumprod(
        R"(cumprod([[[1., 2., 3.], [4., 5., 6.]]], 0, __arg(dtype, "int")))",
        "[[[1, 2, 3], [4, 5, 6]]]");
    test_cumprod(
        R"(cumprod([[[1., 2., 3.], [4., 5., 6.]]], 1, __arg(dtype, "int")))",
        "[[[1, 2, 3], [4, 10, 18]]]");
    test_cumprod(
        R"(cumprod([[[1., 2., 3.], [4., 5., 6.]]], 2, __arg(dtype, "int")))",
        "[[[1, 2, 6], [4, 20, 120]]]");

    test_cumprod(R"(cumprod([[[42]]], __arg(dtype, "float")))", "[42.]");
    test_cumprod(R"(cumprod([[[42]]], 0, __arg(dtype, "float")))", "[[[42.]]]");
    test_cumprod(R"(cumprod([[[42]]], 1, __arg(dtype, "float")))", "[[[42.]]]");
    test_cumprod(R"(cumprod([[[42]]], 2, __arg(dtype, "float")))", "[[[42.]]]");

    test_cumprod(
        R"(cumprod([[[1, 2, 3], [4, 5, 6]]], __arg(dtype, "float")))",
        "[1., 2., 6., 24., 120., 720.]");
    test_cumprod(
        R"(cumprod([[[1, 2, 3], [4, 5, 6]]], 0, __arg(dtype, "float")))",
        "[[[1., 2., 3.], [4., 5., 6.]]]");
    test_cumprod(
        R"(cumprod([[[1, 2, 3], [4, 5, 6]]], 1, __arg(dtype, "float")))",
        "[[[1., 2., 3.], [4., 10., 18.]]]");
    test_cumprod(
        R"(cumprod([[[1, 2, 3], [4, 5, 6]]], 2, __arg(dtype, "float")))",
        "[[[1., 2., 6.], [4., 20., 120.]]]");
}

int main(int argc, char* argv[])
{
    test_cumprod_0d();
    test_cumprod_1d();
    test_cumprod_2d();
    test_cumprod_3d();

    return hpx::util::report_errors();
}
