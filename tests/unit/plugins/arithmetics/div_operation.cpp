//   Copyright (c) 2017 Hartmut Kaiser
//   Copyright (c) 2017 Alireza Kheirkhahan
//
//   Distributed under the Boost Software License, Version 1.0. (See accompanying
//   file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)


#include <phylanx/phylanx.hpp>

#include <hpx/hpx_main.hpp>
#include <hpx/include/lcos.hpp>
#include <hpx/testing.hpp>

#include <iostream>
#include <utility>
#include <vector>
#include <blaze/Math.h>

void test_div_operation_0d()
{
    phylanx::execution_tree::primitive lhs =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(42.0));

    phylanx::execution_tree::primitive rhs =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(7.0));

    phylanx::execution_tree::primitive div =
        phylanx::execution_tree::primitives::create_div_operation(
            hpx::find_here(),
            phylanx::execution_tree::primitive_arguments_type{
                std::move(lhs), std::move(rhs)});

    hpx::future<phylanx::execution_tree::primitive_argument_type> f = div.eval();
    HPX_TEST_EQ(
        6.0, phylanx::execution_tree::extract_numeric_value(f.get())[0]);
}

void test_div_operation_0d_lit()
{
    phylanx::ir::node_data<double> lhs(42.0);

    phylanx::execution_tree::primitive rhs =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(7.0));

    phylanx::execution_tree::primitive div =
        phylanx::execution_tree::primitives::create_div_operation(
            hpx::find_here(),
            phylanx::execution_tree::primitive_arguments_type{
                std::move(lhs), std::move(rhs)});

    hpx::future<phylanx::execution_tree::primitive_argument_type> f = div.eval();
    HPX_TEST_EQ(
        6.0, phylanx::execution_tree::extract_numeric_value(f.get())[0]);
}

void test_div_operation_0d1d()
{
    blaze::Rand<blaze::DynamicVector<double>> gen{};
    blaze::DynamicVector<double> v = gen.generate(1007UL);

    phylanx::execution_tree::primitive lhs =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(42.0));

    phylanx::execution_tree::primitive rhs =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(v));

    phylanx::execution_tree::primitive div =
        phylanx::execution_tree::primitives::create_div_operation(
            hpx::find_here(),
            phylanx::execution_tree::primitive_arguments_type{
                std::move(lhs), std::move(rhs)});

    hpx::future<phylanx::execution_tree::primitive_argument_type> f = div.eval();

    blaze::DynamicVector<double> expected =
        blaze::map(v, [](double x) { return 42.0 / x; });

    HPX_TEST_EQ(phylanx::ir::node_data<double>(std::move(expected)),
        phylanx::execution_tree::extract_numeric_value(f.get()));
}

void test_div_operation_0d1d_lit()
{
    blaze::Rand<blaze::DynamicVector<double>> gen{};
    blaze::DynamicVector<double> v = gen.generate(1007UL);

    phylanx::ir::node_data<double> lhs(42.0);

    phylanx::execution_tree::primitive rhs =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(v));

    phylanx::execution_tree::primitive div =
        phylanx::execution_tree::primitives::create_div_operation(
            hpx::find_here(),
            phylanx::execution_tree::primitive_arguments_type{
                std::move(lhs), std::move(rhs)});

    hpx::future<phylanx::execution_tree::primitive_argument_type> f = div.eval();

    blaze::DynamicVector<double> expected =
        blaze::map(v, [](double x) { return 42.0 / x; });
    HPX_TEST_EQ(phylanx::ir::node_data<double>(std::move(expected)),
        phylanx::execution_tree::extract_numeric_value(f.get()));
}

void test_div_operation_0d2d()
{
    blaze::Rand<blaze::DynamicMatrix<double>> gen{};
    blaze::DynamicMatrix<double> m =
        gen.generate(101UL, 101UL);

    phylanx::execution_tree::primitive lhs =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(6.0));

    phylanx::execution_tree::primitive rhs =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(m));

    phylanx::execution_tree::primitive div =
        phylanx::execution_tree::primitives::create_div_operation(
            hpx::find_here(),
            phylanx::execution_tree::primitive_arguments_type{
                std::move(lhs), std::move(rhs)});

    hpx::future<phylanx::execution_tree::primitive_argument_type> f = div.eval();

    blaze::DynamicMatrix<double> expected =
        blaze::map(m, [](double x) { return 6.0 / x; });
    HPX_TEST_EQ(phylanx::ir::node_data<double>(std::move(expected)),
        phylanx::execution_tree::extract_numeric_value(f.get()));
}

void test_div_operation_0d2d_lit()
{
    blaze::Rand<blaze::DynamicMatrix<double>> gen{};
    blaze::DynamicMatrix<double> m = gen.generate(101UL, 101UL);

    phylanx::ir::node_data<double> lhs(6.0);

    phylanx::execution_tree::primitive rhs =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(m));

    phylanx::execution_tree::primitive div =
        phylanx::execution_tree::primitives::create_div_operation(
            hpx::find_here(),
            phylanx::execution_tree::primitive_arguments_type{
                std::move(lhs), std::move(rhs)});

    hpx::future<phylanx::execution_tree::primitive_argument_type> f = div.eval();

    blaze::DynamicMatrix<double> expected =
        blaze::map(m, [](double x) { return 6.0 / x; });
    HPX_TEST_EQ(phylanx::ir::node_data<double>(std::move(expected)),
        phylanx::execution_tree::extract_numeric_value(f.get()));
}

void test_div_operation_1d0d()
{
    blaze::Rand<blaze::DynamicVector<double>> gen{};
    blaze::DynamicVector<double> v = gen.generate(1007UL);

    phylanx::execution_tree::primitive lhs =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(v));

    phylanx::execution_tree::primitive rhs =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(6.0));

    phylanx::execution_tree::primitive div =
        phylanx::execution_tree::primitives::create_div_operation(
            hpx::find_here(),
            phylanx::execution_tree::primitive_arguments_type{
                std::move(lhs), std::move(rhs)});

    hpx::future<phylanx::execution_tree::primitive_argument_type> f = div.eval();

    blaze::DynamicVector<double> expected =
        blaze::map(v, [](double x) { return x / 6.0; });
    HPX_TEST_EQ(phylanx::ir::node_data<double>(std::move(expected)),
        phylanx::execution_tree::extract_numeric_value(f.get()));
}

void test_div_operation_1d0d_lit()
{
    blaze::Rand<blaze::DynamicVector<double>> gen{};
    blaze::DynamicVector<double> v = gen.generate(1007UL);

    phylanx::ir::node_data<double> lhs(v);

    phylanx::execution_tree::primitive rhs =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(6.0));

    phylanx::execution_tree::primitive div =
        phylanx::execution_tree::primitives::create_div_operation(
            hpx::find_here(),
            phylanx::execution_tree::primitive_arguments_type{
                std::move(lhs), std::move(rhs)});

    hpx::future<phylanx::execution_tree::primitive_argument_type> f = div.eval();

    blaze::DynamicVector<double> expected =
        blaze::map(v, [](double x) { return x / 6.0; });
    HPX_TEST_EQ(phylanx::ir::node_data<double>(std::move(expected)),
        phylanx::execution_tree::extract_numeric_value(f.get()));
}

void test_div_operation_1d()
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

    phylanx::execution_tree::primitive div =
        phylanx::execution_tree::primitives::create_div_operation(
            hpx::find_here(),
            phylanx::execution_tree::primitive_arguments_type{
                std::move(lhs), std::move(rhs)});

    hpx::future<phylanx::execution_tree::primitive_argument_type> f = div.eval();

    blaze::DynamicVector<double> expected =
        blaze::map(v1, v2, [](double x, double y) { return x / y; });
    HPX_TEST_EQ(phylanx::ir::node_data<double>(std::move(expected)),
        phylanx::execution_tree::extract_numeric_value(f.get()));
}

void test_div_operation_1d_lit()
{
    blaze::Rand<blaze::DynamicVector<double>> gen{};
    blaze::DynamicVector<double> v1 = gen.generate(1007UL);
    blaze::DynamicVector<double> v2 = gen.generate(1007UL);

    phylanx::ir::node_data<double> lhs(v1);

    phylanx::execution_tree::primitive rhs =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(v2));

    phylanx::execution_tree::primitive div =
        phylanx::execution_tree::primitives::create_div_operation(
            hpx::find_here(),
            phylanx::execution_tree::primitive_arguments_type{
                std::move(lhs), std::move(rhs)});

    hpx::future<phylanx::execution_tree::primitive_argument_type> f = div.eval();

    blaze::DynamicVector<double> expected =
        blaze::map(v1, v2, [](double x, double y) { return x / y; });
    HPX_TEST_EQ(phylanx::ir::node_data<double>(std::move(expected)),
        phylanx::execution_tree::extract_numeric_value(f.get()));
}

void test_div_operation_1d2d()
{
    blaze::Rand<blaze::DynamicVector<double>> gen{};
    blaze::DynamicVector<double> v = gen.generate(104UL);

    blaze::Rand<blaze::DynamicMatrix<double>> mat_gen{};
    blaze::DynamicMatrix<double> m = mat_gen.generate(101UL, 104UL);

    phylanx::execution_tree::primitive lhs =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(v));

    phylanx::execution_tree::primitive rhs =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(m));

    phylanx::execution_tree::primitive div =
        phylanx::execution_tree::primitives::create_div_operation(
            hpx::find_here(),
            phylanx::execution_tree::primitive_arguments_type{
                std::move(lhs), std::move(rhs)});

    hpx::future<phylanx::execution_tree::primitive_argument_type> f = div.eval();
    blaze::DynamicMatrix<double> expected(m.rows(), m.columns());
    for (size_t i = 0UL; i < m.rows(); ++i)
    {
        blaze::row(expected, i) = blaze::trans(v) / blaze::row(m, i);
    }

    HPX_TEST_EQ(phylanx::ir::node_data<double>(std::move(expected)),
        phylanx::execution_tree::extract_numeric_value(f.get()));
}

void test_div_operation_1d2d_lit()
{
    blaze::Rand<blaze::DynamicVector<double>> gen{};
    blaze::DynamicVector<double> v = gen.generate(104UL);

    blaze::Rand<blaze::DynamicMatrix<double>> mat_gen{};
    blaze::DynamicMatrix<double> m = mat_gen.generate(101UL, 104UL);

    phylanx::ir::node_data<double> lhs(v);

    phylanx::execution_tree::primitive rhs =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(m));

    phylanx::execution_tree::primitive div =
        phylanx::execution_tree::primitives::create_div_operation(
            hpx::find_here(),
            phylanx::execution_tree::primitive_arguments_type{
                std::move(lhs), std::move(rhs)});

    hpx::future<phylanx::execution_tree::primitive_argument_type> f = div.eval();
    blaze::DynamicMatrix<double> expected(m.rows(), m.columns());
    for (size_t i = 0UL; i < m.rows(); ++i)
    {
        blaze::row(expected, i) = blaze::trans(v) / blaze::row(m, i);
    }

    HPX_TEST_EQ(phylanx::ir::node_data<double>(std::move(expected)),
        phylanx::execution_tree::extract_numeric_value(f.get()));
}

void test_div_operation_2d0d()
{
    blaze::Rand<blaze::DynamicMatrix<double>> gen{};
    blaze::DynamicMatrix<double> m = gen.generate(42UL, 42UL);

    phylanx::execution_tree::primitive lhs =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(m));

    phylanx::execution_tree::primitive rhs =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(6.0));

    phylanx::execution_tree::primitive div =
        phylanx::execution_tree::primitives::create_div_operation(
            hpx::find_here(),
            phylanx::execution_tree::primitive_arguments_type{
                std::move(lhs), std::move(rhs)});

    hpx::future<phylanx::execution_tree::primitive_argument_type> f = div.eval();

    blaze::DynamicMatrix<double> expected =
        blaze::map(m, [](double x) { return x / 6.0; });
    HPX_TEST_EQ(phylanx::ir::node_data<double>(std::move(expected)),
        phylanx::execution_tree::extract_numeric_value(f.get()));
}

void test_div_operation_2d0d_lit()
{
    blaze::Rand<blaze::DynamicMatrix<double>> gen{};
    blaze::DynamicMatrix<double> m = gen.generate(42UL, 42UL);

    phylanx::ir::node_data<double> lhs(m);

    phylanx::execution_tree::primitive rhs =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(6.0));

    phylanx::execution_tree::primitive div =
        phylanx::execution_tree::primitives::create_div_operation(
            hpx::find_here(),
            phylanx::execution_tree::primitive_arguments_type{
                std::move(lhs), std::move(rhs)});

    hpx::future<phylanx::execution_tree::primitive_argument_type> f = div.eval();

    blaze::DynamicMatrix<double> expected =
        blaze::map(m, [](double x) { return x / 6.0; });
    HPX_TEST_EQ(phylanx::ir::node_data<double>(std::move(expected)),
        phylanx::execution_tree::extract_numeric_value(f.get()));
}

void test_div_operation_2d()
{
    blaze::Rand<blaze::DynamicMatrix<double>> gen{};
    blaze::DynamicMatrix<double> m1 = gen.generate(42UL, 42UL);
    blaze::DynamicMatrix<double> m2 = gen.generate(42UL, 42UL);

    phylanx::execution_tree::primitive lhs =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(m1));

    phylanx::execution_tree::primitive rhs =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(m2));

    phylanx::execution_tree::primitive div =
        phylanx::execution_tree::primitives::create_div_operation(
            hpx::find_here(),
            phylanx::execution_tree::primitive_arguments_type{
                std::move(lhs), std::move(rhs)});

    hpx::future<phylanx::execution_tree::primitive_argument_type> f = div.eval();

    blaze::DynamicMatrix<double> expected =
        blaze::map(m1, m2, [](double x1, double x2) { return x1 / x2; });
    HPX_TEST_EQ(phylanx::ir::node_data<double>(std::move(expected)),
        phylanx::execution_tree::extract_numeric_value(f.get()));
}

void test_div_operation_2d_lit()
{
    blaze::Rand<blaze::DynamicMatrix<double>> gen{};
    blaze::DynamicMatrix<double> m1 = gen.generate(42UL, 42UL);
    blaze::DynamicMatrix<double> m2 = gen.generate(42UL, 42UL);

    phylanx::ir::node_data<double> lhs(m1);

    phylanx::execution_tree::primitive rhs =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(m2));

    phylanx::execution_tree::primitive div =
        phylanx::execution_tree::primitives::create_div_operation(
            hpx::find_here(),
            phylanx::execution_tree::primitive_arguments_type{
                std::move(lhs), std::move(rhs)});

    hpx::future<phylanx::execution_tree::primitive_argument_type> f = div.eval();

    blaze::DynamicMatrix<double> expected =
        blaze::map(m1, m2, [](double x1, double x2) { return x1 / x2; });
    HPX_TEST_EQ(phylanx::ir::node_data<double>(std::move(expected)),
        phylanx::execution_tree::extract_numeric_value(f.get()));
}

void test_div_operation_2d1d()
{
    blaze::Rand<blaze::DynamicVector<double>> gen{};
    blaze::DynamicVector<double> v = gen.generate(104UL);

    blaze::Rand<blaze::DynamicMatrix<double>> mat_gen{};
    blaze::DynamicMatrix<double> m = mat_gen.generate(101UL, 104UL);

    phylanx::execution_tree::primitive lhs =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(m));

    phylanx::execution_tree::primitive rhs =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(v));

    phylanx::execution_tree::primitive div =
        phylanx::execution_tree::primitives::create_div_operation(
            hpx::find_here(),
            phylanx::execution_tree::primitive_arguments_type{
                std::move(lhs), std::move(rhs)});

    hpx::future<phylanx::execution_tree::primitive_argument_type> f = div.eval();
    blaze::DynamicMatrix<double> expected(m.rows(), m.columns());
    for (size_t i = 0UL; i < m.rows(); ++i)
    {
        blaze::row(expected, i) = blaze::row(m, i) / blaze::trans(v);
    }

    HPX_TEST_EQ(phylanx::ir::node_data<double>(std::move(expected)),
        phylanx::execution_tree::extract_numeric_value(f.get()));
}

void test_div_operation_2d1d_lit()
{
    blaze::Rand<blaze::DynamicVector<double>> gen{};
    blaze::DynamicVector<double> v = gen.generate(104UL);

    blaze::Rand<blaze::DynamicMatrix<double>> mat_gen{};
    blaze::DynamicMatrix<double> m = mat_gen.generate(101UL, 104UL);

    phylanx::ir::node_data<double> lhs(m);

    phylanx::execution_tree::primitive rhs =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(v));

    phylanx::execution_tree::primitive div =
        phylanx::execution_tree::primitives::create_div_operation(
            hpx::find_here(),
            phylanx::execution_tree::primitive_arguments_type{
                std::move(lhs), std::move(rhs)});

    hpx::future<phylanx::execution_tree::primitive_argument_type> f = div.eval();
    blaze::DynamicMatrix<double> expected(m.rows(), m.columns());
    for (size_t i = 0UL; i < m.rows(); ++i)
    {
        blaze::row(expected, i) = blaze::row(m, i) / blaze::trans(v);
    }

    HPX_TEST_EQ(phylanx::ir::node_data<double>(std::move(expected)),
        phylanx::execution_tree::extract_numeric_value(f.get()));
}

int main(int argc, char* argv[])
{
    test_div_operation_0d();
    test_div_operation_0d_lit();

    test_div_operation_0d1d();
    test_div_operation_0d1d_lit();

    test_div_operation_0d2d();
    test_div_operation_0d2d_lit();

    test_div_operation_1d0d();
    test_div_operation_1d0d_lit();

    test_div_operation_1d();
    test_div_operation_1d_lit();

    test_div_operation_1d2d();
    test_div_operation_1d2d_lit();

    test_div_operation_2d0d();
    test_div_operation_2d0d_lit();

    test_div_operation_2d1d();
    test_div_operation_2d1d_lit();

    test_div_operation_2d();
    test_div_operation_2d_lit();

    return hpx::util::report_errors();
}
