// Copyright (c) 2017-2019 Hartmut Kaiser
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/phylanx.hpp>

#include <hpx/hpx_main.hpp>
#include <hpx/runtime/find_here.hpp>
#include <hpx/modules/testing.hpp>

#include <list>
#include <utility>

#include <blaze/Math.h>

void test_define_variable()
{
    phylanx::execution_tree::eval_context ctx;

    auto comp = phylanx::execution_tree::create_compiler();
    auto const& def = comp.compile(hpx::launch::sync, "define(x, 42.0)");

    // executing the define will create the variable, executing whatever it
    // returned will bind the value to the variable
    def.run(ctx);

    auto const& code = comp.compile(hpx::launch::sync, "x");
    auto x = code.run(ctx);

    // executing x() will return the bound value
    HPX_TEST_EQ(42.0,
        phylanx::execution_tree::extract_numeric_value(
            x()
        )[0]);
}

void test_define_variable_block()
{
    auto comp = phylanx::execution_tree::create_compiler();
    auto const& code = comp.compile(hpx::launch::sync, "define(x, 42.0) x");
    auto x = code.run();

    // executing the block will create the variable, and bind the value to the
    // variable
    HPX_TEST_EQ(42.0,
        phylanx::execution_tree::extract_numeric_value(
            x()
        )[0]);
}

void test_define_variable_ref()
{
    auto expr = phylanx::ast::generate_ast(R"(
            define(x, 42.0)
            define(y, x)
            store(x, 0)
            y
        )");

    auto comp = phylanx::execution_tree::create_compiler();
    auto const& code = comp.compile(hpx::launch::sync, expr);
    auto y = code.run();

    HPX_TEST_EQ(42.0,
        phylanx::execution_tree::extract_numeric_value(
            y()
        )[0]);
}

void test_define_variable_ref_expr()
{
    // verify that names are bound to the expression result at the point of
    // definition
    auto expr = phylanx::ast::generate_ast(R"(
            define(x, 42.0)
            define(y, x + 1)
            store(x, 0)
            y
        )");

    auto comp = phylanx::execution_tree::create_compiler();
    auto const& code = comp.compile(hpx::launch::sync, expr);
    auto y = code.run();

    HPX_TEST_EQ(43.0,
        phylanx::execution_tree::extract_numeric_value(
            y()
        )[0]);
}

void test_redefine_variable()
{
    auto expr = phylanx::ast::generate_ast(R"(
            define(x, 42.0)
            define(x, 43.0)
            x
        )");

    auto comp = phylanx::execution_tree::create_compiler();
    auto const& code = comp.compile(hpx::launch::sync, expr);
    auto x = code.run();

    HPX_TEST_EQ(43.0,
        phylanx::execution_tree::extract_numeric_value(
            x()
        )[0]);
}

void test_define_constant_function()
{
    auto expr = phylanx::ast::generate_ast(R"(
            define(x, a, 42.0)
            x
        )");

    auto comp = phylanx::execution_tree::create_compiler();
    auto const& code = comp.compile(hpx::launch::sync, expr);
    auto x = code.run();

    auto arg = phylanx::ir::node_data<double>{41.0};
    HPX_TEST_EQ(42.0,
        phylanx::execution_tree::extract_numeric_value(
            x(std::move(arg))
        )[0]);
}

void test_define_simple_function()
{
    auto expr = phylanx::ast::generate_ast(R"(
            define(x, a, a)
            x
        )");

    auto comp = phylanx::execution_tree::create_compiler();
    auto const& code = comp.compile(hpx::launch::sync, expr);
    auto x = code.run();

    auto arg = phylanx::ir::node_data<double>{42.0};
    HPX_TEST_EQ(42.0,
        phylanx::execution_tree::extract_numeric_value(
            x(std::move(arg))
        )[0]);
}

void test_define_simple_function_arg1()
{
    auto expr = phylanx::ast::generate_ast(R"(
            define(x, a, b, a)
            x
        )");

    auto comp = phylanx::execution_tree::create_compiler();
    auto const& code = comp.compile(hpx::launch::sync, expr);
    auto x = code.run();

    auto arg1 = phylanx::ir::node_data<double>{41.0};
    auto arg2 = phylanx::ir::node_data<double>{42.0};
    HPX_TEST_EQ(41.0,
        phylanx::execution_tree::extract_numeric_value(
            x(std::move(arg1), std::move(arg2))
        )[0]);
}

void test_define_simple_function_arg2()
{
    auto expr = phylanx::ast::generate_ast(R"(
            define(x, a, b, b)
            x
        )");

    auto comp = phylanx::execution_tree::create_compiler();
    auto const& code = comp.compile(hpx::launch::sync, expr);
    auto x = code.run();

    auto arg1 = phylanx::ir::node_data<double>{41.0};
    auto arg2 = phylanx::ir::node_data<double>{42.0};
    HPX_TEST_EQ(42.0,
        phylanx::execution_tree::extract_numeric_value(
            x(std::move(arg1), std::move(arg2))
        )[0]);
}

void test_define_return_simple_function()
{
    auto expr = phylanx::ast::generate_ast(R"(
            define(x, a, a)
            x
        )");

    auto comp = phylanx::execution_tree::create_compiler();
    auto const& code = comp.compile(hpx::launch::sync, expr);
    auto x = code.run();

    auto arg = phylanx::ir::node_data<double>{42.0};
    HPX_TEST_EQ(42.0,
        phylanx::execution_tree::extract_numeric_value(
            x(std::move(arg))
        )[0]);
}

void test_define_return_function()
{
    auto expr = phylanx::ast::generate_ast(R"(
            define(x, a, a)
            define(y, x(42.0))
            y
        )");

    auto comp = phylanx::execution_tree::create_compiler();
    auto const& code = comp.compile(hpx::launch::sync, expr);
    auto y = code.run();

    HPX_TEST_EQ(42.0,
        phylanx::execution_tree::extract_numeric_value(
            y()
        )[0]);
}

void test_define_call_function()
{
    phylanx::execution_tree::eval_context ctx;

    auto expr = phylanx::ast::generate_ast(R"(
            define(x, a, a)
            define(y, a, x(a))
            y
        )");

    auto comp = phylanx::execution_tree::create_compiler();
    auto const& code = comp.compile(hpx::launch::sync, expr);
    auto y = code.run(ctx);

    auto arg = phylanx::ir::node_data<double>{42.0};
    HPX_TEST_EQ(42.0,
        phylanx::execution_tree::extract_numeric_value(
            y(ctx, std::move(arg))
        )[0]);
}

void test_use_builtin_function()
{
    auto comp = phylanx::execution_tree::create_compiler();
    auto const& code =
        comp.compile(hpx::launch::sync, "define(x, a, b, a + b) x");
    auto x = code.run();

    auto arg1 = phylanx::ir::node_data<double>{41.0};
    auto arg2 = phylanx::ir::node_data<double>{1.0};
    HPX_TEST_EQ(42.0,
        phylanx::execution_tree::extract_numeric_value(
            x(std::move(arg1), std::move(arg2))
        )[0]);
}

void test_use_builtin_function_ind()
{
    phylanx::execution_tree::eval_context ctx;

    auto expr = phylanx::ast::generate_ast(R"(
            define(x, f, a, b, f(a, b))
            define(y, a, b, x(__add, a, b))
            y
        )");

    auto comp = phylanx::execution_tree::create_compiler();
    auto const& code = comp.compile(hpx::launch::sync, expr);

    auto y = code.run(ctx);

    auto arg1 = phylanx::ir::node_data<double>{41.0};
    auto arg2 = phylanx::ir::node_data<double>{1.0};
    HPX_TEST_EQ(42.0,
        phylanx::execution_tree::extract_numeric_value(
            y(ctx, std::move(arg1), std::move(arg2))
        )[0]);
}

/// <image url="$(ItemDir)/images/compiler_test_define_curry_function.dot.png" />
//
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

    auto comp = phylanx::execution_tree::create_compiler();
    auto const& code = comp.compile(hpx::launch::sync, expr1);

    auto f1 = code.run();

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
    char const* exprstr1 = R"(
        define(f1, arg0,
            block(
                define(local_arg0, arg0),
                define(f2, arg1, local_arg0 + arg1),
                f2
            )
        )
        f1
    )";

    auto expr1 = phylanx::ast::generate_ast(exprstr1);

    auto comp = phylanx::execution_tree::create_compiler();
    auto const& code = comp.compile(hpx::launch::sync, expr1);

    auto f1 = code.run();

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
    phylanx::execution_tree::eval_context ctx;

    char const* exprstr = R"(
        define(fact, arg0,
            if(arg0 <= 1,
                1,
                arg0 * fact(arg0 - 1)
            )
        )
        fact
    )";

    auto comp = phylanx::execution_tree::create_compiler();
    auto const& code = comp.compile(hpx::launch::sync, exprstr);

    auto fact = code.run(ctx);

    auto arg = phylanx::ir::node_data<double>{10.0};
    HPX_TEST_EQ(3628800.0,
        phylanx::execution_tree::extract_numeric_value(
            fact(ctx, std::move(arg))
        )[0]);
}

void test_define_call_lambda_function_noarg()
{
    phylanx::execution_tree::eval_context ctx;

    auto expr = phylanx::ast::generate_ast(R"(
            define(x, lambda(42))
            define(y, x())
            y
        )");

    auto comp = phylanx::execution_tree::create_compiler();
    auto const& code = comp.compile(hpx::launch::sync, expr);

    auto y = code.run(ctx);

    HPX_TEST_EQ(42.0,
        phylanx::execution_tree::extract_numeric_value(
            y(ctx)
        )[0]);
}

void test_define_call_lambda_function()
{
    auto expr = phylanx::ast::generate_ast(R"(
            define(x, lambda(a, a + 1))
            x
        )");

    auto comp = phylanx::execution_tree::create_compiler();
    auto const& code = comp.compile(hpx::launch::sync, expr);

    auto x = code.run();

    auto arg = phylanx::ir::node_data<double>{42.0};
    HPX_TEST_EQ(43.0,
        phylanx::execution_tree::extract_numeric_value(
            x(std::move(arg))
        )[0]);
}

void test_define_call_lambda_function_direct()
{
    auto expr = phylanx::ast::generate_ast(R"(
            define(x, lambda(a, a + 1))
            x(42)
        )");

    auto comp = phylanx::execution_tree::create_compiler();
    auto const& code = comp.compile(hpx::launch::sync, expr);

    auto x = code.run();

    HPX_TEST_EQ(43.0,
        phylanx::execution_tree::extract_numeric_value(
            x()
        )[0]);
}

void test_define_call_lambda_function_ind1()
{
    auto expr = phylanx::ast::generate_ast(R"(
            define(x, b, block(define(l, lambda(a, a + b)), l))
            x(42)
        )");

    auto comp = phylanx::execution_tree::create_compiler();
    auto const& code = comp.compile(hpx::launch::sync, expr);

    auto lambda = code.run();

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
    auto expr = phylanx::ast::generate_ast(R"(
            define(x, lambda(b, lambda(a, a + b)))
            x(42)
        )");

    auto comp = phylanx::execution_tree::create_compiler();
    auto const& code = comp.compile(hpx::launch::sync, expr);

    auto lambda = code.run();

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
    auto expr = phylanx::ast::generate_ast(R"(
            define(x, b, lambda(a, a + b))
            x(42)
        )");

    auto comp = phylanx::execution_tree::create_compiler();
    auto const& code = comp.compile(hpx::launch::sync, expr);

    auto lambda = code.run();

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
    phylanx::execution_tree::eval_context ctx;

    auto expr = phylanx::ast::generate_ast(R"(
            define(x, b, lambda(a, a + b))
            define(y, b, x(b))
            y
        )");

    auto comp = phylanx::execution_tree::create_compiler();
    auto const& code = comp.compile(hpx::launch::sync, expr);

    auto y = code.run(ctx);

    auto arg_a = phylanx::ir::node_data<double>{42.0};
    phylanx::execution_tree::compiler::function lambda =
        y(ctx, std::move(arg_a));

    auto arg_b = phylanx::ir::node_data<double>{1.0};
    HPX_TEST_EQ(43.0,
        phylanx::execution_tree::extract_numeric_value(
            lambda(ctx, std::move(arg_b))
        )[0]);

    arg_b = phylanx::ir::node_data<double>{2.0};
    HPX_TEST_EQ(44.0,
        phylanx::execution_tree::extract_numeric_value(
            lambda(ctx, std::move(arg_b))
        )[0]);
}

/// <image url="$(ItemDir)/images/test_define_variable_function_call.dot.png" />
//
void test_define_variable_function_call()
{
    phylanx::execution_tree::eval_context ctx;

    auto comp = phylanx::execution_tree::create_compiler();
    auto const& pts = comp.compile(hpx::launch::sync, "[[1, 2], [3, 4]]");

    auto const& def = comp.define_variable(
        hpx::launch::sync, "<unknown>", "sys_argv", pts.run(ctx).arg_);
    def.run(ctx);

    auto expr = phylanx::ast::generate_ast(R"(
        define(f, pts, block(
            define(var, expand_dims(slice_column(pts, 0), 1)),
            argmin(sqrt(power(var - var, 2) + power(var - var, 2)), 1)
        ))
        apply(f, sys_argv)
    )");

    auto const& f = comp.compile(hpx::launch::sync, expr);

    blaze::DynamicVector<double> expected{0, 0};
    auto result =
        phylanx::execution_tree::extract_numeric_value(f.run(ctx).arg_);
    HPX_TEST_EQ(expected, result.vector());
}

int main(int argc, char* argv[])
{
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

    test_define_variable_function_call();

    return hpx::util::report_errors();
}

