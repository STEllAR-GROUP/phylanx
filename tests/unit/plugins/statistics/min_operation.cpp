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
void test_min_operation(std::string const& code,
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

    phylanx::execution_tree::primitive min =
        phylanx::execution_tree::primitives::create_amin_operation(
            hpx::find_here(),
            phylanx::execution_tree::primitive_arguments_type{
                std::move(arg0), std::move(arg1), std::move(arg2) });

    hpx::future<phylanx::execution_tree::primitive_argument_type> f =
        min.eval();

    blaze::DynamicMatrix<std::int64_t> expected{{12}};

    HPX_TEST_EQ(phylanx::ir::node_data<std::int64_t>(std::move(expected)),
        phylanx::execution_tree::extract_integer_value(f.get()));
}
///////////////////////////////////////////////////////////////////////////////
int main(int argc, char* argv[])
{
    test_min_operation("amin(42.)", "42.");
    test_min_operation("amin(42., nil, true)", "42.");
    test_min_operation("amin([13., 42., 33.])", "13.");
    test_min_operation("amin([13., 42., 33.], -1)", "13.");
    test_min_operation("amin([13., 42., 33.],  0, true)", "[13.]");
    test_min_operation("amin([[13., 42., 33.],[101., 12., 65.]])", "12.");
    test_min_operation(
        "amin([[13., 42., 33.],[101., 12., 65.]],  0)", "[13. ,12., 33.]");
    test_min_operation(
        "amin([[13., 42., 33.],[101., 12., 65.]], -2)", "[13. ,12., 33.]");
    test_min_operation(
        "amin([[13., 42., 33.],[101., 12., 65.]],  1)", "[13. ,12.]");
    test_min_operation(
        "amin([[13., 42., 33.],[101., 12., 65.]], -1)", "[13. ,12.]");
    test_min_operation("amin([[13., 42., 33.],[101., 12., 65.]],  0, true)",
        "[[13. ,12., 33.]]");
    test_min_operation("amin([[13., 42., 33.],[101., 12., 65.]],  1, true)",
        "[[13.], [12.]]");
    test_min_operation(
        "amin([[13., 42., 33.],[101., 12., 65.]],  1, true, 12.)",
        "[[12.], [12.]]");
    test_2d_keep_dims_true();
    test_min_operation(
        "amin([[13., 42., 33.],[101., 12., 65.]],  list(0), true)",
        "[[13. ,12., 33.]]");
    test_min_operation(
        "amin([[13., 42., 33.],[101., 12., 65.]],  list(-1, 0), true)",
        "[[12.]]");
    test_min_operation("amin([[13., 42., 33.],[101., 12., 65.]],  "
                       "list(-1, 0), true, 10.)",
        "[[10.]]");

    test_min_operation("amin([[[13., 42., 33.],[101., 12., 65.]]])", "12.");
    test_min_operation("amin([[[13., 42., 33.],[101., 12., 65.]]], 0)",
        "[[13., 42., 33.], [101., 12., 65.]]");
    test_min_operation("amin([[[13., 42., 33.],[101., 12., 65.]]], 1)",
        "[[13., 12., 33.]]");
    test_min_operation("amin([[[13., 42., 33.],[101., 12., 65.]]], -2)",
        "[[13., 12., 33.]]");
    test_min_operation("amin([[[13., 42., 33.],[101., 12., 65.]]], 2)",
        "[[13., 12.]]");
    test_min_operation(
        "amin([[[13., 42., 33.],[101., 12., 65.]]], 2, false, 12.)",
        "[[12., 12.]]");
    test_min_operation("amin([[[13., 42., 33.],[101., 12., 65.]]], -1)",
        "[[13., 12.]]");
    test_min_operation(
        "amin([[[13., 42., 33.],[101., 12., 65.]]], list(0, -1))",
        "[13., 12.]");
    test_min_operation(
        "amin([[[13., 42., 33.],[101., 12., 65.]]], list(0, -1, 1))",
        "12.");
    test_min_operation("amin([[[13., 42., 33.],[101., 12., 65.]]], "
                       "list(0, -1, 1), true, 10.)",
        "[[[10.]]]");
    test_min_operation("amin([[[13., 42., 33.],[101., 12., 65.]]], 0, true)",
        "[[[13., 42., 33.], [101., 12., 65.]]]");
    test_min_operation("amin([[[13., 42., 33.],[101., 12., 65.]]], 1, true)",
        "[[[13., 12., 33.]]]");
    test_min_operation("amin([[[13., 42., 33.],[101., 12., 65.]]], -1, true)",
        "[[[13.], [12.]]]");
    test_min_operation(
        "amin([[[13., 42., 33.],[101., 12., 65.]]], list(0, -1), true)",
        "[[[13.], [12.]]]");
    test_min_operation(
        "amin([[[13., 42., 33.],[101., 12., 65.]]], list(0, 1), true)",
        "[[[13., 12., 33.]]]");
    test_min_operation(
        "amin([[[13., 42., 33.],[101., 12., 65.]]], list(1, -1), true)",
        "[[[12.]]]");
    test_min_operation(
        "amin([[[13., 42., 33.],[101., 12., 65.]]], list(0, -1, 1), true)",
        "[[[12.]]]");

    return hpx::util::report_errors();
}
