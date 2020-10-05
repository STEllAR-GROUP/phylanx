// Copyright (c) 2017-2018 Bibek Wagle
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/phylanx.hpp>

#include <hpx/hpx_main.hpp>
#include <hpx/include/lcos.hpp>
#include <hpx/modules/testing.hpp>

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

void test_norm(char const* code, char const* expectedstr)
{
    auto result = compile_and_run(code);
    auto expected = compile_and_run(expectedstr);

    HPX_TEST_EQ(result, expected);
}

// vector norms
void test_vector_norms()
{
    test_norm(R"(
            norm([-4., -3., -2., -1., 0., 1., 2., 3., 4.])
        )",
        "7.745966692414834");
    test_norm(R"(
        norm(
            [-4., -3., -2., -1., 0., 1., 2., 3., 4.],
            __arg(keepdims, true)
        ))",
        "[7.745966692414834]");

    test_norm(R"(
        norm(
            [-4., -3., -2., -1., 0., 1., 2., 3., 4.],
            __arg(axis, 0)
        ))",
        "7.745966692414834");
    test_norm(R"(
        norm(
            [-4., -3., -2., -1., 0., 1., 2., 3., 4.],
            __arg(axis, -1),
            __arg(keepdims, true)
        ))",
        "[7.745966692414834]");

    test_norm(R"(
        norm(
            [-4., -3., -2., -1., 0., 1., 2., 3., 4.],
            __arg(ord, inf),
            __arg(keepdims, false)
        ))",
        "4.0");
    test_norm(R"(
        norm(
            [-4., -3., -2., -1., 0., 1., 2., 3., 4.],
            __arg(ord, inf),
            __arg(keepdims, true)
        ))",
        "[4.0]");

    test_norm(R"(
        norm(
            [-4., -3., -2., -1., 0., 1., 2., 3., 4.],
            __arg(ord, ninf),
            __arg(keepdims, false)
        ))",
        "0.0");
    test_norm(R"(
        norm(
            [-4., -3., -2., -1., 0., 1., 2., 3., 4.],
            __arg(ord, ninf),
            __arg(keepdims, true)
        ))",
        "[0.0]");

    test_norm(R"(
        norm(
            [-4., -3., -2., -1., 0., 1., 2., 3., 4.],
            __arg(ord, 0),
            __arg(keepdims, false)
        ))",
        "8.0");
    test_norm(R"(
        norm(
            [-4., -3., -2., -1., 0., 1., 2., 3., 4.],
            __arg(ord, 0),
            __arg(keepdims, true)
        ))",
        "[8.0]");

    test_norm(R"(
        norm(
            [-4., -3., -2., -1., 0., 1., 2., 3., 4.],
            __arg(ord, 2),
            __arg(keepdims, false)
        ))",
        "7.745966692414834");
    test_norm(R"(
        norm(
            [-4., -3., -2., -1., 0., 1., 2., 3., 4.],
            __arg(ord, 2),
            __arg(keepdims, true)
        ))",
        "[7.745966692414834]");
}

// matrix norms
void test_matrix_norms()
{
    test_norm(R"(
            norm([[-4., -3., -2.], [-1., 0., 1.], [2., 3., 4.]])
        )",
        "7.745966692414834");
    test_norm(R"(
        norm(
            [[-4., -3., -2.], [-1., 0., 1.], [2., 3., 4.]],
            __arg(keepdims, true)
        ))",
        "[[7.745966692414834]]");

    test_norm(R"(
        norm(
            [[-4., -3., -2.], [-1., 0., 1.], [2., 3., 4.]],
            __arg(ord, 2),
            __arg(keepdims, false)
        ))",
        "7.745966692414834");
    test_norm(R"(
        norm(
            [[-4., -3., -2.], [-1., 0., 1.], [2., 3., 4.]],
            __arg(ord, 2),
            __arg(keepdims, true)
        ))",
        "[[7.745966692414834]]");
}

int main(int argc, char* argv[])
{
    test_vector_norms();
    test_matrix_norms();

    return hpx::util::report_errors();
}
