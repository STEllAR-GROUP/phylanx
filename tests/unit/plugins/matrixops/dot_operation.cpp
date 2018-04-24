//   Copyright (c) 2017 Hartmut Kaiser
//   Copyright (c) 2017 Parsa Amini
//
//   Distributed under the Boost Software License, Version 1.0. (See accompanying
//   file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/phylanx.hpp>

#include <hpx/hpx_main.hpp>
#include <hpx/include/lcos.hpp>
#include <hpx/util/lightweight_test.hpp>

#include <iostream>
#include <utility>
#include <vector>

#include <blaze/Math.h>

void test_dot_operation_0d()
{
    phylanx::execution_tree::primitive lhs =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(6.0));

    phylanx::execution_tree::primitive rhs =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(7.0));

    phylanx::execution_tree::primitive dot =
        phylanx::execution_tree::primitives::create_dot_operation(
            hpx::find_here(),
            std::vector<phylanx::execution_tree::primitive_argument_type>{
                std::move(lhs), std::move(rhs)});

    hpx::future<phylanx::execution_tree::primitive_argument_type> f =
        dot.eval();
    HPX_TEST_EQ(
        42.0, phylanx::execution_tree::extract_numeric_value(f.get())[0]);
}

void test_dot_operation_0d_lit()
{
    phylanx::ir::node_data<double> lhs(6.0);

    phylanx::execution_tree::primitive rhs =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(7.0));

    phylanx::execution_tree::primitive dot =
        phylanx::execution_tree::primitives::create_dot_operation(
            hpx::find_here(),
            std::vector<phylanx::execution_tree::primitive_argument_type>{
                std::move(lhs), std::move(rhs)});

    hpx::future<phylanx::execution_tree::primitive_argument_type> f =
        dot.eval();
    HPX_TEST_EQ(
        42.0, phylanx::execution_tree::extract_numeric_value(f.get())[0]);
}

void test_dot_operation_0d1d()
{
    blaze::Rand<blaze::DynamicVector<double>> gen{};
    blaze::DynamicVector<double> v = gen.generate(1007UL);

    phylanx::execution_tree::primitive lhs =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(6.0));

    phylanx::execution_tree::primitive rhs =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(v));

    phylanx::execution_tree::primitive dot =
        phylanx::execution_tree::primitives::create_dot_operation(
            hpx::find_here(),
            std::vector<phylanx::execution_tree::primitive_argument_type>{
                std::move(lhs), std::move(rhs)});

    hpx::future<phylanx::execution_tree::primitive_argument_type> f =
        dot.eval();

    blaze::DynamicVector<double> expected = 6.0 * v;

    HPX_TEST_EQ(phylanx::ir::node_data<double>{expected},
        phylanx::execution_tree::extract_numeric_value(f.get()));
}

void test_dot_operation_0d1d_lit()
{
    blaze::Rand<blaze::DynamicVector<double>> gen{};
    blaze::DynamicVector<double> v = gen.generate(1007UL);

    phylanx::ir::node_data<double> lhs(6.0);

    phylanx::execution_tree::primitive rhs =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(v));

    phylanx::execution_tree::primitive dot =
        phylanx::execution_tree::primitives::create_dot_operation(
            hpx::find_here(),
            std::vector<phylanx::execution_tree::primitive_argument_type>{
                std::move(lhs), std::move(rhs)});

    hpx::future<phylanx::execution_tree::primitive_argument_type> f =
        dot.eval();

    blaze::DynamicVector<double> expected = 6.0 * v;

    HPX_TEST_EQ(phylanx::ir::node_data<double>{expected},
        phylanx::execution_tree::extract_numeric_value(f.get()));
}

void test_dot_operation_0d1d_numpy()
{
    blaze::DynamicVector<double> v{1.0, 3.0, 4.0};

    phylanx::execution_tree::primitive lhs =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(6.0));

    phylanx::execution_tree::primitive rhs =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(v));

    phylanx::execution_tree::primitive dot =
        phylanx::execution_tree::primitives::create_dot_operation(
            hpx::find_here(),
            std::vector<phylanx::execution_tree::primitive_argument_type>{
                std::move(lhs), std::move(rhs)});

    hpx::future<phylanx::execution_tree::primitive_argument_type> f =
        dot.eval();
    blaze::DynamicVector<double> expected{6.0, 18.0, 24.0};
    HPX_TEST_EQ(phylanx::ir::node_data<double>{expected},
        phylanx::execution_tree::extract_numeric_value(f.get()));
}

void test_dot_operation_0d2d()
{
    blaze::Rand<blaze::DynamicMatrix<double>> gen{};
    blaze::DynamicMatrix<double> m = gen.generate(101UL, 101UL);

    phylanx::execution_tree::primitive lhs =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(6.0));

    phylanx::execution_tree::primitive rhs =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(m));

    phylanx::execution_tree::primitive dot =
        phylanx::execution_tree::primitives::create_dot_operation(
            hpx::find_here(),
            std::vector<phylanx::execution_tree::primitive_argument_type>{
                std::move(lhs), std::move(rhs)});

    hpx::future<phylanx::execution_tree::primitive_argument_type> f =
        dot.eval();

    blaze::DynamicMatrix<double> expected = 6.0 * m;

    HPX_TEST_EQ(phylanx::ir::node_data<double>{expected},
        phylanx::execution_tree::extract_numeric_value(f.get()));
}

void test_dot_operation_0d2d_lit()
{
    blaze::Rand<blaze::DynamicMatrix<double>> gen{};
    blaze::DynamicMatrix<double> m = gen.generate(101UL, 101UL);

    phylanx::ir::node_data<double> lhs(6.0);

    phylanx::execution_tree::primitive rhs =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(m));

    phylanx::execution_tree::primitive dot =
        phylanx::execution_tree::primitives::create_dot_operation(
            hpx::find_here(),
            std::vector<phylanx::execution_tree::primitive_argument_type>{
                std::move(lhs), std::move(rhs)});

    hpx::future<phylanx::execution_tree::primitive_argument_type> f =
        dot.eval();

    blaze::DynamicMatrix<double> expected = 6.0 * m;

    HPX_TEST_EQ(phylanx::ir::node_data<double>{expected},
        phylanx::execution_tree::extract_numeric_value(f.get()));
}

void test_dot_operation_0d2d_numpy()
{
    blaze::DynamicMatrix<double> m{{1.0, 3.0, 4.0}, {2.0, 5.0, 6.0}};

    phylanx::execution_tree::primitive lhs =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(6.0));

    phylanx::execution_tree::primitive rhs =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(m));

    phylanx::execution_tree::primitive dot =
        phylanx::execution_tree::primitives::create_dot_operation(
            hpx::find_here(),
            std::vector<phylanx::execution_tree::primitive_argument_type>{
                std::move(lhs), std::move(rhs)});

    hpx::future<phylanx::execution_tree::primitive_argument_type> f =
        dot.eval();

    blaze::DynamicMatrix<double> expected{
        {6.0, 18.0, 24.0}, {12.0, 30.0, 36.0}};
    HPX_TEST_EQ(phylanx::ir::node_data<double>{expected},
        phylanx::execution_tree::extract_numeric_value(f.get()));
}

void test_dot_operation_1d0d()
{
    blaze::Rand<blaze::DynamicVector<double>> gen{};
    blaze::DynamicVector<double> v = gen.generate(1007UL);

    phylanx::execution_tree::primitive lhs =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(v));

    phylanx::execution_tree::primitive rhs =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(6.0));

    phylanx::execution_tree::primitive dot =
        phylanx::execution_tree::primitives::create_dot_operation(
            hpx::find_here(),
            std::vector<phylanx::execution_tree::primitive_argument_type>{
                std::move(lhs), std::move(rhs)});

    hpx::future<phylanx::execution_tree::primitive_argument_type> f =
        dot.eval();

    blaze::DynamicVector<double> expected = v * 6.0;

    HPX_TEST_EQ(phylanx::ir::node_data<double>{expected},
        phylanx::execution_tree::extract_numeric_value(f.get()));
}

void test_dot_operation_1d0d_lit()
{
    blaze::Rand<blaze::DynamicVector<double>> gen{};
    blaze::DynamicVector<double> v = gen.generate(1007UL);

    phylanx::ir::node_data<double> lhs(v);

    phylanx::execution_tree::primitive rhs =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(6.0));

    phylanx::execution_tree::primitive dot =
        phylanx::execution_tree::primitives::create_dot_operation(
            hpx::find_here(),
            std::vector<phylanx::execution_tree::primitive_argument_type>{
                std::move(lhs), std::move(rhs)});

    hpx::future<phylanx::execution_tree::primitive_argument_type> f =
        dot.eval();

    blaze::DynamicVector<double> expected = v * 6.0;

    HPX_TEST_EQ(phylanx::ir::node_data<double>{expected},
        phylanx::execution_tree::extract_numeric_value(f.get()));
}

void test_dot_operation_1d0d_numpy()
{
    blaze::DynamicVector<double> v{1.0, 3.0, 4.0};

    phylanx::execution_tree::primitive lhs =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(v));

    phylanx::execution_tree::primitive rhs =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(6.0));

    phylanx::execution_tree::primitive dot =
        phylanx::execution_tree::primitives::create_dot_operation(
            hpx::find_here(),
            std::vector<phylanx::execution_tree::primitive_argument_type>{
                std::move(lhs), std::move(rhs)});

    hpx::future<phylanx::execution_tree::primitive_argument_type> f =
        dot.eval();
    blaze::DynamicVector<double> expected{6.0, 18.0, 24.0};
    HPX_TEST_EQ(phylanx::ir::node_data<double>{expected},
        phylanx::execution_tree::extract_numeric_value(f.get()));
}

void test_dot_operation_1d()
{
    blaze::Rand<blaze::DynamicVector<double>> gen{};
    blaze::DynamicVector<double> v1 = gen.generate(1007UL);
    blaze::DynamicVector<double> v2 = gen.generate(1007UL);

    phylanx::execution_tree::primitive lhs =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(v1));

    phylanx::execution_tree::primitive rhs =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(v2));

    phylanx::execution_tree::primitive dot =
        phylanx::execution_tree::primitives::create_dot_operation(
            hpx::find_here(),
            std::vector<phylanx::execution_tree::primitive_argument_type>{
                std::move(lhs), std::move(rhs)});

    hpx::future<phylanx::execution_tree::primitive_argument_type> f =
        dot.eval();

    double expected = blaze::dot(v1, v2);

    HPX_TEST_EQ(phylanx::ir::node_data<double>{expected},
        phylanx::execution_tree::extract_numeric_value(f.get()));
}

void test_dot_operation_1d_lit()
{
    blaze::Rand<blaze::DynamicVector<double>> gen{};
    blaze::DynamicVector<double> v1 = gen.generate(1007UL);
    blaze::DynamicVector<double> v2 = gen.generate(1007UL);

    phylanx::ir::node_data<double> lhs(v1);

    phylanx::execution_tree::primitive rhs =
            phylanx::execution_tree::primitives::create_variable(
                    hpx::find_here(), phylanx::ir::node_data<double>(v2));

    phylanx::execution_tree::primitive dot =
            phylanx::execution_tree::primitives::create_dot_operation(
                    hpx::find_here(),
                    std::vector<phylanx::execution_tree::primitive_argument_type>{
                            std::move(lhs), std::move(rhs)});

    hpx::future<phylanx::execution_tree::primitive_argument_type> f =
            dot.eval();

    double expected = blaze::dot(v1, v2);

    HPX_TEST_EQ(phylanx::ir::node_data<double>{expected},
                phylanx::execution_tree::extract_numeric_value(f.get()));
}

void test_dot_operation_1d_numpy()
{
    blaze::DynamicVector<double> v1{1.0, 4.0, 9.0, 7.0};
    blaze::DynamicVector<double> v2{3.0, 2.0, 1.0, 0.0};

    phylanx::execution_tree::primitive lhs =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(v1));

    phylanx::execution_tree::primitive rhs =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(v2));

    phylanx::execution_tree::primitive dot =
        phylanx::execution_tree::primitives::create_dot_operation(
            hpx::find_here(),
            std::vector<phylanx::execution_tree::primitive_argument_type>{
                std::move(lhs), std::move(rhs)});

    hpx::future<phylanx::execution_tree::primitive_argument_type> f =
        dot.eval();

    double expected=20.0;

    HPX_TEST_EQ(phylanx::ir::node_data<double>{expected},
        phylanx::execution_tree::extract_numeric_value(f.get()));
}

void test_dot_operation_1d2d()
{
    blaze::Rand<blaze::DynamicVector<double>> gen{};
    blaze::DynamicVector<double> v = gen.generate(101UL);

    blaze::Rand<blaze::DynamicMatrix<double>> mat_gen{};
    blaze::DynamicMatrix<double> m = mat_gen.generate(101UL, 104UL);

    phylanx::execution_tree::primitive lhs =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(v));

    phylanx::execution_tree::primitive rhs =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(m));

    phylanx::execution_tree::primitive dot =
        phylanx::execution_tree::primitives::create_dot_operation(
            hpx::find_here(),
            std::vector<phylanx::execution_tree::primitive_argument_type>{
                std::move(lhs), std::move(rhs)});

    hpx::future<phylanx::execution_tree::primitive_argument_type> f =
        dot.eval();

    blaze::DynamicVector<double> expected = blaze::trans(blaze::trans(v) * m);

    HPX_TEST_EQ(phylanx::ir::node_data<double>{expected},
        phylanx::execution_tree::extract_numeric_value(f.get()));
}

void test_dot_operation_1d2d_lit()
{
    blaze::Rand<blaze::DynamicVector<double>> gen{};
    blaze::DynamicVector<double> v = gen.generate(101UL);

    blaze::Rand<blaze::DynamicMatrix<double>> mat_gen{};
    blaze::DynamicMatrix<double> m = mat_gen.generate(101UL, 104UL);

    phylanx::ir::node_data<double> lhs(v);

    phylanx::execution_tree::primitive rhs =
            phylanx::execution_tree::primitives::create_variable(
                    hpx::find_here(), phylanx::ir::node_data<double>(m));

    phylanx::execution_tree::primitive dot =
            phylanx::execution_tree::primitives::create_dot_operation(
                    hpx::find_here(),
                    std::vector<phylanx::execution_tree::primitive_argument_type>{
                            std::move(lhs), std::move(rhs)});

    hpx::future<phylanx::execution_tree::primitive_argument_type> f =
            dot.eval();

    blaze::DynamicVector<double> expected = blaze::trans(blaze::trans(v) * m);

    HPX_TEST_EQ(phylanx::ir::node_data<double>{expected},
                phylanx::execution_tree::extract_numeric_value(f.get()));
}

void test_dot_operation_1d2d_numpy()
{
    blaze::DynamicVector<double> v{4.0, 2.0, 3.0};
    blaze::DynamicMatrix<double> m{
        {2.0, 3.0, 4.0}, {1.0, 3.0, 4.0}, {2.0, 5.0, 6.0}};

    phylanx::execution_tree::primitive lhs =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(v));

    phylanx::execution_tree::primitive rhs =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(m));

    phylanx::execution_tree::primitive dot =
        phylanx::execution_tree::primitives::create_dot_operation(
            hpx::find_here(),
            std::vector<phylanx::execution_tree::primitive_argument_type>{
                std::move(lhs), std::move(rhs)});

    hpx::future<phylanx::execution_tree::primitive_argument_type> f =
        dot.eval();

    blaze::DynamicVector<double> expected{16.0, 33.0, 42.0};
    HPX_TEST_EQ(phylanx::ir::node_data<double>(std::move(expected)),
        phylanx::execution_tree::extract_numeric_value(f.get()));
}

void test_dot_operation_2d0d()
{
    blaze::Rand<blaze::DynamicMatrix<double>> gen{};
    blaze::DynamicMatrix<double> m = gen.generate(101UL, 101UL);

    phylanx::execution_tree::primitive lhs =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(m));

    phylanx::execution_tree::primitive rhs =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(6.0));

    phylanx::execution_tree::primitive dot =
        phylanx::execution_tree::primitives::create_dot_operation(
            hpx::find_here(),
            std::vector<phylanx::execution_tree::primitive_argument_type>{
                std::move(lhs), std::move(rhs)});

    hpx::future<phylanx::execution_tree::primitive_argument_type> f =
        dot.eval();

    blaze::DynamicMatrix<double> expected = m * 6.0;

    HPX_TEST_EQ(phylanx::ir::node_data<double>{expected},
        phylanx::execution_tree::extract_numeric_value(f.get()));
}

void test_dot_operation_2d0d_lit()
{
    blaze::Rand<blaze::DynamicMatrix<double>> gen{};
    blaze::DynamicMatrix<double> m = gen.generate(101UL, 101UL);

    phylanx::ir::node_data<double> lhs(m);

    phylanx::execution_tree::primitive rhs =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(6.0));

    phylanx::execution_tree::primitive dot =
        phylanx::execution_tree::primitives::create_dot_operation(
            hpx::find_here(),
            std::vector<phylanx::execution_tree::primitive_argument_type>{
                std::move(lhs), std::move(rhs)});

    hpx::future<phylanx::execution_tree::primitive_argument_type> f =
        dot.eval();

    blaze::DynamicMatrix<double> expected = m * 6.0;

    HPX_TEST_EQ(phylanx::ir::node_data<double>{expected},
        phylanx::execution_tree::extract_numeric_value(f.get()));
}

void test_dot_operation_2d0d_numpy()
{
    blaze::DynamicMatrix<double> m{{1.0, 3.0, 4.0}, {2.0, 5.0, 6.0}};

    phylanx::execution_tree::primitive lhs =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(m));

    phylanx::execution_tree::primitive rhs =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(6.0));

    phylanx::execution_tree::primitive dot =
        phylanx::execution_tree::primitives::create_dot_operation(
            hpx::find_here(),
            std::vector<phylanx::execution_tree::primitive_argument_type>{
                std::move(lhs), std::move(rhs)});

    hpx::future<phylanx::execution_tree::primitive_argument_type> f =
        dot.eval();

    blaze::DynamicMatrix<double> expected{
        {6.0, 18.0, 24.0}, {12.0, 30.0, 36.0}};
    HPX_TEST_EQ(phylanx::ir::node_data<double>{expected},
        phylanx::execution_tree::extract_numeric_value(f.get()));
}

void test_dot_operation_2d1d()
{
    blaze::Rand<blaze::DynamicMatrix<double>> gen1{};
    blaze::DynamicMatrix<double> m = gen1.generate(1007UL, 42UL);

    blaze::Rand<blaze::DynamicVector<double>> gen2{};
    blaze::DynamicVector<double> v = gen2.generate(42UL);

    blaze::DynamicVector<double> expected = m * v;

    phylanx::execution_tree::primitive lhs =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(m));

    phylanx::execution_tree::primitive rhs =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(v));

    phylanx::execution_tree::primitive dot =
        phylanx::execution_tree::primitives::create_dot_operation(
            hpx::find_here(),
            std::vector<phylanx::execution_tree::primitive_argument_type>{
        std::move(lhs), std::move(rhs)
    });

    hpx::future<phylanx::execution_tree::primitive_argument_type> f =
        dot.eval();

    HPX_TEST_EQ(phylanx::ir::node_data<double>(std::move(expected)),
        phylanx::execution_tree::extract_numeric_value(f.get()));
}

void test_dot_operation_2d1d_lit()
{
    blaze::Rand<blaze::DynamicMatrix<double>> gen1{};
    blaze::DynamicMatrix<double> m = gen1.generate(1007UL, 42UL);

    blaze::Rand<blaze::DynamicVector<double>> gen2{};
    blaze::DynamicVector<double> v = gen2.generate(42UL);

    blaze::DynamicVector<double> expected = m * v;

    phylanx::ir::node_data<double> lhs(m);

    phylanx::execution_tree::primitive rhs =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(v));

    phylanx::execution_tree::primitive dot =
        phylanx::execution_tree::primitives::create_dot_operation(
            hpx::find_here(),
            std::vector<phylanx::execution_tree::primitive_argument_type>{
                std::move(lhs), std::move(rhs)});

    hpx::future<phylanx::execution_tree::primitive_argument_type> f =
        dot.eval();

    HPX_TEST_EQ(phylanx::ir::node_data<double>(std::move(expected)),
        phylanx::execution_tree::extract_numeric_value(f.get()));
}

void test_dot_operation_2d1d_numpy()
{
    blaze::DynamicVector<double> v{4.0, 2.0, 3.0};
    blaze::DynamicMatrix<double> m{
        {2.0, 3.0, 4.0}, {1.0, 3.0, 4.0}};

    phylanx::execution_tree::primitive lhs =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(m));

    phylanx::execution_tree::primitive rhs =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(v));

    phylanx::execution_tree::primitive dot =
        phylanx::execution_tree::primitives::create_dot_operation(
            hpx::find_here(),
            std::vector<phylanx::execution_tree::primitive_argument_type>{
                std::move(lhs), std::move(rhs)});

    hpx::future<phylanx::execution_tree::primitive_argument_type> f =
        dot.eval();

    blaze::DynamicVector<double> expected{26.0, 22.0};
    HPX_TEST_EQ(phylanx::ir::node_data<double>(std::move(expected)),
        phylanx::execution_tree::extract_numeric_value(f.get()));
}

void test_dot_operation_2d2d()
{
    blaze::Rand<blaze::DynamicMatrix<double>> gen{};
    blaze::DynamicMatrix<double> m1 = gen.generate(42UL, 42UL);
    blaze::DynamicMatrix<double> m2 = gen.generate(42UL, 42UL);

    blaze::DynamicMatrix<double> expected = m1 * m2;

    phylanx::execution_tree::primitive lhs =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(m1));

    phylanx::execution_tree::primitive rhs =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(m2));

    phylanx::execution_tree::primitive dot =
        phylanx::execution_tree::primitives::create_dot_operation(
            hpx::find_here(),
            std::vector<phylanx::execution_tree::primitive_argument_type>{
                std::move(lhs), std::move(rhs)
            });

    hpx::future<phylanx::execution_tree::primitive_argument_type> f =
        dot.eval();

    HPX_TEST_EQ(phylanx::ir::node_data<double>(std::move(expected)),
        phylanx::execution_tree::extract_numeric_value(f.get()));
}

void test_dot_operation_2d2d_lit()
{
    blaze::Rand<blaze::DynamicMatrix<double>> gen{};
    blaze::DynamicMatrix<double> m1 = gen.generate(42UL, 42UL);
    blaze::DynamicMatrix<double> m2 = gen.generate(42UL, 42UL);

    blaze::DynamicMatrix<double> expected = m1 * m2;

    phylanx::ir::node_data<double> lhs(m1);

    phylanx::execution_tree::primitive rhs =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(m2));

    phylanx::execution_tree::primitive dot =
        phylanx::execution_tree::primitives::create_dot_operation(
            hpx::find_here(),
            std::vector<phylanx::execution_tree::primitive_argument_type>{
                std::move(lhs), std::move(rhs)});

    hpx::future<phylanx::execution_tree::primitive_argument_type> f =
        dot.eval();

    HPX_TEST_EQ(phylanx::ir::node_data<double>(std::move(expected)),
        phylanx::execution_tree::extract_numeric_value(f.get()));
}

void test_dot_operation_2d2d_numpy()
{
    blaze::DynamicMatrix<double> m1{{4.0, 2.0}, {3.0, 5.0}};
    blaze::DynamicMatrix<double> m2{{2.0, 3.0, 4.0}, {1.0, 3.0, 4.0}};

    phylanx::execution_tree::primitive lhs =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(m1));

    phylanx::execution_tree::primitive rhs =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(m2));

    phylanx::execution_tree::primitive dot =
        phylanx::execution_tree::primitives::create_dot_operation(
            hpx::find_here(),
            std::vector<phylanx::execution_tree::primitive_argument_type>{
                std::move(lhs), std::move(rhs)});

    hpx::future<phylanx::execution_tree::primitive_argument_type> f =
        dot.eval();

    blaze::DynamicMatrix<double> expected{
        {10.0, 18.0, 24.0}, {11.0, 24.0, 32.0}};
    HPX_TEST_EQ(phylanx::ir::node_data<double>(std::move(expected)),
        phylanx::execution_tree::extract_numeric_value(f.get()));
}

int main(int argc, char* argv[])
{
    test_dot_operation_0d();
    test_dot_operation_0d_lit();
    test_dot_operation_0d1d();
    test_dot_operation_0d1d_lit();
    test_dot_operation_0d1d_numpy();
    test_dot_operation_0d2d();
    test_dot_operation_0d2d_lit();
    test_dot_operation_0d2d_numpy();

    test_dot_operation_1d0d();
    test_dot_operation_1d0d_lit();
    test_dot_operation_1d0d_numpy();
    test_dot_operation_1d();
    test_dot_operation_1d_lit();
    test_dot_operation_1d_numpy();
    test_dot_operation_1d2d();
    test_dot_operation_1d2d_lit();
    test_dot_operation_1d2d_numpy();

    test_dot_operation_2d0d();
    test_dot_operation_2d0d_lit();
    test_dot_operation_2d0d_numpy();
    test_dot_operation_2d1d();
    test_dot_operation_2d1d_lit();
    test_dot_operation_2d1d_numpy();
    test_dot_operation_2d2d();
    test_dot_operation_2d2d_lit();
    test_dot_operation_2d2d_numpy();

    return hpx::util::report_errors();
}

