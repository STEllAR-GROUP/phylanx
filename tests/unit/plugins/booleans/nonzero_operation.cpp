// Copyright (c) 2018 Parsa Amini
// Copyright (c) 2018 Hartmut Kaiser
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/phylanx.hpp>

#include <hpx/hpx_main.hpp>
#include <hpx/util/lightweight_test.hpp>

#include <cstdint>
#include <string>

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
void test_nonzero_operation(std::string const& code,
    std::string const& expected_str)
{
    HPX_TEST_EQ(compile_and_run(code), compile_and_run(expected_str));
}

///////////////////////////////////////////////////////////////////////////////
int main(int argc, char* argv[])
{
    // test 0d data (scalars)
    test_nonzero_operation("nonzero(false)", "list(hstack())");
    test_nonzero_operation("nonzero(0)", "list(hstack())");
    test_nonzero_operation("nonzero(0.)", "list(hstack())");
    test_nonzero_operation("nonzero(true)", "list(hstack(0))");
    test_nonzero_operation("nonzero(1)", "list(hstack(0))");
    test_nonzero_operation("nonzero(1.)", "list(hstack(0))");

    // test 1d data (vectors)
    test_nonzero_operation("nonzero(hstack())", "list(hstack())");

    test_nonzero_operation("nonzero(hstack(false))", "list(hstack())");
    test_nonzero_operation("nonzero(hstack(0))", "list(hstack())");
    test_nonzero_operation("nonzero(hstack(0.))", "list(hstack())");

    test_nonzero_operation("nonzero(hstack(true))", "list(hstack(0))");
    test_nonzero_operation("nonzero(hstack(1))", "list(hstack(0))");
    test_nonzero_operation("nonzero(hstack(1.))", "list(hstack(0))");

    test_nonzero_operation(
        "nonzero(hstack(false, true, false, true))", "list(hstack(1, 3))");
    test_nonzero_operation("nonzero(hstack(0, 1, 2, 0))", "list(hstack(1, 2))");
    test_nonzero_operation(
        "nonzero(hstack(1., 0., 42., 43.))", "list(hstack(0, 2, 3))");

    // test 2d data (matrix)
    test_nonzero_operation(
        "nonzero(vstack(hstack()))", "list(hstack(), hstack())");
    test_nonzero_operation(
        "nonzero(vstack(hstack(0, 0), hstack(0, 0)))",
        "list(hstack(), hstack())");

    test_nonzero_operation(
        "nonzero(vstack(hstack(0, 1), hstack(2, 0)))",
        "list(hstack(0, 1), hstack(1, 0))");
    test_nonzero_operation(
        "nonzero(vstack(hstack(0., 1.), hstack(2., 0.)))",
        "list(hstack(0, 1), hstack(1, 0))");

    return hpx::util::report_errors();
}
