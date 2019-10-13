//   Copyright (c) 2018 Hartmut Kaiser
//
//   Distributed under the Boost Software License, Version 1.0. (See accompanying
//   file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/phylanx.hpp>

#include <hpx/hpx_main.hpp>
#include <hpx/include/lcos.hpp>
#include <hpx/testing.hpp>

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

void test_add_list_operation(std::string const& code,
    std::string const& expected_str)
{
    HPX_TEST_EQ(compile_and_run(code), compile_and_run(expected_str));
}

int main(int argc, char* argv[])
{
    test_add_list_operation(
        "list(1, 2, 3, 4) + 5",
        "list(1, 2, 3, 4, 5)");

    test_add_list_operation(
        "list(1, 2, 3, 4) + list(5)",
        "list(1, 2, 3, 4, 5)");

    test_add_list_operation(
        "list(1, 2, 3, 4) + list(list(5))",
        "list(1, 2, 3, 4, list(5))");

    test_add_list_operation(
        "list(1, 2, 3, 4) + list(5, 6, 7, 8)",
        "list(1, 2, 3, 4, 5, 6, 7, 8)");

    test_add_list_operation(
        "list(1, 2, 3, 4) + list(list(5, 6, 7, 8))",
        "list(1, 2, 3, 4, list(5, 6, 7, 8))");

    return hpx::util::report_errors();
}
