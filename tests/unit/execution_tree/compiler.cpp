//  Copyright (c) 2017 Hartmut Kaiser
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/phylanx.hpp>

#include <hpx/hpx_main.hpp>
#include <hpx/runtime/find_here.hpp>
#include <hpx/util/lightweight_test.hpp>

void test_builtin_environment()
{
    hpx::id_type here = hpx::find_here();

    // generate default environment
    phylanx::execution_tree::compiler::environment env =
        phylanx::execution_tree::compiler::default_environment(
            phylanx::execution_tree::get_all_known_patterns(),
            here);

    // add a variable 'x = 41.0'
    auto create_var = phylanx::execution_tree::compiler::primitive_variable{here};
    auto varx = create_var(phylanx::ir::node_data<double>{41.0});
    env.define("x", phylanx::execution_tree::compiler::external_function{varx});

    // extract factory for compiling variables
    phylanx::execution_tree::compiler::compiled_function* cfx = env.find("x");
    HPX_TEST(cfx != nullptr);
    auto x = (*cfx)(phylanx::execution_tree::compiler::function_list{});

    // invoke 'x' primitive
    HPX_TEST_EQ(41.0,
        phylanx::execution_tree::extract_numeric_value(
            x()
        )[0]);
}

void test_builtin_environment_vars()
{
    hpx::id_type here = hpx::find_here();

    // generate default environment
    phylanx::execution_tree::compiler::environment env =
        phylanx::execution_tree::compiler::default_environment(
            phylanx::execution_tree::get_all_known_patterns(),
            here);

    // add two variables, 'x' and 'y'
    auto create_var = phylanx::execution_tree::compiler::primitive_variable{here};

    auto varx = create_var(phylanx::ir::node_data<double>{41.0});
    auto vary = create_var(phylanx::ir::node_data<double>{1.0});

    env.define("x", phylanx::execution_tree::compiler::external_function{varx});
    env.define("y", phylanx::execution_tree::compiler::external_function{vary});

    // extract factory for compiling the '+' primitive
    phylanx::execution_tree::compiler::compiled_function* cfadd = env.find("add");
    HPX_TEST(cfadd != nullptr);

    // extract factory for compiling variables
    phylanx::execution_tree::compiler::compiled_function* cfx = env.find("x");
    HPX_TEST(cfx != nullptr);
    phylanx::execution_tree::compiler::compiled_function* cfy = env.find("y");
    HPX_TEST(cfy != nullptr);

    auto x = (*cfx)(phylanx::execution_tree::compiler::function_list{});
    auto y = (*cfy)(phylanx::execution_tree::compiler::function_list{});
    auto add = (*cfadd)(phylanx::execution_tree::compiler::function_list{x, y});

    // invoke '+' primitive
    HPX_TEST_EQ(42.0,
        phylanx::execution_tree::extract_numeric_value(
            add()
        )[0]);
}

void test_define_variable()
{
    phylanx::ast::expression expr =
        phylanx::ast::generate_ast("define(x, 42.0)");

    phylanx::execution_tree::compiler::function_list snippets;
    auto x = phylanx::execution_tree::compile(expr, snippets);

    HPX_TEST_EQ(42.0,
        phylanx::execution_tree::extract_numeric_value(
            x()
        )[0]);
}

void test_define_variable_ref()
{
    phylanx::ast::expression expr1 =
        phylanx::ast::generate_ast("define(x, 42.0)");
    phylanx::ast::expression expr2 =
        phylanx::ast::generate_ast("define(y, x)");

    phylanx::execution_tree::compiler::function_list snippets;
    phylanx::execution_tree::compiler::environment env =
        phylanx::execution_tree::compiler::default_environment();

    phylanx::execution_tree::compile(expr1, snippets, env);
    auto y = phylanx::execution_tree::compile(expr2, snippets, env);

    HPX_TEST_EQ(42.0,
        phylanx::execution_tree::extract_numeric_value(
            y()
        )[0]);
}

void test_define_constant_function()
{
    phylanx::ast::expression expr =
        phylanx::ast::generate_ast("define(x, a, 42.0)");

    phylanx::execution_tree::compiler::function_list snippets;
    auto x = phylanx::execution_tree::compile(expr, snippets);

    auto arg = phylanx::ir::node_data<double>{41.0};
    HPX_TEST_EQ(42.0,
        phylanx::execution_tree::extract_numeric_value(
            x(std::move(arg))
        )[0]);
}

void test_define_simple_function()
{
    phylanx::ast::expression expr =
        phylanx::ast::generate_ast("define(x, a, a)");

    phylanx::execution_tree::compiler::function_list snippets;
    auto x = phylanx::execution_tree::compile(expr, snippets);

    auto arg = phylanx::ir::node_data<double>{42.0};
    HPX_TEST_EQ(42.0,
        phylanx::execution_tree::extract_numeric_value(
            x(std::move(arg))
        )[0]);
}

void test_define_simple_function_arg1()
{
    phylanx::ast::expression expr =
        phylanx::ast::generate_ast("define(x, a, b, a)");

    phylanx::execution_tree::compiler::function_list snippets;
    auto x = phylanx::execution_tree::compile(expr, snippets);

    auto arg1 = phylanx::ir::node_data<double>{41.0};
    auto arg2 = phylanx::ir::node_data<double>{42.0};
    HPX_TEST_EQ(41.0,
        phylanx::execution_tree::extract_numeric_value(
            x(std::move(arg1), std::move(arg2))
        )[0]);
}

void test_define_simple_function_arg2()
{
    phylanx::ast::expression expr =
        phylanx::ast::generate_ast("define(x, a, b, b)");

    phylanx::execution_tree::compiler::function_list snippets;
    auto x = phylanx::execution_tree::compile(expr, snippets);

    auto arg1 = phylanx::ir::node_data<double>{41.0};
    auto arg2 = phylanx::ir::node_data<double>{42.0};
    HPX_TEST_EQ(42.0,
        phylanx::execution_tree::extract_numeric_value(
            x(std::move(arg1), std::move(arg2))
        )[0]);
}

void test_define_return_function()
{
    phylanx::ast::expression expr1 =
        phylanx::ast::generate_ast("define(x, a, a)");
    phylanx::ast::expression expr2 =
        phylanx::ast::generate_ast("define(y, x(42.0))");

    phylanx::execution_tree::compiler::function_list snippets;
    phylanx::execution_tree::compiler::environment env =
        phylanx::execution_tree::compiler::default_environment();

    phylanx::execution_tree::compile(expr1, snippets, env);
    auto y = phylanx::execution_tree::compile(expr2, snippets, env);
    HPX_TEST_EQ(42.0,
        phylanx::execution_tree::extract_numeric_value(
            y()
        )[0]);
}

void test_define_call_function()
{
    phylanx::ast::expression expr1 =
        phylanx::ast::generate_ast("define(x, a, a)");
    phylanx::ast::expression expr2 =
        phylanx::ast::generate_ast("define(y, a, x(a))");

    phylanx::execution_tree::compiler::function_list snippets;
    phylanx::execution_tree::compiler::environment env =
        phylanx::execution_tree::compiler::default_environment();

    phylanx::execution_tree::compile(expr1, snippets, env);
    auto y = phylanx::execution_tree::compile(expr2, snippets, env);

    auto arg = phylanx::ir::node_data<double>{42.0};
    HPX_TEST_EQ(42.0,
        phylanx::execution_tree::extract_numeric_value(
            y(std::move(arg))
        )[0]);
}

void test_use_builtin_function()
{
    phylanx::ast::expression expr1 =
        phylanx::ast::generate_ast("define(x, a, b, a + b)");

    phylanx::execution_tree::compiler::function_list snippets;
    phylanx::execution_tree::compiler::environment env =
        phylanx::execution_tree::compiler::default_environment();

    auto x = phylanx::execution_tree::compile(expr1, snippets, env);

    auto arg1 = phylanx::ir::node_data<double>{41.0};
    auto arg2 = phylanx::ir::node_data<double>{1.0};
    HPX_TEST_EQ(42.0,
        phylanx::execution_tree::extract_numeric_value(
            x(std::move(arg1), std::move(arg2))
        )[0]);
}

void test_use_builtin_function_ind()
{
    phylanx::ast::expression expr1 =
        phylanx::ast::generate_ast("define(x, f, a, b, f(a, b))");
    phylanx::ast::expression expr2 =
        phylanx::ast::generate_ast("define(y, a, b, x(add, a, b))");

    phylanx::execution_tree::compiler::function_list snippets;
    phylanx::execution_tree::compiler::environment env =
        phylanx::execution_tree::compiler::default_environment();

    phylanx::execution_tree::compile(expr1, snippets, env);
    auto y = phylanx::execution_tree::compile(expr2, snippets, env);

    auto arg1 = phylanx::ir::node_data<double>{41.0};
    auto arg2 = phylanx::ir::node_data<double>{1.0};
    HPX_TEST_EQ(42.0,
        phylanx::execution_tree::extract_numeric_value(
            y(std::move(arg1), std::move(arg2))
        )[0]);
}

// void test_define_curry_function()
// {
//     char const* exprstr1 = R"(
//         define(f1, arg0,
//             block(
//                 define(f2, arg1, arg0 + arg1),
//                 f2
//             )
//         )
//     )";
//
//     phylanx::ast::expression expr1 = phylanx::ast::generate_ast(exprstr1);
//
//     phylanx::execution_tree::compiler::function_list snippets;
//     phylanx::execution_tree::compiler::environment env =
//         phylanx::execution_tree::compiler::default_environment();
//
//     auto f1 = phylanx::execution_tree::compile(expr1, snippets, env);
//
//     auto arg1 = phylanx::ir::node_data<double>{41.0};
//     auto arg2 = phylanx::ir::node_data<double>{1.0};
//
//     phylanx::execution_tree::compiler::function f2 = f1(std::move(arg1));
//
//     HPX_TEST_EQ(42.0,
//         phylanx::execution_tree::extract_numeric_value(
//             f2(std::move(arg2))
//         )[0]);
// }

void test_recursive_function()
{
    char const* exprstr = R"(
        define(fact, arg0,
            if(arg0 <= 1,
                1,
                arg0 * fact(arg0 - 1)
            )
        )
    )";

    phylanx::execution_tree::compiler::function_list snippets;
    auto fact = phylanx::execution_tree::compile(exprstr, snippets);

    auto arg = phylanx::ir::node_data<double>{10.0};
    HPX_TEST_EQ(3628800.0,
        phylanx::execution_tree::extract_numeric_value(
            fact(std::move(arg))
        )[0]);
}

int main(int argc, char* argv[])
{
    test_builtin_environment();
    test_builtin_environment_vars();

    test_define_variable();
    test_define_variable_ref();

    test_define_constant_function();
    test_define_simple_function();
    test_define_simple_function_arg1();
    test_define_simple_function_arg2();

    test_define_return_function();
    test_define_call_function();

    test_use_builtin_function();
    test_use_builtin_function_ind();

//     test_define_curry_function();

    test_recursive_function();

    return hpx::util::report_errors();
}

