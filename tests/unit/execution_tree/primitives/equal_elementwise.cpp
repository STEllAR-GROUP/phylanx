//   Copyright (c) 2018 Hartmut Kaiser
//   Copyright (c) 2018 Shahrzad Shirzad
//
//   Distributed under the Boost Software License, Version 1.0. (See accompanying
//   file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/phylanx.hpp>

#include <hpx/hpx_main.hpp>
#include <hpx/include/lcos.hpp>
#include <hpx/util/lightweight_test.hpp>

#include <phylanx/execution_tree/primitives/equal_elementwise.hpp>
#include <iostream>
#include <utility>
#include <vector>

void test_equal_elementwise_0d_false()
{
    phylanx::execution_tree::primitive lhs =
        hpx::new_<phylanx::execution_tree::primitives::variable>(
            hpx::find_here(), phylanx::ir::node_data<double>(41.0));

    phylanx::execution_tree::primitive rhs =
        hpx::new_<phylanx::execution_tree::primitives::variable>(
            hpx::find_here(), phylanx::ir::node_data<double>(1.0));

    phylanx::execution_tree::primitive equal_elementwise;
    equal_elementwise =
        hpx::new_<phylanx::execution_tree::primitives::equal_elementwise>(
            hpx::find_here(),
            std::vector<phylanx::execution_tree::primitive_argument_type>{
                std::move(lhs), std::move(rhs)});

    phylanx::execution_tree::primitive_result_type f =
        equal_elementwise.eval().get();

    HPX_TEST_EQ(phylanx::ir::node_data<double>(0.0),
        phylanx::execution_tree::extract_numeric_value(f));
    HPX_TEST_EQ(phylanx::ir::node_data<bool>(false),
        phylanx::execution_tree::extract_boolean_data(f));
}

void test_equal_elementwise_0d_true()
{
    phylanx::execution_tree::primitive lhs =
        hpx::new_<phylanx::execution_tree::primitives::variable>(
            hpx::find_here(), phylanx::ir::node_data<double>(1.0));

    phylanx::execution_tree::primitive rhs =
        hpx::new_<phylanx::execution_tree::primitives::variable>(
            hpx::find_here(), phylanx::ir::node_data<double>(1.0));

    phylanx::execution_tree::primitive equal_elementwise =
        hpx::new_<phylanx::execution_tree::primitives::equal_elementwise>(
            hpx::find_here(),
            std::vector<phylanx::execution_tree::primitive_argument_type>{
                std::move(lhs), std::move(rhs)});

    phylanx::execution_tree::primitive_result_type f =
        equal_elementwise.eval().get();

    HPX_TEST_EQ(phylanx::ir::node_data<double>(1.0),
        phylanx::execution_tree::extract_numeric_value(f));
    HPX_TEST_EQ(phylanx::ir::node_data<bool>(true),
        phylanx::execution_tree::extract_boolean_data(f));
}

void test_equal_elementwise_0d_lit_false()
{
    phylanx::ir::node_data<double> lhs(41.0);

    phylanx::execution_tree::primitive rhs =
        hpx::new_<phylanx::execution_tree::primitives::variable>(
            hpx::find_here(), phylanx::ir::node_data<double>(1.0));

    phylanx::execution_tree::primitive equal_elementwise =
        hpx::new_<phylanx::execution_tree::primitives::equal_elementwise>(
            hpx::find_here(),
            std::vector<phylanx::execution_tree::primitive_argument_type>{
                std::move(lhs), std::move(rhs)});

    phylanx::execution_tree::primitive_result_type f =
        equal_elementwise.eval().get();

    HPX_TEST_EQ(phylanx::ir::node_data<double>(0.0),
        phylanx::execution_tree::extract_numeric_value(f));
    HPX_TEST_EQ(phylanx::ir::node_data<bool>(false),
        phylanx::execution_tree::extract_boolean_data(f));
}

void test_equal_elementwise_0d_lit_true()
{
    phylanx::ir::node_data<double> lhs(1.0);

    phylanx::execution_tree::primitive rhs =
        hpx::new_<phylanx::execution_tree::primitives::variable>(
            hpx::find_here(), phylanx::ir::node_data<double>(1.0));

    phylanx::execution_tree::primitive equal_elementwise =
        hpx::new_<phylanx::execution_tree::primitives::equal_elementwise>(
            hpx::find_here(),
            std::vector<phylanx::execution_tree::primitive_argument_type>{
                std::move(lhs), std::move(rhs)});

    phylanx::execution_tree::primitive_result_type f =
        equal_elementwise.eval().get();

    HPX_TEST_EQ(phylanx::ir::node_data<double>(1.0),
        phylanx::execution_tree::extract_numeric_value(f));
    HPX_TEST_EQ(phylanx::ir::node_data<bool>(std::move(true)),
        phylanx::execution_tree::extract_boolean_data(f));
}

void test_equal_elementwise_0d1d()
{
    blaze::Rand<blaze::DynamicVector<int>> gen{};
    blaze::DynamicVector<double> v = gen.generate(1007UL, 0, 2);

    phylanx::execution_tree::primitive lhs =
        hpx::new_<phylanx::execution_tree::primitives::variable>(
            hpx::find_here(), phylanx::ir::node_data<double>(2.0));

    phylanx::execution_tree::primitive rhs =
        hpx::new_<phylanx::execution_tree::primitives::variable>(
            hpx::find_here(), phylanx::ir::node_data<double>(v));

    phylanx::execution_tree::primitive equal_elementwise =
        hpx::new_<phylanx::execution_tree::primitives::equal_elementwise>(
            hpx::find_here(),
            std::vector<phylanx::execution_tree::primitive_argument_type>{
                std::move(lhs), std::move(rhs)});

    phylanx::execution_tree::primitive_result_type f =
        equal_elementwise.eval().get();

    blaze::DynamicVector<double> expected =
        blaze::map(v, [](double x) { return (x == 2.0); });

    HPX_TEST_EQ(phylanx::ir::node_data<double>(expected),
        phylanx::execution_tree::extract_numeric_value(f));
    HPX_TEST_EQ(phylanx::ir::node_data<bool>(std::move(expected)),
        phylanx::execution_tree::extract_boolean_data(f));
}

void test_equal_elementwise_0d1d_lit()
{
    blaze::Rand<blaze::DynamicVector<int>> gen{};
    blaze::DynamicVector<double> v = gen.generate(1007UL, 0, 2);

    phylanx::ir::node_data<double> lhs(2.0);

    phylanx::execution_tree::primitive rhs =
        hpx::new_<phylanx::execution_tree::primitives::variable>(
            hpx::find_here(), phylanx::ir::node_data<double>(v));

    phylanx::execution_tree::primitive equal_elementwise =
        hpx::new_<phylanx::execution_tree::primitives::equal_elementwise>(
            hpx::find_here(),
            std::vector<phylanx::execution_tree::primitive_argument_type>{
                std::move(lhs), std::move(rhs)});

    phylanx::execution_tree::primitive_result_type f =
        equal_elementwise.eval().get();

    blaze::DynamicVector<double> expected =
        blaze::map(v, [](double x) { return (x == 2.0); });

    HPX_TEST_EQ(phylanx::ir::node_data<double>(expected),
        phylanx::execution_tree::extract_numeric_value(f));
    HPX_TEST_EQ(phylanx::ir::node_data<bool>(std::move(expected)),
        phylanx::execution_tree::extract_boolean_data(f));
}

void test_equal_elementwise_0d2d()
{
    blaze::Rand<blaze::DynamicMatrix<int>> gen{};
    blaze::DynamicMatrix<double> m = gen.generate(101UL, 101UL, 0, 2);

    phylanx::execution_tree::primitive lhs =
        hpx::new_<phylanx::execution_tree::primitives::variable>(
            hpx::find_here(), phylanx::ir::node_data<double>(2.0));

    phylanx::execution_tree::primitive rhs =
        hpx::new_<phylanx::execution_tree::primitives::variable>(
            hpx::find_here(), phylanx::ir::node_data<double>(m));

    phylanx::execution_tree::primitive equal_elementwise =
        hpx::new_<phylanx::execution_tree::primitives::equal_elementwise>(
            hpx::find_here(),
            std::vector<phylanx::execution_tree::primitive_argument_type>{
                std::move(lhs), std::move(rhs)});

    phylanx::execution_tree::primitive_result_type f =
        equal_elementwise.eval().get();

    blaze::DynamicMatrix<double> expected =
        blaze::map(m, [](double x) { return (x == 2.0); });

    HPX_TEST_EQ(phylanx::ir::node_data<double>(expected),
        phylanx::execution_tree::extract_numeric_value(f));
    HPX_TEST_EQ(phylanx::ir::node_data<bool>(std::move(expected)),
        phylanx::execution_tree::extract_boolean_data(f));
}

void test_equal_elementwise_0d2d_lit()
{
    blaze::Rand<blaze::DynamicMatrix<int>> gen{};
    blaze::DynamicMatrix<double> m = gen.generate(101UL, 101UL, 0, 2);

    phylanx::ir::node_data<double> lhs(2.0);

    phylanx::execution_tree::primitive rhs =
        hpx::new_<phylanx::execution_tree::primitives::variable>(
            hpx::find_here(), phylanx::ir::node_data<double>(m));

    phylanx::execution_tree::primitive equal_elementwise =
        hpx::new_<phylanx::execution_tree::primitives::equal_elementwise>(
            hpx::find_here(),
            std::vector<phylanx::execution_tree::primitive_argument_type>{
                std::move(lhs), std::move(rhs)});

    phylanx::execution_tree::primitive_result_type f =
        equal_elementwise.eval().get();

    blaze::DynamicMatrix<double> expected =
        blaze::map(m, [](double x) { return (x == 2.0); });

    HPX_TEST_EQ(phylanx::ir::node_data<double>(expected),
        phylanx::execution_tree::extract_numeric_value(f));
    HPX_TEST_EQ(phylanx::ir::node_data<bool>(std::move(expected)),
        phylanx::execution_tree::extract_boolean_data(f));
}

void test_equal_elementwise_1d()
{
    blaze::Rand<blaze::DynamicVector<int>> gen{};
    blaze::DynamicVector<double> v1 = gen.generate(100UL, 0, 2);
    blaze::DynamicVector<double> v2 = gen.generate(100UL, 0, 2);

    phylanx::execution_tree::primitive lhs =
        hpx::new_<phylanx::execution_tree::primitives::variable>(
            hpx::find_here(), phylanx::ir::node_data<double>(v1));

    phylanx::execution_tree::primitive rhs =
        hpx::new_<phylanx::execution_tree::primitives::variable>(
            hpx::find_here(), phylanx::ir::node_data<double>(v2));

    phylanx::execution_tree::primitive equal_elementwise =
        hpx::new_<phylanx::execution_tree::primitives::equal_elementwise>(
            hpx::find_here(),
            std::vector<phylanx::execution_tree::primitive_argument_type>{
                std::move(lhs), std::move(rhs)});

    phylanx::execution_tree::primitive_result_type f =
        equal_elementwise.eval().get();

    blaze::DynamicVector<double> expected =
        blaze::map(v1, v2, [](double x, double y) { return x == y; });

    HPX_TEST_EQ(phylanx::ir::node_data<double>((expected)),
        phylanx::execution_tree::extract_numeric_value(f));
    HPX_TEST_EQ(phylanx::ir::node_data<bool>(std::move(expected)),
        phylanx::execution_tree::extract_boolean_data(f));
}

void test_equal_elementwise_1d_lit()
{
    blaze::Rand<blaze::DynamicVector<int>> gen{};
    blaze::DynamicVector<double> v1 = gen.generate(1007UL, 0, 2);
    blaze::DynamicVector<double> v2 = gen.generate(1007UL, 0, 2);

    phylanx::ir::node_data<double> lhs(v1);

    phylanx::execution_tree::primitive rhs =
        hpx::new_<phylanx::execution_tree::primitives::variable>(
            hpx::find_here(), phylanx::ir::node_data<double>(v2));

    phylanx::execution_tree::primitive equal_elementwise =
        hpx::new_<phylanx::execution_tree::primitives::equal_elementwise>(
            hpx::find_here(),
            std::vector<phylanx::execution_tree::primitive_argument_type>{
                std::move(lhs), std::move(rhs)});

    phylanx::execution_tree::primitive_result_type f =
        equal_elementwise.eval().get();

    blaze::DynamicVector<double> expected =
        blaze::map(v1, v2, [](double x, double y) { return x == y; });

    HPX_TEST_EQ(phylanx::ir::node_data<double>(expected),
        phylanx::execution_tree::extract_numeric_value(f));
    HPX_TEST_EQ(phylanx::ir::node_data<bool>(std::move(expected)),
        phylanx::execution_tree::extract_boolean_data(f));
}

void test_equal_elementwise_1d0d()
{
    blaze::Rand<blaze::DynamicVector<int>> gen{};
    blaze::DynamicVector<double> v = gen.generate(1007UL, 0, 2);

    phylanx::execution_tree::primitive lhs =
        hpx::new_<phylanx::execution_tree::primitives::variable>(
            hpx::find_here(), phylanx::ir::node_data<double>(v));

    phylanx::execution_tree::primitive rhs =
        hpx::new_<phylanx::execution_tree::primitives::variable>(
            hpx::find_here(), phylanx::ir::node_data<double>(2.0));

    phylanx::execution_tree::primitive equal_elementwise =
        hpx::new_<phylanx::execution_tree::primitives::equal_elementwise>(
            hpx::find_here(),
            std::vector<phylanx::execution_tree::primitive_argument_type>{
                std::move(lhs), std::move(rhs)});

    phylanx::execution_tree::primitive_result_type f =
        equal_elementwise.eval().get();

    blaze::DynamicVector<double> expected =
        blaze::map(v, [](double x) { return (x == 2.0); });

    HPX_TEST_EQ(phylanx::ir::node_data<double>(expected),
        phylanx::execution_tree::extract_numeric_value(f));
    HPX_TEST_EQ(phylanx::ir::node_data<bool>(std::move(expected)),
        phylanx::execution_tree::extract_boolean_data(f));
}

void test_equal_elementwise_1d0d_lit()
{
    blaze::Rand<blaze::DynamicVector<int>> gen{};
    blaze::DynamicVector<double> v = gen.generate(1007UL, 0, 2);

    phylanx::ir::node_data<double> lhs(v);

    phylanx::execution_tree::primitive rhs =
        hpx::new_<phylanx::execution_tree::primitives::variable>(
            hpx::find_here(), phylanx::ir::node_data<double>(2.0));

    phylanx::execution_tree::primitive equal_elementwise =
        hpx::new_<phylanx::execution_tree::primitives::equal_elementwise>(
            hpx::find_here(),
            std::vector<phylanx::execution_tree::primitive_argument_type>{
                std::move(lhs), std::move(rhs)});

    phylanx::execution_tree::primitive_result_type f =
        equal_elementwise.eval().get();

    blaze::DynamicVector<double> expected =
        blaze::map(v, [](double x) { return (x == 2.0); });

    HPX_TEST_EQ(phylanx::ir::node_data<double>(expected),
        phylanx::execution_tree::extract_numeric_value(f));
    HPX_TEST_EQ(phylanx::ir::node_data<bool>(std::move(expected)),
        phylanx::execution_tree::extract_boolean_data(f));
}

void test_equal_elementwise_1d2d()
{
    blaze::Rand<blaze::DynamicVector<int>> gen{};
    blaze::DynamicVector<double> v = gen.generate(104UL, 0, 2);

    blaze::Rand<blaze::DynamicMatrix<int>> mat_gen{};
    blaze::DynamicMatrix<double> m = mat_gen.generate(101UL, 104UL, 0, 2);

    phylanx::execution_tree::primitive lhs =
        hpx::new_<phylanx::execution_tree::primitives::variable>(
            hpx::find_here(), phylanx::ir::node_data<double>(v));

    phylanx::execution_tree::primitive rhs =
        hpx::new_<phylanx::execution_tree::primitives::variable>(
            hpx::find_here(), phylanx::ir::node_data<double>(m));

    phylanx::execution_tree::primitive equal_elementwise =
        hpx::new_<phylanx::execution_tree::primitives::equal_elementwise>(
            hpx::find_here(),
            std::vector<phylanx::execution_tree::primitive_argument_type>{
                std::move(lhs), std::move(rhs)});

    phylanx::execution_tree::primitive_result_type f =
        equal_elementwise.eval().get();

    blaze::DynamicMatrix<double> expected{m.rows(), m.columns()};

    for (size_t i = 0UL; i < m.rows(); i++)
        blaze::row(expected, i) = blaze::map(blaze::row(m, i),
            blaze::trans(v),
            [](double x, double y) { return x == y; });

    HPX_TEST_EQ(phylanx::ir::node_data<double>(expected),
        phylanx::execution_tree::extract_numeric_value(f));
    HPX_TEST_EQ(phylanx::ir::node_data<bool>(std::move(expected)),
        phylanx::execution_tree::extract_boolean_data(f));
}

void test_equal_elementwise_1d2d_lit()
{
    blaze::Rand<blaze::DynamicVector<int>> gen{};
    blaze::DynamicVector<double> v = gen.generate(104UL, 0, 2);

    blaze::Rand<blaze::DynamicMatrix<double>> mat_gen{};
    blaze::DynamicMatrix<double> m = mat_gen.generate(101UL, 104UL, 0, 2);

    phylanx::ir::node_data<double> lhs(v);

    phylanx::execution_tree::primitive rhs =
        hpx::new_<phylanx::execution_tree::primitives::variable>(
            hpx::find_here(), phylanx::ir::node_data<double>(m));

    phylanx::execution_tree::primitive equal_elementwise =
        hpx::new_<phylanx::execution_tree::primitives::equal_elementwise>(
            hpx::find_here(),
            std::vector<phylanx::execution_tree::primitive_argument_type>{
                std::move(lhs), std::move(rhs)});

    phylanx::execution_tree::primitive_result_type f =
        equal_elementwise.eval().get();

    blaze::DynamicMatrix<double> expected{m.rows(), m.columns()};

    for (size_t i = 0UL; i < m.rows(); i++)
        blaze::row(expected, i) = blaze::map(blaze::row(m, i),
            blaze::trans(v),
            [](double x, double y) { return x == y; });

    HPX_TEST_EQ(phylanx::ir::node_data<double>(expected),
        phylanx::execution_tree::extract_numeric_value(f));
    HPX_TEST_EQ(phylanx::ir::node_data<bool>(std::move(expected)),
        phylanx::execution_tree::extract_boolean_data(f));
}

void test_equal_elementwise_2d()
{
    blaze::Rand<blaze::DynamicMatrix<int>> gen{};
    blaze::DynamicMatrix<double> m1 = gen.generate(1007UL, 1007UL, 0, 2);
    blaze::DynamicMatrix<double> m2 = gen.generate(1007UL, 1007UL, 0, 2);

    phylanx::execution_tree::primitive lhs =
        hpx::new_<phylanx::execution_tree::primitives::variable>(
            hpx::find_here(), phylanx::ir::node_data<double>(m1));

    phylanx::execution_tree::primitive rhs =
        hpx::new_<phylanx::execution_tree::primitives::variable>(
            hpx::find_here(), phylanx::ir::node_data<double>(m2));

    phylanx::execution_tree::primitive equal_elementwise =
        hpx::new_<phylanx::execution_tree::primitives::equal_elementwise>(
            hpx::find_here(),
            std::vector<phylanx::execution_tree::primitive_argument_type>{
                std::move(lhs), std::move(rhs)});

    phylanx::execution_tree::primitive_result_type f =
        equal_elementwise.eval().get();

    blaze::DynamicMatrix<double> expected =
        blaze::map(m1, m2, [](double x, double y) { return x == y; });

    HPX_TEST_EQ(phylanx::ir::node_data<double>(expected),
        phylanx::execution_tree::extract_numeric_value(f));
    HPX_TEST_EQ(phylanx::ir::node_data<bool>(std::move(expected)),
        phylanx::execution_tree::extract_boolean_data(f));
}

void test_equal_elementwise_2d_lit()
{
    blaze::Rand<blaze::DynamicMatrix<int>> gen{};
    blaze::DynamicMatrix<double> m1 = gen.generate(1007UL, 1007UL, 0, 2);
    blaze::DynamicMatrix<double> m2 = gen.generate(1007UL, 1007UL, 0, 2);

    phylanx::ir::node_data<double> lhs(m1);

    phylanx::execution_tree::primitive rhs =
        hpx::new_<phylanx::execution_tree::primitives::variable>(
            hpx::find_here(), phylanx::ir::node_data<double>(m2));

    phylanx::execution_tree::primitive equal_elementwise =
        hpx::new_<phylanx::execution_tree::primitives::equal_elementwise>(
            hpx::find_here(),
            std::vector<phylanx::execution_tree::primitive_argument_type>{
                std::move(lhs), std::move(rhs)});

    phylanx::execution_tree::primitive_result_type f =
        equal_elementwise.eval().get();

    blaze::DynamicMatrix<double> expected =
        blaze::map(m1, m2, [](double x, double y) { return x == y; });

    HPX_TEST_EQ(phylanx::ir::node_data<double>(expected),
        phylanx::execution_tree::extract_numeric_value(f));
    HPX_TEST_EQ(phylanx::ir::node_data<bool>(std::move(expected)),
        phylanx::execution_tree::extract_boolean_data(f));
}

void test_equal_elementwise_2d0d()
{
    blaze::Rand<blaze::DynamicMatrix<int>> gen{};
    blaze::DynamicMatrix<double> m = gen.generate(101UL, 101UL, 0, 2);

    phylanx::execution_tree::primitive lhs =
        hpx::new_<phylanx::execution_tree::primitives::variable>(
            hpx::find_here(), phylanx::ir::node_data<double>(m));

    phylanx::execution_tree::primitive rhs =
        hpx::new_<phylanx::execution_tree::primitives::variable>(
            hpx::find_here(), phylanx::ir::node_data<double>(2.0));

    phylanx::execution_tree::primitive equal_elementwise =
        hpx::new_<phylanx::execution_tree::primitives::equal_elementwise>(
            hpx::find_here(),
            std::vector<phylanx::execution_tree::primitive_argument_type>{
                std::move(lhs), std::move(rhs)});

    phylanx::execution_tree::primitive_result_type f =
        equal_elementwise.eval().get();

    blaze::DynamicMatrix<double> expected =
        blaze::map(m, [](double x) { return (x == 2.0); });

    HPX_TEST_EQ(phylanx::ir::node_data<double>(expected),
        phylanx::execution_tree::extract_numeric_value(f));
    HPX_TEST_EQ(phylanx::ir::node_data<bool>(std::move(expected)),
        phylanx::execution_tree::extract_boolean_data(f));
}

void test_equal_elementwise_2d0d_lit()
{
    blaze::Rand<blaze::DynamicMatrix<int>> gen{};
    blaze::DynamicMatrix<double> m = gen.generate(101UL, 101UL, 0, 2);

    phylanx::ir::node_data<double> lhs(m);

    phylanx::execution_tree::primitive rhs =
        hpx::new_<phylanx::execution_tree::primitives::variable>(
            hpx::find_here(), phylanx::ir::node_data<double>(2.0));

    phylanx::execution_tree::primitive equal_elementwise =
        hpx::new_<phylanx::execution_tree::primitives::equal_elementwise>(
            hpx::find_here(),
            std::vector<phylanx::execution_tree::primitive_argument_type>{
                std::move(lhs), std::move(rhs)});

    phylanx::execution_tree::primitive_result_type f =
        equal_elementwise.eval().get();

    blaze::DynamicMatrix<double> expected =
        blaze::map(m, [](double x) { return (x == 2.0); });

    HPX_TEST_EQ(phylanx::ir::node_data<double>(expected),
        phylanx::execution_tree::extract_numeric_value(f));
    HPX_TEST_EQ(phylanx::ir::node_data<bool>(std::move(expected)),
        phylanx::execution_tree::extract_boolean_data(f));
}

void test_equal_elementwise_2d1d()
{
    blaze::Rand<blaze::DynamicVector<int>> gen{};
    blaze::DynamicVector<double> v = gen.generate(104UL, 0, 2);

    blaze::Rand<blaze::DynamicMatrix<double>> mat_gen{};
    blaze::DynamicMatrix<double> m = mat_gen.generate(101UL, 104UL, 0, 2);

    phylanx::execution_tree::primitive lhs =
        hpx::new_<phylanx::execution_tree::primitives::variable>(
            hpx::find_here(), phylanx::ir::node_data<double>(m));

    phylanx::execution_tree::primitive rhs =
        hpx::new_<phylanx::execution_tree::primitives::variable>(
            hpx::find_here(), phylanx::ir::node_data<double>(v));

    phylanx::execution_tree::primitive equal_elementwise =
        hpx::new_<phylanx::execution_tree::primitives::equal_elementwise>(
            hpx::find_here(),
            std::vector<phylanx::execution_tree::primitive_argument_type>{
                std::move(lhs), std::move(rhs)});

    phylanx::execution_tree::primitive_result_type f =
        equal_elementwise.eval().get();

    blaze::DynamicMatrix<double> expected{m.rows(), m.columns()};
    for (size_t i = 0UL; i < m.rows(); i++)
        blaze::row(expected, i) = blaze::map(blaze::row(m, i),
            blaze::trans(v),
            [](double x, double y) { return x == y; });

    HPX_TEST_EQ(phylanx::ir::node_data<double>(expected),
        phylanx::execution_tree::extract_numeric_value(f));
    HPX_TEST_EQ(phylanx::ir::node_data<bool>(std::move(expected)),
        phylanx::execution_tree::extract_boolean_data(f));
}

void test_equal_elementwise_2d1d_lit()
{
    blaze::Rand<blaze::DynamicVector<int>> gen{};
    blaze::DynamicVector<double> v = gen.generate(104UL, 0, 2);

    blaze::Rand<blaze::DynamicMatrix<double>> mat_gen{};
    blaze::DynamicMatrix<double> m = mat_gen.generate(101UL, 104UL, 0, 2);

    phylanx::ir::node_data<double> lhs(m);

    phylanx::execution_tree::primitive rhs =
        hpx::new_<phylanx::execution_tree::primitives::variable>(
            hpx::find_here(), phylanx::ir::node_data<double>(v));

    phylanx::execution_tree::primitive equal_elementwise =
        hpx::new_<phylanx::execution_tree::primitives::equal_elementwise>(
            hpx::find_here(),
            std::vector<phylanx::execution_tree::primitive_argument_type>{
                std::move(lhs), std::move(rhs)});

    phylanx::execution_tree::primitive_result_type f =
        equal_elementwise.eval().get();

    blaze::DynamicMatrix<double> expected{m.rows(), m.columns()};
    for (size_t i = 0UL; i < m.rows(); i++)
        blaze::row(expected, i) = blaze::map(blaze::row(m, i),
            blaze::trans(v),
            [](double x, double y) { return x == y; });

    HPX_TEST_EQ(phylanx::ir::node_data<double>(expected),
        phylanx::execution_tree::extract_numeric_value(f));
    HPX_TEST_EQ(phylanx::ir::node_data<bool>(std::move(expected)),
        phylanx::execution_tree::extract_boolean_data(f));
}

int main(int argc, char* argv[])
{
    test_equal_elementwise_0d_false();
    test_equal_elementwise_0d_true();
    test_equal_elementwise_0d_lit_false();
    test_equal_elementwise_0d_lit_true();
    test_equal_elementwise_0d1d();
    test_equal_elementwise_0d1d_lit();
    test_equal_elementwise_0d2d();
    test_equal_elementwise_0d2d_lit();

    test_equal_elementwise_1d();
    test_equal_elementwise_1d_lit();
    test_equal_elementwise_1d0d();
    test_equal_elementwise_1d0d_lit();
    test_equal_elementwise_1d2d();
    test_equal_elementwise_1d2d_lit();

    test_equal_elementwise_2d();
    test_equal_elementwise_2d_lit();
    test_equal_elementwise_2d0d();
    test_equal_elementwise_2d0d_lit();
    test_equal_elementwise_2d1d();
    test_equal_elementwise_2d1d_lit();

    return hpx::util::report_errors();
}