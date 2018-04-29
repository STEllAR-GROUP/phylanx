//   Copyright (c) 2017-2018 Hartmut Kaiser
//   Copyright (c) 2018 Shahrzad Shirzad
//
//   Distributed under the Boost Software License, Version 1.0. (See accompanying
//   file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/phylanx.hpp>

#include <hpx/hpx_main.hpp>
#include <hpx/include/lcos.hpp>
#include <hpx/util/lightweight_test.hpp>

#include <cstdint>
#include <iostream>
#include <utility>
#include <vector>

void test_or_operation_0d_false()
{
    phylanx::execution_tree::primitive lhs =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<std::uint8_t>(false));

    phylanx::execution_tree::primitive rhs =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<std::uint8_t>(false));

    phylanx::execution_tree::primitive or_operation;
    or_operation = phylanx::execution_tree::primitives::create_or_operation(
        hpx::find_here(),
        std::vector<phylanx::execution_tree::primitive_argument_type>{
            std::move(lhs), std::move(rhs)});

    phylanx::execution_tree::primitive_argument_type f =
        or_operation.eval().get();

    HPX_TEST_EQ(phylanx::ir::node_data<std::uint8_t>(false),
        phylanx::execution_tree::extract_boolean_data(f));
}

void test_or_operation_0d_double_false()
{
    phylanx::execution_tree::primitive lhs =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(0.0));

    phylanx::execution_tree::primitive rhs =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(0.0));

    phylanx::execution_tree::primitive or_operation;
    or_operation = phylanx::execution_tree::primitives::create_or_operation(
        hpx::find_here(),
        std::vector<phylanx::execution_tree::primitive_argument_type>{
            std::move(lhs), std::move(rhs)});

    phylanx::execution_tree::primitive_argument_type f =
        or_operation.eval().get();

    HPX_TEST_EQ(phylanx::ir::node_data<std::uint8_t>(false),
        phylanx::execution_tree::extract_boolean_data(f));
}

void test_or_operation_0d_true()
{
    phylanx::execution_tree::primitive lhs =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<std::uint8_t>(true));

    phylanx::execution_tree::primitive rhs =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<std::uint8_t>(false));

    phylanx::execution_tree::primitive or_operation;
    or_operation = phylanx::execution_tree::primitives::create_or_operation(
        hpx::find_here(),
        std::vector<phylanx::execution_tree::primitive_argument_type>{
            std::move(lhs), std::move(rhs)});

    phylanx::execution_tree::primitive_argument_type f =
        or_operation.eval().get();

    HPX_TEST_EQ(phylanx::ir::node_data<std::uint8_t>(true),
        phylanx::execution_tree::extract_boolean_data(f));
}

void test_or_operation_0d_double_true()
{
    phylanx::execution_tree::primitive lhs =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(2.0));

    phylanx::execution_tree::primitive rhs =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(0.0));

    phylanx::execution_tree::primitive or_operation;
    or_operation = phylanx::execution_tree::primitives::create_or_operation(
        hpx::find_here(),
        std::vector<phylanx::execution_tree::primitive_argument_type>{
            std::move(lhs), std::move(rhs)});

    phylanx::execution_tree::primitive_argument_type f =
        or_operation.eval().get();

    HPX_TEST_EQ(phylanx::ir::node_data<std::uint8_t>(true),
        phylanx::execution_tree::extract_boolean_data(f));
}

void test_or_operation_0d_lit_false()
{
    phylanx::ir::node_data<std::uint8_t> lhs(false);

    phylanx::execution_tree::primitive rhs =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<std::uint8_t>(false));

    phylanx::execution_tree::primitive or_operation;
    or_operation = phylanx::execution_tree::primitives::create_or_operation(
        hpx::find_here(),
        std::vector<phylanx::execution_tree::primitive_argument_type>{
            std::move(lhs), std::move(rhs)});

    phylanx::execution_tree::primitive_argument_type f =
        or_operation.eval().get();

    HPX_TEST_EQ(phylanx::ir::node_data<std::uint8_t>(false),
        phylanx::execution_tree::extract_boolean_data(f));
}

void test_or_operation_0d_double_lit_false()
{
    phylanx::ir::node_data<double> lhs(0.0);

    phylanx::execution_tree::primitive rhs =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(0.0));

    phylanx::execution_tree::primitive or_operation;
    or_operation = phylanx::execution_tree::primitives::create_or_operation(
        hpx::find_here(),
        std::vector<phylanx::execution_tree::primitive_argument_type>{
            std::move(lhs), std::move(rhs)});

    phylanx::execution_tree::primitive_argument_type f =
        or_operation.eval().get();

    HPX_TEST_EQ(phylanx::ir::node_data<std::uint8_t>(false),
        phylanx::execution_tree::extract_boolean_data(f));
}

void test_or_operation_0d_lit_true()
{
    phylanx::ir::node_data<std::uint8_t> lhs(true);

    phylanx::execution_tree::primitive rhs =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<std::uint8_t>(true));

    phylanx::execution_tree::primitive or_operation;
    or_operation = phylanx::execution_tree::primitives::create_or_operation(
        hpx::find_here(),
        std::vector<phylanx::execution_tree::primitive_argument_type>{
            std::move(lhs), std::move(rhs)});

    phylanx::execution_tree::primitive_argument_type f =
        or_operation.eval().get();

    HPX_TEST_EQ(phylanx::ir::node_data<std::uint8_t>(true),
        phylanx::execution_tree::extract_boolean_data(f));
}

void test_or_operation_0d_double_lit_true()
{
    phylanx::ir::node_data<double> lhs(2.0);

    phylanx::execution_tree::primitive rhs =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(1.0));

    phylanx::execution_tree::primitive or_operation;
    or_operation = phylanx::execution_tree::primitives::create_or_operation(
        hpx::find_here(),
        std::vector<phylanx::execution_tree::primitive_argument_type>{
            std::move(lhs), std::move(rhs)});

    phylanx::execution_tree::primitive_argument_type f =
        or_operation.eval().get();

    HPX_TEST_EQ(phylanx::ir::node_data<std::uint8_t>(true),
        phylanx::execution_tree::extract_boolean_data(f));
}

void test_or_operation_0d1d_false()
{
    blaze::Rand<blaze::DynamicVector<int>> gen{};
    blaze::DynamicVector<std::uint8_t> v = gen.generate(1007UL, 0, 1);

    phylanx::execution_tree::primitive lhs =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<std::uint8_t>(false));

    phylanx::execution_tree::primitive rhs =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<std::uint8_t>(v));

    phylanx::execution_tree::primitive or_operation =
        phylanx::execution_tree::primitives::create_or_operation(
            hpx::find_here(),
            std::vector<phylanx::execution_tree::primitive_argument_type>{
                std::move(lhs), std::move(rhs)});

    phylanx::execution_tree::primitive_argument_type f =
        or_operation.eval().get();

    blaze::DynamicVector<std::uint8_t> expected =
        blaze::map(v, [](bool x) { return (x || false); });

    HPX_TEST_EQ(phylanx::ir::node_data<std::uint8_t>(std::move(expected)),
        phylanx::execution_tree::extract_boolean_data(f));
}

void test_or_operation_0d1d_lit_false()
{
    blaze::Rand<blaze::DynamicVector<int>> gen{};
    blaze::DynamicVector<std::uint8_t> v = gen.generate(1007UL, 0, 1);

    phylanx::ir::node_data<std::uint8_t> lhs(false);

    phylanx::execution_tree::primitive rhs =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<std::uint8_t>(v));

    phylanx::execution_tree::primitive or_operation =
        phylanx::execution_tree::primitives::create_or_operation(
            hpx::find_here(),
            std::vector<phylanx::execution_tree::primitive_argument_type>{
                std::move(lhs), std::move(rhs)});

    phylanx::execution_tree::primitive_argument_type f =
        or_operation.eval().get();

    blaze::DynamicVector<std::uint8_t> expected =
        blaze::map(v, [](bool x) { return (x || false); });

    HPX_TEST_EQ(phylanx::ir::node_data<std::uint8_t>(std::move(expected)),
        phylanx::execution_tree::extract_boolean_data(f));
}

void test_or_operation_0d1d_double_false()
{
    blaze::Rand<blaze::DynamicVector<int>> gen{};
    blaze::DynamicVector<double> v = gen.generate(1007UL, 0, 1);

    phylanx::execution_tree::primitive lhs =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(0.0));

    phylanx::execution_tree::primitive rhs =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(v));

    phylanx::execution_tree::primitive or_operation =
        phylanx::execution_tree::primitives::create_or_operation(
            hpx::find_here(),
            std::vector<phylanx::execution_tree::primitive_argument_type>{
                std::move(lhs), std::move(rhs)});

    phylanx::execution_tree::primitive_argument_type f =
        or_operation.eval().get();

    blaze::DynamicVector<std::uint8_t> expected =
        blaze::map(v, [](double x) { return (x || 0.0); });

    HPX_TEST_EQ(phylanx::ir::node_data<std::uint8_t>(std::move(expected)),
        phylanx::execution_tree::extract_boolean_data(f));
}

void test_or_operation_0d1d_double_lit_false()
{
    blaze::Rand<blaze::DynamicVector<int>> gen{};
    blaze::DynamicVector<double> v = gen.generate(1007UL, 0, 1);

    phylanx::ir::node_data<double> lhs(0.0);

    phylanx::execution_tree::primitive rhs =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(v));

    phylanx::execution_tree::primitive or_operation =
        phylanx::execution_tree::primitives::create_or_operation(
            hpx::find_here(),
            std::vector<phylanx::execution_tree::primitive_argument_type>{
                std::move(lhs), std::move(rhs)});

    phylanx::execution_tree::primitive_argument_type f =
        or_operation.eval().get();

    blaze::DynamicVector<std::uint8_t> expected =
        blaze::map(v, [](double x) { return (x || 0.0); });

    HPX_TEST_EQ(phylanx::ir::node_data<std::uint8_t>(std::move(expected)),
        phylanx::execution_tree::extract_boolean_data(f));
}

void test_or_operation_0d1d_true()
{
    blaze::Rand<blaze::DynamicVector<int>> gen{};
    blaze::DynamicVector<std::uint8_t> v = gen.generate(1007UL, 0, 1);

    phylanx::execution_tree::primitive lhs =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<std::uint8_t>(true));

    phylanx::execution_tree::primitive rhs =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<std::uint8_t>(v));

    phylanx::execution_tree::primitive or_operation =
        phylanx::execution_tree::primitives::create_or_operation(
            hpx::find_here(),
            std::vector<phylanx::execution_tree::primitive_argument_type>{
                std::move(lhs), std::move(rhs)});

    phylanx::execution_tree::primitive_argument_type f =
        or_operation.eval().get();

    blaze::DynamicVector<std::uint8_t> expected =
        blaze::map(v, [](bool x) { return (x || true); });

    HPX_TEST_EQ(phylanx::ir::node_data<std::uint8_t>(std::move(expected)),
        phylanx::execution_tree::extract_boolean_data(f));
}

void test_or_operation_0d1d_lit_true()
{
    blaze::Rand<blaze::DynamicVector<int>> gen{};
    blaze::DynamicVector<std::uint8_t> v = gen.generate(1007UL, 0, 1);

    phylanx::ir::node_data<std::uint8_t> lhs(true);

    phylanx::execution_tree::primitive rhs =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<std::uint8_t>(v));

    phylanx::execution_tree::primitive or_operation =
        phylanx::execution_tree::primitives::create_or_operation(
            hpx::find_here(),
            std::vector<phylanx::execution_tree::primitive_argument_type>{
                std::move(lhs), std::move(rhs)});

    phylanx::execution_tree::primitive_argument_type f =
        or_operation.eval().get();

    blaze::DynamicVector<std::uint8_t> expected =
        blaze::map(v, [](bool x) { return (x || true); });

    HPX_TEST_EQ(phylanx::ir::node_data<std::uint8_t>(std::move(expected)),
        phylanx::execution_tree::extract_boolean_data(f));
}

void test_or_operation_0d1d_double_true()
{
    blaze::Rand<blaze::DynamicVector<int>> gen{};
    blaze::DynamicVector<double> v = gen.generate(1007UL, 0, 1);

    phylanx::execution_tree::primitive lhs =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(1.0));

    phylanx::execution_tree::primitive rhs =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(v));

    phylanx::execution_tree::primitive or_operation =
        phylanx::execution_tree::primitives::create_or_operation(
            hpx::find_here(),
            std::vector<phylanx::execution_tree::primitive_argument_type>{
                std::move(lhs), std::move(rhs)});

    phylanx::execution_tree::primitive_argument_type f =
        or_operation.eval().get();

    blaze::DynamicVector<double> expected(v.size(), 1.0);

    HPX_TEST_EQ(phylanx::ir::node_data<std::uint8_t>(std::move(expected)),
        phylanx::execution_tree::extract_boolean_data(f));
}

void test_or_operation_0d1d_double_lit_true()
{
    blaze::Rand<blaze::DynamicVector<int>> gen{};
    blaze::DynamicVector<double> v = gen.generate(1007UL, 0, 1);

    phylanx::ir::node_data<double> lhs(1.0);

    phylanx::execution_tree::primitive rhs =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(v));

    phylanx::execution_tree::primitive or_operation =
        phylanx::execution_tree::primitives::create_or_operation(
            hpx::find_here(),
            std::vector<phylanx::execution_tree::primitive_argument_type>{
                std::move(lhs), std::move(rhs)});

    phylanx::execution_tree::primitive_argument_type f =
        or_operation.eval().get();

    blaze::DynamicVector<double> expected(v.size(), 1.0);

    HPX_TEST_EQ(phylanx::ir::node_data<std::uint8_t>(std::move(expected)),
        phylanx::execution_tree::extract_boolean_data(f));
}

void test_or_operation_0d2d_false()
{
    blaze::Rand<blaze::DynamicMatrix<int>> gen{};
    blaze::DynamicMatrix<std::uint8_t> m = gen.generate(101UL, 101UL, 0, 1);

    phylanx::execution_tree::primitive lhs =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<std::uint8_t>(false));

    phylanx::execution_tree::primitive rhs =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<std::uint8_t>(m));

    phylanx::execution_tree::primitive or_operation =
        phylanx::execution_tree::primitives::create_or_operation(
            hpx::find_here(),
            std::vector<phylanx::execution_tree::primitive_argument_type>{
                std::move(lhs), std::move(rhs)});

    phylanx::execution_tree::primitive_argument_type f =
        or_operation.eval().get();

    blaze::DynamicMatrix<std::uint8_t> expected =
        blaze::map(m, [](bool x) { return (x || false); });

    HPX_TEST_EQ(phylanx::ir::node_data<std::uint8_t>(std::move(expected)),
        phylanx::execution_tree::extract_boolean_data(f));
}

void test_or_operation_0d2d_lit_false()
{
    blaze::Rand<blaze::DynamicMatrix<int>> gen{};
    blaze::DynamicMatrix<std::uint8_t> m = gen.generate(101UL, 101UL, 0, 1);

    phylanx::ir::node_data<std::uint8_t> lhs(false);

    phylanx::execution_tree::primitive rhs =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<std::uint8_t>(m));

    phylanx::execution_tree::primitive or_operation =
        phylanx::execution_tree::primitives::create_or_operation(
            hpx::find_here(),
            std::vector<phylanx::execution_tree::primitive_argument_type>{
                std::move(lhs), std::move(rhs)});

    phylanx::execution_tree::primitive_argument_type f =
        or_operation.eval().get();

    blaze::DynamicMatrix<std::uint8_t> expected =
        blaze::map(m, [](bool x) { return (x || false); });

    HPX_TEST_EQ(phylanx::ir::node_data<std::uint8_t>(std::move(expected)),
        phylanx::execution_tree::extract_boolean_data(f));
}

void test_or_operation_0d2d_double_false()
{
    blaze::Rand<blaze::DynamicMatrix<int>> gen{};
    blaze::DynamicMatrix<double> m = gen.generate(101UL, 101UL, 0, 1);

    phylanx::execution_tree::primitive lhs =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(0.0));

    phylanx::execution_tree::primitive rhs =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(m));

    phylanx::execution_tree::primitive or_operation =
        phylanx::execution_tree::primitives::create_or_operation(
            hpx::find_here(),
            std::vector<phylanx::execution_tree::primitive_argument_type>{
                std::move(lhs), std::move(rhs)});

    phylanx::execution_tree::primitive_argument_type f =
        or_operation.eval().get();

    blaze::DynamicMatrix<std::uint8_t> expected =
        blaze::map(m, [](double x) { return (x != 0.0); });

    HPX_TEST_EQ(phylanx::ir::node_data<std::uint8_t>(std::move(expected)),
        phylanx::execution_tree::extract_boolean_data(f));
}

void test_or_operation_0d2d_double_lit_false()
{
    blaze::Rand<blaze::DynamicMatrix<int>> gen{};
    blaze::DynamicMatrix<double> m = gen.generate(101UL, 101UL, 0, 1);

    phylanx::ir::node_data<double> lhs(0.0);

    phylanx::execution_tree::primitive rhs =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(m));

    phylanx::execution_tree::primitive or_operation =
        phylanx::execution_tree::primitives::create_or_operation(
            hpx::find_here(),
            std::vector<phylanx::execution_tree::primitive_argument_type>{
                std::move(lhs), std::move(rhs)});

    phylanx::execution_tree::primitive_argument_type f =
        or_operation.eval().get();

    blaze::DynamicMatrix<std::uint8_t> expected =
        blaze::map(m, [](double x) { return (x != 0.0); });

    HPX_TEST_EQ(phylanx::ir::node_data<std::uint8_t>(std::move(expected)),
        phylanx::execution_tree::extract_boolean_data(f));
}

void test_or_operation_0d2d_true()
{
    blaze::Rand<blaze::DynamicMatrix<int>> gen{};
    blaze::DynamicMatrix<std::uint8_t> m = gen.generate(101UL, 101UL, 0, 1);

    phylanx::execution_tree::primitive lhs =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<std::uint8_t>(true));

    phylanx::execution_tree::primitive rhs =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<std::uint8_t>(m));

    phylanx::execution_tree::primitive or_operation =
        phylanx::execution_tree::primitives::create_or_operation(
            hpx::find_here(),
            std::vector<phylanx::execution_tree::primitive_argument_type>{
                std::move(lhs), std::move(rhs)});

    phylanx::execution_tree::primitive_argument_type f =
        or_operation.eval().get();

    blaze::DynamicMatrix<std::uint8_t> expected =
        blaze::map(m, [](bool x) { return (x || true); });

    HPX_TEST_EQ(phylanx::ir::node_data<std::uint8_t>(std::move(expected)),
        phylanx::execution_tree::extract_boolean_data(f));
}

void test_or_operation_0d2d_lit_true()
{
    blaze::Rand<blaze::DynamicMatrix<int>> gen{};
    blaze::DynamicMatrix<std::uint8_t> m = gen.generate(101UL, 101UL, 0, 1);

    phylanx::ir::node_data<std::uint8_t> lhs(true);

    phylanx::execution_tree::primitive rhs =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<std::uint8_t>(m));

    phylanx::execution_tree::primitive or_operation =
        phylanx::execution_tree::primitives::create_or_operation(
            hpx::find_here(),
            std::vector<phylanx::execution_tree::primitive_argument_type>{
                std::move(lhs), std::move(rhs)});

    phylanx::execution_tree::primitive_argument_type f =
        or_operation.eval().get();

    blaze::DynamicMatrix<std::uint8_t> expected =
        blaze::map(m, [](bool x) { return (x || true); });

    HPX_TEST_EQ(phylanx::ir::node_data<std::uint8_t>(std::move(expected)),
        phylanx::execution_tree::extract_boolean_data(f));
}

void test_or_operation_0d2d_double_true()
{
    blaze::Rand<blaze::DynamicMatrix<int>> gen{};
    blaze::DynamicMatrix<double> m = gen.generate(101UL, 101UL, 0, 1);

    phylanx::execution_tree::primitive lhs =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(2.0));

    phylanx::execution_tree::primitive rhs =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(m));

    phylanx::execution_tree::primitive or_operation =
        phylanx::execution_tree::primitives::create_or_operation(
            hpx::find_here(),
            std::vector<phylanx::execution_tree::primitive_argument_type>{
                std::move(lhs), std::move(rhs)});

    phylanx::execution_tree::primitive_argument_type f =
        or_operation.eval().get();

    blaze::DynamicMatrix<double> expected(m.rows(), m.columns(), 1.0);

    HPX_TEST_EQ(phylanx::ir::node_data<std::uint8_t>(std::move(expected)),
        phylanx::execution_tree::extract_boolean_data(f));
}

void test_or_operation_0d2d_double_lit_true()
{
    blaze::Rand<blaze::DynamicMatrix<int>> gen{};
    blaze::DynamicMatrix<double> m = gen.generate(101UL, 101UL, 0, 1);

    phylanx::ir::node_data<double> lhs(2.0);

    phylanx::execution_tree::primitive rhs =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(m));

    phylanx::execution_tree::primitive or_operation =
        phylanx::execution_tree::primitives::create_or_operation(
            hpx::find_here(),
            std::vector<phylanx::execution_tree::primitive_argument_type>{
                std::move(lhs), std::move(rhs)});

    phylanx::execution_tree::primitive_argument_type f =
        or_operation.eval().get();

    blaze::DynamicMatrix<double> expected(m.rows(), m.columns(), 1.0);

    HPX_TEST_EQ(phylanx::ir::node_data<std::uint8_t>(std::move(expected)),
        phylanx::execution_tree::extract_boolean_data(f));
}

void test_or_operation_1d()
{
    blaze::Rand<blaze::DynamicVector<int>> gen{};
    blaze::DynamicVector<std::uint8_t> v1 = gen.generate(100UL, 0, 1);
    blaze::DynamicVector<std::uint8_t> v2 = gen.generate(100UL, 0, 1);

    phylanx::execution_tree::primitive lhs =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<std::uint8_t>(v1));

    phylanx::execution_tree::primitive rhs =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<std::uint8_t>(v2));

    phylanx::execution_tree::primitive or_operation =
        phylanx::execution_tree::primitives::create_or_operation(
            hpx::find_here(),
            std::vector<phylanx::execution_tree::primitive_argument_type>{
                std::move(lhs), std::move(rhs)});

    phylanx::execution_tree::primitive_argument_type f =
        or_operation.eval().get();

    blaze::DynamicVector<std::uint8_t> expected =
        blaze::map(v1, v2, [](bool x, bool y) { return x || y; });

    HPX_TEST_EQ(phylanx::ir::node_data<std::uint8_t>(std::move(expected)),
        phylanx::execution_tree::extract_boolean_data(f));
}

void test_or_operation_1d_lit()
{
    blaze::Rand<blaze::DynamicVector<int>> gen{};
    blaze::DynamicVector<std::uint8_t> v1 = gen.generate(1007UL, 0, 1);
    blaze::DynamicVector<std::uint8_t> v2 = gen.generate(1007UL, 0, 1);

    phylanx::ir::node_data<std::uint8_t> lhs(v1);

    phylanx::execution_tree::primitive rhs =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<std::uint8_t>(v2));

    phylanx::execution_tree::primitive or_operation =
        phylanx::execution_tree::primitives::create_or_operation(
            hpx::find_here(),
            std::vector<phylanx::execution_tree::primitive_argument_type>{
                std::move(lhs), std::move(rhs)});

    phylanx::execution_tree::primitive_argument_type f =
        or_operation.eval().get();

    blaze::DynamicVector<std::uint8_t> expected =
        blaze::map(v1, v2, [](bool x, bool y) { return x || y; });

    HPX_TEST_EQ(phylanx::ir::node_data<std::uint8_t>(std::move(expected)),
        phylanx::execution_tree::extract_boolean_data(f));
}

void test_or_operation_1d_double()
{
    blaze::Rand<blaze::DynamicVector<int>> gen{};
    blaze::DynamicVector<double> v1 = gen.generate(100UL, 0, 1);
    blaze::DynamicVector<double> v2 = gen.generate(100UL, 0, 1);

    phylanx::execution_tree::primitive lhs =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(v1));

    phylanx::execution_tree::primitive rhs =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(v2));

    phylanx::execution_tree::primitive or_operation =
        phylanx::execution_tree::primitives::create_or_operation(
            hpx::find_here(),
            std::vector<phylanx::execution_tree::primitive_argument_type>{
                std::move(lhs), std::move(rhs)});

    phylanx::execution_tree::primitive_argument_type f =
        or_operation.eval().get();

    blaze::DynamicVector<std::uint8_t> expected =
        blaze::map(v1, v2, [](double x, double y) { return x || y; });

    HPX_TEST_EQ(phylanx::ir::node_data<std::uint8_t>(std::move(expected)),
        phylanx::execution_tree::extract_boolean_data(f));
}

void test_or_operation_1d_double_lit()
{
    blaze::Rand<blaze::DynamicVector<int>> gen{};
    blaze::DynamicVector<double> v1 = gen.generate(1007UL, 0, 1);
    blaze::DynamicVector<double> v2 = gen.generate(1007UL, 0, 1);

    phylanx::ir::node_data<double> lhs(v1);

    phylanx::execution_tree::primitive rhs =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(v2));

    phylanx::execution_tree::primitive or_operation =
        phylanx::execution_tree::primitives::create_or_operation(
            hpx::find_here(),
            std::vector<phylanx::execution_tree::primitive_argument_type>{
                std::move(lhs), std::move(rhs)});

    phylanx::execution_tree::primitive_argument_type f =
        or_operation.eval().get();

    blaze::DynamicVector<std::uint8_t> expected =
        blaze::map(v1, v2, [](double x, double y) { return x || y; });

    HPX_TEST_EQ(phylanx::ir::node_data<std::uint8_t>(std::move(expected)),
        phylanx::execution_tree::extract_boolean_data(f));
}

void test_or_operation_1d0d_false()
{
    blaze::Rand<blaze::DynamicVector<int>> gen{};
    blaze::DynamicVector<std::uint8_t> v = gen.generate(1007UL, 0, 1);

    phylanx::execution_tree::primitive lhs =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<std::uint8_t>(v));

    phylanx::execution_tree::primitive rhs =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<std::uint8_t>(false));

    phylanx::execution_tree::primitive or_operation =
        phylanx::execution_tree::primitives::create_or_operation(
            hpx::find_here(),
            std::vector<phylanx::execution_tree::primitive_argument_type>{
                std::move(lhs), std::move(rhs)});

    phylanx::execution_tree::primitive_argument_type f =
        or_operation.eval().get();

    blaze::DynamicVector<std::uint8_t> expected =
        blaze::map(v, [](bool x) { return (x || false); });

    HPX_TEST_EQ(phylanx::ir::node_data<std::uint8_t>(std::move(expected)),
        phylanx::execution_tree::extract_boolean_data(f));
}

void test_or_operation_1d0d_lit_false()
{
    blaze::Rand<blaze::DynamicVector<int>> gen{};
    blaze::DynamicVector<std::uint8_t> v = gen.generate(1007UL, 0, 1);

    phylanx::ir::node_data<std::uint8_t> lhs(v);

    phylanx::execution_tree::primitive rhs =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<std::uint8_t>(false));

    phylanx::execution_tree::primitive or_operation =
        phylanx::execution_tree::primitives::create_or_operation(
            hpx::find_here(),
            std::vector<phylanx::execution_tree::primitive_argument_type>{
                std::move(lhs), std::move(rhs)});

    phylanx::execution_tree::primitive_argument_type f =
        or_operation.eval().get();

    blaze::DynamicVector<std::uint8_t> expected =
        blaze::map(v, [](bool x) { return (x || false); });

    HPX_TEST_EQ(phylanx::ir::node_data<std::uint8_t>(std::move(expected)),
        phylanx::execution_tree::extract_boolean_data(f));
}

void test_or_operation_1d0d_double_false()
{
    blaze::Rand<blaze::DynamicVector<int>> gen{};
    blaze::DynamicVector<double> v = gen.generate(1007UL, 0, 1);

    phylanx::execution_tree::primitive lhs =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(v));

    phylanx::execution_tree::primitive rhs =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(0.0));

    phylanx::execution_tree::primitive or_operation =
        phylanx::execution_tree::primitives::create_or_operation(
            hpx::find_here(),
            std::vector<phylanx::execution_tree::primitive_argument_type>{
                std::move(lhs), std::move(rhs)});

    phylanx::execution_tree::primitive_argument_type f =
        or_operation.eval().get();

    blaze::DynamicVector<std::uint8_t> expected =
        blaze::map(v, [](double x) { return (x != 0.0); });

    HPX_TEST_EQ(phylanx::ir::node_data<std::uint8_t>(std::move(expected)),
        phylanx::execution_tree::extract_boolean_data(f));
}

void test_or_operation_1d0d_double_lit_false()
{
    blaze::Rand<blaze::DynamicVector<int>> gen{};
    blaze::DynamicVector<double> v = gen.generate(1007UL, 0, 1);

    phylanx::ir::node_data<double> lhs(v);

    phylanx::execution_tree::primitive rhs =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(0.0));

    phylanx::execution_tree::primitive or_operation =
        phylanx::execution_tree::primitives::create_or_operation(
            hpx::find_here(),
            std::vector<phylanx::execution_tree::primitive_argument_type>{
                std::move(lhs), std::move(rhs)});

    phylanx::execution_tree::primitive_argument_type f =
        or_operation.eval().get();

    blaze::DynamicVector<std::uint8_t> expected =
        blaze::map(v, [](double x) { return (x != 0.0); });

    HPX_TEST_EQ(phylanx::ir::node_data<std::uint8_t>(std::move(expected)),
        phylanx::execution_tree::extract_boolean_data(f));
}

void test_or_operation_1d0d_true()
{
    blaze::Rand<blaze::DynamicVector<int>> gen{};
    blaze::DynamicVector<std::uint8_t> v = gen.generate(1007UL, 0, 1);

    phylanx::execution_tree::primitive lhs =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<std::uint8_t>(v));

    phylanx::execution_tree::primitive rhs =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<std::uint8_t>(true));

    phylanx::execution_tree::primitive or_operation =
        phylanx::execution_tree::primitives::create_or_operation(
            hpx::find_here(),
            std::vector<phylanx::execution_tree::primitive_argument_type>{
                std::move(lhs), std::move(rhs)});

    phylanx::execution_tree::primitive_argument_type f =
        or_operation.eval().get();

    blaze::DynamicVector<std::uint8_t> expected =
        blaze::map(v, [](bool x) { return (x || true); });

    HPX_TEST_EQ(phylanx::ir::node_data<std::uint8_t>(std::move(expected)),
        phylanx::execution_tree::extract_boolean_data(f));
}

void test_or_operation_1d0d_lit_true()
{
    blaze::Rand<blaze::DynamicVector<int>> gen{};
    blaze::DynamicVector<std::uint8_t> v = gen.generate(1007UL, 0, 1);

    phylanx::ir::node_data<std::uint8_t> lhs(v);

    phylanx::execution_tree::primitive rhs =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<std::uint8_t>(true));

    phylanx::execution_tree::primitive or_operation =
        phylanx::execution_tree::primitives::create_or_operation(
            hpx::find_here(),
            std::vector<phylanx::execution_tree::primitive_argument_type>{
                std::move(lhs), std::move(rhs)});

    phylanx::execution_tree::primitive_argument_type f =
        or_operation.eval().get();

    blaze::DynamicVector<std::uint8_t> expected =
        blaze::map(v, [](bool x) { return (x || true); });

    HPX_TEST_EQ(phylanx::ir::node_data<std::uint8_t>(std::move(expected)),
        phylanx::execution_tree::extract_boolean_data(f));
}

void test_or_operation_1d0d_double_true()
{
    blaze::Rand<blaze::DynamicVector<int>> gen{};
    blaze::DynamicVector<double> v = gen.generate(1007UL, 0, 1);

    phylanx::execution_tree::primitive lhs =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(v));

    phylanx::execution_tree::primitive rhs =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(2.0));

    phylanx::execution_tree::primitive or_operation =
        phylanx::execution_tree::primitives::create_or_operation(
            hpx::find_here(),
            std::vector<phylanx::execution_tree::primitive_argument_type>{
                std::move(lhs), std::move(rhs)});

    phylanx::execution_tree::primitive_argument_type f =
        or_operation.eval().get();

    blaze::DynamicVector<double> expected(v.size(), 1.0);

    HPX_TEST_EQ(phylanx::ir::node_data<std::uint8_t>(std::move(expected)),
        phylanx::execution_tree::extract_boolean_data(f));
}

void test_or_operation_1d0d_double_lit_true()
{
    blaze::Rand<blaze::DynamicVector<int>> gen{};
    blaze::DynamicVector<double> v = gen.generate(1007UL, 0, 1);

    phylanx::ir::node_data<double> lhs(v);

    phylanx::execution_tree::primitive rhs =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(2.0));

    phylanx::execution_tree::primitive or_operation =
        phylanx::execution_tree::primitives::create_or_operation(
            hpx::find_here(),
            std::vector<phylanx::execution_tree::primitive_argument_type>{
                std::move(lhs), std::move(rhs)});

    phylanx::execution_tree::primitive_argument_type f =
        or_operation.eval().get();

    blaze::DynamicVector<double> expected(v.size(), 1.0);

    HPX_TEST_EQ(phylanx::ir::node_data<std::uint8_t>(std::move(expected)),
        phylanx::execution_tree::extract_boolean_data(f));
}

void test_or_operation_1d2d()
{
    blaze::Rand<blaze::DynamicVector<int>> gen{};
    blaze::DynamicVector<std::uint8_t> v = gen.generate(104UL, 0, 1);

    blaze::Rand<blaze::DynamicMatrix<int>> mat_gen{};
    blaze::DynamicMatrix<std::uint8_t> m = mat_gen.generate(101UL, 104UL, 0, 1);

    phylanx::execution_tree::primitive lhs =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<std::uint8_t>(v));

    phylanx::execution_tree::primitive rhs =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<std::uint8_t>(m));

    phylanx::execution_tree::primitive or_operation =
        phylanx::execution_tree::primitives::create_or_operation(
            hpx::find_here(),
            std::vector<phylanx::execution_tree::primitive_argument_type>{
                std::move(lhs), std::move(rhs)});

    phylanx::execution_tree::primitive_argument_type f =
        or_operation.eval().get();

    blaze::DynamicMatrix<std::uint8_t> expected{m.rows(), m.columns()};

    for (size_t i = 0UL; i < m.rows(); i++)
        blaze::row(expected, i) = blaze::map(blaze::row(m, i),
            blaze::trans(v),
            [](bool x, bool y) { return x || y; });

    HPX_TEST_EQ(phylanx::ir::node_data<std::uint8_t>(std::move(expected)),
        phylanx::execution_tree::extract_boolean_data(f));
}

void test_or_operation_1d2d_lit()
{
    blaze::Rand<blaze::DynamicVector<int>> gen{};
    blaze::DynamicVector<std::uint8_t> v = gen.generate(104UL, 0, 1);

    blaze::Rand<blaze::DynamicMatrix<int>> mat_gen{};
    blaze::DynamicMatrix<std::uint8_t> m = mat_gen.generate(101UL, 104UL, 0, 1);

    phylanx::ir::node_data<std::uint8_t> lhs(v);

    phylanx::execution_tree::primitive rhs =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<std::uint8_t>(m));

    phylanx::execution_tree::primitive or_operation =
        phylanx::execution_tree::primitives::create_or_operation(
            hpx::find_here(),
            std::vector<phylanx::execution_tree::primitive_argument_type>{
                std::move(lhs), std::move(rhs)});

    phylanx::execution_tree::primitive_argument_type f =
        or_operation.eval().get();

    blaze::DynamicMatrix<std::uint8_t> expected{m.rows(), m.columns()};

    for (size_t i = 0UL; i < m.rows(); i++)
        blaze::row(expected, i) = blaze::map(blaze::row(m, i),
            blaze::trans(v),
            [](bool x, bool y) { return x || y; });

    HPX_TEST_EQ(phylanx::ir::node_data<std::uint8_t>(std::move(expected)),
        phylanx::execution_tree::extract_boolean_data(f));
}

void test_or_operation_1d2d_double()
{
    blaze::Rand<blaze::DynamicVector<int>> gen{};
    blaze::DynamicVector<double> v = gen.generate(104UL, 0, 1);

    blaze::Rand<blaze::DynamicMatrix<int>> mat_gen{};
    blaze::DynamicMatrix<double> m = mat_gen.generate(101UL, 104UL, 0, 1);

    phylanx::execution_tree::primitive lhs =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(v));

    phylanx::execution_tree::primitive rhs =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(m));

    phylanx::execution_tree::primitive or_operation =
        phylanx::execution_tree::primitives::create_or_operation(
            hpx::find_here(),
            std::vector<phylanx::execution_tree::primitive_argument_type>{
                std::move(lhs), std::move(rhs)});

    phylanx::execution_tree::primitive_argument_type f =
        or_operation.eval().get();

    blaze::DynamicMatrix<std::uint8_t> expected{m.rows(), m.columns()};

    for (size_t i = 0UL; i < m.rows(); i++)
        blaze::row(expected, i) = blaze::map(blaze::row(m, i),
            blaze::trans(v),
            [](double x, double y) { return x || y; });

    HPX_TEST_EQ(phylanx::ir::node_data<std::uint8_t>(std::move(expected)),
        phylanx::execution_tree::extract_boolean_data(f));
}

void test_or_operation_1d2d_double_lit()
{
    blaze::Rand<blaze::DynamicVector<int>> gen{};
    blaze::DynamicVector<double> v = gen.generate(104UL, 0, 1);

    blaze::Rand<blaze::DynamicMatrix<int>> mat_gen{};
    blaze::DynamicMatrix<double> m = mat_gen.generate(101UL, 104UL, 0, 1);

    phylanx::ir::node_data<double> lhs(v);

    phylanx::execution_tree::primitive rhs =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(m));

    phylanx::execution_tree::primitive or_operation =
        phylanx::execution_tree::primitives::create_or_operation(
            hpx::find_here(),
            std::vector<phylanx::execution_tree::primitive_argument_type>{
                std::move(lhs), std::move(rhs)});

    phylanx::execution_tree::primitive_argument_type f =
        or_operation.eval().get();

    blaze::DynamicMatrix<std::uint8_t> expected{m.rows(), m.columns()};

    for (size_t i = 0UL; i < m.rows(); i++)
        blaze::row(expected, i) = blaze::map(blaze::row(m, i),
            blaze::trans(v),
            [](double x, double y) { return x || y; });

    HPX_TEST_EQ(phylanx::ir::node_data<std::uint8_t>(std::move(expected)),
        phylanx::execution_tree::extract_boolean_data(f));
}

void test_or_operation_2d()
{
    blaze::Rand<blaze::DynamicMatrix<int>> gen{};
    blaze::DynamicMatrix<std::uint8_t> m1 = gen.generate(1007UL, 1007UL, 0, 1);
    blaze::DynamicMatrix<std::uint8_t> m2 = gen.generate(1007UL, 1007UL, 0, 1);

    phylanx::execution_tree::primitive lhs =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<std::uint8_t>(m1));

    phylanx::execution_tree::primitive rhs =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<std::uint8_t>(m2));

    phylanx::execution_tree::primitive or_operation =
        phylanx::execution_tree::primitives::create_or_operation(
            hpx::find_here(),
            std::vector<phylanx::execution_tree::primitive_argument_type>{
                std::move(lhs), std::move(rhs)});

    phylanx::execution_tree::primitive_argument_type f =
        or_operation.eval().get();

    blaze::DynamicMatrix<std::uint8_t> expected =
        blaze::map(m1, m2, [](bool x, bool y) { return x || y; });

    HPX_TEST_EQ(phylanx::ir::node_data<std::uint8_t>(std::move(expected)),
        phylanx::execution_tree::extract_boolean_data(f));
}

void test_or_operation_2d_lit()
{
    blaze::Rand<blaze::DynamicMatrix<int>> gen{};
    blaze::DynamicMatrix<std::uint8_t> m1 = gen.generate(1007UL, 1007UL, 0, 1);
    blaze::DynamicMatrix<std::uint8_t> m2 = gen.generate(1007UL, 1007UL, 0, 1);

    phylanx::ir::node_data<std::uint8_t> lhs(m1);

    phylanx::execution_tree::primitive rhs =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<std::uint8_t>(m2));

    phylanx::execution_tree::primitive or_operation =
        phylanx::execution_tree::primitives::create_or_operation(
            hpx::find_here(),
            std::vector<phylanx::execution_tree::primitive_argument_type>{
                std::move(lhs), std::move(rhs)});

    phylanx::execution_tree::primitive_argument_type f =
        or_operation.eval().get();

    blaze::DynamicMatrix<std::uint8_t> expected =
        blaze::map(m1, m2, [](bool x, bool y) { return x || y; });

    HPX_TEST_EQ(phylanx::ir::node_data<std::uint8_t>(std::move(expected)),
        phylanx::execution_tree::extract_boolean_data(f));
}

void test_or_operation_2d_double()
{
    blaze::Rand<blaze::DynamicMatrix<int>> gen{};
    blaze::DynamicMatrix<double> m1 = gen.generate(1007UL, 1007UL, 0, 1);
    blaze::DynamicMatrix<double> m2 = gen.generate(1007UL, 1007UL, 0, 1);

    phylanx::execution_tree::primitive lhs =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(m1));

    phylanx::execution_tree::primitive rhs =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(m2));

    phylanx::execution_tree::primitive or_operation =
        phylanx::execution_tree::primitives::create_or_operation(
            hpx::find_here(),
            std::vector<phylanx::execution_tree::primitive_argument_type>{
                std::move(lhs), std::move(rhs)});

    phylanx::execution_tree::primitive_argument_type f =
        or_operation.eval().get();

    blaze::DynamicMatrix<std::uint8_t> expected =
        blaze::map(m1, m2, [](double x, double y) { return x || y; });

    HPX_TEST_EQ(phylanx::ir::node_data<std::uint8_t>(std::move(expected)),
        phylanx::execution_tree::extract_boolean_data(f));
}

void test_or_operation_2d_double_lit()
{
    blaze::Rand<blaze::DynamicMatrix<int>> gen{};
    blaze::DynamicMatrix<double> m1 = gen.generate(1007UL, 1007UL, 0, 1);
    blaze::DynamicMatrix<double> m2 = gen.generate(1007UL, 1007UL, 0, 1);

    phylanx::ir::node_data<double> lhs(m1);

    phylanx::execution_tree::primitive rhs =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(m2));

    phylanx::execution_tree::primitive or_operation =
        phylanx::execution_tree::primitives::create_or_operation(
            hpx::find_here(),
            std::vector<phylanx::execution_tree::primitive_argument_type>{
                std::move(lhs), std::move(rhs)});

    phylanx::execution_tree::primitive_argument_type f =
        or_operation.eval().get();

    blaze::DynamicMatrix<std::uint8_t> expected =
        blaze::map(m1, m2, [](double x, double y) { return x || y; });

    HPX_TEST_EQ(phylanx::ir::node_data<std::uint8_t>(std::move(expected)),
        phylanx::execution_tree::extract_boolean_data(f));
}

void test_or_operation_2d0d_true()
{
    blaze::Rand<blaze::DynamicMatrix<int>> gen{};
    blaze::DynamicMatrix<std::uint8_t> m = gen.generate(101UL, 101UL, 0, 1);

    phylanx::execution_tree::primitive lhs =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<std::uint8_t>(m));

    phylanx::execution_tree::primitive rhs =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<std::uint8_t>(true));

    phylanx::execution_tree::primitive or_operation =
        phylanx::execution_tree::primitives::create_or_operation(
            hpx::find_here(),
            std::vector<phylanx::execution_tree::primitive_argument_type>{
                std::move(lhs), std::move(rhs)});

    phylanx::execution_tree::primitive_argument_type f =
        or_operation.eval().get();

    blaze::DynamicMatrix<std::uint8_t> expected =
        blaze::map(m, [](bool x) { return (x || true); });

    HPX_TEST_EQ(phylanx::ir::node_data<std::uint8_t>(std::move(expected)),
        phylanx::execution_tree::extract_boolean_data(f));
}

void test_or_operation_2d0d_lit_true()
{
    blaze::Rand<blaze::DynamicMatrix<int>> gen{};
    blaze::DynamicMatrix<std::uint8_t> m = gen.generate(101UL, 101UL, 0, 1);

    phylanx::ir::node_data<std::uint8_t> lhs(m);

    phylanx::execution_tree::primitive rhs =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<std::uint8_t>(true));

    phylanx::execution_tree::primitive or_operation =
        phylanx::execution_tree::primitives::create_or_operation(
            hpx::find_here(),
            std::vector<phylanx::execution_tree::primitive_argument_type>{
                std::move(lhs), std::move(rhs)});

    phylanx::execution_tree::primitive_argument_type f =
        or_operation.eval().get();

    blaze::DynamicMatrix<std::uint8_t> expected =
        blaze::map(m, [](bool x) { return (x || true); });

    HPX_TEST_EQ(phylanx::ir::node_data<std::uint8_t>(std::move(expected)),
        phylanx::execution_tree::extract_boolean_data(f));
}

void test_or_operation_2d0d_false()
{
    blaze::Rand<blaze::DynamicMatrix<int>> gen{};
    blaze::DynamicMatrix<std::uint8_t> m = gen.generate(101UL, 101UL, 0, 1);

    phylanx::execution_tree::primitive lhs =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<std::uint8_t>(m));

    phylanx::execution_tree::primitive rhs =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<std::uint8_t>(false));

    phylanx::execution_tree::primitive or_operation =
        phylanx::execution_tree::primitives::create_or_operation(
            hpx::find_here(),
            std::vector<phylanx::execution_tree::primitive_argument_type>{
                std::move(lhs), std::move(rhs)});

    phylanx::execution_tree::primitive_argument_type f =
        or_operation.eval().get();

    blaze::DynamicMatrix<std::uint8_t> expected =
        blaze::map(m, [](bool x) { return (x || false); });

    HPX_TEST_EQ(phylanx::ir::node_data<std::uint8_t>(std::move(expected)),
        phylanx::execution_tree::extract_boolean_data(f));
}

void test_or_operation_2d0d_lit_false()
{
    blaze::Rand<blaze::DynamicMatrix<int>> gen{};
    blaze::DynamicMatrix<std::uint8_t> m = gen.generate(101UL, 101UL, 0, 1);

    phylanx::ir::node_data<std::uint8_t> lhs(m);

    phylanx::execution_tree::primitive rhs =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<std::uint8_t>(false));

    phylanx::execution_tree::primitive or_operation =
        phylanx::execution_tree::primitives::create_or_operation(
            hpx::find_here(),
            std::vector<phylanx::execution_tree::primitive_argument_type>{
                std::move(lhs), std::move(rhs)});

    phylanx::execution_tree::primitive_argument_type f =
        or_operation.eval().get();

    blaze::DynamicMatrix<std::uint8_t> expected =
        blaze::map(m, [](bool x) { return (x || false); });

    HPX_TEST_EQ(phylanx::ir::node_data<std::uint8_t>(std::move(expected)),
        phylanx::execution_tree::extract_boolean_data(f));
}

void test_or_operation_2d0d_double_true()
{
    blaze::Rand<blaze::DynamicMatrix<int>> gen{};
    blaze::DynamicMatrix<double> m = gen.generate(3UL, 3UL, 0, 1);

    phylanx::execution_tree::primitive lhs =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(m));

    phylanx::execution_tree::primitive rhs =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(2.0));

    phylanx::execution_tree::primitive or_operation =
        phylanx::execution_tree::primitives::create_or_operation(
            hpx::find_here(),
            std::vector<phylanx::execution_tree::primitive_argument_type>{
                std::move(lhs), std::move(rhs)});

    phylanx::execution_tree::primitive_argument_type f =
        or_operation.eval().get();

    blaze::DynamicMatrix<double> expected(m.rows(), m.columns(), 1.0);

    HPX_TEST_EQ(phylanx::ir::node_data<std::uint8_t>(std::move(expected)),
        phylanx::execution_tree::extract_boolean_data(f));
}

void test_or_operation_2d0d_double_lit_true()
{
    blaze::Rand<blaze::DynamicMatrix<int>> gen{};
    blaze::DynamicMatrix<double> m = gen.generate(101UL, 101UL, 0, 1);

    phylanx::ir::node_data<double> lhs(m);

    phylanx::execution_tree::primitive rhs =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(2.0));

    phylanx::execution_tree::primitive or_operation =
        phylanx::execution_tree::primitives::create_or_operation(
            hpx::find_here(),
            std::vector<phylanx::execution_tree::primitive_argument_type>{
                std::move(lhs), std::move(rhs)});

    phylanx::execution_tree::primitive_argument_type f =
        or_operation.eval().get();

    blaze::DynamicMatrix<double> expected(m.rows(), m.columns(), 1.0);

    HPX_TEST_EQ(phylanx::ir::node_data<std::uint8_t>(std::move(expected)),
        phylanx::execution_tree::extract_boolean_data(f));
}

void test_or_operation_2d0d_double_false()
{
    blaze::Rand<blaze::DynamicMatrix<int>> gen{};
    blaze::DynamicMatrix<double> m = gen.generate(101UL, 101UL, 0, 1);

    phylanx::execution_tree::primitive lhs =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(m));

    phylanx::execution_tree::primitive rhs =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(0.0));

    phylanx::execution_tree::primitive or_operation =
        phylanx::execution_tree::primitives::create_or_operation(
            hpx::find_here(),
            std::vector<phylanx::execution_tree::primitive_argument_type>{
                std::move(lhs), std::move(rhs)});

    phylanx::execution_tree::primitive_argument_type f =
        or_operation.eval().get();

    blaze::DynamicMatrix<std::uint8_t> expected =
        blaze::map(m, [](double x) { return (x != 0.0); });

    HPX_TEST_EQ(phylanx::ir::node_data<std::uint8_t>(std::move(expected)),
        phylanx::execution_tree::extract_boolean_data(f));
}

void test_or_operation_2d0d_double_lit_false()
{
    blaze::Rand<blaze::DynamicMatrix<int>> gen{};
    blaze::DynamicMatrix<double> m = gen.generate(101UL, 101UL, 0, 1);

    phylanx::ir::node_data<double> lhs(m);

    phylanx::execution_tree::primitive rhs =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(0.0));

    phylanx::execution_tree::primitive or_operation =
        phylanx::execution_tree::primitives::create_or_operation(
            hpx::find_here(),
            std::vector<phylanx::execution_tree::primitive_argument_type>{
                std::move(lhs), std::move(rhs)});

    phylanx::execution_tree::primitive_argument_type f =
        or_operation.eval().get();

    blaze::DynamicMatrix<std::uint8_t> expected =
        blaze::map(m, [](bool x) { return (x != 0.0); });

    HPX_TEST_EQ(phylanx::ir::node_data<std::uint8_t>(std::move(expected)),
        phylanx::execution_tree::extract_boolean_data(f));
}

void test_or_operation_2d1d()
{
    blaze::Rand<blaze::DynamicVector<int>> gen{};
    blaze::DynamicVector<std::uint8_t> v = gen.generate(104UL, 0, 1);

    blaze::Rand<blaze::DynamicMatrix<int>> mat_gen{};
    blaze::DynamicMatrix<std::uint8_t> m = mat_gen.generate(101UL, 104UL, 0, 1);

    phylanx::execution_tree::primitive lhs =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<std::uint8_t>(m));

    phylanx::execution_tree::primitive rhs =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<std::uint8_t>(v));

    phylanx::execution_tree::primitive or_operation =
        phylanx::execution_tree::primitives::create_or_operation(
            hpx::find_here(),
            std::vector<phylanx::execution_tree::primitive_argument_type>{
                std::move(lhs), std::move(rhs)});

    phylanx::execution_tree::primitive_argument_type f =
        or_operation.eval().get();

    blaze::DynamicMatrix<std::uint8_t> expected{m.rows(), m.columns()};
    for (size_t i = 0UL; i < m.rows(); i++)
        blaze::row(expected, i) = blaze::map(blaze::row(m, i),
            blaze::trans(v),
            [](bool x, bool y) { return x || y; });

    HPX_TEST_EQ(phylanx::ir::node_data<std::uint8_t>(std::move(expected)),
        phylanx::execution_tree::extract_boolean_data(f));
}

void test_or_operation_2d1d_lit()
{
    blaze::Rand<blaze::DynamicVector<int>> gen{};
    blaze::DynamicVector<std::uint8_t> v = gen.generate(104UL, 0, 1);

    blaze::Rand<blaze::DynamicMatrix<int>> mat_gen{};
    blaze::DynamicMatrix<std::uint8_t> m = mat_gen.generate(101UL, 104UL, 0, 1);

    phylanx::ir::node_data<std::uint8_t> lhs(m);

    phylanx::execution_tree::primitive rhs =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<std::uint8_t>(v));

    phylanx::execution_tree::primitive or_operation =
        phylanx::execution_tree::primitives::create_or_operation(
            hpx::find_here(),
            std::vector<phylanx::execution_tree::primitive_argument_type>{
                std::move(lhs), std::move(rhs)});

    phylanx::execution_tree::primitive_argument_type f =
        or_operation.eval().get();

    blaze::DynamicMatrix<std::uint8_t> expected{m.rows(), m.columns()};
    for (size_t i = 0UL; i < m.rows(); i++)
        blaze::row(expected, i) = blaze::map(blaze::row(m, i),
            blaze::trans(v),
            [](bool x, bool y) { return x || y; });

    HPX_TEST_EQ(phylanx::ir::node_data<std::uint8_t>(std::move(expected)),
        phylanx::execution_tree::extract_boolean_data(f));
}

void test_or_operation_2d1d_double()
{
    blaze::Rand<blaze::DynamicVector<int>> gen{};
    blaze::DynamicVector<double> v = gen.generate(104UL, 0, 1);

    blaze::Rand<blaze::DynamicMatrix<int>> mat_gen{};
    blaze::DynamicMatrix<double> m = mat_gen.generate(101UL, 104UL, 0, 1);

    phylanx::execution_tree::primitive lhs =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(m));

    phylanx::execution_tree::primitive rhs =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(v));

    phylanx::execution_tree::primitive or_operation =
        phylanx::execution_tree::primitives::create_or_operation(
            hpx::find_here(),
            std::vector<phylanx::execution_tree::primitive_argument_type>{
                std::move(lhs), std::move(rhs)});

    phylanx::execution_tree::primitive_argument_type f =
        or_operation.eval().get();

    blaze::DynamicMatrix<std::uint8_t> expected{m.rows(), m.columns()};
    for (size_t i = 0UL; i < m.rows(); i++)
        blaze::row(expected, i) = blaze::map(blaze::row(m, i),
            blaze::trans(v),
            [](double x, double y) { return x || y; });

    HPX_TEST_EQ(phylanx::ir::node_data<std::uint8_t>(std::move(expected)),
        phylanx::execution_tree::extract_boolean_data(f));
}

void test_or_operation_2d1d_double_lit()
{
    blaze::Rand<blaze::DynamicVector<int>> gen{};
    blaze::DynamicVector<double> v = gen.generate(104UL, 0, 1);

    blaze::Rand<blaze::DynamicMatrix<int>> mat_gen{};
    blaze::DynamicMatrix<double> m = mat_gen.generate(101UL, 104UL, 0, 1);

    phylanx::ir::node_data<double> lhs(m);

    phylanx::execution_tree::primitive rhs =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(v));

    phylanx::execution_tree::primitive or_operation =
        phylanx::execution_tree::primitives::create_or_operation(
            hpx::find_here(),
            std::vector<phylanx::execution_tree::primitive_argument_type>{
                std::move(lhs), std::move(rhs)});

    phylanx::execution_tree::primitive_argument_type f =
        or_operation.eval().get();

    blaze::DynamicMatrix<std::uint8_t> expected{m.rows(), m.columns()};
    for (size_t i = 0UL; i < m.rows(); i++)
        blaze::row(expected, i) = blaze::map(blaze::row(m, i),
            blaze::trans(v),
            [](double x, double y) { return x || y; });

    HPX_TEST_EQ(phylanx::ir::node_data<std::uint8_t>(std::move(expected)),
        phylanx::execution_tree::extract_boolean_data(f));
}

int main(int argc, char* argv[])
{
    test_or_operation_0d_false();
    test_or_operation_0d_true();
    test_or_operation_0d_lit_false();
    test_or_operation_0d_lit_true();
    test_or_operation_0d1d_false();
    test_or_operation_0d1d_lit_false();
    test_or_operation_0d1d_true();
    test_or_operation_0d1d_lit_true();
    test_or_operation_0d2d_true();
    test_or_operation_0d2d_lit_true();
    test_or_operation_0d2d_false();
    test_or_operation_0d2d_lit_false();

    test_or_operation_0d_double_false();
    test_or_operation_0d_double_true();
    test_or_operation_0d_double_lit_false();
    test_or_operation_0d_double_lit_true();
    test_or_operation_0d1d_double_false();
    test_or_operation_0d1d_double_lit_false();
    test_or_operation_0d1d_double_true();
    test_or_operation_0d1d_double_lit_true();
    test_or_operation_0d2d_double_true();
    test_or_operation_0d2d_double_lit_true();
    test_or_operation_0d2d_double_false();
    test_or_operation_0d2d_double_lit_false();

    test_or_operation_1d();
    test_or_operation_1d_lit();
    test_or_operation_1d0d_false();
    test_or_operation_1d0d_lit_false();
    test_or_operation_1d0d_true();
    test_or_operation_1d0d_lit_true();
    test_or_operation_1d2d();
    test_or_operation_1d2d_lit();

    test_or_operation_1d_double();
    test_or_operation_1d_double_lit();
    test_or_operation_1d0d_double_false();
    test_or_operation_1d0d_double_lit_false();
    test_or_operation_1d0d_double_true();
    test_or_operation_1d0d_double_lit_true();
    test_or_operation_1d2d_double();
    test_or_operation_1d2d_double_lit();

    test_or_operation_2d();
    test_or_operation_2d_lit();
    test_or_operation_2d0d_true();
    test_or_operation_2d0d_lit_true();
    test_or_operation_2d0d_false();
    test_or_operation_2d0d_lit_false();
    test_or_operation_2d1d();
    test_or_operation_2d1d_lit();

    test_or_operation_2d_double();
    test_or_operation_2d_double_lit();
    test_or_operation_2d0d_double_true();
    test_or_operation_2d0d_double_lit_true();
    test_or_operation_2d0d_double_false();
    test_or_operation_2d0d_double_lit_false();
    test_or_operation_2d1d_double();
    test_or_operation_2d1d_double_lit();

    return hpx::util::report_errors();
}
