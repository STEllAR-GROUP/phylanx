// Copyright (c) 2019 Bita Hasheminezhad
// Copyright (c) 2019 Hartmut Kaiser
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

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
    return code.run().arg_;
}

void test_logsumexp_operation(std::string const& code,
    std::string const& expected_str)
{
    HPX_TEST_EQ(compile_and_run(code), compile_and_run(expected_str));
}

///////////////////////////////////////////////////////////////////////////////
int main(int argc, char* argv[])
{
    // scalars
    test_logsumexp_operation("logsumexp(42.0)", "42.0");
    test_logsumexp_operation("logsumexp(1)", "1.0");
    test_logsumexp_operation("logsumexp__float(1)", "1.0");

    // vectors
    test_logsumexp_operation("logsumexp([42., 1., 2., 3.])", "42.0");
    test_logsumexp_operation("logsumexp__float([42, 1, 2, 3])", "42.0");

    test_logsumexp_operation("logsumexp([42., 1., 2., 3.], 0)", "42.0");
    test_logsumexp_operation("logsumexp__float([42, 1, 2, 3], -1)", "42.0");

    test_logsumexp_operation(
        "logsumexp([42., 1., 2., 3.], nil, false)", "42.0");
    test_logsumexp_operation(
        "logsumexp([42., 1., 2., 3.], nil, true)", "[42.0]");
    test_logsumexp_operation(
        "logsumexp([42., 1., 2., 3.], 0, false)", "42.0");
    test_logsumexp_operation(
        "logsumexp([42., 1., 2., 3.], 0, true)", "[42.0]");
    test_logsumexp_operation(
        "logsumexp([42., 1., 2., 3.], -1, false)", "42.0");
    test_logsumexp_operation(
        "logsumexp([42., 1., 2., 3.], -1, true)", "[42.0]");

    // matrices
    test_logsumexp_operation("logsumexp([[42., 1.],[2., 3.]])", "42.0");

    test_logsumexp_operation(
        "logsumexp([[42., 1.],[2., 3.]], nil, false)", "42.0");

    test_logsumexp_operation(
        "logsumexp([[42., 1.],[2., 3.]], nil, true)", "[[42.0]]");

    test_logsumexp_operation(
        "logsumexp([[42., 1.],[2., 33.]], 0)", "[42., 33.]");

    test_logsumexp_operation(
        "logsumexp([[42., 1.],[2., 33.]], 1)", "[42., 33.]");

    test_logsumexp_operation(
        "logsumexp([[42., 1.],[2., 33.]], 0, false)", "[42., 33.]");

    test_logsumexp_operation(
        "logsumexp([[42., 1.],[2., 33.]], 1, false)", "[42., 33.]");

    test_logsumexp_operation(
        "logsumexp([[42., 1.],[2., 33.]], 0, true)", "[[42., 33.]]");

    test_logsumexp_operation(
        "logsumexp([[42., 1.],[2., 33.]], 1, true)", "[[42.], [33.]]");

    // tensors
    test_logsumexp_operation(
        "logsumexp([[[ 42.,  4.],[ 2.,  3.]],[[ 3.,  2.],[ 4.,  1.]]])",
        "42.0");

    test_logsumexp_operation("logsumexp([[[ 42.,  4.],[ 2.,  3.]],[[ 3.,  2.]"
                             ",[ 4.,  1.]]], nil, false)",
                             "42.0");

    test_logsumexp_operation("logsumexp([[[ 42.,  4.],[ 2.,  3.]],[[ 3.,  2.]"
                             ",[ 4.,  1.]]], nil, true)",
                             "[[[42.0]]]");

    test_logsumexp_operation("logsumexp([[[ 42.,  4.],[ 2., 33.]],[[ 3., 25.]"
                             ",[ 44.,  1.]]], 0)",
                             "[[ 42.,  25.],[ 44.,  33.]]");

    test_logsumexp_operation("logsumexp([[[ 42.,  4.],[ 2., 33.]],[[ 3., 25.]"
                             ",[ 44.,  1.]]], 1)",
                             "[[ 42.,  33.],[ 44.,  25.]]");

    test_logsumexp_operation("logsumexp([[[ 42.,  4.],[ 2., 33.]],[[ 3., 25.]"
                             ",[ 44.,  1.]]], 2)",
                             "[[ 42.,  33.],[ 25.,  44.]]");

    test_logsumexp_operation("logsumexp([[[ 42.,  4.],[ 2., 33.]],[[ 3., 25.]"
                             ",[ 44.,  1.]]], 0, false)",
                             "[[ 42.,  25.],[ 44.,  33.]]");

    test_logsumexp_operation("logsumexp([[[ 42.,  4.],[ 2., 33.]],[[ 3., 25.]"
                             ",[ 44.,  1.]]], 1, false)",
                             "[[ 42.,  33.],[ 44.,  25.]]");

    test_logsumexp_operation("logsumexp([[[ 42.,  4.],[ 2., 33.]],[[ 3., 25.]"
                             ",[ 44.,  1.]]], 2, false)",
                             "[[ 42.,  33.],[ 25.,  44.]]");

    test_logsumexp_operation("logsumexp([[[ 42.,  4.],[ 2., 33.]],[[ 3., 25.]"
                             ",[ 44.,  1.]]], 0, true)",
                             "[[[ 42.,  25.],[ 44.,  33.]]]");

    test_logsumexp_operation("logsumexp([[[ 42.,  4.],[ 2., 33.]],[[ 3., 25.]"
                             ",[ 44.,  1.]]], 1, true)",
                             "[[[ 42.,  33.]],[[ 44.,  25.]]]");

    test_logsumexp_operation("logsumexp([[[ 42.,  4.],[ 2., 33.]],[[ 3., 25.]"
                             ",[ 44.,  1.]]], 2, true)",
                             "[[[ 42.], [ 33.]],[[ 25.], [ 44.]]]");

    return hpx::util::report_errors();
}
