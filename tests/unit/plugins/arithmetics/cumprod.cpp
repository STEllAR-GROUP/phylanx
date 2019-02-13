// Copyright (c) 2018 Hartmut Kaiser
// Copyright (c) 2019 Bibek Wagle
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/phylanx.hpp>

#include <hpx/hpx_main.hpp>
#include <hpx/include/lcos.hpp>
#include <hpx/util/lightweight_test.hpp>

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
    return code.run();
}

///////////////////////////////////////////////////////////////////////////////
void test_cumprod(std::string const& code, std::string const& expected_str)
{
    HPX_TEST_EQ(compile_and_run(code), compile_and_run(expected_str));
}

///////////////////////////////////////////////////////////////////////////////
void test_cumprod_0d()
{
    test_cumprod("cumprod(42)", "hstack(42)");
    test_cumprod("cumprod(42, 0)", "hstack(42)");

    test_cumprod("cumprod(42.0)", "hstack(42.0)");
    test_cumprod("cumprod(42.0, 0)", "hstack(42.0)");

    test_cumprod("cumprod__bool(true)", "hstack(true)");
    test_cumprod("cumprod__int(42)", "hstack(42)");
    test_cumprod("cumprod__float(42)", "hstack(42.0)");
}

void test_cumprod_1d()
{
    test_cumprod("cumprod(hstack(42))", "hstack(42)");
    test_cumprod("cumprod(hstack(42), 0)", "hstack(42)");

    test_cumprod(
        "cumprod(hstack(1, 2, 3, 4, 5, 6))", "hstack(1, 2, 6, 24, 120, 720)");
    test_cumprod(
        "cumprod(hstack(1, 2, 3, 4, 5, 6), 0)", "hstack(1, 2, 6, 24, 120, 720)");

    test_cumprod("cumprod(hstack(42.0))", "hstack(42.0)");
    test_cumprod("cumprod(hstack(42.0), 0)", "hstack(42.0)");

    test_cumprod(
        "cumprod(hstack(1.0, 2.0, 3.0, 4.0, 5.0, 6.0))",
        "hstack(1.0, 2.0, 6.0, 24.0, 120.0, 720.0)");
    test_cumprod(
        "cumprod(hstack(1.0, 2.0, 3.0, 4.0, 5.0, 6.0), 0)",
        "hstack(1.0, 2.0, 6.0, 24.0, 120.0, 720.0)");

    test_cumprod("cumprod__int(hstack(42))", "hstack(42)");
    test_cumprod("cumprod__int(hstack(42), 0)", "hstack(42)");

    test_cumprod("cumprod__int(hstack(1, 2, 3, 4, 5, 6))",
        "hstack(1, 2, 6, 24, 120, 720)");
    test_cumprod("cumprod__int(hstack(1, 2, 3, 4, 5, 6), 0)",
        "hstack(1, 2, 6, 24, 120, 720)");

    test_cumprod("cumprod__float(hstack(42))", "hstack(42.0)");
    test_cumprod("cumprod__float(hstack(42), 0)", "hstack(42.0)");

    test_cumprod("cumprod__float(hstack(1, 2, 3, 4, 5, 6))",
        "hstack(1.0, 2.0, 6.0, 24.0, 120.0, 720.0)");
    test_cumprod("cumprod__float(hstack(1, 2, 3, 4, 5, 6), 0)",
        "hstack(1.0, 2.0, 6.0, 24.0, 120.0, 720.0)");
}

void test_cumprod_2d()
{
    test_cumprod("cumprod(vstack(hstack(42)))", "hstack(42)");
    test_cumprod("cumprod(vstack(hstack(42)), 0)", "vstack(hstack(42))");
    test_cumprod("cumprod(vstack(hstack(42)), 1)", "vstack(hstack(42))");

    test_cumprod(
        "cumprod(vstack(hstack(1, 2, 3), hstack(4, 5, 6)))",
        "hstack(1, 2, 6, 24, 120, 720)");
    test_cumprod(
        "cumprod(vstack(hstack(1, 2, 3), hstack(4, 5, 6)), 0)",
        "vstack(hstack(1, 2, 3), hstack(4, 10, 18))");
    test_cumprod(
        "cumprod(vstack(hstack(1, 2, 3), hstack(4, 5, 6)), 1)",
        "vstack(hstack(1, 2, 6), hstack(4, 20, 120))");

    test_cumprod("cumprod(vstack(hstack(42.0)))", "hstack(42.0)");
    test_cumprod("cumprod(vstack(hstack(42.0)), 0)", "vstack(hstack(42.0))");
    test_cumprod("cumprod(vstack(hstack(42.0)), 1)", "vstack(hstack(42.0))");

    test_cumprod(
        "cumprod(vstack(hstack(1.0, 2.0, 3.0), hstack(4.0, 5.0, 6.0)))",
        "hstack(1.0, 2.0, 6.0, 24.0, 120.0, 720.0)");
    test_cumprod(
        "cumprod(vstack(hstack(1.0, 2.0, 3.0), hstack(4.0, 5.0, 6.0)), 0)",
        "vstack(hstack(1.0, 2.0, 3.0), hstack(4.0, 10.0, 18.0))");
    test_cumprod(
        "cumprod(vstack(hstack(1.0, 2.0, 3.0), hstack(4.0, 5.0, 6.0)), 1)",
        "vstack(hstack(1.0, 2.0, 6.0), hstack(4.0, 20.0, 120.0))");

    test_cumprod("cumprod__int(vstack(hstack(42)))", "hstack(42)");
    test_cumprod("cumprod__int(vstack(hstack(42)), 0)", "vstack(hstack(42))");
    test_cumprod("cumprod__int(vstack(hstack(42)), 1)", "vstack(hstack(42))");

    test_cumprod(
        "cumprod__int(vstack(hstack(1, 2, 3), hstack(4, 5, 6)))",
        "hstack(1, 2, 6, 24, 120, 720)");
    test_cumprod(
        "cumprod__int(vstack(hstack(1, 2, 3), hstack(4, 5, 6)), 0)",
        "vstack(hstack(1, 2, 3), hstack(4, 10, 18))");
    test_cumprod(
        "cumprod__int(vstack(hstack(1, 2, 3), hstack(4, 5, 6)), 1)",
        "vstack(hstack(1, 2, 6), hstack(4, 20, 120))");

    test_cumprod("cumprod__float(vstack(hstack(42)))", "hstack(42.0)");
    test_cumprod("cumprod__float(vstack(hstack(42)), 0)", "vstack(hstack(42.0))");
    test_cumprod("cumprod__float(vstack(hstack(42)), 1)", "vstack(hstack(42.0))");

    test_cumprod(
        "cumprod__float(vstack(hstack(1, 2, 3), hstack(4, 5, 6)))",
        "hstack(1.0, 2.0, 6.0, 24.0, 120.0, 720.0)");
    test_cumprod(
        "cumprod__float(vstack(hstack(1, 2, 3), hstack(4, 5, 6)), 0)",
        "vstack(hstack(1.0, 2.0, 3.0), hstack(4.0, 10.0, 18.0))");
    test_cumprod(
        "cumprod__float(vstack(hstack(1, 2, 3), hstack(4, 5, 6)), 1)",
        "vstack(hstack(1.0, 2.0, 6.0), hstack(4.0, 20.0, 120.0))");
}

#if defined(PHYLANX_HAVE_BLAZE_TENSOR)
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

    test_cumprod("cumprod__int([[[42.]]])", "[42]");
    test_cumprod("cumprod__int([[[42.]]], 0)", "[[[42]]]");
    test_cumprod("cumprod__int([[[42.]]], 1)", "[[[42]]]");
    test_cumprod("cumprod__int([[[42.]]], 2)", "[[[42]]]");

    test_cumprod("cumprod__int([[[1., 2., 3.], [4., 5., 6.]]])",
        "[1, 2, 6, 24, 120, 720]");
    test_cumprod("cumprod__int([[[1., 2., 3.], [4., 5., 6.]]], 0)",
        "[[[1, 2, 3], [4, 5, 6]]]");
    test_cumprod("cumprod__int([[[1., 2., 3.], [4., 5., 6.]]], 1)",
        "[[[1, 2, 3], [4, 10, 18]]]");
    test_cumprod("cumprod__int([[[1., 2., 3.], [4., 5., 6.]]], 2)",
        "[[[1, 2, 6], [4, 20, 120]]]");

    test_cumprod("cumprod__float([[[42]]])", "[42.]");
    test_cumprod("cumprod__float([[[42]]], 0)", "[[[42.]]]");
    test_cumprod("cumprod__float([[[42]]], 1)", "[[[42.]]]");
    test_cumprod("cumprod__float([[[42]]], 2)", "[[[42.]]]");

    test_cumprod("cumprod__float([[[1, 2, 3], [4, 5, 6]]])",
        "[1., 2., 6., 24., 120., 720.]");
    test_cumprod("cumprod__float([[[1, 2, 3], [4, 5, 6]]], 0)",
        "[[[1., 2., 3.], [4., 5., 6.]]]");
    test_cumprod("cumprod__float([[[1, 2, 3], [4, 5, 6]]], 1)",
        "[[[1., 2., 3.], [4., 10., 18.]]]");
    test_cumprod("cumprod__float([[[1, 2, 3], [4, 5, 6]]], 2)",
        "[[[1., 2., 6.], [4., 20., 120.]]]");
}
#endif

int main(int argc, char* argv[])
{
    test_cumprod_0d();
    test_cumprod_1d();
    test_cumprod_2d();

#if defined(PHYLANX_HAVE_BLAZE_TENSOR)
    test_cumprod_3d();
#endif

    return hpx::util::report_errors();
}
