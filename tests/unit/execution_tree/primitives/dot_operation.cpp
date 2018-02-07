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
    phylanx::ir::node_data<double> lhs(6.0);

    phylanx::execution_tree::primitive rhs =
        hpx::new_<phylanx::execution_tree::primitives::variable>(
            hpx::find_here(), phylanx::ir::node_data<double>(7.0));

    phylanx::execution_tree::primitive dot =
        hpx::new_<phylanx::execution_tree::primitives::dot_operation>(
            hpx::find_here(),
            std::vector<phylanx::execution_tree::primitive_argument_type>{
                std::move(lhs), std::move(rhs)
            });

    hpx::future<phylanx::execution_tree::primitive_argument_type> f =
        dot.eval();
    HPX_TEST_EQ(
        42.0, phylanx::execution_tree::extract_numeric_value(f.get())[0]);
}

void test_dot_operation_1d()
{
    blaze::DynamicVector<double> v1{
        17.99, 20.57, 19.69, 11.42, 20.29, 12.45, 18.25, 13.71};

    blaze::DynamicVector<double> v2 = v1;

    double expected = blaze::dot(v1, v2);

    phylanx::execution_tree::primitive lhs =
        hpx::new_<phylanx::execution_tree::primitives::variable>(
            hpx::find_here(), phylanx::ir::node_data<double>(v1));

    phylanx::execution_tree::primitive rhs =
        hpx::new_<phylanx::execution_tree::primitives::variable>(
            hpx::find_here(), phylanx::ir::node_data<double>(v2));

    phylanx::execution_tree::primitive dot =
        hpx::new_<phylanx::execution_tree::primitives::dot_operation>(
            hpx::find_here(),
            std::vector<phylanx::execution_tree::primitive_argument_type>{
                std::move(lhs), std::move(rhs)
            });

    hpx::future<phylanx::execution_tree::primitive_argument_type> f =
        dot.eval();

    HPX_TEST_EQ(
        expected, phylanx::execution_tree::extract_numeric_value(f.get())[0]);
}

void test_dot_operation_1d2d()
{
    blaze::DynamicVector<double> v{ 0.95469394,  0.91224303 };

    blaze::DynamicMatrix<double> m{
        {0.67667122, 0.31796917, 0.04583045, 0.99004279, 0.72412093},
        {0.96827937, 0.01405224, 0.67164958, 0.81945895, 0.25461236}};

    blaze::DynamicVector<double> expected{
        1.52932001, 0.3163823, 0.65646171, 1.69273356, 0.92358221};

    phylanx::execution_tree::primitive lhs =
        hpx::new_<phylanx::execution_tree::primitives::variable>(
            hpx::find_here(), phylanx::ir::node_data<double>(v));

    phylanx::execution_tree::primitive rhs =
        hpx::new_<phylanx::execution_tree::primitives::variable>(
            hpx::find_here(), phylanx::ir::node_data<double>(m));

    phylanx::execution_tree::primitive dot =
        hpx::new_<phylanx::execution_tree::primitives::dot_operation>(
            hpx::find_here(),
            std::vector<phylanx::execution_tree::primitive_argument_type>{
        std::move(lhs), std::move(rhs)
    });

    hpx::future<phylanx::execution_tree::primitive_argument_type> f =
        dot.eval();


    HPX_TEST_EQ(phylanx::ir::node_data<double>(std::move(expected)),
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
        hpx::new_<phylanx::execution_tree::primitives::variable>(
            hpx::find_here(), phylanx::ir::node_data<double>(m));

    phylanx::execution_tree::primitive rhs =
        hpx::new_<phylanx::execution_tree::primitives::variable>(
            hpx::find_here(), phylanx::ir::node_data<double>(v));

    phylanx::execution_tree::primitive dot =
        hpx::new_<phylanx::execution_tree::primitives::dot_operation>(
            hpx::find_here(),
            std::vector<phylanx::execution_tree::primitive_argument_type>{
        std::move(lhs), std::move(rhs)
    });

    hpx::future<phylanx::execution_tree::primitive_argument_type> f =
        dot.eval();

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
        hpx::new_<phylanx::execution_tree::primitives::variable>(
            hpx::find_here(), phylanx::ir::node_data<double>(m1));

    phylanx::execution_tree::primitive rhs =
        hpx::new_<phylanx::execution_tree::primitives::variable>(
            hpx::find_here(), phylanx::ir::node_data<double>(m2));

    phylanx::execution_tree::primitive dot =
        hpx::new_<phylanx::execution_tree::primitives::dot_operation>(
            hpx::find_here(),
            std::vector<phylanx::execution_tree::primitive_argument_type>{
                std::move(lhs), std::move(rhs)
            });

    hpx::future<phylanx::execution_tree::primitive_argument_type> f =
        dot.eval();


    HPX_TEST_EQ(phylanx::ir::node_data<double>(std::move(expected)),
        phylanx::execution_tree::extract_numeric_value(f.get()));
}

int main(int argc, char* argv[])
{
    test_dot_operation_0d();
    test_dot_operation_1d();
    test_dot_operation_1d2d();
    test_dot_operation_2d1d();
    test_dot_operation_2d2d();

    return hpx::util::report_errors();
}

