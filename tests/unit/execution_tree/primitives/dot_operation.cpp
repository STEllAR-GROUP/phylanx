//   Copyright (c) 2017 Hartmut Kaiser
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

    hpx::future<phylanx::execution_tree::primitive_result_type> f =
        dot.eval();
    HPX_TEST_EQ(
        42.0, phylanx::execution_tree::extract_numeric_value(f.get())[0]);
}

void test_dot_operation_1d()
{
    blaze::DynamicVector<double, rowVector> v1{ 17.99, 20.57, 19.69, 11.42, 20.29, 12.45, 18.25, 13.71 };

    blaze::DynamicVector<double, rowVector> v2 = v1;

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

    hpx::future<phylanx::execution_tree::primitive_result_type> f =
        dot.eval();
    double expected = blaze::dot(v1, v2);

    HPX_TEST_EQ(
        expected, phylanx::execution_tree::extract_numeric_value(f.get())[0]);
}

void test_dot_operation_2d1()
{
    blaze::DynamicVector<double, rowVector> x1{ { 17.99, 20.57, 19.69, 11.42, 20.29, 12.45, 18.25, 13.71 } };
    blaze::DynamicMatrix<double> v1(1UL, x1.size());
    blaze::row(v1, 0UL) = x1;

    blaze::DynamicMatrix<double> v2 = v1;
    blaze::transpose(v2);

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

    hpx::future<phylanx::execution_tree::primitive_result_type> f =
        dot.eval();

    double expected =
        blaze::dot(v1, v1);

    HPX_TEST_EQ(expected,
        phylanx::execution_tree::extract_numeric_value(f.get())[0]);
}

void test_dot_operation_2d2()
{
    blaze::DynamicVector<double, rowVector> v1{ 17.99, 20.57, 19.69, 11.42, 20.29, 12.45, 18.25, 13.71 };

    blaze::DynamicVector v2 = v1;

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

    hpx::future<phylanx::execution_tree::primitive_result_type> f =
        dot.eval();

    double expected = blaze::dot(v1, v2);

    HPX_TEST_EQ(expected,
        phylanx::execution_tree::extract_numeric_value(f.get())[0]);
}

int main(int argc, char* argv[])
{
    test_dot_operation_0d();
    test_dot_operation_1d();
    test_dot_operation_2d1();
    test_dot_operation_2d2();

    return hpx::util::report_errors();
}

