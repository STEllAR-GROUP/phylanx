// Copyright (c) 2019 R. Tohid
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// #817: PhySL: lambda returning nil

#include <phylanx/phylanx.hpp>

#include <hpx/hpx_main.hpp>
#include <hpx/modules/testing.hpp>

void test_eq_nil()
{
    char const* const codestr = R"(
        define(test, a, __eq(a, nil))
        test(nil)
    )";

    phylanx::execution_tree::compiler::function_list snippets;
    phylanx::execution_tree::compiler::environment env =
        phylanx::execution_tree::compiler::default_environment();

    auto const& code = phylanx::execution_tree::compile(
        "test", codestr, snippets, env);
    auto test = code.run();

    HPX_TEST_EQ(
        true, phylanx::execution_tree::extract_scalar_boolean_value(test()));
}

void test_ne_nil()
{
    char const* const codestr = R"(
        define(test, a, __ne(a, nil))
        test(nil)
    )";

    phylanx::execution_tree::compiler::function_list snippets;
    phylanx::execution_tree::compiler::environment env =
        phylanx::execution_tree::compiler::default_environment();

    auto const& code = phylanx::execution_tree::compile(
        "test", codestr, snippets, env);
    auto test = code.run();

    HPX_TEST_EQ(
        false, phylanx::execution_tree::extract_scalar_boolean_value(test()));
}

int main(int argc, char* argv[])
{
    test_eq_nil();
    test_ne_nil();

    return hpx::util::report_errors();
}
