//   Copyright (c) 2017-2019 Hartmut Kaiser
//
//   Distributed under the Boost Software License, Version 1.0. (See accompanying
//   file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/phylanx.hpp>

#include <hpx/hpx_main.hpp>
#include <hpx/include/lcos.hpp>
#include <hpx/modules/testing.hpp>

#include <cstdint>
#include <iostream>
#include <string>
#include <utility>
#include <vector>

void test_greater_equal_operation_0d_true()
{
    phylanx::execution_tree::primitive lhs =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(41.0));

    phylanx::execution_tree::primitive rhs =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(41.0));

    phylanx::execution_tree::primitive greater_equal;
    greater_equal =
        phylanx::execution_tree::primitives::create_greater_equal(
            hpx::find_here(),
            phylanx::execution_tree::primitive_arguments_type{
                std::move(lhs), std::move(rhs)});

    phylanx::execution_tree::primitive_argument_type f =
        greater_equal.eval().get();

    HPX_TEST_EQ(phylanx::ir::node_data<double>(1.0),
        phylanx::execution_tree::extract_numeric_value(f));
    HPX_TEST_EQ(phylanx::ir::node_data<std::uint8_t>(true),
        phylanx::execution_tree::extract_boolean_value_strict(f));
}

void test_greater_equal_operation_0d_false()
{
    phylanx::execution_tree::primitive lhs =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(1.0));

    phylanx::execution_tree::primitive rhs =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(41.0));

    phylanx::execution_tree::primitive greater_equal =
        phylanx::execution_tree::primitives::create_greater_equal(
            hpx::find_here(),
            phylanx::execution_tree::primitive_arguments_type{
                std::move(lhs), std::move(rhs)});

    phylanx::execution_tree::primitive_argument_type f =
        greater_equal.eval().get();

    HPX_TEST_EQ(phylanx::ir::node_data<double>(0.0),
        phylanx::execution_tree::extract_numeric_value(f));
    HPX_TEST_EQ(phylanx::ir::node_data<std::uint8_t>(false),
        phylanx::execution_tree::extract_boolean_value_strict(f));
}

void test_greater_equal_operation_0d_lit_true()
{
    phylanx::ir::node_data<double> lhs(41.0);

    phylanx::execution_tree::primitive rhs =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(1.0));

    phylanx::execution_tree::primitive greater_equal =
        phylanx::execution_tree::primitives::create_greater_equal(
            hpx::find_here(),
            phylanx::execution_tree::primitive_arguments_type{
                std::move(lhs), std::move(rhs)});

    phylanx::execution_tree::primitive_argument_type f =
        greater_equal.eval().get();

    HPX_TEST_EQ(phylanx::ir::node_data<double>(1.0),
        phylanx::execution_tree::extract_numeric_value(f));
    HPX_TEST_EQ(phylanx::ir::node_data<std::uint8_t>(true),
        phylanx::execution_tree::extract_boolean_value_strict(f));
}

void test_greater_equal_operation_0d_lit_false()
{
    phylanx::ir::node_data<double> lhs(1.0);

    phylanx::execution_tree::primitive rhs =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(41.0));

    phylanx::execution_tree::primitive greater_equal =
        phylanx::execution_tree::primitives::create_greater_equal(
            hpx::find_here(),
            phylanx::execution_tree::primitive_arguments_type{
                std::move(lhs), std::move(rhs)});

    phylanx::execution_tree::primitive_argument_type f =
        greater_equal.eval().get();

    HPX_TEST_EQ(phylanx::ir::node_data<double>(0.0),
        phylanx::execution_tree::extract_numeric_value(f));
    HPX_TEST_EQ(phylanx::ir::node_data<std::uint8_t>(std::move(false)),
        phylanx::execution_tree::extract_boolean_value_strict(f));
}

void test_greater_equal_operation_0d1d()
{
    blaze::Rand<blaze::DynamicVector<int>> gen{};
    blaze::DynamicVector<double> v = gen.generate(1007UL, 0, 2);

    phylanx::execution_tree::primitive lhs =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(1.0));

    phylanx::execution_tree::primitive rhs =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(v));

    phylanx::execution_tree::primitive greater_equal =
        phylanx::execution_tree::primitives::create_greater_equal(
            hpx::find_here(),
            phylanx::execution_tree::primitive_arguments_type{
                std::move(lhs), std::move(rhs)});

    phylanx::execution_tree::primitive_argument_type f =
        greater_equal.eval().get();

    blaze::DynamicVector<double> expected =
        blaze::map(v, [](double x) { return (1.0 >= x); });

    HPX_TEST_EQ(phylanx::ir::node_data<double>(expected),
        phylanx::execution_tree::extract_numeric_value(f));
    HPX_TEST_EQ(phylanx::ir::node_data<std::uint8_t>(std::move(expected)),
        phylanx::execution_tree::extract_boolean_value_strict(f));
}

void test_greater_equal_operation_0d1d_lit()
{
    blaze::Rand<blaze::DynamicVector<int>> gen{};
    blaze::DynamicVector<double> v = gen.generate(1007UL, 0, 2);

    phylanx::ir::node_data<double> lhs(1.0);

    phylanx::execution_tree::primitive rhs =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(v));

    phylanx::execution_tree::primitive greater_equal =
        phylanx::execution_tree::primitives::create_greater_equal(
            hpx::find_here(),
            phylanx::execution_tree::primitive_arguments_type{
                std::move(lhs), std::move(rhs)});

    phylanx::execution_tree::primitive_argument_type f =
        greater_equal.eval().get();

    blaze::DynamicVector<double> expected =
        blaze::map(v, [](double x) { return (1.0 >= x); });

    HPX_TEST_EQ(phylanx::ir::node_data<double>(expected),
        phylanx::execution_tree::extract_numeric_value(f));
    HPX_TEST_EQ(phylanx::ir::node_data<std::uint8_t>(std::move(expected)),
        phylanx::execution_tree::extract_boolean_value_strict(f));
}

void test_greater_equal_operation_0d2d()
{
    blaze::Rand<blaze::DynamicMatrix<int>> gen{};
    blaze::DynamicMatrix<double> m = gen.generate(101UL, 101UL, 0, 2);

    phylanx::execution_tree::primitive lhs =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(1.0));

    phylanx::execution_tree::primitive rhs =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(m));

    phylanx::execution_tree::primitive greater_equal =
        phylanx::execution_tree::primitives::create_greater_equal(
            hpx::find_here(),
            phylanx::execution_tree::primitive_arguments_type{
                std::move(lhs), std::move(rhs)});

    phylanx::execution_tree::primitive_argument_type f =
        greater_equal.eval().get();

    blaze::DynamicMatrix<double> expected =
        blaze::map(m, [](double x) { return (1.0 >= x); });

    HPX_TEST_EQ(phylanx::ir::node_data<double>(expected),
        phylanx::execution_tree::extract_numeric_value(f));
    HPX_TEST_EQ(phylanx::ir::node_data<std::uint8_t>(std::move(expected)),
        phylanx::execution_tree::extract_boolean_value_strict(f));
}

void test_greater_equal_operation_0d2d_lit()
{
    blaze::Rand<blaze::DynamicMatrix<int>> gen{};
    blaze::DynamicMatrix<double> m = gen.generate(101UL, 101UL, 0, 2);

    phylanx::ir::node_data<double> lhs(1.0);

    phylanx::execution_tree::primitive rhs =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(m));

    phylanx::execution_tree::primitive greater_equal =
        phylanx::execution_tree::primitives::create_greater_equal(
            hpx::find_here(),
            phylanx::execution_tree::primitive_arguments_type{
                std::move(lhs), std::move(rhs)});

    phylanx::execution_tree::primitive_argument_type f =
        greater_equal.eval().get();

    blaze::DynamicMatrix<double> expected =
        blaze::map(m, [](double x) { return (1.0 >= x); });

    HPX_TEST_EQ(phylanx::ir::node_data<double>(expected),
        phylanx::execution_tree::extract_numeric_value(f));
    HPX_TEST_EQ(phylanx::ir::node_data<std::uint8_t>(std::move(expected)),
        phylanx::execution_tree::extract_boolean_value_strict(f));
}

void test_greater_equal_operation_1d()
{
    blaze::Rand<blaze::DynamicVector<int>> gen{};
    blaze::DynamicVector<double> v1 = gen.generate(100UL, 0, 2);
    blaze::DynamicVector<double> v2 = gen.generate(100UL, 0, 2);

    phylanx::execution_tree::primitive lhs =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(v1));

    phylanx::execution_tree::primitive rhs =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(v2));

    phylanx::execution_tree::primitive greater_equal =
        phylanx::execution_tree::primitives::create_greater_equal(
            hpx::find_here(),
            phylanx::execution_tree::primitive_arguments_type{
                std::move(lhs), std::move(rhs)});

    phylanx::execution_tree::primitive_argument_type f =
        greater_equal.eval().get();

    blaze::DynamicVector<double> expected =
        blaze::map(v1, v2, [](double x, double y) { return x >= y; });

    HPX_TEST_EQ(phylanx::ir::node_data<double>((expected)),
        phylanx::execution_tree::extract_numeric_value(f));
    HPX_TEST_EQ(phylanx::ir::node_data<std::uint8_t>(std::move(expected)),
        phylanx::execution_tree::extract_boolean_value_strict(f));
}

void test_greater_equal_operation_1d_lit()
{
    blaze::Rand<blaze::DynamicVector<int>> gen{};
    blaze::DynamicVector<double> v1 = gen.generate(1007UL, 0, 2);
    blaze::DynamicVector<double> v2 = gen.generate(1007UL, 0, 2);

    phylanx::ir::node_data<double> lhs(v1);

    phylanx::execution_tree::primitive rhs =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(v2));

    phylanx::execution_tree::primitive greater_equal =
        phylanx::execution_tree::primitives::create_greater_equal(
            hpx::find_here(),
            phylanx::execution_tree::primitive_arguments_type{
                std::move(lhs), std::move(rhs)});

    phylanx::execution_tree::primitive_argument_type f =
        greater_equal.eval().get();

    blaze::DynamicVector<double> expected =
        blaze::map(v1, v2, [](double x, double y) { return x >= y; });

    HPX_TEST_EQ(phylanx::ir::node_data<double>(expected),
        phylanx::execution_tree::extract_numeric_value(f));
    HPX_TEST_EQ(phylanx::ir::node_data<std::uint8_t>(std::move(expected)),
        phylanx::execution_tree::extract_boolean_value_strict(f));
}

void test_greater_equal_operation_1d0d()
{
    blaze::Rand<blaze::DynamicVector<int>> gen{};
    blaze::DynamicVector<double> v = gen.generate(1007UL, 0, 2);

    phylanx::execution_tree::primitive lhs =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(v));

    phylanx::execution_tree::primitive rhs =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(1.0));

    phylanx::execution_tree::primitive greater_equal =
        phylanx::execution_tree::primitives::create_greater_equal(
            hpx::find_here(),
            phylanx::execution_tree::primitive_arguments_type{
                std::move(lhs), std::move(rhs)});

    phylanx::execution_tree::primitive_argument_type f =
        greater_equal.eval().get();

    blaze::DynamicVector<double> expected =
        blaze::map(v, [](double x) { return (x >= 1.0); });

    HPX_TEST_EQ(phylanx::ir::node_data<double>(expected),
        phylanx::execution_tree::extract_numeric_value(f));
    HPX_TEST_EQ(phylanx::ir::node_data<std::uint8_t>(std::move(expected)),
        phylanx::execution_tree::extract_boolean_value_strict(f));
}

void test_greater_equal_operation_1d0d_lit()
{
    blaze::Rand<blaze::DynamicVector<int>> gen{};
    blaze::DynamicVector<double> v = gen.generate(1007UL, 0, 2);

    phylanx::ir::node_data<double> lhs(v);

    phylanx::execution_tree::primitive rhs =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(1.0));

    phylanx::execution_tree::primitive greater_equal =
        phylanx::execution_tree::primitives::create_greater_equal(
            hpx::find_here(),
            phylanx::execution_tree::primitive_arguments_type{
                std::move(lhs), std::move(rhs)});

    phylanx::execution_tree::primitive_argument_type f =
        greater_equal.eval().get();

    blaze::DynamicVector<double> expected =
        blaze::map(v, [](double x) { return (x >= 1.0); });

    HPX_TEST_EQ(phylanx::ir::node_data<double>(expected),
        phylanx::execution_tree::extract_numeric_value(f));
    HPX_TEST_EQ(phylanx::ir::node_data<std::uint8_t>(std::move(expected)),
        phylanx::execution_tree::extract_boolean_value_strict(f));
}

void test_greater_equal_operation_1d2d()
{
    blaze::Rand<blaze::DynamicVector<int>> gen{};
    blaze::DynamicVector<double> v = gen.generate(104UL, 0, 2);

    blaze::Rand<blaze::DynamicMatrix<int>> mat_gen{};
    blaze::DynamicMatrix<double> m = mat_gen.generate(101UL, 104UL, 0, 2);

    phylanx::execution_tree::primitive lhs =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(v));

    phylanx::execution_tree::primitive rhs =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(m));

    phylanx::execution_tree::primitive greater_equal =
        phylanx::execution_tree::primitives::create_greater_equal(
            hpx::find_here(),
            phylanx::execution_tree::primitive_arguments_type{
                std::move(lhs), std::move(rhs)});

    phylanx::execution_tree::primitive_argument_type f =
        greater_equal.eval().get();

    blaze::DynamicMatrix<double> expected{m.rows(), m.columns()};

    for (size_t i = 0UL; i < m.rows(); i++)
        blaze::row(expected, i) = blaze::map(blaze::trans(v), blaze::row(m, i),
            [](double x, double y) { return x >= y; });

    HPX_TEST_EQ(phylanx::ir::node_data<double>(expected),
        phylanx::execution_tree::extract_numeric_value(f));
    HPX_TEST_EQ(phylanx::ir::node_data<std::uint8_t>(std::move(expected)),
        phylanx::execution_tree::extract_boolean_value_strict(f));
}

void test_greater_equal_operation_1d2d_lit()
{
    blaze::Rand<blaze::DynamicVector<int>> gen{};
    blaze::DynamicVector<double> v = gen.generate(104UL, 0, 2);

    blaze::Rand<blaze::DynamicMatrix<double>> mat_gen{};
    blaze::DynamicMatrix<double> m = mat_gen.generate(101UL, 104UL, 0, 2);

    phylanx::ir::node_data<double> lhs(v);

    phylanx::execution_tree::primitive rhs =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(m));

    phylanx::execution_tree::primitive greater_equal =
        phylanx::execution_tree::primitives::create_greater_equal(
            hpx::find_here(),
            phylanx::execution_tree::primitive_arguments_type{
                std::move(lhs), std::move(rhs)});

    phylanx::execution_tree::primitive_argument_type f =
        greater_equal.eval().get();

    blaze::DynamicMatrix<double> expected{m.rows(), m.columns()};

    for (size_t i = 0UL; i < m.rows(); i++)
        blaze::row(expected, i) = blaze::map(blaze::trans(v), blaze::row(m, i),
            [](double x, double y) { return x >= y; });

    HPX_TEST_EQ(phylanx::ir::node_data<double>(expected),
        phylanx::execution_tree::extract_numeric_value(f));
    HPX_TEST_EQ(phylanx::ir::node_data<std::uint8_t>(std::move(expected)),
        phylanx::execution_tree::extract_boolean_value_strict(f));
}

void test_greater_equal_operation_2d()
{
    blaze::Rand<blaze::DynamicMatrix<int>> gen{};
    blaze::DynamicMatrix<double> m1 = gen.generate(1007UL, 1007UL, 0, 2);
    blaze::DynamicMatrix<double> m2 = gen.generate(1007UL, 1007UL, 0, 2);

    phylanx::execution_tree::primitive lhs =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(m1));

    phylanx::execution_tree::primitive rhs =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(m2));

    phylanx::execution_tree::primitive greater_equal =
        phylanx::execution_tree::primitives::create_greater_equal(
            hpx::find_here(),
            phylanx::execution_tree::primitive_arguments_type{
                std::move(lhs), std::move(rhs)});

    phylanx::execution_tree::primitive_argument_type f =
        greater_equal.eval().get();

    blaze::DynamicMatrix<double> expected =
        blaze::map(m1, m2, [](double x, double y) { return x >= y; });

    HPX_TEST_EQ(phylanx::ir::node_data<double>(expected),
        phylanx::execution_tree::extract_numeric_value(f));
    HPX_TEST_EQ(phylanx::ir::node_data<std::uint8_t>(std::move(expected)),
        phylanx::execution_tree::extract_boolean_value_strict(f));
}

void test_greater_equal_operation_2d_lit()
{
    blaze::Rand<blaze::DynamicMatrix<int>> gen{};
    blaze::DynamicMatrix<double> m1 = gen.generate(1007UL, 1007UL, 0, 2);
    blaze::DynamicMatrix<double> m2 = gen.generate(1007UL, 1007UL, 0, 2);

    phylanx::ir::node_data<double> lhs(m1);

    phylanx::execution_tree::primitive rhs =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(m2));

    phylanx::execution_tree::primitive greater_equal =
        phylanx::execution_tree::primitives::create_greater_equal(
            hpx::find_here(),
            phylanx::execution_tree::primitive_arguments_type{
                std::move(lhs), std::move(rhs)});

    phylanx::execution_tree::primitive_argument_type f =
        greater_equal.eval().get();

    blaze::DynamicMatrix<double> expected =
        blaze::map(m1, m2, [](double x, double y) { return x >= y; });

    HPX_TEST_EQ(phylanx::ir::node_data<double>(expected),
        phylanx::execution_tree::extract_numeric_value(f));
    HPX_TEST_EQ(phylanx::ir::node_data<std::uint8_t>(std::move(expected)),
        phylanx::execution_tree::extract_boolean_value_strict(f));
}

void test_greater_equal_operation_2d0d()
{
    blaze::Rand<blaze::DynamicMatrix<int>> gen{};
    blaze::DynamicMatrix<double> m = gen.generate(101UL, 101UL, 0, 2);

    phylanx::execution_tree::primitive lhs =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(m));

    phylanx::execution_tree::primitive rhs =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(1.0));

    phylanx::execution_tree::primitive greater_equal =
        phylanx::execution_tree::primitives::create_greater_equal(
            hpx::find_here(),
            phylanx::execution_tree::primitive_arguments_type{
                std::move(lhs), std::move(rhs)});

    phylanx::execution_tree::primitive_argument_type f =
        greater_equal.eval().get();

    blaze::DynamicMatrix<double> expected =
        blaze::map(m, [](double x) { return (x >= 1.0); });

    HPX_TEST_EQ(phylanx::ir::node_data<double>(expected),
        phylanx::execution_tree::extract_numeric_value(f));
    HPX_TEST_EQ(phylanx::ir::node_data<std::uint8_t>(std::move(expected)),
        phylanx::execution_tree::extract_boolean_value_strict(f));
}

void test_greater_equal_operation_2d0d_lit()
{
    blaze::Rand<blaze::DynamicMatrix<int>> gen{};
    blaze::DynamicMatrix<double> m = gen.generate(101UL, 101UL, 0, 2);

    phylanx::ir::node_data<double> lhs(m);

    phylanx::execution_tree::primitive rhs =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(1.0));

    phylanx::execution_tree::primitive greater_equal =
        phylanx::execution_tree::primitives::create_greater_equal(
            hpx::find_here(),
            phylanx::execution_tree::primitive_arguments_type{
                std::move(lhs), std::move(rhs)});

    phylanx::execution_tree::primitive_argument_type f =
        greater_equal.eval().get();

    blaze::DynamicMatrix<double> expected =
        blaze::map(m, [](double x) { return (x >= 1.0); });

    HPX_TEST_EQ(phylanx::ir::node_data<double>(expected),
        phylanx::execution_tree::extract_numeric_value(f));
    HPX_TEST_EQ(phylanx::ir::node_data<std::uint8_t>(std::move(expected)),
        phylanx::execution_tree::extract_boolean_value_strict(f));
}

void test_greater_equal_operation_2d1d()
{
    blaze::Rand<blaze::DynamicVector<int>> gen{};
    blaze::DynamicVector<double> v = gen.generate(104UL, 0, 2);

    blaze::Rand<blaze::DynamicMatrix<double>> mat_gen{};
    blaze::DynamicMatrix<double> m = mat_gen.generate(101UL, 104UL, 0, 2);

    phylanx::execution_tree::primitive lhs =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(m));

    phylanx::execution_tree::primitive rhs =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(v));

    phylanx::execution_tree::primitive greater_equal =
        phylanx::execution_tree::primitives::create_greater_equal(
            hpx::find_here(),
            phylanx::execution_tree::primitive_arguments_type{
                std::move(lhs), std::move(rhs)});

    phylanx::execution_tree::primitive_argument_type f =
        greater_equal.eval().get();

    blaze::DynamicMatrix<double> expected{m.rows(), m.columns()};
    for (size_t i = 0UL; i < m.rows(); i++)
        blaze::row(expected, i) = blaze::map(blaze::row(m, i),
            blaze::trans(v),
            [](double x, double y) { return x >= y; });

    HPX_TEST_EQ(phylanx::ir::node_data<double>(expected),
        phylanx::execution_tree::extract_numeric_value(f));
    HPX_TEST_EQ(phylanx::ir::node_data<std::uint8_t>(std::move(expected)),
        phylanx::execution_tree::extract_boolean_value_strict(f));
}

void test_greater_equal_operation_2d1d_lit()
{
    blaze::Rand<blaze::DynamicVector<int>> gen{};
    blaze::DynamicVector<double> v = gen.generate(104UL, 0, 2);

    blaze::Rand<blaze::DynamicMatrix<double>> mat_gen{};
    blaze::DynamicMatrix<double> m = mat_gen.generate(101UL, 104UL, 0, 2);

    phylanx::ir::node_data<double> lhs(m);

    phylanx::execution_tree::primitive rhs =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(v));

    phylanx::execution_tree::primitive greater_equal =
        phylanx::execution_tree::primitives::create_greater_equal(
            hpx::find_here(),
            phylanx::execution_tree::primitive_arguments_type{
                std::move(lhs), std::move(rhs)});

    phylanx::execution_tree::primitive_argument_type f =
        greater_equal.eval().get();

    blaze::DynamicMatrix<double> expected{m.rows(), m.columns()};
    for (size_t i = 0UL; i < m.rows(); i++)
        blaze::row(expected, i) = blaze::map(blaze::row(m, i),
            blaze::trans(v),
            [](double x, double y) { return x >= y; });

    HPX_TEST_EQ(phylanx::ir::node_data<double>(expected),
        phylanx::execution_tree::extract_numeric_value(f));
    HPX_TEST_EQ(phylanx::ir::node_data<std::uint8_t>(std::move(expected)),
        phylanx::execution_tree::extract_boolean_value_strict(f));
}

void test_greater_equal_operation_0d_true_return_double()
{
    phylanx::execution_tree::primitive lhs =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(41.0));

    phylanx::execution_tree::primitive rhs =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(1.0));

    phylanx::execution_tree::primitive greater_equal;
    greater_equal =
        phylanx::execution_tree::primitives::create_greater_equal(hpx::find_here(),
            phylanx::execution_tree::primitive_arguments_type{
                std::move(lhs), std::move(rhs),
                phylanx::ir::node_data<std::uint8_t>(true)});

    phylanx::execution_tree::primitive_argument_type f = greater_equal.eval().get();

    HPX_TEST_EQ(phylanx::ir::node_data<double>(1.0),
        phylanx::execution_tree::extract_numeric_value(f));
    HPX_TEST_EQ(f.index(), 4);    //node_data<double>
}

void test_greater_equal_operation_0d1d_return_double()
{
    blaze::Rand<blaze::DynamicVector<int>> gen{};
    blaze::DynamicVector<double> v = gen.generate(1007UL, 0, 2);

    phylanx::execution_tree::primitive lhs =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(1.0));

    phylanx::execution_tree::primitive rhs =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(v));

    phylanx::execution_tree::primitive greater_equal =
        phylanx::execution_tree::primitives::create_greater_equal(hpx::find_here(),
            phylanx::execution_tree::primitive_arguments_type{
                std::move(lhs), std::move(rhs),
                phylanx::ir::node_data<std::uint8_t>(true)});

    phylanx::execution_tree::primitive_argument_type f = greater_equal.eval().get();

    blaze::DynamicVector<double> expected =
        blaze::map(v, [](double x) { return (1.0 >= x); });

    HPX_TEST_EQ(phylanx::ir::node_data<double>(expected),
        phylanx::execution_tree::extract_numeric_value(f));
    HPX_TEST_EQ(f.index(), 4);    //node_data<double>
}

void test_greater_equal_operation_0d1d_return_bool()
{
    blaze::Rand<blaze::DynamicVector<int>> gen{};
    blaze::DynamicVector<double> v = gen.generate(1007UL, 0, 2);

    phylanx::execution_tree::primitive lhs =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(1.0));

    phylanx::execution_tree::primitive rhs =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(v));

    phylanx::execution_tree::primitive greater_equal =
        phylanx::execution_tree::primitives::create_greater_equal(hpx::find_here(),
            phylanx::execution_tree::primitive_arguments_type{
                std::move(lhs), std::move(rhs),
                phylanx::ir::node_data<std::uint8_t>(false)});

    phylanx::execution_tree::primitive_argument_type f = greater_equal.eval().get();

    blaze::DynamicVector<double> expected =
        blaze::map(v, [](double x) { return (1.0 >= x); });

    HPX_TEST_EQ(phylanx::ir::node_data<std::uint8_t>(expected),
        phylanx::execution_tree::extract_boolean_value_strict(f));
    HPX_TEST_EQ(f.index(), 1);    //node_data<std::uint8_t>
}

void test_greater_equal_operation_0d2d_return_double()
{
    blaze::Rand<blaze::DynamicMatrix<int>> gen{};
    blaze::DynamicMatrix<double> m = gen.generate(101UL, 101UL, 0, 2);

    phylanx::execution_tree::primitive lhs =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(1.0));

    phylanx::execution_tree::primitive rhs =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(m));

    phylanx::execution_tree::primitive greater_equal =
        phylanx::execution_tree::primitives::create_greater_equal(hpx::find_here(),
            phylanx::execution_tree::primitive_arguments_type{
                std::move(lhs), std::move(rhs),
                phylanx::ir::node_data<std::uint8_t>(true)});

    phylanx::execution_tree::primitive_argument_type f = greater_equal.eval().get();

    blaze::DynamicMatrix<double> expected =
        blaze::map(m, [](double x) { return (1.0 >= x); });

    HPX_TEST_EQ(phylanx::ir::node_data<double>(expected),
        phylanx::execution_tree::extract_numeric_value(f));
    HPX_TEST_EQ(f.index(), 4);
}

void test_greater_equal_operation_1d_return_double()
{
    blaze::Rand<blaze::DynamicVector<int>> gen{};
    blaze::DynamicVector<double> v1 = gen.generate(100UL, 0, 2);
    blaze::DynamicVector<double> v2 = gen.generate(100UL, 0, 2);

    phylanx::execution_tree::primitive lhs =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(v1));

    phylanx::execution_tree::primitive rhs =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(v2));

    phylanx::execution_tree::primitive greater_equal =
        phylanx::execution_tree::primitives::create_greater_equal(hpx::find_here(),
            phylanx::execution_tree::primitive_arguments_type{
                std::move(lhs), std::move(rhs),
                phylanx::ir::node_data<std::uint8_t>(true)});

    phylanx::execution_tree::primitive_argument_type f = greater_equal.eval().get();

    blaze::DynamicVector<double> expected =
        blaze::map(v1, v2, [](double x, double y) { return x >= y; });

    HPX_TEST_EQ(phylanx::ir::node_data<double>((expected)),
        phylanx::execution_tree::extract_numeric_value(f));
    HPX_TEST_EQ(f.index(), 4);
}

void test_greater_equal_operation_1d0d_return_double()
{
    blaze::Rand<blaze::DynamicVector<int>> gen{};
    blaze::DynamicVector<double> v = gen.generate(1007UL, 0, 2);

    phylanx::execution_tree::primitive lhs =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(v));

    phylanx::execution_tree::primitive rhs =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(1.0));

    phylanx::execution_tree::primitive greater_equal =
        phylanx::execution_tree::primitives::create_greater_equal(
            hpx::find_here(),
            phylanx::execution_tree::primitive_arguments_type{
                std::move(lhs), std::move(rhs),
                phylanx::ir::node_data<std::uint8_t>(true)});

    phylanx::execution_tree::primitive_argument_type f = greater_equal.eval().get();

    blaze::DynamicVector<double> expected =
        blaze::map(v, [](double x) { return (x >= 1.0); });

    HPX_TEST_EQ(phylanx::ir::node_data<double>(expected),
        phylanx::execution_tree::extract_numeric_value(f));
    HPX_TEST_EQ(f.index(), 4);    //node_data<double>
}

void test_greater_equal_operation_1d2d_return_double()
{
    blaze::Rand<blaze::DynamicVector<int>> gen{};
    blaze::DynamicVector<double> v = gen.generate(104UL, 0, 2);

    blaze::Rand<blaze::DynamicMatrix<int>> mat_gen{};
    blaze::DynamicMatrix<double> m = mat_gen.generate(101UL, 104UL, 0, 2);

    phylanx::execution_tree::primitive lhs =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(v));

    phylanx::execution_tree::primitive rhs =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(m));

    phylanx::execution_tree::primitive greater_equal =
        phylanx::execution_tree::primitives::create_greater_equal(hpx::find_here(),
            phylanx::execution_tree::primitive_arguments_type{
                std::move(lhs), std::move(rhs),
                phylanx::ir::node_data<std::uint8_t>(true)});

    phylanx::execution_tree::primitive_argument_type f = greater_equal.eval().get();

    blaze::DynamicMatrix<double> expected{m.rows(), m.columns()};

    for (size_t i = 0UL; i < m.rows(); i++)
        blaze::row(expected, i) = blaze::map(blaze::trans(v), blaze::row(m, i),
            [](double x, double y) { return x >= y; });

    HPX_TEST_EQ(phylanx::ir::node_data<double>(expected),
        phylanx::execution_tree::extract_numeric_value(f));
    HPX_TEST_EQ(f.index(), 4);
}

void test_greater_equal_operation_2d_return_double()
{
    blaze::Rand<blaze::DynamicMatrix<int>> gen{};
    blaze::DynamicMatrix<double> m1 = gen.generate(1007UL, 1007UL, 0, 2);
    blaze::DynamicMatrix<double> m2 = gen.generate(1007UL, 1007UL, 0, 2);

    phylanx::execution_tree::primitive lhs =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(m1));

    phylanx::execution_tree::primitive rhs =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(m2));

    phylanx::execution_tree::primitive greater_equal =
        phylanx::execution_tree::primitives::create_greater_equal(hpx::find_here(),
            phylanx::execution_tree::primitive_arguments_type{
                std::move(lhs), std::move(rhs),
                phylanx::ir::node_data<std::uint8_t>(true)});

    phylanx::execution_tree::primitive_argument_type f = greater_equal.eval().get();

    blaze::DynamicMatrix<double> expected =
        blaze::map(m1, m2, [](double x, double y) { return x >= y; });

    HPX_TEST_EQ(phylanx::ir::node_data<double>(expected),
        phylanx::execution_tree::extract_numeric_value(f));
    HPX_TEST_EQ(f.index(), 4);
}

void test_greater_equal_operation_2d0d_return_double()
{
    blaze::Rand<blaze::DynamicMatrix<int>> gen{};
    blaze::DynamicMatrix<double> m = gen.generate(101UL, 101UL, 0, 2);

    phylanx::execution_tree::primitive lhs =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(m));

    phylanx::execution_tree::primitive rhs =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(1.0));

    phylanx::execution_tree::primitive greater_equal =
        phylanx::execution_tree::primitives::create_greater_equal(hpx::find_here(),
            phylanx::execution_tree::primitive_arguments_type{
                std::move(lhs), std::move(rhs),
                phylanx::ir::node_data<std::uint8_t>(true)});

    phylanx::execution_tree::primitive_argument_type f = greater_equal.eval().get();

    blaze::DynamicMatrix<double> expected =
        blaze::map(m, [](double x) { return (x >= 1.0); });

    HPX_TEST_EQ(phylanx::ir::node_data<double>(expected),
        phylanx::execution_tree::extract_numeric_value(f));
    HPX_TEST_EQ(f.index(), 4);
}

void test_greater_equal_operation_2d1d_return_double()
{
    blaze::Rand<blaze::DynamicVector<int>> gen{};
    blaze::DynamicVector<double> v = gen.generate(104UL, 0, 2);

    blaze::Rand<blaze::DynamicMatrix<double>> mat_gen{};
    blaze::DynamicMatrix<double> m = mat_gen.generate(101UL, 104UL, 0, 2);

    phylanx::execution_tree::primitive lhs =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(m));

    phylanx::execution_tree::primitive rhs =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(v));

    phylanx::execution_tree::primitive greater_equal =
        phylanx::execution_tree::primitives::create_greater_equal(hpx::find_here(),
            phylanx::execution_tree::primitive_arguments_type{
                std::move(lhs), std::move(rhs),
                phylanx::ir::node_data<std::uint8_t>(true)});

    phylanx::execution_tree::primitive_argument_type f = greater_equal.eval().get();

    blaze::DynamicMatrix<double> expected{m.rows(), m.columns()};
    for (size_t i = 0UL; i < m.rows(); i++)
        blaze::row(expected, i) = blaze::map(blaze::row(m, i),
            blaze::trans(v),
            [](double x, double y) { return x >= y; });

    HPX_TEST_EQ(phylanx::ir::node_data<double>(expected),
        phylanx::execution_tree::extract_numeric_value(f));
    HPX_TEST_EQ(f.index(), 4);
}

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

void test_greater_equal_operation(std::string const& code,
    std::string const& expected_str)
{
    HPX_TEST_EQ(compile_and_run(code), compile_and_run(expected_str));
}

void test_greater_equal_operation_3d()
{
    // 0d
    test_greater_equal_operation(
        R"(4 >= [[[1, 2], [3, 4]], [[5, 6], [7, 8]]])",
        R"(astype([[[1, 1], [1, 1]], [[0, 0], [0, 0]]], "bool"))");
    test_greater_equal_operation(
        R"([[[1, 2], [3, 4]], [[5, 6], [7, 8]]] >= 4)",
        R"(astype([[[0, 0], [0, 1]], [[1, 1], [1, 1]]], "bool"))");

    // 1d
    test_greater_equal_operation(
        R"([3, 4] >= [[[1, 2], [3, 4]], [[5, 6], [7, 8]]])",
        R"(astype([[[1, 1], [1, 1]], [[0, 0], [0, 0]]], "bool"))");
    test_greater_equal_operation(
        R"([[[1, 2], [3, 4]], [[5, 6], [7, 8]]] >= [3, 4])",
        R"(astype([[[0, 0], [1, 1]], [[1, 1], [1, 1]]], "bool"))");

    // 2d
    test_greater_equal_operation(
        R"([[1, 2], [3, 4]] >= [[[1, 2], [3, 4]], [[5, 6], [7, 8]]])",
        R"(astype([[[1, 1], [1, 1]], [[0, 0], [0, 0]]], "bool"))");
    test_greater_equal_operation(
        R"([[[1, 2], [3, 4]], [[5, 6], [7, 8]]] >= [[1, 2], [3, 4]])",
        R"(astype([[[1, 1], [1, 1]], [[1, 1], [1, 1]]], "bool"))");

    // 3d
    test_greater_equal_operation(
        R"([[[1, 2], [3, 4]], [[5, 6], [7, 8]]] >=
           [[[1, 2], [3, 4]], [[5, 6], [7, 8]]])",
        R"(astype([[[1, 1], [1, 1]], [[1, 1], [1, 1]]], "bool"))");
    test_greater_equal_operation(
        R"([[[1, 2], [3, 4]], [[5, 6], [7, 8]]] >=
           [[[8, 7], [6, 5]], [[4, 3], [2, 1]]])",
        R"(astype([[[0, 0], [0, 0]], [[1, 1], [1, 1]]], "bool"))");
}

void test_greater_equal_operation_4d()
{
    // 0d
    test_greater_equal_operation(
        R"(4 >= [[[[1, 2], [3, 4]], [[5, 6], [7, 8]]],
            [[[-1, -2], [-3, -4]], [[-5, 6], [-7, -8]]]])",
        R"(astype([[[[1, 1], [1, 1]], [[0, 0], [0, 0]]],
                   [[[1, 1], [1, 1]], [[1, 0], [1, 1]]]], "bool"))");
    test_greater_equal_operation(
        R"([[[[1, 2], [3, 4]], [[5, 6], [7, 8]]],
            [[[-1, -2], [-3, -4]], [[-5, 6], [-7, -8]]]] >= 4)",
        R"(astype([[[[0, 0], [0, 1]], [[1, 1], [1, 1]]],
                   [[[0, 0], [0, 0]], [[0, 1], [0, 0]]]], "bool"))");

    // 1d
    test_greater_equal_operation(
        R"([3, 4, -1] >= [[[[1, 2, 3], [3, 4, 5]], [[5, 6, 7], [7, 8, 9]]],
            [[[0, -1, -2], [-3, -4, -5]], [[2, -5, 6], [3, -7, -8]]]])",
        R"(astype([[[[1, 1, 0],[1, 1, 0]],[[0, 0, 0],[0, 0, 0]]],
                   [[[1, 1, 1],[1, 1, 1]],[[1, 1, 0],[1, 1, 1]]]], "bool"))");
    test_greater_equal_operation(
        R"([[[[1, 2, 3], [3, 4, 5]], [[5, 6, 7], [7, 8, 9]]],
            [[[0, -1, -2], [-3, -4, -5]], [[2, -5, 6], [3, -7, -8]]]]
            >= [3, 4, -1])",
        R"(astype([[[[0, 0, 1],[1, 1, 1]],[[1, 1, 1],[1, 1, 1]]],
                   [[[0, 0, 0],[0, 0, 0]],[[0, 0, 1],[1, 0, 0]]]], "bool"))");

    // 2d
    test_greater_equal_operation(
        R"([[3, 4, -1],[1, 0, 1]] >=
            [[[[1, 2, 3], [3, 4, 5]], [[5, 6, 7], [7, 8, 9]]],
            [[[0, -1, -2], [-3, -4, -5]], [[2, -5, 6], [3, -7, -8]]]])",
        R"(astype([[[[1, 1, 0],[0, 0, 0]],[[0, 0, 0],[0, 0, 0]]],
                   [[[1, 1, 1],[1, 1, 1]],[[1, 1, 0],[0, 1, 1]]]], "bool"))");
    test_greater_equal_operation(
        R"([[[[1, 2, 3], [3, 4, 5]], [[5, 6, 7], [7, 8, 9]]],
            [[[0, -1, -2], [-3, -4, -5]], [[2, -5, 6], [3, -7, -8]]]]
            >= [[3, 4, -1],[1, 0, 1]])",
        R"(astype([[[[0, 0, 1],[1, 1, 1]],[[1, 1, 1],[1, 1, 1]]],
                   [[[0, 0, 0],[0, 0, 0]],[[0, 0, 1],[1, 0, 0]]]], "bool"))");

    // 3d
    test_greater_equal_operation(
        R"([[[3, 4, -1],[1, 0, 1]], [[-3, 0, -1],[2, 2, 1]]] >=
            [[[[1, 2, 3], [3, 4, 5]], [[5, 6, 7], [7, 8, 9]]],
            [[[0, -1, -2], [-3, -4, -5]], [[2, -5, 6], [3, -7, -8]]]])",
        R"(astype([[[[1, 1, 0],[0, 0, 0]],[[0, 0, 0],[0, 0, 0]]],
                   [[[1, 1, 1],[1, 1, 1]],[[0, 1, 0],[0, 1, 1]]]], "bool"))");
    test_greater_equal_operation(
        R"([[[[1, 2, 3], [3, 4, 5]], [[5, 6, 7], [7, 8, 9]]],
            [[[0, -1, -2], [-3, -4, -5]], [[2, -5, 6], [3, -7, -8]]]]
            >= [[[3, 4, -1],[1, 0, 1]], [[-3, 0, -1],[2, 2, 1]]])",
        R"(astype([[[[0, 0, 1],[1, 1, 1]],[[1, 1, 1],[1, 1, 1]]],
                   [[[0, 0, 0],[0, 0, 0]],[[1, 0, 1],[1, 0, 0]]]], "bool"))");

    // 4d
    test_greater_equal_operation(
        R"([[[[3, 4, -1],[1, 0, 1]], [[-3, 0, -1],[2, 2, 1]]],
            [[[3, 4, -1],[1, 0, 1]], [[-3, 0, -1],[2, 2, 1]]]] >=
            [[[[1, 2, 3], [3, 4, 5]], [[5, 6, 7], [7, 8, 9]]],
            [[[0, -1, -2], [-3, -4, -5]], [[2, -5, 6], [3, -7, -8]]]])",
        R"(astype([[[[1, 1, 0],[0, 0, 0]],[[0, 0, 0],[0, 0, 0]]],
                   [[[1, 1, 1],[1, 1, 1]],[[0, 1, 0],[0, 1, 1]]]], "bool"))");
    test_greater_equal_operation(
        R"([[[[1, 2, 3], [3, 4, 5]], [[5, 6, 7], [7, 8, 9]]],
            [[[0, -1, -2], [-3, -4, -5]], [[2, -5, 6], [3, -7, -8]]]]
            >= [[[[3, 4, -1],[1, 0, 1]], [[-3, 0, -1],[2, 2, 1]]],
            [[[3, 4, -1],[1, 0, 1]], [[-3, 0, -1],[2, 2, 1]]]])",
        R"(astype([[[[0, 0, 1],[1, 1, 1]],[[1, 1, 1],[1, 1, 1]]],
                   [[[0, 0, 0],[0, 0, 0]],[[1, 0, 1],[1, 0, 0]]]], "bool"))");
}

int main(int argc, char* argv[])
{
    test_greater_equal_operation_0d_false();
    test_greater_equal_operation_0d_true();
    test_greater_equal_operation_0d_lit_false();
    test_greater_equal_operation_0d_lit_true();
    test_greater_equal_operation_0d1d();
    test_greater_equal_operation_0d1d_lit();
    test_greater_equal_operation_0d2d();
    test_greater_equal_operation_0d2d_lit();

    test_greater_equal_operation_1d();
    test_greater_equal_operation_1d_lit();
    test_greater_equal_operation_1d0d();
    test_greater_equal_operation_1d0d_lit();
    test_greater_equal_operation_1d2d();
    test_greater_equal_operation_1d2d_lit();

    test_greater_equal_operation_2d();
    test_greater_equal_operation_2d_lit();
    test_greater_equal_operation_2d0d();
    test_greater_equal_operation_2d0d_lit();
    test_greater_equal_operation_2d1d();
    test_greater_equal_operation_2d1d_lit();

    test_greater_equal_operation_0d_true_return_double();
    test_greater_equal_operation_0d1d_return_double();
    test_greater_equal_operation_0d1d_return_bool();
    test_greater_equal_operation_0d2d_return_double();
    test_greater_equal_operation_1d_return_double();
    test_greater_equal_operation_1d0d_return_double();
    test_greater_equal_operation_1d2d_return_double();
    test_greater_equal_operation_2d_return_double();
    test_greater_equal_operation_2d0d_return_double();
    test_greater_equal_operation_2d1d_return_double();

    test_greater_equal_operation_3d();

    test_greater_equal_operation_4d();

    return hpx::util::report_errors();
}
