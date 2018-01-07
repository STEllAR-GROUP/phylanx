//   Copyright (c) 2017-2018 Hartmut Kaiser
//
//   Distributed under the Boost Software License, Version 1.0. (See accompanying
//   file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/phylanx.hpp>

#include <hpx/hpx_main.hpp>
#include <hpx/include/lcos.hpp>
#include <hpx/util/lightweight_test.hpp>

#include <utility>
#include <vector>
#include <blaze/Math.h>

void test_add_operation_0d()
{
    phylanx::execution_tree::primitive lhs =
        hpx::new_<phylanx::execution_tree::primitives::variable>(
            hpx::find_here(), phylanx::ir::node_data<double>(41.0));

    phylanx::execution_tree::primitive rhs =
        hpx::new_<phylanx::execution_tree::primitives::variable>(
            hpx::find_here(), phylanx::ir::node_data<double>(1.0));

    phylanx::execution_tree::primitive add =
        hpx::new_<phylanx::execution_tree::primitives::add_operation>(
            hpx::find_here(),
            std::vector<phylanx::execution_tree::primitive_argument_type>{
                std::move(lhs), std::move(rhs)});

    hpx::future<phylanx::execution_tree::primitive_result_type> f = add.eval();

    HPX_TEST_EQ(
        42.0, phylanx::execution_tree::extract_numeric_value(f.get())[0]);
}

void test_add_operation_0d_lit()
{
    phylanx::ir::node_data<double> lhs(41.0);

    phylanx::execution_tree::primitive rhs =
        hpx::new_<phylanx::execution_tree::primitives::variable>(
            hpx::find_here(), phylanx::ir::node_data<double>(1.0));

    phylanx::execution_tree::primitive add =
        hpx::new_<phylanx::execution_tree::primitives::add_operation>(
            hpx::find_here(),
            std::vector<phylanx::execution_tree::primitive_argument_type>{
                std::move(lhs), std::move(rhs)});

    hpx::future<phylanx::execution_tree::primitive_result_type> f = add.eval();

    HPX_TEST_EQ(
        42.0, phylanx::execution_tree::extract_numeric_value(f.get())[0]);
}

void test_add_operation_0d1d()
{
    blaze::Rand<blaze::DynamicVector<double>> gen{};
    blaze::DynamicVector<double> v = gen.generate(1007UL);

    phylanx::execution_tree::primitive lhs =
        hpx::new_<phylanx::execution_tree::primitives::variable>(
            hpx::find_here(), phylanx::ir::node_data<double>(41.0));

    phylanx::execution_tree::primitive rhs =
        hpx::new_<phylanx::execution_tree::primitives::variable>(
            hpx::find_here(), phylanx::ir::node_data<double>(v));

    phylanx::execution_tree::primitive add =
        hpx::new_<phylanx::execution_tree::primitives::add_operation>(
            hpx::find_here(),
            std::vector<phylanx::execution_tree::primitive_argument_type>{
                std::move(lhs), std::move(rhs)});

    hpx::future<phylanx::execution_tree::primitive_result_type> f = add.eval();
    blaze::DynamicVector<double> expected =
        blaze::map(v, [](double x) { return 41.0 + x; });

    HPX_TEST_EQ(phylanx::ir::node_data<double>(std::move(expected)),
        phylanx::execution_tree::extract_numeric_value(f.get()));
}

void test_add_operation_0d1d_lit()
{
    blaze::Rand<blaze::DynamicVector<double>> gen{};
    blaze::DynamicVector<double> v = gen.generate(1007UL);

    phylanx::ir::node_data<double> lhs(41.0);

    phylanx::execution_tree::primitive rhs =
        hpx::new_<phylanx::execution_tree::primitives::variable>(
            hpx::find_here(), phylanx::ir::node_data<double>(v));

    phylanx::execution_tree::primitive add =
        hpx::new_<phylanx::execution_tree::primitives::add_operation>(
            hpx::find_here(),
            std::vector<phylanx::execution_tree::primitive_argument_type>{
                std::move(lhs), std::move(rhs)});

    hpx::future<phylanx::execution_tree::primitive_result_type> f = add.eval();
    blaze::DynamicVector<double> expected =
        blaze::map(v, [](double x) { return 41.0 + x; });

    HPX_TEST_EQ(phylanx::ir::node_data<double>(std::move(expected)),
        phylanx::execution_tree::extract_numeric_value(f.get()));
}

void test_add_operation_0d2d()
{
    blaze::Rand<blaze::DynamicMatrix<double>> gen{};
    blaze::DynamicMatrix<double> m = gen.generate(101UL, 101UL);

    phylanx::execution_tree::primitive lhs =
        hpx::new_<phylanx::execution_tree::primitives::variable>(
            hpx::find_here(), phylanx::ir::node_data<double>(41.0));

    phylanx::execution_tree::primitive rhs =
        hpx::new_<phylanx::execution_tree::primitives::variable>(
            hpx::find_here(), phylanx::ir::node_data<double>(m));

    phylanx::execution_tree::primitive add =
        hpx::new_<phylanx::execution_tree::primitives::add_operation>(
            hpx::find_here(),
            std::vector<phylanx::execution_tree::primitive_argument_type>{
                std::move(lhs), std::move(rhs)});

    hpx::future<phylanx::execution_tree::primitive_result_type> f = add.eval();
    blaze::DynamicMatrix<double> expected =
        blaze::map(m, [](double x) { return 41.0 + x; });

    HPX_TEST_EQ(phylanx::ir::node_data<double>(std::move(expected)),
        phylanx::execution_tree::extract_numeric_value(f.get()));
}

void test_add_operation_0d2d_lit()
{
    blaze::Rand<blaze::DynamicMatrix<double>> gen{};
    blaze::DynamicMatrix<double> m = gen.generate(101UL, 101UL);

    phylanx::ir::node_data<double> lhs(41.0);

    phylanx::execution_tree::primitive rhs =
        hpx::new_<phylanx::execution_tree::primitives::variable>(
            hpx::find_here(), phylanx::ir::node_data<double>(m));

    phylanx::execution_tree::primitive add =
        hpx::new_<phylanx::execution_tree::primitives::add_operation>(
            hpx::find_here(),
            std::vector<phylanx::execution_tree::primitive_argument_type>{
                std::move(lhs), std::move(rhs)});

    hpx::future<phylanx::execution_tree::primitive_result_type> f = add.eval();
    blaze::DynamicMatrix<double> expected =
        blaze::map(m, [](double x) { return 41.0 + x; });

    HPX_TEST_EQ(phylanx::ir::node_data<double>(std::move(expected)),
        phylanx::execution_tree::extract_numeric_value(f.get()));
}

void test_add_operation_1d()
{
    blaze::Rand<blaze::DynamicVector<double>> gen{};
    blaze::DynamicVector<double> v1 = gen.generate(1007UL);
    blaze::DynamicVector<double> v2 = gen.generate(1007UL);

    phylanx::execution_tree::primitive lhs =
        hpx::new_<phylanx::execution_tree::primitives::variable>(
            hpx::find_here(), phylanx::ir::node_data<double>(v1));

    phylanx::execution_tree::primitive rhs =
        hpx::new_<phylanx::execution_tree::primitives::variable>(
            hpx::find_here(), phylanx::ir::node_data<double>(v2));

    phylanx::execution_tree::primitive add =
        hpx::new_<phylanx::execution_tree::primitives::add_operation>(
            hpx::find_here(),
            std::vector<phylanx::execution_tree::primitive_argument_type>{
                std::move(lhs), std::move(rhs)});

    hpx::future<phylanx::execution_tree::primitive_result_type> f = add.eval();

    blaze::DynamicVector<double> expected = v1 + v2;
    HPX_TEST_EQ(phylanx::ir::node_data<double>(std::move(expected)),
        phylanx::execution_tree::extract_numeric_value(f.get()));
}

void test_add_operation_1d_lit()
{
    blaze::Rand<blaze::DynamicVector<double>> gen{};
    blaze::DynamicVector<double> v1 = gen.generate(1007UL);
    blaze::DynamicVector<double> v2 = gen.generate(1007UL);

    phylanx::ir::node_data<double> lhs(v1);

    phylanx::execution_tree::primitive rhs =
        hpx::new_<phylanx::execution_tree::primitives::variable>(
            hpx::find_here(), phylanx::ir::node_data<double>(v2));

    phylanx::execution_tree::primitive add =
        hpx::new_<phylanx::execution_tree::primitives::add_operation>(
            hpx::find_here(),
            std::vector<phylanx::execution_tree::primitive_argument_type>{
                std::move(lhs), std::move(rhs)});

    hpx::future<phylanx::execution_tree::primitive_result_type> f = add.eval();

    blaze::DynamicVector<double> expected = v1 + v2;
    HPX_TEST_EQ(phylanx::ir::node_data<double>(std::move(expected)),
        phylanx::execution_tree::extract_numeric_value(f.get()));
}

void test_add_operation_1d0d()
{
    blaze::Rand<blaze::DynamicVector<double>> gen{};
    blaze::DynamicVector<double> v = gen.generate(1007UL);

    phylanx::execution_tree::primitive lhs =
        hpx::new_<phylanx::execution_tree::primitives::variable>(
            hpx::find_here(), phylanx::ir::node_data<double>(v));

    phylanx::execution_tree::primitive rhs =
        hpx::new_<phylanx::execution_tree::primitives::variable>(
            hpx::find_here(), phylanx::ir::node_data<double>(41.0));

    phylanx::execution_tree::primitive add =
        hpx::new_<phylanx::execution_tree::primitives::add_operation>(
            hpx::find_here(),
            std::vector<phylanx::execution_tree::primitive_argument_type>{
                std::move(lhs), std::move(rhs)});

    hpx::future<phylanx::execution_tree::primitive_result_type> f = add.eval();
    blaze::DynamicVector<double> expected =
        blaze::map(v, [](double x) { return 41.0 + x; });

    HPX_TEST_EQ(phylanx::ir::node_data<double>(std::move(expected)),
        phylanx::execution_tree::extract_numeric_value(f.get()));
}

void test_add_operation_1d0d_lit()
{
    blaze::Rand<blaze::DynamicVector<double>> gen{};
    blaze::DynamicVector<double> v = gen.generate(1007UL);

    phylanx::ir::node_data<double> lhs(v);

    phylanx::execution_tree::primitive rhs =
        hpx::new_<phylanx::execution_tree::primitives::variable>(
            hpx::find_here(), phylanx::ir::node_data<double>(41.0));

    phylanx::execution_tree::primitive add =
        hpx::new_<phylanx::execution_tree::primitives::add_operation>(
            hpx::find_here(),
            std::vector<phylanx::execution_tree::primitive_argument_type>{
                std::move(lhs), std::move(rhs)});

    hpx::future<phylanx::execution_tree::primitive_result_type> f = add.eval();
    blaze::DynamicVector<double> expected =
        blaze::map(v, [](double x) { return 41.0 + x; });

    HPX_TEST_EQ(phylanx::ir::node_data<double>(std::move(expected)),
        phylanx::execution_tree::extract_numeric_value(f.get()));
}

void test_add_operation_1d2d()
{
    blaze::Rand<blaze::DynamicVector<double>> gen{};
    blaze::DynamicVector<double> v = gen.generate(104UL);

    blaze::Rand<blaze::DynamicMatrix<double>> mat_gen{};
    blaze::DynamicMatrix<double> m = mat_gen.generate(101UL, 104UL);

    phylanx::execution_tree::primitive lhs =
        hpx::new_<phylanx::execution_tree::primitives::variable>(
            hpx::find_here(), phylanx::ir::node_data<double>(v));

    phylanx::execution_tree::primitive rhs =
        hpx::new_<phylanx::execution_tree::primitives::variable>(
            hpx::find_here(), phylanx::ir::node_data<double>(m));

    phylanx::execution_tree::primitive add =
        hpx::new_<phylanx::execution_tree::primitives::add_operation>(
            hpx::find_here(),
            std::vector<phylanx::execution_tree::primitive_argument_type>{
                std::move(lhs), std::move(rhs)});

    hpx::future<phylanx::execution_tree::primitive_result_type> f = add.eval();
    blaze::DynamicMatrix<double> expected(m.rows(), m.columns());
    for (size_t i = 0UL; i < m.rows(); ++i)
    {
        blaze::row(expected, i) = blaze::row(m, i) + blaze::trans(v);
    }

    HPX_TEST_EQ(phylanx::ir::node_data<double>(std::move(expected)),
        phylanx::execution_tree::extract_numeric_value(f.get()));
}

void test_add_operation_1d2d_lit()
{
    blaze::Rand<blaze::DynamicVector<double>> gen{};
    blaze::DynamicVector<double> v = gen.generate(104UL);

    blaze::Rand<blaze::DynamicMatrix<double>> mat_gen{};
    blaze::DynamicMatrix<double> m = mat_gen.generate(101UL, 104UL);

    phylanx::ir::node_data<double> lhs(v);

    phylanx::execution_tree::primitive rhs =
        hpx::new_<phylanx::execution_tree::primitives::variable>(
            hpx::find_here(), phylanx::ir::node_data<double>(m));

    phylanx::execution_tree::primitive add =
        hpx::new_<phylanx::execution_tree::primitives::add_operation>(
            hpx::find_here(),
            std::vector<phylanx::execution_tree::primitive_argument_type>{
                std::move(lhs), std::move(rhs)});

    hpx::future<phylanx::execution_tree::primitive_result_type> f = add.eval();
    blaze::DynamicMatrix<double> expected(m.rows(), m.columns());
    for (size_t i = 0UL; i < m.rows(); ++i)
    {
        blaze::row(expected, i) = blaze::row(m, i) + blaze::trans(v);
    }

    HPX_TEST_EQ(phylanx::ir::node_data<double>(std::move(expected)),
        phylanx::execution_tree::extract_numeric_value(f.get()));
}

void test_add_operation_2d()
{
    blaze::Rand<blaze::DynamicMatrix<double>> gen{};
    blaze::DynamicMatrix<double> m1 = gen.generate(101UL, 101UL);
    blaze::DynamicMatrix<double> m2 = gen.generate(101UL, 101UL);

    phylanx::execution_tree::primitive lhs =
        hpx::new_<phylanx::execution_tree::primitives::variable>(
            hpx::find_here(), phylanx::ir::node_data<double>(m1));

    phylanx::execution_tree::primitive rhs =
        hpx::new_<phylanx::execution_tree::primitives::variable>(
            hpx::find_here(), phylanx::ir::node_data<double>(m2));

    phylanx::execution_tree::primitive add =
        hpx::new_<phylanx::execution_tree::primitives::add_operation>(
            hpx::find_here(),
            std::vector<phylanx::execution_tree::primitive_argument_type>{
                std::move(lhs), std::move(rhs)});

    hpx::future<phylanx::execution_tree::primitive_result_type> f = add.eval();

    blaze::DynamicMatrix<double> expected = m1 + m2;
    HPX_TEST_EQ(phylanx::ir::node_data<double>(std::move(expected)),
        phylanx::execution_tree::extract_numeric_value(f.get()));
}

void test_add_operation_2d_lit()
{
    blaze::Rand<blaze::DynamicMatrix<double>> gen{};
    blaze::DynamicMatrix<double> m1 = gen.generate(101UL, 101UL);
    blaze::DynamicMatrix<double> m2 = gen.generate(101UL, 101UL);

    phylanx::ir::node_data<double> lhs(m1);

    phylanx::execution_tree::primitive rhs =
        hpx::new_<phylanx::execution_tree::primitives::variable>(
            hpx::find_here(), phylanx::ir::node_data<double>(m2));

    phylanx::execution_tree::primitive add =
        hpx::new_<phylanx::execution_tree::primitives::add_operation>(
            hpx::find_here(),
            std::vector<phylanx::execution_tree::primitive_argument_type>{
                std::move(lhs), std::move(rhs)});

    hpx::future<phylanx::execution_tree::primitive_result_type> f = add.eval();

    blaze::DynamicMatrix<double> expected = m1 + m2;
    HPX_TEST_EQ(phylanx::ir::node_data<double>(std::move(expected)),
        phylanx::execution_tree::extract_numeric_value(f.get()));
}

void test_add_operation_2d0d()
{
    blaze::Rand<blaze::DynamicMatrix<double>> gen{};
    blaze::DynamicMatrix<double> m = gen.generate(101UL, 101UL);

    phylanx::execution_tree::primitive lhs =
        hpx::new_<phylanx::execution_tree::primitives::variable>(
            hpx::find_here(), phylanx::ir::node_data<double>(m));

    phylanx::execution_tree::primitive rhs =
        hpx::new_<phylanx::execution_tree::primitives::variable>(
            hpx::find_here(), phylanx::ir::node_data<double>(41.0));

    phylanx::execution_tree::primitive add =
        hpx::new_<phylanx::execution_tree::primitives::add_operation>(
            hpx::find_here(),
            std::vector<phylanx::execution_tree::primitive_argument_type>{
                std::move(lhs), std::move(rhs)});

    hpx::future<phylanx::execution_tree::primitive_result_type> f = add.eval();

    blaze::DynamicMatrix<double> expected =
        blaze::map(m, [](double x) { return x + 41.0; });
    HPX_TEST_EQ(phylanx::ir::node_data<double>(std::move(expected)),
        phylanx::execution_tree::extract_numeric_value(f.get()));
}

void test_add_operation_2d0d_lit()
{
    blaze::Rand<blaze::DynamicMatrix<double>> gen{};
    blaze::DynamicMatrix<double> m = gen.generate(101UL, 101UL);

    phylanx::ir::node_data<double> lhs(m);

    phylanx::execution_tree::primitive rhs =
        hpx::new_<phylanx::execution_tree::primitives::variable>(
            hpx::find_here(), phylanx::ir::node_data<double>(41.0));

    phylanx::execution_tree::primitive add =
        hpx::new_<phylanx::execution_tree::primitives::add_operation>(
            hpx::find_here(),
            std::vector<phylanx::execution_tree::primitive_argument_type>{
                std::move(lhs), std::move(rhs)});

    hpx::future<phylanx::execution_tree::primitive_result_type> f = add.eval();

    blaze::DynamicMatrix<double> expected =
        blaze::map(m, [](double x) { return x + 41.0; });
    HPX_TEST_EQ(phylanx::ir::node_data<double>(std::move(expected)),
        phylanx::execution_tree::extract_numeric_value(f.get()));
}

void test_add_operation_2d1d()
{
    blaze::Rand<blaze::DynamicVector<double>> gen{};
    blaze::DynamicVector<double> v = gen.generate(104UL);

    blaze::Rand<blaze::DynamicMatrix<double>> mat_gen{};
    blaze::DynamicMatrix<double> m = mat_gen.generate(101UL, 104UL);

    phylanx::execution_tree::primitive lhs =
        hpx::new_<phylanx::execution_tree::primitives::variable>(
            hpx::find_here(), phylanx::ir::node_data<double>(m));

    phylanx::execution_tree::primitive rhs =
        hpx::new_<phylanx::execution_tree::primitives::variable>(
            hpx::find_here(), phylanx::ir::node_data<double>(v));

    phylanx::execution_tree::primitive add =
        hpx::new_<phylanx::execution_tree::primitives::add_operation>(
            hpx::find_here(),
            std::vector<phylanx::execution_tree::primitive_argument_type>{
                std::move(lhs), std::move(rhs)});

    hpx::future<phylanx::execution_tree::primitive_result_type> f = add.eval();
    blaze::DynamicMatrix<double> expected(m.rows(), m.columns());
    for (size_t i = 0UL; i < m.rows(); ++i)
    {
        blaze::row(expected, i) = blaze::row(m, i) + blaze::trans(v);
    }

    HPX_TEST_EQ(phylanx::ir::node_data<double>(std::move(expected)),
        phylanx::execution_tree::extract_numeric_value(f.get()));
}

void test_add_operation_2d1d_lit()
{
    blaze::Rand<blaze::DynamicVector<double>> gen{};
    blaze::DynamicVector<double> v = gen.generate(104UL);

    blaze::Rand<blaze::DynamicMatrix<double>> mat_gen{};
    blaze::DynamicMatrix<double> m = mat_gen.generate(101UL, 104UL);

    phylanx::ir::node_data<double> lhs(m);

    phylanx::execution_tree::primitive rhs =
        hpx::new_<phylanx::execution_tree::primitives::variable>(
            hpx::find_here(), phylanx::ir::node_data<double>(v));

    phylanx::execution_tree::primitive add =
        hpx::new_<phylanx::execution_tree::primitives::add_operation>(
            hpx::find_here(),
            std::vector<phylanx::execution_tree::primitive_argument_type>{
                std::move(lhs), std::move(rhs)});

    hpx::future<phylanx::execution_tree::primitive_result_type> f = add.eval();
    blaze::DynamicMatrix<double> expected(m.rows(), m.columns());
    for (size_t i = 0UL; i < m.rows(); ++i)
    {
        blaze::row(expected, i) = blaze::row(m, i) + blaze::trans(v);
    }

    HPX_TEST_EQ(phylanx::ir::node_data<double>(std::move(expected)),
        phylanx::execution_tree::extract_numeric_value(f.get()));
}

int main(int argc, char* argv[])
{
    test_add_operation_0d();
    test_add_operation_0d_lit();

    test_add_operation_0d1d();
    test_add_operation_0d1d_lit();

    test_add_operation_0d2d();
    test_add_operation_0d2d_lit();

    test_add_operation_1d();
    test_add_operation_1d_lit();

    test_add_operation_1d0d();
    test_add_operation_1d0d_lit();

    test_add_operation_1d2d();
    test_add_operation_1d2d_lit();

    test_add_operation_2d();
    test_add_operation_2d_lit();

    test_add_operation_2d0d();
    test_add_operation_2d0d_lit();

    test_add_operation_2d1d();
    test_add_operation_2d1d_lit();

    return hpx::util::report_errors();
}
