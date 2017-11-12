//   Copyright (c) 2017 Hartmut Kaiser
//
//   Distributed under the Boost Software License, Version 1.0. (See accompanying
//   file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)


#include <phylanx/phylanx.hpp>

#include <hpx/hpx_main.hpp>
#include <hpx/util/lightweight_test.hpp>

#include <vector>

void test_define_operation_var(
    char const* expr, char const* name, double expected)
{
    phylanx::execution_tree::compiler::environment env =
        phylanx::execution_tree::compiler::default_environment();
    phylanx::execution_tree::compiler::function_list snippets;

    std::size_t num_entries = env.size();

    auto f = phylanx::execution_tree::compile(expr, snippets, env);
    f();        // bind expressions

    HPX_TEST_EQ(env.size(), num_entries + 1);

    // verify function name
    auto p = env.find(name);
    HPX_TEST(p != nullptr);

    auto var = (*p)(phylanx::execution_tree::compiler::function_list{});

    // evaluate expression
    HPX_TEST_EQ(expected,
        phylanx::execution_tree::extract_numeric_value(
            var()
        )[0]);
}

void test_define_operation(char const* expr, char const* name, double expected,
    std::vector<double> const& argvalues)
{
    phylanx::execution_tree::compiler::environment env =
        phylanx::execution_tree::compiler::default_environment();
    phylanx::execution_tree::compiler::function_list snippets;

    std::size_t num_entries = env.size();

    auto f = phylanx::execution_tree::compile(expr, snippets, env);
    f();        // bind expressions

    HPX_TEST_EQ(env.size(), num_entries + 1);

    auto p = env.find(name);
    HPX_TEST(p != nullptr);

    auto var = (*p)(phylanx::execution_tree::compiler::function_list{});

    // evaluate expression
    std::vector<phylanx::execution_tree::primitive_argument_type> values;
    for (double d : argvalues)
    {
        values.push_back(phylanx::ir::node_data<double>{d});
    }

    HPX_TEST_EQ(expected,
        phylanx::execution_tree::extract_numeric_value(
            var(std::move(values))
        )[0]);
}

int main(int argc, char* argv[])
{
    test_define_operation_var("define(x, 3.14)", "x", 3.14);

    test_define_operation("define(y, x, x + 1)", "y", 2.0, {1.0});
    test_define_operation(
        "define(add1, x, y, x + y)", "add1", 42.0, {41.0, 1.0});

    return hpx::util::report_errors();
}

