// Copyright (c) 2019 Shahrzad Shirzad
// Copyright (c) 2019 Hartmut Kaiser
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
void test_softsign_operation(
    std::string const& code, std::string const& expected_str)
{
    HPX_TEST_EQ(compile_and_run(code), compile_and_run(expected_str));
}

///////////////////////////////////////////////////////////////////////////////
int main(int argc, char* argv[])
{
    test_softsign_operation("softsign(4.)", "0.8");
    test_softsign_operation("softsign(-3.)", "-0.75");
    test_softsign_operation(
        "softsign([-1., -4., 3., 9.])", "[-0.5, -0.8, 0.75, 0.9]");
    test_softsign_operation("softsign([[-1., -4., 3.], [-9., 4., 1.5]])",
        "[[-0.5, -0.8, 0.75], [-0.9, 0.8, 0.6]]");
    test_softsign_operation("softsign([[[-1., -4., 3.], [-9., 4., 1.5]]])",
        "[[[-0.5, -0.8, 0.75], [-0.9, 0.8, 0.6]]]");

    return hpx::util::report_errors();
}
