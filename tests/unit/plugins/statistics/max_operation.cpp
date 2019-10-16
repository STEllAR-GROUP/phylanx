// Copyright (c) 2018 Bita Hasheminezhad
// Copyright (c) 2018 Hartmut Kaiser
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/phylanx.hpp>

#include <hpx/hpx_main.hpp>
#include <hpx/testing.hpp>

#include <cstdint>
#include <string>
#include <utility>

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

///////////////////////////////////////////////////////////////////////////////
void test_max_operation(std::string const& code,
    std::string const& expected_str)
{
    HPX_TEST_EQ(compile_and_run(code), compile_and_run(expected_str));
}

void test_2d_keep_dims_true()
{
    blaze::DynamicMatrix<std::int64_t> subject{{13, 42, 33}, {101, 12, 65}};
    phylanx::execution_tree::primitive arg0 =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<std::int64_t>(subject));
    phylanx::execution_tree::primitive arg1 =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ast::nil{});
    phylanx::execution_tree::primitive arg2 =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<std::uint8_t>(true));

    phylanx::execution_tree::primitive max =
        phylanx::execution_tree::primitives::create_amax_operation(
            hpx::find_here(),
            phylanx::execution_tree::primitive_arguments_type{
                std::move(arg0), std::move(arg1), std::move(arg2) });

    hpx::future<phylanx::execution_tree::primitive_argument_type> f =
        max.eval();

    blaze::DynamicMatrix<std::int64_t> expected {{101}};

    HPX_TEST_EQ(phylanx::ir::node_data<std::int64_t>(std::move(expected)),
        phylanx::execution_tree::extract_integer_value(f.get()));
}
///////////////////////////////////////////////////////////////////////////////
int main(int argc, char* argv[])
{
    test_max_operation("amax(42.)", "42.");
    test_max_operation("amax(42., nil, true)", "42.");
    test_max_operation("amax([13., 42., 33.])", "42.");
    test_max_operation("amax([13., 42., 33.], -1)", "42.");
    test_max_operation("amax([13., 42., 33.],  0, true)", "[42.]");
    test_max_operation("amax([[13., 42., 33.],[101, 12, 65]])", "101.");
    test_max_operation("amax([[13., 42., 33.],[101, 12, 65]], 0, false, 100.)",
        "[101., 100., 100.]");
    test_max_operation(
        "amax([[13., 42., 33.],[101, 12, 65]],  0)", "[101. ,42., 65.]");
    test_max_operation(
        "amax([[13., 42., 33.],[101, 12, 65]], -2)", "[101. ,42., 65.]");
    test_max_operation(
        "amax([[13., 42., 33.],[101, 12, 65]],  1)", "[42., 101.]");
    test_max_operation(
        "amax([[13., 42., 33.],[101, 12, 65]], -1)", "[42., 101.]");
    test_max_operation("amax([[13., 42., 33.],[101, 12, 65]], -1, false, 45.)",
        "[45., 101.]");
    test_max_operation("amax([[13., 42., 33.],[101, 12, 65]],  list(-1, 0))",
                       "101.");
    test_max_operation(
        "amax([[13., 42., 33.],[101, 12, 65]],  list(-1, 0), false, 45.)",
        "101.");
    test_max_operation("amax([[13., 42., 33.],[101, 12, 65]],  0, true)",
        "[[101. ,42., 65.]]");
    test_max_operation("amax([[13., 42., 33.],[101, 12, 65]],  0, true, 55.)",
                       "[[101. ,55., 65.]]");
    test_max_operation("amax([[13., 42., 33.],[101, 12, 65]],  1, true)",
        "[[42.], [101.]]");
    test_max_operation(
        "amax([[13., 42., 33.],[101., 12., 65.]],  list(0), true)",
        "[[101. ,42., 65.]]");
    test_max_operation(
        "amax([[13., 42., 33.],[101., 12., 65.]],  list(-1, 0), true)",
        "[[101.]]");
    test_max_operation("amax([[13., 42., 33.],[101., 12., 65.]],  "
                       "list(-1, 0), true, 110.)",
        "[[110.]]");
    test_2d_keep_dims_true();

    test_max_operation("amax([[[13., 42., 33.],[101., 12., 65.]]])", "101.");
    test_max_operation("amax([[[13., 42., 33.],[101., 12., 65.]]], 0)",
        "[[13., 42., 33.], [101., 12., 65.]]");
    test_max_operation("amax([[[13., 42., 33.],[101., 12., 65.]]], 1)",
        "[[101., 42., 65.]]");
    test_max_operation("amax([[[13., 42., 33.],[101., 12., 65.]]], 2)",
        "[[42., 101.]]");
    test_max_operation("amax([[[13., 42., 33.],[101., 12., 65.]]], -1)",
        "[[42., 101.]]");
    test_max_operation(
        "amax([[[13., 42., 33.],[101., 12., 65.]]], list(0, -1))",
        "[42., 101.]");
    test_max_operation("amax([[[13., 42., 33.],[101., 12., 65.]]], "
                       "list(0, -1), false, 45.)",
        "[45., 101.]");
    test_max_operation(
        "amax([[[13., 42., 33.],[101., 12., 65.]]], list(0, -1, 1))",
        "101.");
    test_max_operation("amax([[[13., 42., 33.],[101., 12., 65.]]], 0, true)",
        "[[[13., 42., 33.], [101., 12., 65.]]]");
    test_max_operation("amax([[[13., 42., 33.],[101., 12., 65.]]], 1, true)",
        "[[[101., 42., 65.]]]");
    test_max_operation("amax([[[13., 42., 33.],[101., 12., 65.]]], -1, true)",
        "[[[42.], [101.]]]");
    test_max_operation(
        "amax([[[13., 42., 33.],[101., 12., 65.]]], list(1, -1), true)",
        "[[[101.]]]");
    test_max_operation(
        "amax([[[13., 42., 33.],[101., 12., 65.]]], list(0, -1), true)",
        "[[[42.], [101.]]]");
    test_max_operation("amax([[[13., 42., 33.],[101., 12., 65.]]], "
                       "list(0, -1), true, 45.)",
        "[[[45.], [101.]]]");
    test_max_operation(
        "amax([[[13., 42., 33.],[101., 12., 65.]]], list(0, 1), true)",
        "[[[101., 42., 65.]]]");
    test_max_operation("amax([[[13., 42., 33.],[101., 12., 65.]]], "
                       "list(0, 1), true, 55.)",
        "[[[101., 55., 65.]]]");
    test_max_operation(
        "amax([[[13., 42., 33.],[101., 12., 65.]]], list(0, -1, 1), true)",
        "[[[101.]]]");

    return hpx::util::report_errors();
}
