// Copyright (c) 2017-2018 Bibek Wagle
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

void test_slice_operation(std::string const& code,
    std::string const& expected_str)
{
    HPX_TEST_EQ(compile_and_run(code), compile_and_run(expected_str));
}

///////////////////////////////////////////////////////////////////////////////
int main(int argc, char* argv[])
{
    test_slice_operation(
        R"(block(
            define(a, list(1, 2, 3, 4, 5, 6, 7, 8)),
            slice(a, 2)
        ))", "3");

    test_slice_operation(
        R"(block(
            define(a, list(1, 2, 3, 4, 5, 6, 7, 8)),
            slice(a, list(2))
        ))", "3");

    test_slice_operation(
        R"(block(
            define(a, list(1, 2, 3, 4, 5, 6, 7, 8)),
            slice(a, list(2, 2))
        ))", "list()");

    test_slice_operation(
        R"(block(
            define(a, list(1, 2, 3, 4, 5, 6, 7, 8)),
            slice(a, list(2, 4))
        ))", "list(3, 4)");

    test_slice_operation(
        R"(block(
            define(a, list(1, 2, 3, 4, 5, 6, 7, 8)),
            slice(a, list(2, 6, 2))
        ))", "list(3, 5)");

    test_slice_operation(
        R"(block(
            define(a, list(1, 2, 3, 4, 5, 6, 7, 8)),
            slice(a, list(6, 2, -2))
        ))", "list(7, 5)");

    test_slice_operation(
        R"(block(
            define(a, list(1, 2, 3, 4, 5, 6, 7, 8)),
            slice(a, list(-6, -2))
        ))", "list(3, 4, 5, 6)");

    test_slice_operation(
        R"(block(
            define(a, list(1, 2, 3, 4, 5, 6, 7, 8)),
            slice(a, list(-6, -5))
        ))", "list(3)");

    test_slice_operation(
        R"(block(
            define(a, list(1, 2, 3, 4, 5, 6, 7, 8)),
            slice(a, list(0, -1))
        ))", "list(1, 2, 3, 4, 5, 6, 7)");

    test_slice_operation(
        R"(block(
            define(a, list(1, 2, 3, 4, 5, 6, 7, 8)),
            slice(a, list(-2, -6, -2))
        ))", "list(7, 5)");

    test_slice_operation(
        R"(block(
            define(a, list(1, 2, 3, 4, 5, 6, 7, 8)),
            slice(a, 2)
        ))", "3");

    test_slice_operation(
        R"(block(
            define(a, list(1, 2, 3, 4, 5, 6, 7, 8)),
            slice(a, -2)
        ))", "7");

    return hpx::util::report_errors();
}
