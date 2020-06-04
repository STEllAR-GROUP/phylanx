// Copyright (c) 2019 Hartmut Kaiser
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/phylanx.hpp>

#include <hpx/hpx_main.hpp>
#include <hpx/modules/testing.hpp>

#include <string>

void test_define_global_variable()
{
    phylanx::execution_tree::compiler::function_list snippets;
    phylanx::execution_tree::compiler::environment env =
        phylanx::execution_tree::compiler::default_environment();

    phylanx::execution_tree::eval_context ctx;

    auto const& def =
        phylanx::execution_tree::compile("define(x, 42.0)", snippets, env);

    def.run(ctx);

    auto const& code = phylanx::execution_tree::compile("x", snippets, env);
    auto x = code.run(ctx);

    HPX_TEST_EQ(
        42.0, phylanx::execution_tree::extract_scalar_numeric_value(x()));
}

void test_define_two_global_variables()
{
    phylanx::execution_tree::compiler::function_list snippets;
    phylanx::execution_tree::compiler::environment env =
        phylanx::execution_tree::compiler::default_environment();

    phylanx::execution_tree::eval_context ctx;

    std::string code = R"(
        define(x, 42.0)
        define(y, 43.0)
    )";

    auto const& def = phylanx::execution_tree::compile(code, snippets, env);
    def.run(ctx);

    auto x = phylanx::execution_tree::compile("x", snippets, env).run(ctx);
    auto y = phylanx::execution_tree::compile("y", snippets, env).run(ctx);

    HPX_TEST_EQ(
        42.0, phylanx::execution_tree::extract_scalar_numeric_value(x()));
    HPX_TEST_EQ(
        43.0, phylanx::execution_tree::extract_scalar_numeric_value(y()));
}

void test_define_and_set_global_variable()
{
    phylanx::execution_tree::compiler::function_list snippets;
    phylanx::execution_tree::compiler::environment env =
        phylanx::execution_tree::compiler::default_environment();

    phylanx::execution_tree::eval_context ctx;

    std::string code = R"(
        define(x, 42.0)
        store(x, 43.0)
    )";

    auto const& def = phylanx::execution_tree::compile(code, snippets, env);
    def.run(ctx);

    auto x = phylanx::execution_tree::compile("x", snippets, env).run(ctx);

    HPX_TEST_EQ(
        43.0, phylanx::execution_tree::extract_scalar_numeric_value(x()));
}

void test_define_local_global_variable()
{
    phylanx::execution_tree::compiler::function_list snippets;
    phylanx::execution_tree::compiler::environment env =
        phylanx::execution_tree::compiler::default_environment();

    phylanx::execution_tree::eval_context ctx;

    std::string code = R"(
        define(x, 42.0)
        define(f, a, block(
            define(y, a),
            y
        ))
        f(x)
    )";

    auto const& def = phylanx::execution_tree::compile(code, snippets, env);
    auto result = def.run(ctx);

    HPX_TEST_EQ(
        42.0, phylanx::execution_tree::extract_scalar_numeric_value(result()));
}

void test_define_local_hidden_global_variable()
{
    phylanx::execution_tree::compiler::function_list snippets;
    phylanx::execution_tree::compiler::environment env =
        phylanx::execution_tree::compiler::default_environment();

    phylanx::execution_tree::eval_context ctx;

    std::string code = R"(
        define(x, 42.0)
        define(f, a, block(
            define(x, a),
            x
        ))
        f(41.0)
    )";

    auto const& def = phylanx::execution_tree::compile(code, snippets, env);
    auto result = def.run(ctx);

    HPX_TEST_EQ(
        41.0, phylanx::execution_tree::extract_scalar_numeric_value(result()));
}

void test_local_variable_repeated_call()
{
    phylanx::execution_tree::compiler::function_list snippets;
    phylanx::execution_tree::compiler::environment env =
        phylanx::execution_tree::compiler::default_environment();

    phylanx::execution_tree::eval_context ctx;

    std::string code = R"(
        define(foo, n, block(
            define(x, 0),
            if(n == 1,
                block(
                    foo(2), x
                ),
                if(n == 2,
                    store(x, 3)
                )
            )
        ))
        foo(1)
    )";

    auto const& def = phylanx::execution_tree::compile(code, snippets, env);
    auto result = def.run(ctx);

    HPX_TEST_EQ(
        0, phylanx::execution_tree::extract_scalar_integer_value(result()));
}

///////////////////////////////////////////////////////////////////////////////
int main(int argc, char* argv[])
{
    test_define_global_variable();
    test_define_two_global_variables();

    test_define_and_set_global_variable();

    test_define_local_global_variable();
    test_define_local_hidden_global_variable();

    test_local_variable_repeated_call();

    return hpx::util::report_errors();
}

