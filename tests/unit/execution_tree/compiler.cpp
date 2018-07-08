// Copyright (c) 2017-2018 Hartmut Kaiser
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/phylanx.hpp>

#include <hpx/hpx_main.hpp>
#include <hpx/runtime/find_here.hpp>
#include <hpx/util/lightweight_test.hpp>

#include <list>
#include <utility>

void test_builtin_environment()
{
    hpx::id_type here = hpx::find_here();

    // generate default environment
    phylanx::execution_tree::compiler::environment env =
        phylanx::execution_tree::compiler::default_environment(
            phylanx::execution_tree::get_all_known_patterns(),
            here);

    // add a variable 'x = 41.0'
    auto create_var = phylanx::execution_tree::compiler::define_operation{here};
    auto varx = create_var(phylanx::ir::node_data<double>{41.0}, "x");
    env.define("x", phylanx::execution_tree::compiler::access_variable{varx});

    // extract factory for compiling variables
    phylanx::execution_tree::compiler::compiled_function* cfx = env.find("x");
    HPX_TEST(cfx != nullptr);
    auto def = (*cfx)(std::list<phylanx::execution_tree::compiler::function>{},
        "x", "<unknown>");

    // invoking the factory function actually creates and binds the variable
    auto x = def();

    // invoke 'x' primitive
    HPX_TEST_EQ(41.0, phylanx::execution_tree::extract_numeric_value(x())[0]);
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
    auto create_var = phylanx::execution_tree::compiler::define_operation{here};

    auto varx = create_var(phylanx::ir::node_data<double>{41.0}, "x");
    auto vary = create_var(phylanx::ir::node_data<double>{1.0}, "y");

    env.define("x", phylanx::execution_tree::compiler::access_variable{varx});
    env.define("y", phylanx::execution_tree::compiler::access_variable{vary});

    // extract factory for compiling the '+' primitive
    phylanx::execution_tree::compiler::compiled_function* cfadd = env.find("__add");
    HPX_TEST(cfadd != nullptr);

    // extract factory for compiling variables
    phylanx::execution_tree::compiler::compiled_function* cfx = env.find("x");
    HPX_TEST(cfx != nullptr);
    phylanx::execution_tree::compiler::compiled_function* cfy = env.find("y");
    HPX_TEST(cfy != nullptr);

    std::list<phylanx::execution_tree::compiler::function> funcs;
    auto defx = (*cfx)(funcs, "x", "<unknown>");
    auto defy = (*cfy)(funcs, "y", "<unknown>");

    // invoking the factory functions defx and defy actually creates and binds
    // the variables
    auto add = (*cfadd)(
        std::list<phylanx::execution_tree::compiler::function>{defx(), defy()},
        "add", "<unknown>");

    // invoke '+' primitive
    HPX_TEST_EQ(42.0, phylanx::execution_tree::extract_numeric_value(add())[0]);
}

void test_builtin_environment_vars_lazy()
{
    hpx::id_type here = hpx::find_here();

    // generate default environment
    phylanx::execution_tree::compiler::environment env =
        phylanx::execution_tree::compiler::default_environment(
            phylanx::execution_tree::get_all_known_patterns(),
            here);

    // add two variables, 'x' and 'y'
    auto create_var = phylanx::execution_tree::compiler::define_operation{here};

    auto varx = create_var(phylanx::ir::node_data<double>{41.0}, "x");
    auto vary = create_var(phylanx::ir::node_data<double>{1.0}, "y");

    env.define("x", phylanx::execution_tree::compiler::access_variable{varx});
    env.define("y", phylanx::execution_tree::compiler::access_variable{vary});

    // extract factory for compiling the '+' primitive
    phylanx::execution_tree::compiler::compiled_function* cfadd = env.find("__add");
    HPX_TEST(cfadd != nullptr);

    // extract factory for compiling variables
    phylanx::execution_tree::compiler::compiled_function* cfx = env.find("x");
    HPX_TEST(cfx != nullptr);
    phylanx::execution_tree::compiler::compiled_function* cfy = env.find("y");
    HPX_TEST(cfy != nullptr);

    std::list<phylanx::execution_tree::compiler::function> funcs;
    auto defx = (*cfx)(funcs, "x", "<unknown>");
    auto defy = (*cfy)(funcs, "y", "<unknown>");

    auto add = (*cfadd)(funcs, "__add", "<unknown>");

    // invoking the factory functions defx and defy actually creates and binds
    // the variables

    // invoke '+' primitive
    HPX_TEST_EQ(42.0,
        phylanx::execution_tree::extract_numeric_value(
            add({defx(), defy()}))[0]);
}

void test_define_variable()
{
    phylanx::execution_tree::compiler::function_list snippets;
    phylanx::execution_tree::compiler::environment env =
        phylanx::execution_tree::compiler::default_environment();

    auto defexpr = phylanx::ast::generate_ast("define(x, 42.0)");
    auto def = phylanx::execution_tree::compile(defexpr, snippets, env);

    // executing the define will create the variable, executing whatever it
    // returned will bind the value to the variable
    HPX_TEST_EQ(42.0,
        phylanx::execution_tree::numeric_operand_sync(
            def(), {}
        )[0]);

    auto xexpr = phylanx::ast::generate_ast("x");
    auto x = phylanx::execution_tree::compile(xexpr, snippets, env);

    // executing x() will return the bound value
    HPX_TEST_EQ(42.0,
        phylanx::execution_tree::extract_numeric_value(
            x()
        )[0]);
}

void test_define_variable_block()
{
    phylanx::execution_tree::compiler::function_list snippets;
    auto xexpr = phylanx::ast::generate_ast("block(define(x, 42.0), x)");
    auto x = phylanx::execution_tree::compile(xexpr, snippets);

    // executing the block will create the variable, and bind the value to the
    // variable
    HPX_TEST_EQ(42.0,
        phylanx::execution_tree::extract_numeric_value(
            x()
        )[0]);
}

void test_define_variable_ref()
{
    auto expr = phylanx::ast::generate_ast(R"(block(
            define(x, 42.0),
            define(y, x),
            store(x, 0),
            y
        ))");

    phylanx::execution_tree::compiler::function_list snippets;
    phylanx::execution_tree::compiler::environment env =
        phylanx::execution_tree::compiler::default_environment();

    auto y = phylanx::execution_tree::compile(expr, snippets, env);

    HPX_TEST_EQ(42.0,
        phylanx::execution_tree::extract_numeric_value(
            y()
        )[0]);
}

void test_define_variable_ref_expr()
{
    // verify that names are bound to the expression result at the point of
    // definition
    auto expr = phylanx::ast::generate_ast(R"(block(
            define(x, 42.0),
            define(y, x + 1),
            store(x, 0),
            y
        ))");

    phylanx::execution_tree::compiler::function_list snippets;
    phylanx::execution_tree::compiler::environment env =
        phylanx::execution_tree::compiler::default_environment();

    auto y = phylanx::execution_tree::compile(expr, snippets, env);

    HPX_TEST_EQ(43.0,
        phylanx::execution_tree::extract_numeric_value(
            y()
        )[0]);
}

void test_redefine_variable()
{
    auto expr = phylanx::ast::generate_ast(R"(block(
            define(x, 42.0),
            define(x, 43.0),
            x
        ))");

    phylanx::execution_tree::compiler::function_list snippets;
    phylanx::execution_tree::compiler::environment env =
        phylanx::execution_tree::compiler::default_environment();

    auto x = phylanx::execution_tree::compile(expr, snippets, env);

    HPX_TEST_EQ(43.0,
        phylanx::execution_tree::extract_numeric_value(
            x()
        )[0]);
}

void test_define_constant_function()
{
    auto expr = phylanx::ast::generate_ast("block(define(x, a, 42.0), x)");

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
    auto expr = phylanx::ast::generate_ast("block(define(x, a, a), x)");

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
    auto expr = phylanx::ast::generate_ast("block(define(x, a, b, a), x)");

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
    auto expr = phylanx::ast::generate_ast("block(define(x, a, b, b), x)");

    phylanx::execution_tree::compiler::function_list snippets;
    auto x = phylanx::execution_tree::compile(expr, snippets);

    auto arg1 = phylanx::ir::node_data<double>{41.0};
    auto arg2 = phylanx::ir::node_data<double>{42.0};
    HPX_TEST_EQ(42.0,
        phylanx::execution_tree::extract_numeric_value(
            x(std::move(arg1), std::move(arg2))
        )[0]);
}

void test_define_return_simple_function()
{
    auto expr = phylanx::ast::generate_ast(R"(block(
            define(x, a, a),
            x
        ))");

    phylanx::execution_tree::compiler::function_list snippets;
    phylanx::execution_tree::compiler::environment env =
        phylanx::execution_tree::compiler::default_environment();

    auto x = phylanx::execution_tree::compile(expr, snippets, env);

    auto arg = phylanx::ir::node_data<double>{42.0};
    HPX_TEST_EQ(42.0,
        phylanx::execution_tree::extract_numeric_value(
            x(std::move(arg))
        )[0]);
}

void test_define_return_function()
{
    auto expr = phylanx::ast::generate_ast(R"(block(
            define(x, a, a),
            define(y, x(42.0)),
            y
        ))");

    phylanx::execution_tree::compiler::function_list snippets;
    phylanx::execution_tree::compiler::environment env =
        phylanx::execution_tree::compiler::default_environment();

    auto y = phylanx::execution_tree::compile(expr, snippets, env);

    HPX_TEST_EQ(42.0,
        phylanx::execution_tree::extract_numeric_value(
            y()
        )[0]);
}

void test_define_call_function()
{
    auto expr = phylanx::ast::generate_ast(R"(block(
            define(x, a, a),
            define(y, a, x(a)),
            y
        ))");

    phylanx::execution_tree::compiler::function_list snippets;
    phylanx::execution_tree::compiler::environment env =
        phylanx::execution_tree::compiler::default_environment();

    auto y = phylanx::execution_tree::compile(expr, snippets, env);

    auto arg = phylanx::ir::node_data<double>{42.0};
    HPX_TEST_EQ(42.0,
        phylanx::execution_tree::extract_numeric_value(
            y(std::move(arg))
        )[0]);
}

void test_use_builtin_function()
{
    auto expr1 = phylanx::ast::generate_ast("block(define(x, a, b, a + b), x)");

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
    auto expr = phylanx::ast::generate_ast(R"(block(
            define(x, f, a, b, f(a, b)),
            define(y, a, b, x(__add, a, b)),
            y
        ))");

    phylanx::execution_tree::compiler::function_list snippets;
    phylanx::execution_tree::compiler::environment env =
        phylanx::execution_tree::compiler::default_environment();

    auto y = phylanx::execution_tree::compile(expr, snippets, env);

    auto arg1 = phylanx::ir::node_data<double>{41.0};
    auto arg2 = phylanx::ir::node_data<double>{1.0};
    HPX_TEST_EQ(42.0,
        phylanx::execution_tree::extract_numeric_value(
            y(std::move(arg1), std::move(arg2))
        )[0]);
}

void test_define_curry_function()
{
    char const* exprstr1 = R"(block(
        define(f1, arg0,
            block(
                define(f2, arg1, arg0 + arg1),
                f2
            )
        ),
        f1
    ))";

    auto expr1 = phylanx::ast::generate_ast(exprstr1);

    phylanx::execution_tree::compiler::function_list snippets;
    phylanx::execution_tree::compiler::environment env =
        phylanx::execution_tree::compiler::default_environment();

    auto f1 = phylanx::execution_tree::compile(expr1, snippets, env);

    auto arg0 = phylanx::ir::node_data<double>{41.0};
    auto arg1 = phylanx::ir::node_data<double>{1.0};

    phylanx::execution_tree::compiler::function f2 = f1(std::move(arg0));

    HPX_TEST_EQ(42.0,
        phylanx::execution_tree::extract_numeric_value(
            f2(std::move(arg1))
        )[0]);

    auto arg3 = phylanx::ir::node_data<double>{43.0};

    f2 = f1(std::move(arg3));

    HPX_TEST_EQ(44.0,
        phylanx::execution_tree::extract_numeric_value(
            f2(std::move(arg1))
        )[0]);
}

/// <image url="$(ItemDir)/images/compiler_test_embedded_function.dot.png" />
//
void test_define_embedded_function()
{
    char const* exprstr1 = R"(block(
        define(f1, arg0,
            block(
                define(local_arg0, arg0),
                define(f2, arg1, local_arg0 + arg1),
                f2
            )
        ),
        f1
    ))";

    auto expr1 = phylanx::ast::generate_ast(exprstr1);

    phylanx::execution_tree::compiler::function_list snippets;
    phylanx::execution_tree::compiler::environment env =
        phylanx::execution_tree::compiler::default_environment();

    auto f1 = phylanx::execution_tree::compile(expr1, snippets, env);

    auto arg0 = phylanx::ir::node_data<double>{41.0};
    auto arg1 = phylanx::ir::node_data<double>{1.0};

    phylanx::execution_tree::compiler::function f2 = f1(std::move(arg0));

    HPX_TEST_EQ(42.0,
        phylanx::execution_tree::extract_numeric_value(
            f2(std::move(arg1))
        )[0]);

    auto arg3 = phylanx::ir::node_data<double>{43.0};

    f2 = f1(std::move(arg3));

    HPX_TEST_EQ(44.0,
        phylanx::execution_tree::extract_numeric_value(
            f2(std::move(arg1))
        )[0]);
}

void test_recursive_function()
{
    char const* exprstr = R"(block(
        define(fact, arg0,
            if(arg0 <= 1,
                1,
                arg0 * fact(arg0 - 1)
            )
        ),
        fact
    ))";

    phylanx::execution_tree::compiler::function_list snippets;
    auto fact = phylanx::execution_tree::compile(exprstr, snippets);

    auto arg = phylanx::ir::node_data<double>{10.0};
    HPX_TEST_EQ(3628800.0,
        phylanx::execution_tree::extract_numeric_value(
            fact(std::move(arg))
        )[0]);
}

void test_define_call_lambda_function_noarg()
{
    auto expr = phylanx::ast::generate_ast(R"(block(
            define(x, lambda(42)),
            define(y, x()),
            y
        ))");

    phylanx::execution_tree::compiler::function_list snippets;
    phylanx::execution_tree::compiler::environment env =
        phylanx::execution_tree::compiler::default_environment();

    auto y = phylanx::execution_tree::compile(expr, snippets, env);

    HPX_TEST_EQ(42.0,
        phylanx::execution_tree::extract_numeric_value(
            y()
        )[0]);
}

void test_define_call_lambda_function()
{
    auto expr = phylanx::ast::generate_ast(R"(block(
            define(x, lambda(a, a + 1)),
            x
        ))");

    phylanx::execution_tree::compiler::function_list snippets;
    phylanx::execution_tree::compiler::environment env =
        phylanx::execution_tree::compiler::default_environment();

    auto x = phylanx::execution_tree::compile(expr, snippets, env);

    auto arg = phylanx::ir::node_data<double>{42.0};
    HPX_TEST_EQ(43.0,
        phylanx::execution_tree::extract_numeric_value(
            x(std::move(arg))
        )[0]);
}

void test_define_call_lambda_function_direct()
{
    auto expr = phylanx::ast::generate_ast(R"(block(
            define(x, lambda(a, a + 1)),
            x(42)
        ))");

    phylanx::execution_tree::compiler::function_list snippets;
    phylanx::execution_tree::compiler::environment env =
        phylanx::execution_tree::compiler::default_environment();

    auto x = phylanx::execution_tree::compile(expr, snippets, env);

    HPX_TEST_EQ(43.0,
        phylanx::execution_tree::extract_numeric_value(
            x()
        )[0]);
}

void test_define_call_lambda_function_ind1()
{
    auto expr = phylanx::ast::generate_ast(R"(block(
            define(x, b, block(define(l, lambda(a, a + b)), l)),
            x(42)
        ))");

    phylanx::execution_tree::compiler::function_list snippets;
    phylanx::execution_tree::compiler::environment env =
        phylanx::execution_tree::compiler::default_environment();

    auto lambda = phylanx::execution_tree::compile(expr, snippets, env);

    auto arg = phylanx::ir::node_data<double>{1.0};
    HPX_TEST_EQ(43.0,
        phylanx::execution_tree::extract_numeric_value(
            lambda(std::move(arg))
        )[0]);

    arg = phylanx::ir::node_data<double>{2.0};
    HPX_TEST_EQ(44.0,
        phylanx::execution_tree::extract_numeric_value(
            lambda(std::move(arg))
        )[0]);
}

/// <image url="$(ItemDir)/images/compiler_test_define_call_lambda_function_ind2.dot.png" />
void test_define_call_lambda_function_ind2()
{
    auto expr = phylanx::ast::generate_ast(R"(block(
            define(x, lambda(b, lambda(a, a + b))),
            x(42)
        ))");

    phylanx::execution_tree::compiler::function_list snippets;
    phylanx::execution_tree::compiler::environment env =
        phylanx::execution_tree::compiler::default_environment();

    auto lambda = phylanx::execution_tree::compile(expr, snippets, env);

    auto arg = phylanx::ir::node_data<double>{1.0};
    HPX_TEST_EQ(43.0,
        phylanx::execution_tree::extract_numeric_value(
            lambda(std::move(arg))
        )[0]);

    arg = phylanx::ir::node_data<double>{2.0};
    HPX_TEST_EQ(44.0,
        phylanx::execution_tree::extract_numeric_value(
            lambda(std::move(arg))
        )[0]);
}

void test_define_call_lambda_function_ind3()
{
    auto expr = phylanx::ast::generate_ast(R"(block(
            define(x, b, lambda(a, a + b)),
            x(42)
        ))");

    phylanx::execution_tree::compiler::function_list snippets;
    phylanx::execution_tree::compiler::environment env =
        phylanx::execution_tree::compiler::default_environment();

    auto lambda = phylanx::execution_tree::compile(expr, snippets, env);

    auto arg = phylanx::ir::node_data<double>{1.0};
    HPX_TEST_EQ(43.0,
        phylanx::execution_tree::extract_numeric_value(
            lambda(std::move(arg))
        )[0]);

    arg = phylanx::ir::node_data<double>{2.0};
    HPX_TEST_EQ(44.0,
        phylanx::execution_tree::extract_numeric_value(
            lambda(std::move(arg))
        )[0]);
}

void test_define_call_lambda_function_ind4()
{
    auto expr = phylanx::ast::generate_ast(R"(block(
            define(x, b, lambda(a, a + b)),
            define(y, b, x(b)),
            y
        ))");

    phylanx::execution_tree::compiler::function_list snippets;
    phylanx::execution_tree::compiler::environment env =
        phylanx::execution_tree::compiler::default_environment();

    auto y = phylanx::execution_tree::compile(expr, snippets, env);

    auto arg_a = phylanx::ir::node_data<double>{42.0};
    phylanx::execution_tree::compiler::function lambda = y(std::move(arg_a));

    auto arg_b = phylanx::ir::node_data<double>{1.0};
    HPX_TEST_EQ(43.0,
        phylanx::execution_tree::extract_numeric_value(
            lambda(std::move(arg_b))
        )[0]);

    arg_b = phylanx::ir::node_data<double>{2.0};
    HPX_TEST_EQ(44.0,
        phylanx::execution_tree::extract_numeric_value(
            lambda(std::move(arg_b))
        )[0]);
}

int main(int argc, char* argv[])
{
    test_builtin_environment();
    test_builtin_environment_vars();
    test_builtin_environment_vars_lazy();

    test_define_variable();
    test_define_variable_block();
    test_define_variable_ref();
    test_define_variable_ref_expr();
    test_redefine_variable();

    test_define_constant_function();
    test_define_simple_function();
    test_define_simple_function_arg1();
    test_define_simple_function_arg2();

    test_define_return_simple_function();
    test_define_return_function();
    test_define_call_function();

    test_use_builtin_function();
    test_use_builtin_function_ind();

    test_define_curry_function();
    test_define_embedded_function();

    test_recursive_function();

    test_define_call_lambda_function_noarg();
    test_define_call_lambda_function();
    test_define_call_lambda_function_direct();

    test_define_call_lambda_function_ind1();
    test_define_call_lambda_function_ind2();
    test_define_call_lambda_function_ind3();
    test_define_call_lambda_function_ind4();

    return hpx::util::report_errors();
}

