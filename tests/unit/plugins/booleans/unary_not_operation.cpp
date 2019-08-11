//   Copyright (c) 2017-2019 Hartmut Kaiser
//   Copyright (c) 2018 Shahrzad Shirzad
//
//   Distributed under the Boost Software License, Version 1.0. (See accompanying
//   file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/phylanx.hpp>

#include <hpx/hpx_main.hpp>
#include <hpx/include/lcos.hpp>
#include <hpx/testing.hpp>

#include <cstdint>
#include <string>
#include <utility>
#include <vector>

void test_unary_not_operation_0d_false()
{
    phylanx::execution_tree::primitive val =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<std::uint8_t>{false});

    phylanx::execution_tree::primitive unary_not =
        phylanx::execution_tree::primitives::create_unary_not_operation(
            hpx::find_here(),
            phylanx::execution_tree::primitive_arguments_type{
                std::move(val)});

    hpx::future<phylanx::execution_tree::primitive_argument_type> f =
        unary_not.eval();

    HPX_TEST_EQ(phylanx::ir::node_data<std::uint8_t>(true),
        phylanx::execution_tree::extract_boolean_value_strict(f.get()));
}

void test_unary_not_operation_0d_true()
{
    phylanx::execution_tree::primitive val =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<std::uint8_t>{true});

    phylanx::execution_tree::primitive unary_not =
        phylanx::execution_tree::primitives::create_unary_not_operation(
            hpx::find_here(),
            phylanx::execution_tree::primitive_arguments_type{
                std::move(val)});

    hpx::future<phylanx::execution_tree::primitive_argument_type> f =
        unary_not.eval();

    HPX_TEST_EQ(phylanx::ir::node_data<std::uint8_t>(false),
        phylanx::execution_tree::extract_boolean_value_strict(f.get()));
}

void test_unary_not_operation_0d_false_lit()
{
    phylanx::execution_tree::primitive unary_not =
        phylanx::execution_tree::primitives::create_unary_not_operation(
            hpx::find_here(),
            phylanx::execution_tree::primitive_arguments_type{
                phylanx::ir::node_data<std::uint8_t>{false}});

    hpx::future<phylanx::execution_tree::primitive_argument_type> f =
        unary_not.eval();

    HPX_TEST_EQ(phylanx::ir::node_data<std::uint8_t>(true),
        phylanx::execution_tree::extract_boolean_value_strict(f.get()));
}

void test_unary_not_operation_0d_true_lit()
{
    phylanx::execution_tree::primitive unary_not =
        phylanx::execution_tree::primitives::create_unary_not_operation(
            hpx::find_here(),
            phylanx::execution_tree::primitive_arguments_type{
                phylanx::ir::node_data<std::uint8_t>{true}});

    hpx::future<phylanx::execution_tree::primitive_argument_type> f =
        unary_not.eval();

    HPX_TEST_EQ(phylanx::ir::node_data<std::uint8_t>(false),
        phylanx::execution_tree::extract_boolean_value_strict(f.get()));
}

void test_unary_not_operation_1d()
{
    blaze::Rand<blaze::DynamicVector<int>> gen{};
    blaze::DynamicVector<std::uint8_t> v = gen.generate(1007UL, 0, 1);

    phylanx::execution_tree::primitive val =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<std::uint8_t>(v));

    phylanx::execution_tree::primitive unary_not =
        phylanx::execution_tree::primitives::create_unary_not_operation(
            hpx::find_here(),
            phylanx::execution_tree::primitive_arguments_type{
                std::move(val)});

    hpx::future<phylanx::execution_tree::primitive_argument_type> f =
        unary_not.eval();

    blaze::DynamicVector<std::uint8_t> expected =
        blaze::map(v, [](bool x) { return (x == false); });

    HPX_TEST_EQ(phylanx::ir::node_data<std::uint8_t>(std::move(expected)),
        phylanx::execution_tree::extract_boolean_value_strict(f.get()));
}

void test_unary_not_operation_1d_lit()
{
    blaze::Rand<blaze::DynamicVector<int>> gen{};
    blaze::DynamicVector<std::uint8_t> v = gen.generate(1007UL, 0, 1);

    phylanx::ir::node_data<std::uint8_t> val(v);

    phylanx::execution_tree::primitive unary_not =
        phylanx::execution_tree::primitives::create_unary_not_operation(
            hpx::find_here(),
            phylanx::execution_tree::primitive_arguments_type{
                std::move(val)});

    hpx::future<phylanx::execution_tree::primitive_argument_type> f =
        unary_not.eval();

    blaze::DynamicVector<std::uint8_t> expected =
        blaze::map(v, [](bool x) { return (x == false); });

    HPX_TEST_EQ(phylanx::ir::node_data<std::uint8_t>(std::move(expected)),
        phylanx::execution_tree::extract_boolean_value_strict(f.get()));
}

void test_unary_not_operation_1d_double()
{
    blaze::Rand<blaze::DynamicVector<int>> gen{};
    blaze::DynamicVector<double> v = gen.generate(1007UL, 0, 1);

    phylanx::execution_tree::primitive val =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(v));

    phylanx::execution_tree::primitive unary_not =
        phylanx::execution_tree::primitives::create_unary_not_operation(
            hpx::find_here(),
            phylanx::execution_tree::primitive_arguments_type{
                std::move(val)});

    hpx::future<phylanx::execution_tree::primitive_argument_type> f =
        unary_not.eval();

    blaze::DynamicVector<std::uint8_t> expected =
        blaze::map(v, [](double x) { return (x == 0.0); });

    HPX_TEST_EQ(phylanx::ir::node_data<std::uint8_t>(std::move(expected)),
        phylanx::execution_tree::extract_boolean_value_strict(f.get()));
}

void test_unary_not_operation_1d_double_lit()
{
    blaze::Rand<blaze::DynamicVector<int>> gen{};
    blaze::DynamicVector<double> v = gen.generate(1007UL, 0, 1);

    phylanx::ir::node_data<double> val(v);

    phylanx::execution_tree::primitive unary_not =
        phylanx::execution_tree::primitives::create_unary_not_operation(
            hpx::find_here(),
            phylanx::execution_tree::primitive_arguments_type{
                std::move(val)});

    hpx::future<phylanx::execution_tree::primitive_argument_type> f =
        unary_not.eval();

    blaze::DynamicVector<std::uint8_t> expected =
        blaze::map(v, [](double x) { return (x == 0.0); });

    HPX_TEST_EQ(phylanx::ir::node_data<std::uint8_t>(std::move(expected)),
        phylanx::execution_tree::extract_boolean_value_strict(f.get()));
}

void test_unary_not_operation_2d()
{
    blaze::Rand<blaze::DynamicMatrix<double>> gen{};
    blaze::DynamicMatrix<std::uint8_t> m = gen.generate(42UL, 42UL);

    phylanx::execution_tree::primitive val =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<std::uint8_t>(m));

    phylanx::execution_tree::primitive unary_not =
        phylanx::execution_tree::primitives::create_unary_not_operation(
            hpx::find_here(),
            phylanx::execution_tree::primitive_arguments_type{
                std::move(val)});

    hpx::future<phylanx::execution_tree::primitive_argument_type> f =
        unary_not.eval();

    blaze::DynamicMatrix<std::uint8_t> expected =
        blaze::map(m, [](bool x) { return (x == false); });

    HPX_TEST_EQ(phylanx::ir::node_data<std::uint8_t>(std::move(expected)),
        phylanx::execution_tree::extract_boolean_value_strict(f.get()));
}

void test_unary_not_operation_2d_lit()
{
    blaze::Rand<blaze::DynamicMatrix<double>> gen{};
    blaze::DynamicMatrix<std::uint8_t> m = gen.generate(42UL, 42UL);

    phylanx::ir::node_data<std::uint8_t> val(m);

    phylanx::execution_tree::primitive unary_not =
        phylanx::execution_tree::primitives::create_unary_not_operation(
            hpx::find_here(),
            phylanx::execution_tree::primitive_arguments_type{
                std::move(val)});

    hpx::future<phylanx::execution_tree::primitive_argument_type> f =
        unary_not.eval();

    blaze::DynamicMatrix<std::uint8_t> expected =
        blaze::map(m, [](bool x) { return (x == false); });

    HPX_TEST_EQ(phylanx::ir::node_data<std::uint8_t>(std::move(expected)),
        phylanx::execution_tree::extract_boolean_value_strict(f.get()));
}

void test_unary_not_operation_2d_double()
{
    blaze::Rand<blaze::DynamicMatrix<double>> gen{};
    blaze::DynamicMatrix<double> m = gen.generate(42UL, 42UL);

    phylanx::execution_tree::primitive val =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(m));

    phylanx::execution_tree::primitive unary_not =
        phylanx::execution_tree::primitives::create_unary_not_operation(
            hpx::find_here(),
            phylanx::execution_tree::primitive_arguments_type{
                std::move(val)});

    hpx::future<phylanx::execution_tree::primitive_argument_type> f =
        unary_not.eval();

    blaze::DynamicMatrix<std::uint8_t> expected =
        blaze::map(m, [](double x) { return (x == 0.0); });

    HPX_TEST_EQ(phylanx::ir::node_data<std::uint8_t>(std::move(expected)),
        phylanx::execution_tree::extract_boolean_value_strict(f.get()));
}

void test_unary_not_operation_2d_double_lit()
{
    blaze::Rand<blaze::DynamicMatrix<double>> gen{};
    blaze::DynamicMatrix<double> m = gen.generate(42UL, 42UL);

    phylanx::ir::node_data<double> val(m);

    phylanx::execution_tree::primitive unary_not =
        phylanx::execution_tree::primitives::create_unary_not_operation(
            hpx::find_here(),
            phylanx::execution_tree::primitive_arguments_type{
                std::move(val)});

    hpx::future<phylanx::execution_tree::primitive_argument_type> f =
        unary_not.eval();

    blaze::DynamicMatrix<std::uint8_t> expected =
        blaze::map(m, [](double x) { return (x == 0.0); });

    HPX_TEST_EQ(phylanx::ir::node_data<std::uint8_t>(std::move(expected)),
        phylanx::execution_tree::extract_boolean_value_strict(f.get()));
}

///////////////////////////////////////////////////////////////////////////////
phylanx::execution_tree::primitive_argument_type compile_and_run(
    std::string const& codestr)
{
    phylanx::execution_tree::compiler::function_list snippets;
    phylanx::execution_tree::compiler::environment env =
        phylanx::execution_tree::compiler::default_environment();

    auto const& code = phylanx::execution_tree::compile(codestr, snippets, env);
    return code.run();
}

void test_unary_not_operation(std::string const& code,
    std::string const& expected_str)
{
    HPX_TEST_EQ(compile_and_run(code), compile_and_run(expected_str));
}

void test_unary_not_operation_3d()
{
    test_unary_not_operation(
        R"(!astype([[[1, 1], [1, 0]], [[1, 1], [1, 1]]], "bool"))",
        R"( astype([[[0, 0], [0, 1]], [[0, 0], [0, 0]]], "bool"))");
    test_unary_not_operation(
        R"(!!astype([[[1, 1], [1, 0]], [[1, 1], [1, 1]]], "bool"))",
        R"(  astype([[[1, 1], [1, 0]], [[1, 1], [1, 1]]], "bool"))");
}

int main(int argc, char* argv[])
{
    test_unary_not_operation_0d_false();
    test_unary_not_operation_0d_false_lit();

    test_unary_not_operation_0d_true();
    test_unary_not_operation_0d_true_lit();

    test_unary_not_operation_1d();
    test_unary_not_operation_1d_lit();
    test_unary_not_operation_1d_double();
    test_unary_not_operation_1d_double_lit();

    test_unary_not_operation_2d();
    test_unary_not_operation_2d_lit();
    test_unary_not_operation_2d_double();
    test_unary_not_operation_2d_double_lit();

    test_unary_not_operation_3d();

    return hpx::util::report_errors();
}
