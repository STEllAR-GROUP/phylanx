// Copyright (c) 2018 Hartmut Kaiser
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/phylanx.hpp>

#include <hpx/hpx_main.hpp>
#include <hpx/include/lcos.hpp>
#include <hpx/util/lightweight_test.hpp>

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
    return code.run();
}

///////////////////////////////////////////////////////////////////////////////
void test_cumsum(std::string const& code, std::string const& expected_str)
{
    HPX_TEST_EQ(compile_and_run(code), compile_and_run(expected_str));
}

///////////////////////////////////////////////////////////////////////////////
void test_cumsum_0d()
{
    test_cumsum("cumsum(42)", "hstack(42)");
    test_cumsum("cumsum(42, 0)", "hstack(42)");
}

void test_cumsum_1d()
{
    test_cumsum("cumsum(hstack(42))", "hstack(42)");
    test_cumsum("cumsum(hstack(42), 0)", "hstack(42)");

    test_cumsum(
        "cumsum(hstack(1, 2, 3, 4, 5, 6))", "hstack(1, 3, 6, 10, 15, 21)");
    test_cumsum(
        "cumsum(hstack(1, 2, 3, 4, 5, 6), 0)", "hstack(1, 3, 6, 10, 15, 21)");
}

void test_cumsum_2d()
{
    test_cumsum("cumsum(vstack(hstack(42)))", "hstack(42)");
    test_cumsum("cumsum(vstack(hstack(42)), 0)", "vstack(hstack(42))");
    test_cumsum("cumsum(vstack(hstack(42)), 1)", "vstack(hstack(42))");

    test_cumsum(
        "cumsum(vstack(hstack(1, 2, 3), hstack(4, 5, 6)))",
        "hstack(1, 3, 6, 10, 15, 21)");
    test_cumsum(
        "cumsum(vstack(hstack(1, 2, 3), hstack(4, 5, 6)), 0)",
        "vstack(hstack(1, 2, 3), hstack(5, 7, 9))");
    test_cumsum(
        "cumsum(vstack(hstack(1, 2, 3), hstack(4, 5, 6)), 1)",
        "vstack(hstack(1, 3, 6), hstack(4, 9, 15))");
}

int main(int argc, char* argv[])
{
    test_cumsum_0d();
    test_cumsum_1d();
    test_cumsum_2d();

    return hpx::util::report_errors();
}
