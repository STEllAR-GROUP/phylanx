//   Copyright (c) 2018 Hartmut Kaiser
//
//   Distributed under the Boost Software License, Version 1.0. (See accompanying
//   file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/phylanx.hpp>

#include <hpx/hpx_main.hpp>
#include <hpx/include/lcos.hpp>
#include <hpx/util/lightweight_test.hpp>

#include <string>
#include <utility>
#include <vector>

///////////////////////////////////////////////////////////////////////////////
phylanx::execution_tree::compiler::function compile(std::string const& codestr)
{
    phylanx::execution_tree::compiler::function_list snippets;
    phylanx::execution_tree::compiler::environment env =
        phylanx::execution_tree::compiler::default_environment();

    auto const& code = phylanx::execution_tree::compile(codestr, snippets, env);
    return code.run();
}

void test_add_list_operation(std::string const& code,
    std::string const& expected_str)
{
    HPX_TEST_EQ(compile(code)(), compile(expected_str)());
}

int main(int argc, char* argv[])
{
    test_add_list_operation(
        "make_list(1, 2, 3, 4) + 5",
        "make_list(1, 2, 3, 4, 5)");

    test_add_list_operation(
        "make_list(1, 2, 3, 4) + make_list(5)",
        "make_list(1, 2, 3, 4, 5)");

    test_add_list_operation(
        "make_list(1, 2, 3, 4) + make_list(make_list(5))",
        "make_list(1, 2, 3, 4, make_list(5))");

    test_add_list_operation(
        "make_list(1, 2, 3, 4) + make_list(5, 6, 7, 8)",
        "make_list(1, 2, 3, 4, 5, 6, 7, 8)");

    test_add_list_operation(
        "make_list(1, 2, 3, 4) + make_list(make_list(5, 6, 7, 8))",
        "make_list(1, 2, 3, 4, make_list(5, 6, 7, 8))");

    return hpx::util::report_errors();
}
