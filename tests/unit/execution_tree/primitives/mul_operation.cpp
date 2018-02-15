//   Copyright (c) 2017 Hartmut Kaiser
//   Copyright (c) 2017 Alireza Kheirkhahan
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

void test_mul_operation_0d()
{
    phylanx::execution_tree::primitive lhs =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(6.0));

    phylanx::execution_tree::primitive rhs =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(7.0));

    phylanx::execution_tree::primitive mul =
        phylanx::execution_tree::primitives::create_mul_operation(
            hpx::find_here(),
            std::vector<phylanx::execution_tree::primitive_argument_type>{
                std::move(lhs), std::move(rhs)
            });

    hpx::future<phylanx::execution_tree::primitive_argument_type> f =
        mul.eval();
    HPX_TEST_EQ(
        42.0, phylanx::execution_tree::extract_numeric_value(f.get())[0]);
}

void test_mul_operation_0d_lit()
{
    phylanx::ir::node_data<double> lhs(6.0);

    phylanx::execution_tree::primitive rhs =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(7.0));

    phylanx::execution_tree::primitive mul =
        phylanx::execution_tree::primitives::create_mul_operation(
            hpx::find_here(),
            std::vector<phylanx::execution_tree::primitive_argument_type>{
                std::move(lhs), std::move(rhs)
            });

    hpx::future<phylanx::execution_tree::primitive_argument_type> f =
        mul.eval();
    HPX_TEST_EQ(
        42.0, phylanx::execution_tree::extract_numeric_value(f.get())[0]);
}

void test_mul_operation_0d1d()
{
    blaze::Rand<blaze::DynamicVector<double>> gen{};
    blaze::DynamicVector<double> v = gen.generate(1007ul);

    phylanx::execution_tree::primitive lhs =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(6.0));

    phylanx::execution_tree::primitive rhs =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(v));

    phylanx::execution_tree::primitive mul =
        phylanx::execution_tree::primitives::create_mul_operation(
            hpx::find_here(),
            std::vector<phylanx::execution_tree::primitive_argument_type>{
                std::move(lhs), std::move(rhs)
            });

    hpx::future<phylanx::execution_tree::primitive_argument_type> f =
        mul.eval();

    blaze::DynamicVector<double> expected = 6.0 * v;
    HPX_TEST_EQ(
        phylanx::ir::node_data<double>(std::move(expected)),
        phylanx::execution_tree::extract_numeric_value(f.get()));
}

void test_mul_operation_0d1d_lit()
{
    blaze::Rand<blaze::DynamicVector<double>> gen{};
    blaze::DynamicVector<double> v = gen.generate(1007ul);

    phylanx::ir::node_data<double> lhs(6.0);

    phylanx::execution_tree::primitive rhs =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(v));

    phylanx::execution_tree::primitive mul =
        phylanx::execution_tree::primitives::create_mul_operation(
            hpx::find_here(),
            std::vector<phylanx::execution_tree::primitive_argument_type>{
                std::move(lhs), std::move(rhs)
            });

    hpx::future<phylanx::execution_tree::primitive_argument_type> f =
        mul.eval();

    blaze::DynamicVector<double> expected = 6.0 * v;
    HPX_TEST_EQ(
        phylanx::ir::node_data<double>(std::move(expected)),
        phylanx::execution_tree::extract_numeric_value(f.get()));
}

void test_mul_operation_0d2d()
{
    blaze::Rand<blaze::DynamicMatrix<double>> gen{};
    blaze::DynamicMatrix<double> m = gen.generate(101ul, 101ul);

    phylanx::execution_tree::primitive lhs =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(6.0));

    phylanx::execution_tree::primitive rhs =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(m));

    phylanx::execution_tree::primitive mul =
        phylanx::execution_tree::primitives::create_mul_operation(
            hpx::find_here(),
            std::vector<phylanx::execution_tree::primitive_argument_type>{
                std::move(lhs), std::move(rhs)
            });

    hpx::future<phylanx::execution_tree::primitive_argument_type> f =
        mul.eval();

    blaze::DynamicMatrix<double> expected = 6.0 * m;
    HPX_TEST_EQ(
        phylanx::ir::node_data<double>(std::move(expected)),
        phylanx::execution_tree::extract_numeric_value(f.get()));
}

void test_mul_operation_0d2d_lit()
{
    blaze::Rand<blaze::DynamicMatrix<double>> gen{};
    blaze::DynamicMatrix<double> m = gen.generate(101ul, 101ul);

    phylanx::ir::node_data<double> lhs(6.0);

    phylanx::execution_tree::primitive rhs =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(m));

    phylanx::execution_tree::primitive mul =
        phylanx::execution_tree::primitives::create_mul_operation(
            hpx::find_here(),
            std::vector<phylanx::execution_tree::primitive_argument_type>{
                std::move(lhs), std::move(rhs)
            });

    hpx::future<phylanx::execution_tree::primitive_argument_type> f =
        mul.eval();

    blaze::DynamicMatrix<double> expected = m * 6.0;
    HPX_TEST_EQ(
        phylanx::ir::node_data<double>(std::move(expected)),
        phylanx::execution_tree::extract_numeric_value(f.get()));
}

void test_mul_operation_1d0d()
{
    blaze::Rand<blaze::DynamicVector<double>> gen{};
    blaze::DynamicVector<double> v = gen.generate(1007ul);

    phylanx::execution_tree::primitive lhs =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(v));

    phylanx::execution_tree::primitive rhs =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(6.0));

    phylanx::execution_tree::primitive mul =
        phylanx::execution_tree::primitives::create_mul_operation(
            hpx::find_here(),
            std::vector<phylanx::execution_tree::primitive_argument_type>{
                std::move(lhs), std::move(rhs)
            });

    hpx::future<phylanx::execution_tree::primitive_argument_type> f =
        mul.eval();

    blaze::DynamicVector<double> expected = v * 6.0;
    HPX_TEST_EQ(
        phylanx::ir::node_data<double>(std::move(expected)),
        phylanx::execution_tree::extract_numeric_value(f.get()));
}

void test_mul_operation_1d0d_lit()
{
    blaze::Rand<blaze::DynamicVector<double>> gen{};
    blaze::DynamicVector<double> v = gen.generate(1007ul);

    phylanx::ir::node_data<double> lhs(v);

    phylanx::execution_tree::primitive rhs =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(6.0));

    phylanx::execution_tree::primitive mul =
        phylanx::execution_tree::primitives::create_mul_operation(
            hpx::find_here(),
            std::vector<phylanx::execution_tree::primitive_argument_type>{
                std::move(lhs), std::move(rhs)
            });

    hpx::future<phylanx::execution_tree::primitive_argument_type> f =
        mul.eval();

    blaze::DynamicVector<double> expected = v * 6.0;
    HPX_TEST_EQ(
        phylanx::ir::node_data<double>(std::move(expected)),
        phylanx::execution_tree::extract_numeric_value(f.get()));
}

void test_mul_operation_1d1d()
{
    blaze::Rand<blaze::DynamicVector<double>> gen{};
    blaze::DynamicVector<double> v1 = gen.generate(1007ul);
    blaze::DynamicVector<double> v2 = gen.generate(1007ul);

    blaze::DynamicVector<double> expected = v1 * v2;

    phylanx::execution_tree::primitive lhs =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(v1));

    phylanx::execution_tree::primitive rhs =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(v2));

    phylanx::execution_tree::primitive mul =
        phylanx::execution_tree::primitives::create_mul_operation(
            hpx::find_here(),
            std::vector<phylanx::execution_tree::primitive_argument_type>{
        std::move(lhs), std::move(rhs)
    });

    hpx::future<phylanx::execution_tree::primitive_argument_type> f =
        mul.eval();
    HPX_TEST_EQ(
        phylanx::ir::node_data<double>(std::move(expected)),
        phylanx::execution_tree::extract_numeric_value(f.get()));
}

void test_mul_operation_2d0d()
{
    blaze::Rand<blaze::DynamicMatrix<double>> gen{};
    blaze::DynamicMatrix<double> m = gen.generate(42ul, 42ul);

    phylanx::execution_tree::primitive lhs =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(m));

    phylanx::execution_tree::primitive rhs =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(6.0));

    phylanx::execution_tree::primitive mul =
        phylanx::execution_tree::primitives::create_mul_operation(
            hpx::find_here(),
            std::vector<phylanx::execution_tree::primitive_argument_type>{
                std::move(lhs), std::move(rhs)
            });

    hpx::future<phylanx::execution_tree::primitive_argument_type> f =
        mul.eval();

    blaze::DynamicMatrix<double> expected = m * 6.0;
    HPX_TEST_EQ(
        phylanx::ir::node_data<double>(std::move(expected)),
        phylanx::execution_tree::extract_numeric_value(f.get()));
}

void test_mul_operation_2d0d_lit()
{
    blaze::Rand<blaze::DynamicMatrix<double>> gen{};
    blaze::DynamicMatrix<double> m = gen.generate(42ul, 42ul);

    phylanx::ir::node_data<double> lhs(m);

    phylanx::execution_tree::primitive rhs =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(6.0));

    phylanx::execution_tree::primitive mul =
        phylanx::execution_tree::primitives::create_mul_operation(
            hpx::find_here(),
            std::vector<phylanx::execution_tree::primitive_argument_type>{
                std::move(lhs), std::move(rhs)
            });

    hpx::future<phylanx::execution_tree::primitive_argument_type> f =
        mul.eval();

    blaze::DynamicMatrix<double> expected = m * 6.0;
    HPX_TEST_EQ(
        phylanx::ir::node_data<double>(std::move(expected)),
        phylanx::execution_tree::extract_numeric_value(f.get()));
}

void test_mul_operation_2d1d()
{
    blaze::Rand<blaze::DynamicMatrix<double>> gen1{};
    blaze::DynamicMatrix<double> m = gen1.generate(107ul, 42ul);
    blaze::Rand<blaze::DynamicVector<double>> gen2{};
    blaze::DynamicVector<double> v = gen2.generate(42ul);

    phylanx::execution_tree::primitive lhs =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(m));

    phylanx::execution_tree::primitive rhs =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(m));

    phylanx::execution_tree::primitive mul =
        phylanx::execution_tree::primitives::create_mul_operation(
            hpx::find_here(),
            std::vector<phylanx::execution_tree::primitive_argument_type>{
        std::move(lhs), std::move(rhs)
    });

    hpx::future<phylanx::execution_tree::primitive_argument_type> f =
        mul.eval();

    blaze::DynamicVector<double> expected = m * v;
    HPX_TEST_EQ(
        phylanx::ir::node_data<double>(std::move(expected)),
        phylanx::execution_tree::extract_numeric_value(f.get()));
}

void test_mul_operation_2d()
{
    blaze::Rand<blaze::DynamicMatrix<double>> gen{};
    blaze::DynamicMatrix<double> m1 = gen.generate(42ul, 42ul);
    blaze::DynamicMatrix<double> m2 = gen.generate(42ul, 42ul);

    phylanx::execution_tree::primitive lhs =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(m1));

    phylanx::execution_tree::primitive rhs =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(m2));

    phylanx::execution_tree::primitive mul =
        phylanx::execution_tree::primitives::create_mul_operation(
            hpx::find_here(),
            std::vector<phylanx::execution_tree::primitive_argument_type>{
                std::move(lhs), std::move(rhs)
            });

    hpx::future<phylanx::execution_tree::primitive_argument_type> f =
        mul.eval();

    blaze::DynamicMatrix<double> expected = m1 * m2;
    HPX_TEST_EQ(
        phylanx::ir::node_data<double>(std::move(expected)),
        phylanx::execution_tree::extract_numeric_value(f.get()));
}

void test_mul_operation_2d_lit()
{
    blaze::Rand<blaze::DynamicMatrix<double>> gen{};
    blaze::DynamicMatrix<double> m1 = gen.generate(42ul, 42ul);
    blaze::DynamicMatrix<double> m2 = gen.generate(42ul, 42ul);

    phylanx::ir::node_data<double> lhs(m1);

    phylanx::execution_tree::primitive rhs =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(m2));

    phylanx::execution_tree::primitive mul =
        phylanx::execution_tree::primitives::create_mul_operation(
            hpx::find_here(),
            std::vector<phylanx::execution_tree::primitive_argument_type>{
                std::move(lhs), std::move(rhs)
            });

    hpx::future<phylanx::execution_tree::primitive_argument_type> f =
        mul.eval();

    blaze::DynamicMatrix<double> expected = m1 * m2;
    HPX_TEST_EQ(
        phylanx::ir::node_data<double>(std::move(expected)),
        phylanx::execution_tree::extract_numeric_value(f.get()));
}

int main(int argc, char* argv[])
{
    test_mul_operation_0d();
    test_mul_operation_0d_lit();

    test_mul_operation_0d1d();
    test_mul_operation_0d1d_lit();

    test_mul_operation_0d2d();
    test_mul_operation_0d2d_lit();

    test_mul_operation_1d0d();
    test_mul_operation_1d0d_lit();

    test_mul_operation_1d1d();

    test_mul_operation_2d0d();
    test_mul_operation_2d0d_lit();

    test_mul_operation_2d();
    test_mul_operation_2d_lit();

    return hpx::util::report_errors();
}

