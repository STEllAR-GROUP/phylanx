// Copyright (c) 2018-2109 Hartmut Kaiser
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
void test_min_operation(std::string const& code,
    std::string const& expected_str)
{
    HPX_TEST_EQ(compile_and_run(code), compile_and_run(expected_str));
}

///////////////////////////////////////////////////////////////////////////////
int main(int argc, char* argv[])
{
    test_min_operation("maximum(42., 43.)", "43.");
    test_min_operation("maximum(42., 0, true)", "42.");

    test_min_operation(
        "maximum([13., 42., 33.], [101., 12., 65.])", "[101., 42., 65.]");
    test_min_operation("maximum("
            "[13., 42., 33.], -1)", "[13., 42., 33.]");
    test_min_operation("maximum("
            "[13., 42., 33.],  0, true)", "[13., 42., 33.]");

    test_min_operation("maximum("
            "[[13., 42., 33.], [101., 12., 65.]], "
            "[[101., 12., 65.], [13., 42., 33.]])",
        "[[101., 42., 65.], [101., 42., 65.]]");
    test_min_operation("maximum("
            "[[13., 42., 33.], [101., 12., 33.]], 0)",
        "[[13., 42., 33.], [101., 12., 33.]]");
    test_min_operation("maximum("
            "[[13., 42., 33.], [101., 12., 65.]], [101., 11., 65.])",
        "[[101., 42., 65.], [101., 12., 65.]]");

    test_min_operation("maximum("
            "[[[13., 42., 33.]], [[101., 12., 65.]], [[101., 12., 65.]]], 15.)",
        "[[[15., 42., 33.]], [[101., 15., 65.]], [[101., 15., 65.]]]");
    test_min_operation("maximum("
            "[[[13., 42., 33.]], [[101., 12., 65.]], [[101., 12., 65.]]], "
            "[5., 15., 7.])",
        "[[[13., 42., 33.]], [[101., 15., 65.]], [[101., 15., 65.]]]");
    test_min_operation("maximum("
            "[[[13., 42., 33.]], [[101., 12., 65.]], [[101., 12., 65.]]], "
            "[[5., 16., 7.]])",
        "[[[13., 42., 33.]], [[101., 16., 65.]], [[101., 16., 65.]]]");
    test_min_operation("maximum("
            "[[[13., 42., 33.]], [[101., 12., 65.]], [[101., 12., 65.]]], "
            "[[[5., 16., 7.]], [[5., 16., 7.]], [[5., 16., 7.]]])",
        "[[[13., 42., 33.]], [[101., 16., 65.]], [[101., 16., 65.]]]");

    return hpx::util::report_errors();
}
