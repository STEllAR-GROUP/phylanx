// Copyright (c) 2018-2019 Hartmut Kaiser
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
    test_min_operation("minimum(42., 43.)", "42.");
    test_min_operation("minimum(42., 0, true)", "0.");

    test_min_operation(
        "minimum([13., 42., 33.], [101., 12., 65.])", "[13., 12., 33.]");
    test_min_operation("minimum("
            "[13., 42., 33.], -1)", "[-1., -1., -1.]");
    test_min_operation("minimum("
            "[13., 42., 33.],  0, true)", "[0., 0., 0.]");

    test_min_operation("minimum("
            "[[13., 42., 33.], [101., 12., 65.]], "
            "[[101., 12., 65.], [13., 42., 33.]])",
        "[[13., 12., 33.], [13., 12., 33.]]");
    test_min_operation("minimum("
            "[[13., 42., 33.], [101., 12., 33.]], 0)",
        "[[0., 0., 0.], [0., 0., 0.]]");
    test_min_operation("minimum("
            "[[13., 42., 33.], [101., 12., 65.]], [101., 11., 65.])",
        "[[13., 11., 33.], [101., 11., 65.]]");

    test_min_operation("minimum("
            "[[[13., 42., 33.]], [[101., 12., 65.]], [[101., 12., 65.]]], 15.)",
        "[[[13., 15., 15.]], [[15., 12., 15.]], [[15., 12., 15.]]]");
    test_min_operation("minimum("
            "[[[13., 42., 33.]], [[101., 12., 65.]], [[101., 12., 65.]]], "
            "[5., 15., 7.])",
        "[[[5., 15., 7.]], [[5., 12., 7.]], [[5., 12., 7.]]]");
    test_min_operation("minimum("
            "[[[13., 42., 33.]], [[101., 12., 65.]], [[101., 12., 65.]]], "
            "[[5., 16., 7.]])",
        "[[[5., 16., 7.]], [[5., 12., 7.]], [[5., 12., 7.]]]");
    test_min_operation("minimum("
            "[[[13., 42., 33.]], [[101., 12., 65.]], [[101., 12., 65.]]], "
            "[[[5., 16., 7.]], [[5., 16., 7.]], [[5., 16., 7.]]])",
        "[[[5., 16., 7.]], [[5., 12., 7.]], [[5., 12., 7.]]]");

    return hpx::util::report_errors();
}
