// Copyright (c) 2017-2018 Hartmut Kaiser
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

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

void test_count_nonzero_operation(std::string const& code,
    std::string const& expected_str)
{
    HPX_TEST_EQ(compile_and_run(code), compile_and_run(expected_str));
}

///////////////////////////////////////////////////////////////////////////////
int main(int argc, char* argv[])
{
    test_count_nonzero_operation("count_nonzero(false)", "0");
    test_count_nonzero_operation("count_nonzero(true)", "1");

    test_count_nonzero_operation("count_nonzero(0)", "0");
    test_count_nonzero_operation("count_nonzero(1)", "1");

    test_count_nonzero_operation("count_nonzero(0.)", "0");
    test_count_nonzero_operation("count_nonzero(1.)", "1");

    test_count_nonzero_operation("count_nonzero([])", "0");
    test_count_nonzero_operation("count_nonzero([1, 2, 3, 4])", "4");
    test_count_nonzero_operation("count_nonzero([1, 0, 2, 0])", "2");
    test_count_nonzero_operation("count_nonzero([0, 0, 0, 0])", "0");

    test_count_nonzero_operation(
        "count_nonzero([[1, 2, 3, 4]])", "4");
    test_count_nonzero_operation(
        "count_nonzero([[1, 0, 2, 0]])", "2");
    test_count_nonzero_operation(
        "count_nonzero([[0, 0, 0, 0]])", "0");

    test_count_nonzero_operation(
        "count_nonzero([[1, 2], [3, 4]])", "4");
    test_count_nonzero_operation(
        "count_nonzero([[1, 0], [2, 0]])", "2");
    test_count_nonzero_operation(
        "count_nonzero([[0, 0], [0, 0]])", "0");

    return hpx::util::report_errors();
}
