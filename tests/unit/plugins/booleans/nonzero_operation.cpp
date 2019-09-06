// Copyright (c) 2018 Parsa Amini
// Copyright (c) 2018 Hartmut Kaiser
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/phylanx.hpp>

#include <hpx/hpx_main.hpp>
#include <hpx/testing.hpp>

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
    return code.run().arg_;
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
    test_nonzero_operation("nonzero(false)",
        R"(list(hstack(list(), __arg(dtype, "int"))))");
    test_nonzero_operation("nonzero(0)",
        R"(list(hstack(list(), __arg(dtype, "int"))))");
    test_nonzero_operation("nonzero(0.)",
        R"(list(hstack(list(), __arg(dtype, "int"))))");
    test_nonzero_operation("nonzero(true)", "list([0])");
    test_nonzero_operation("nonzero(1)", "list([0])");
    test_nonzero_operation("nonzero(1.)", "list([0])");

    // test 1d data (vectors)
    test_nonzero_operation("nonzero([])",
        R"(list(hstack(list(), __arg(dtype, "int"))))");

    test_nonzero_operation("nonzero(hstack(list(false)))",
        R"(list(hstack(list(), __arg(dtype, "int"))))");
    test_nonzero_operation("nonzero([0])",
        R"(list(hstack(list(), __arg(dtype, "int"))))");
    test_nonzero_operation("nonzero([0.])",
        R"(list(hstack(list(), __arg(dtype, "int"))))");

    test_nonzero_operation("nonzero(hstack(list(true)))", "list([0])");
    test_nonzero_operation("nonzero([1])", "list([0])");
    test_nonzero_operation("nonzero([1.])", "list([0])");

    test_nonzero_operation(
        "nonzero(hstack(list(false, true, false, true)))", "list([1, 3])");
    test_nonzero_operation("nonzero([0, 1, 2, 0])", "list([1, 2])");
    test_nonzero_operation(
        "nonzero([1., 0., 42., 43.])", "list([0, 2, 3])");

    // test 2d data (matrix)
    test_nonzero_operation(
        "nonzero([[]])",
        R"(list(hstack(list(), __arg(dtype, "int")),
                hstack(list(), __arg(dtype, "int"))))");
    test_nonzero_operation(
        "nonzero([[0, 0], [0, 0]])",
        R"(list(hstack(list(), __arg(dtype, "int")),
                hstack(list(), __arg(dtype, "int"))))");

    test_nonzero_operation(
        "nonzero([[0, 1], [2, 0]])",
        "list([0, 1], [1, 0])");
    test_nonzero_operation(
        "nonzero([[0., 1.], [2., 0.]])",
        "list([0, 1], [1, 0])");

    return hpx::util::report_errors();
}
