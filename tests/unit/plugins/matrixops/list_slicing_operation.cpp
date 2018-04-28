// Copyright (c) 2017-2018 Bibek Wagle
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/phylanx.hpp>

#include <hpx/hpx_main.hpp>
#include <hpx/include/lcos.hpp>
#include <hpx/util/lightweight_test.hpp>

#include <iostream>
#include <string>
#include <utility>
#include <vector>

///////////////////////////////////////////////////////////////////////////////
phylanx::execution_tree::compiler::function compile(std::string const& code)
{
    phylanx::execution_tree::compiler::function_list snippets;
    phylanx::execution_tree::compiler::environment env =
        phylanx::execution_tree::compiler::default_environment();

    return phylanx::execution_tree::compile(code, snippets, env);
}

void test_slice_operation(std::string const& code,
    std::string const& expected_str)
{
    HPX_TEST_EQ(compile(code)(), compile(expected_str)());
}

///////////////////////////////////////////////////////////////////////////////
int main(int argc, char* argv[])
{
    test_slice_operation(
        R"(block(
            define(a, make_list(1, 2, 3, 4, 5, 6, 7, 8)),
            slice(a, make_list(2, 2))
        ))", "make_list()");

    test_slice_operation(
        R"(block(
            define(a, make_list(1, 2, 3, 4, 5, 6, 7, 8)),
            slice(a, make_list(2, 4))
        ))", "make_list(3, 4)");

    test_slice_operation(
        R"(block(
            define(a, make_list(1, 2, 3, 4, 5, 6, 7, 8)),
            slice(a, make_list(2, 6, 2))
        ))", "make_list(3, 5)");

    test_slice_operation(
        R"(block(
            define(a, make_list(1, 2, 3, 4, 5, 6, 7, 8)),
            slice(a, make_list(6, 2, -2))
        ))", "make_list(7, 5)");

    test_slice_operation(
        R"(block(
            define(a, make_list(1, 2, 3, 4, 5, 6, 7, 8)),
            slice(a, make_list(-6, -2))
        ))", "make_list(3, 4, 5, 6)");

    test_slice_operation(
        R"(block(
            define(a, make_list(1, 2, 3, 4, 5, 6, 7, 8)),
            slice(a, make_list(-6, -5))
        ))", "make_list(3)");

    test_slice_operation(
        R"(block(
            define(a, make_list(1, 2, 3, 4, 5, 6, 7, 8)),
            slice(a, make_list(0, -1))
        ))", "make_list(1, 2, 3, 4, 5, 6, 7)");

    test_slice_operation(
        R"(block(
            define(a, make_list(1, 2, 3, 4, 5, 6, 7, 8)),
            slice(a, make_list(-2, -6, -2))
        ))", "make_list(7, 5)");

    test_slice_operation(
        R"(block(
            define(a, make_list(1, 2, 3, 4, 5, 6, 7, 8)),
            slice(a, 2)
        ))", "3");

    test_slice_operation(
        R"(block(
            define(a, make_list(1, 2, 3, 4, 5, 6, 7, 8)),
            slice(a, -2)
        ))", "7");

    return hpx::util::report_errors();
}
