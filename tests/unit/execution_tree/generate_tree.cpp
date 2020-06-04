//  Copyright (c) 2017 Hartmut Kaiser
//  Copyright (c) 2017 Bibek Wagle
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/phylanx.hpp>

#include <hpx/hpx_main.hpp>
#include <hpx/modules/testing.hpp>

#include <string>

void test_generate_tree(
    std::string const& exprstr, char const* variables, double expected_result)
{
    phylanx::execution_tree::eval_context ctx;

    phylanx::execution_tree::compiler::environment env =
        phylanx::execution_tree::compiler::default_environment();
    phylanx::execution_tree::compiler::function_list snippets;

    auto const& vars =
        phylanx::execution_tree::compile(variables, snippets, env);
    vars.run(ctx);

    auto const& code = phylanx::execution_tree::compile(exprstr, snippets, env);
    auto f = code.run(ctx);

    HPX_TEST_EQ(expected_result,
        phylanx::execution_tree::extract_scalar_numeric_value(f(ctx)));
}

void test_generate_tree(
    std::string const& exprstr, char const* variables, bool expected_result)
{
    phylanx::execution_tree::eval_context ctx;

    phylanx::execution_tree::compiler::environment env =
        phylanx::execution_tree::compiler::default_environment();
    phylanx::execution_tree::compiler::function_list snippets;

    auto const& vars =
        phylanx::execution_tree::compile(variables, snippets, env);
    vars.run(ctx);

    auto const& code = phylanx::execution_tree::compile(exprstr, snippets, env);
    auto f = code.run(ctx);

    HPX_TEST_EQ(expected_result,
        phylanx::execution_tree::extract_scalar_boolean_value(f(ctx)) != 0);
}

void test_generate_tree_nil(std::string const& exprstr, char const* variables)
{
    phylanx::execution_tree::eval_context ctx;

    phylanx::execution_tree::compiler::environment env =
        phylanx::execution_tree::compiler::default_environment();
    phylanx::execution_tree::compiler::function_list snippets;

    auto const& vars =
        phylanx::execution_tree::compile(variables, snippets, env);
    vars.run(ctx);

    auto const& code = phylanx::execution_tree::compile(exprstr, snippets, env);
    auto f = code.run(ctx);

    HPX_TEST(!phylanx::execution_tree::valid(f(ctx)));
}

phylanx::execution_tree::primitive create_literal_value(double value)
{
    return phylanx::execution_tree::primitive(
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(value)));
}

void test_add_primitive()
{
    char const* variables = R"(
        define(A, 41.0)
        define(B, 1.0)
        define(C, 13.0)
    )";

    test_generate_tree("A + B", variables, 42.0);
    test_generate_tree("A + (B + C)", variables, 55.0);
    test_generate_tree("A + (B + A)", variables, 83.0);
    test_generate_tree("(A + B) + C", variables, 55.0);
}

void test_sub_primitive()
{
    char const* variables = R"(
        define(A, 41.0)
        define(B, 1.0)
        define(C, 13.0)
    )";

    test_generate_tree("A - B", variables, 40.0);
    test_generate_tree("A - (B - C)", variables, 53.0);
    test_generate_tree("A - (B - A)", variables, 81.0);
    test_generate_tree("(A - B) - C", variables, 27.0);
}

void test_mul_primitive()
{
    char const* variables = R"(
        define(A, 41.0)
        define(B, 1.0)
        define(C, 13.0)
    )";

    test_generate_tree("A * B", variables, 41.0);
    test_generate_tree("A * (B * C)", variables, 533.0);
    test_generate_tree("A * (B * A)", variables, 1681.0);
    test_generate_tree("(A * B) * C", variables, 533.0);
}

void test_math_primitives()
{
    char const* variables = R"(
        define(A, 41.0)
        define(B, 1.0)
        define(C, 13.0)
        define(D, 5.0)
    )";

    test_generate_tree("A + ((B - C) * A)", variables, -451.0);
    test_generate_tree("(A * (B + A)) + C", variables, 1735.0);
    test_generate_tree("(A * B) - C", variables, 28.0);
    test_generate_tree("exp(D) * 2", variables, std::exp(5.0) * 2);
    test_generate_tree("exp(D) + 2", variables, std::exp(5.0) + 2);
    test_generate_tree("exp(D) - 2", variables, std::exp(5.0) - 2);
}

void test_file_io_primitives()
{
    char const* variables = R"(
        define(A, 41.0)
        define(B, 1.0)
        define(C, 13.0)
    )";

    test_generate_tree("file_write(\"test_file\", A)", variables, 41.0);
    test_generate_tree("file_read(\"test_file\")", variables, 41.0);
}

void test_boolean_primitives()
{
    char const* variables = R"(
        define(A, 41.0)
        define(B, 0.0)
        define(C, true)
        define(D, false)
    )";

    test_generate_tree("!A", variables, false);
    test_generate_tree("!!A", variables, true);

    test_generate_tree("-A", variables, -41.0);
    test_generate_tree("--A", variables, 41.0);

    test_generate_tree("A && A", variables, true);
    test_generate_tree("A && B", variables, false);

    test_generate_tree("B || B", variables, false);
    test_generate_tree("A || B", variables, true);

    test_generate_tree("C && C", variables, true);
    test_generate_tree("C && D", variables, false);

    test_generate_tree("D || D", variables, false);
    test_generate_tree("C || D", variables, true);

    test_generate_tree("A == A", variables, true);
    test_generate_tree("A == B", variables, false);

    test_generate_tree("A != A", variables, false);
    test_generate_tree("A != B", variables, true);

    test_generate_tree("C == C", variables, true);
    test_generate_tree("C == D", variables, false);

    test_generate_tree("C != C", variables, false);
    test_generate_tree("C != D", variables, true);

    test_generate_tree("A <= A", variables, true);
    test_generate_tree("A <= B", variables, false);

    test_generate_tree("A < A", variables, false);
    test_generate_tree("A < B", variables, false);

    test_generate_tree("A >= A", variables, true);
    test_generate_tree("A >= B", variables, true);

    test_generate_tree("A > A", variables, false);
    test_generate_tree("A > B", variables, true);
}

void test_block_primitives()
{
    char const* variables = R"(
        define(A, 41.0)
        define(B, 1.0)
        define(C, 13.0)
    )";

    test_generate_tree("block(A, B, A + B + C)", variables, 55.0);
    test_generate_tree("parallel_block(A, B, A + B + C)", variables, 55.0);
}

void test_store_primitive()
{
    char const* variables = R"(
        define(A, 41.0)
        define(B, 1.0)
        define(C, 13.0)
        define(D, 42.0)
    )";

    test_generate_tree_nil("store(C, A)", variables);
    test_generate_tree_nil("store(C, A + B)", variables);

    test_generate_tree(
        "block(store(B, constant_like(0.0, A)), B)", variables, 0.0);
    test_generate_tree_nil("store(D, 0)", variables);
}

void test_complex_expression()
{
    char const* variables = R"(
        define(A, 3.0)
        define(B, 2.0)
    )";

    test_generate_tree("dot(A, B)", variables, 6.0);
    test_generate_tree("-dot(A, B)", variables, -6.0);
    test_generate_tree("exp(-dot(A, B))", variables, std::exp(-6.0));
    test_generate_tree("1.0 + exp(-dot(A, B))",
        variables, 1.0 + std::exp(-6.0));
    test_generate_tree("1.0 / (1.0 + exp(-dot(A, B)))",
        variables, 1.0 / (1.0 + std::exp(-6.0)));
}

void test_if_conditional()
{
    // Test 1
    //  two outcome true case
    char const* variables1 = R"(
        define(cond, 1.0)
        define(true_case, 42.0)
        define(false_case, 54.0)
    )";

    test_generate_tree("if(cond, true_case, false_case)", variables1, 42.0);

    // Test 2
    //  two outcome false case
    char const* variables2 = R"(
        define(cond, false)
        define(true_case, 42.0)
        define(false_case, 54.0)
    )";

    test_generate_tree("if(cond, true_case, false_case)", variables2, 54.0);

    // Test 3
    //  one outcome true case
    char const* variables3 = R"(
        define(cond, true)
        define(true_case, 42.0)
    )";

    test_generate_tree("if(cond, true_case)", variables3, 42.0);

    // Test 4
    //  one outcome false case
    char const* variables4 = R"(
        define(cond, false)
        define(true_case, 42.0)
    )";

    test_generate_tree_nil("if(cond, true_case)", variables4);
}

int main(int argc, char* argv[])
{
    test_add_primitive();
    test_sub_primitive();
    test_mul_primitive();
    test_math_primitives();
    test_file_io_primitives();
    test_boolean_primitives();
    test_block_primitives();
    test_store_primitive();
    test_complex_expression();
    test_if_conditional();

    return hpx::util::report_errors();
}

