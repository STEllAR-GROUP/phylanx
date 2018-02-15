//   Copyright (c) 2017-2018 Hartmut Kaiser
//   Copyright (c) 2018 Shahrzad Shirzad
//
//   Distributed under the Boost Software License, Version 1.0. (See accompanying
//   file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/phylanx.hpp>

#include <hpx/hpx_main.hpp>
#include <hpx/include/lcos.hpp>
#include <hpx/util/lightweight_test.hpp>

#include <utility>
#include <vector>

void test_unary_not_operation_0d_false()
{
    phylanx::execution_tree::primitive val =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<bool>{false});

    phylanx::execution_tree::primitive unary_not =
        phylanx::execution_tree::primitives::create_unary_not_operation(
            hpx::find_here(),
            std::vector<phylanx::execution_tree::primitive_argument_type>{
                std::move(val)});

    hpx::future<phylanx::execution_tree::primitive_argument_type> f =
        unary_not.eval();

    HPX_TEST_EQ(phylanx::ir::node_data<bool>(true),
        phylanx::execution_tree::extract_boolean_data(f.get()));
}

void test_unary_not_operation_0d_true()
{
    phylanx::execution_tree::primitive val =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<bool>{true});

    phylanx::execution_tree::primitive unary_not =
        phylanx::execution_tree::primitives::create_unary_not_operation(
            hpx::find_here(),
            std::vector<phylanx::execution_tree::primitive_argument_type>{
                std::move(val)});

    hpx::future<phylanx::execution_tree::primitive_argument_type> f =
        unary_not.eval();

    HPX_TEST_EQ(phylanx::ir::node_data<bool>(false),
        phylanx::execution_tree::extract_boolean_data(f.get()));
}

void test_unary_not_operation_0d_false_lit()
{
    phylanx::execution_tree::primitive unary_not =
        phylanx::execution_tree::primitives::create_unary_not_operation(
            hpx::find_here(),
            std::vector<phylanx::execution_tree::primitive_argument_type>{
                phylanx::ir::node_data<bool>{false}});

    hpx::future<phylanx::execution_tree::primitive_argument_type> f =
        unary_not.eval();

    HPX_TEST_EQ(phylanx::ir::node_data<bool>(true),
        phylanx::execution_tree::extract_boolean_data(f.get()));
}

void test_unary_not_operation_0d_true_lit()
{
    phylanx::execution_tree::primitive unary_not =
        phylanx::execution_tree::primitives::create_unary_not_operation(
            hpx::find_here(),
            std::vector<phylanx::execution_tree::primitive_argument_type>{
                phylanx::ir::node_data<bool>{true}});

    hpx::future<phylanx::execution_tree::primitive_argument_type> f =
        unary_not.eval();

    HPX_TEST_EQ(phylanx::ir::node_data<bool>(false),
        phylanx::execution_tree::extract_boolean_data(f.get()));
}

void test_unary_not_operation_1d()
{
    blaze::Rand<blaze::DynamicVector<int>> gen{};
    blaze::DynamicVector<bool> v = gen.generate(1007UL, 0, 1);

    phylanx::execution_tree::primitive val =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<bool>(v));

    phylanx::execution_tree::primitive unary_not =
        phylanx::execution_tree::primitives::create_unary_not_operation(
            hpx::find_here(),
            std::vector<phylanx::execution_tree::primitive_argument_type>{
                std::move(val)});

    hpx::future<phylanx::execution_tree::primitive_argument_type> f =
        unary_not.eval();

    blaze::DynamicVector<bool> expected =
        blaze::map(v, [](bool x) { return (x == false); });

    HPX_TEST_EQ(phylanx::ir::node_data<bool>(std::move(expected)),
        phylanx::execution_tree::extract_boolean_data(f.get()));
}

void test_unary_not_operation_1d_lit()
{
    blaze::Rand<blaze::DynamicVector<int>> gen{};
    blaze::DynamicVector<bool> v = gen.generate(1007UL, 0, 1);

    phylanx::ir::node_data<bool> val(v);

    phylanx::execution_tree::primitive unary_not =
        phylanx::execution_tree::primitives::create_unary_not_operation(
            hpx::find_here(),
            std::vector<phylanx::execution_tree::primitive_argument_type>{
                std::move(val)});

    hpx::future<phylanx::execution_tree::primitive_argument_type> f =
        unary_not.eval();

    blaze::DynamicVector<bool> expected =
        blaze::map(v, [](bool x) { return (x == false); });

    HPX_TEST_EQ(phylanx::ir::node_data<bool>(std::move(expected)),
        phylanx::execution_tree::extract_boolean_data(f.get()));
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
            std::vector<phylanx::execution_tree::primitive_argument_type>{
                std::move(val)});

    hpx::future<phylanx::execution_tree::primitive_argument_type> f =
        unary_not.eval();

    blaze::DynamicVector<bool> expected =
        blaze::map(v, [](double x) { return (x == false); });

    HPX_TEST_EQ(phylanx::ir::node_data<bool>(std::move(expected)),
        phylanx::execution_tree::extract_boolean_data(f.get()));
}

void test_unary_not_operation_1d_double_lit()
{
    blaze::Rand<blaze::DynamicVector<int>> gen{};
    blaze::DynamicVector<double> v = gen.generate(1007UL, 0, 1);

    phylanx::ir::node_data<double> val(v);

    phylanx::execution_tree::primitive unary_not =
        phylanx::execution_tree::primitives::create_unary_not_operation(
            hpx::find_here(),
            std::vector<phylanx::execution_tree::primitive_argument_type>{
                std::move(val)});

    hpx::future<phylanx::execution_tree::primitive_argument_type> f =
        unary_not.eval();

    blaze::DynamicVector<bool> expected =
        blaze::map(v, [](double x) { return (x == false); });

    HPX_TEST_EQ(phylanx::ir::node_data<bool>(std::move(expected)),
        phylanx::execution_tree::extract_boolean_data(f.get()));
}

void test_unary_not_operation_2d()
{
    blaze::Rand<blaze::DynamicMatrix<double>> gen{};
    blaze::DynamicMatrix<bool> m = gen.generate(42UL, 42UL);

    phylanx::execution_tree::primitive val =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<bool>(m));

    phylanx::execution_tree::primitive unary_not =
        phylanx::execution_tree::primitives::create_unary_not_operation(
            hpx::find_here(),
            std::vector<phylanx::execution_tree::primitive_argument_type>{
                std::move(val)});

    hpx::future<phylanx::execution_tree::primitive_argument_type> f =
        unary_not.eval();

    blaze::DynamicMatrix<bool> expected =
        blaze::map(m, [](bool x) { return (x == false); });

    HPX_TEST_EQ(phylanx::ir::node_data<bool>(std::move(expected)),
        phylanx::execution_tree::extract_boolean_data(f.get()));
}

void test_unary_not_operation_2d_lit()
{
    blaze::Rand<blaze::DynamicMatrix<double>> gen{};
    blaze::DynamicMatrix<bool> m = gen.generate(42UL, 42UL);

    phylanx::ir::node_data<bool> val(m);

    phylanx::execution_tree::primitive unary_not =
        phylanx::execution_tree::primitives::create_unary_not_operation(
            hpx::find_here(),
            std::vector<phylanx::execution_tree::primitive_argument_type>{
                std::move(val)});

    hpx::future<phylanx::execution_tree::primitive_argument_type> f =
        unary_not.eval();

    blaze::DynamicMatrix<bool> expected =
        blaze::map(m, [](bool x) { return (x == false); });

    HPX_TEST_EQ(phylanx::ir::node_data<bool>(std::move(expected)),
        phylanx::execution_tree::extract_boolean_data(f.get()));
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
            std::vector<phylanx::execution_tree::primitive_argument_type>{
                std::move(val)});

    hpx::future<phylanx::execution_tree::primitive_argument_type> f =
        unary_not.eval();

    blaze::DynamicMatrix<bool> expected =
        blaze::map(m, [](double x) { return (x == false); });

    HPX_TEST_EQ(phylanx::ir::node_data<bool>(std::move(expected)),
        phylanx::execution_tree::extract_boolean_data(f.get()));
}

void test_unary_not_operation_2d_double_lit()
{
    blaze::Rand<blaze::DynamicMatrix<double>> gen{};
    blaze::DynamicMatrix<double> m = gen.generate(42UL, 42UL);

    phylanx::ir::node_data<double> val(m);

    phylanx::execution_tree::primitive unary_not =
        phylanx::execution_tree::primitives::create_unary_not_operation(
            hpx::find_here(),
            std::vector<phylanx::execution_tree::primitive_argument_type>{
                std::move(val)});

    hpx::future<phylanx::execution_tree::primitive_argument_type> f =
        unary_not.eval();

    blaze::DynamicMatrix<bool> expected =
        blaze::map(m, [](double x) { return (x == false); });

    HPX_TEST_EQ(phylanx::ir::node_data<bool>(std::move(expected)),
        phylanx::execution_tree::extract_boolean_data(f.get()));
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

    return hpx::util::report_errors();
}
