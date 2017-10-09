//   Copyright (c) 2017 Hartmut Kaiser
//
//   Distributed under the Boost Software License, Version 1.0. (See accompanying
//   file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/phylanx.hpp>

#include <hpx/hpx_main.hpp>
#include <hpx/include/lcos.hpp>
#include <hpx/util/lightweight_test.hpp>

#include <Eigen/Dense>

#include <iostream>
#include <utility>
#include <vector>

void test_or_operation_0d_false()
{
    phylanx::execution_tree::primitive or_ =
        hpx::new_<phylanx::execution_tree::primitives::or_operation>(
            hpx::find_here(),
            std::vector<phylanx::execution_tree::primitive_argument_type>{
                false, false
            });

    hpx::future<phylanx::execution_tree::primitive_result_type> f =
        or_.eval();

    HPX_TEST(!phylanx::execution_tree::extract_boolean_value(f.get()));
}

void test_or_operation_0d_true()
{
    phylanx::execution_tree::primitive or_ =
        hpx::new_<phylanx::execution_tree::primitives::or_operation>(
            hpx::find_here(),
            std::vector<phylanx::execution_tree::primitive_argument_type>{
                false, true
            });

    hpx::future<phylanx::execution_tree::primitive_result_type> f =
        or_.eval();

    HPX_TEST(phylanx::execution_tree::extract_boolean_value(f.get()));
}

void test_or_operation_0d_lit_false()
{
    phylanx::ir::node_data<double> lhs(0.0);

    phylanx::execution_tree::primitive rhs =
        hpx::new_<phylanx::execution_tree::primitives::variable>(
            hpx::find_here(), phylanx::ir::node_data<double>(0.0));

    phylanx::execution_tree::primitive or_ =
        hpx::new_<phylanx::execution_tree::primitives::or_operation>(
            hpx::find_here(),
            std::vector<phylanx::execution_tree::primitive_argument_type>{
                std::move(lhs), std::move(rhs)
            });

    hpx::future<phylanx::execution_tree::primitive_result_type> f =
        or_.eval();

    HPX_TEST(!phylanx::execution_tree::extract_boolean_value(f.get()));
}

void test_or_operation_0d_lit_true()
{
    phylanx::ir::node_data<double> lhs(41.0);

    phylanx::execution_tree::primitive rhs =
        hpx::new_<phylanx::execution_tree::primitives::variable>(
            hpx::find_here(), phylanx::ir::node_data<double>(1.0));

    phylanx::execution_tree::primitive or_ =
        hpx::new_<phylanx::execution_tree::primitives::or_operation>(
            hpx::find_here(),
            std::vector<phylanx::execution_tree::primitive_argument_type>{
                std::move(lhs), std::move(rhs)
            });

    hpx::future<phylanx::execution_tree::primitive_result_type> f =
        or_.eval();

    HPX_TEST(phylanx::execution_tree::extract_boolean_value(f.get()));
}

void test_or_operation_1d()
{
    Eigen::VectorXd v1 = Eigen::VectorXd::Random(1007);
    Eigen::VectorXd v2 = Eigen::VectorXd::Random(1007);

    phylanx::execution_tree::primitive lhs =
        hpx::new_<phylanx::execution_tree::primitives::variable>(
            hpx::find_here(), phylanx::ir::node_data<double>(v1));

    phylanx::execution_tree::primitive rhs =
        hpx::new_<phylanx::execution_tree::primitives::variable>(
            hpx::find_here(), phylanx::ir::node_data<double>(v2));

    phylanx::execution_tree::primitive or_ =
        hpx::new_<phylanx::execution_tree::primitives::or_operation>(
            hpx::find_here(),
            std::vector<phylanx::execution_tree::primitive_argument_type>{
                std::move(lhs), std::move(rhs)
            });

    hpx::future<phylanx::execution_tree::primitive_result_type> f =
        or_.eval();

    HPX_TEST_EQ(
        (v1.norm() != 0.0) || (v2.norm() != 0.0),
        phylanx::execution_tree::extract_boolean_value(f.get()) != 0);
}

void test_or_operation_1d_lit()
{
    Eigen::VectorXd v1 = Eigen::VectorXd::Random(1007);
    Eigen::VectorXd v2 = Eigen::VectorXd::Random(1007);

    phylanx::ir::node_data<double> lhs(v1);

    phylanx::execution_tree::primitive rhs =
        hpx::new_<phylanx::execution_tree::primitives::variable>(
            hpx::find_here(), phylanx::ir::node_data<double>(v2));

    phylanx::execution_tree::primitive or_ =
        hpx::new_<phylanx::execution_tree::primitives::or_operation>(
            hpx::find_here(),
            std::vector<phylanx::execution_tree::primitive_argument_type>{
                std::move(lhs), std::move(rhs)
            });

    hpx::future<phylanx::execution_tree::primitive_result_type> f =
        or_.eval();

    HPX_TEST_EQ(
        (v1.norm() != 0.0) || (v2.norm() != 0.0),
        phylanx::execution_tree::extract_boolean_value(f.get()) != 0);
}

void test_or_operation_2d()
{
    Eigen::MatrixXd m1 = Eigen::MatrixXd::Random(101, 101);
    Eigen::MatrixXd m2 = Eigen::MatrixXd::Random(101, 101);

    phylanx::execution_tree::primitive lhs =
        hpx::new_<phylanx::execution_tree::primitives::variable>(
            hpx::find_here(), phylanx::ir::node_data<double>(m1));

    phylanx::execution_tree::primitive rhs =
        hpx::new_<phylanx::execution_tree::primitives::variable>(
            hpx::find_here(), phylanx::ir::node_data<double>(m2));

    phylanx::execution_tree::primitive or_ =
        hpx::new_<phylanx::execution_tree::primitives::or_operation>(
            hpx::find_here(),
            std::vector<phylanx::execution_tree::primitive_argument_type>{
                std::move(lhs), std::move(rhs)
            });

    hpx::future<phylanx::execution_tree::primitive_result_type> f =
        or_.eval();

    HPX_TEST_EQ(
        (m1.norm() != 0.0) || (m2.norm() != 0.0),
        phylanx::execution_tree::extract_boolean_value(f.get()) != 0);
}

void test_or_operation_2d_lit()
{
    Eigen::MatrixXd m1 = Eigen::MatrixXd::Random(101, 101);
    Eigen::MatrixXd m2 = Eigen::MatrixXd::Random(101, 101);

    phylanx::ir::node_data<double> lhs(m1);

    phylanx::execution_tree::primitive rhs =
        hpx::new_<phylanx::execution_tree::primitives::variable>(
            hpx::find_here(), phylanx::ir::node_data<double>(m2));

    phylanx::execution_tree::primitive or_ =
        hpx::new_<phylanx::execution_tree::primitives::or_operation>(
            hpx::find_here(),
            std::vector<phylanx::execution_tree::primitive_argument_type>{
                std::move(lhs), std::move(rhs)
            });

    hpx::future<phylanx::execution_tree::primitive_result_type> f =
        or_.eval();

    HPX_TEST_EQ(
        (m1.norm() != 0.0) || (m2.norm() != 0.0),
        phylanx::execution_tree::extract_boolean_value(f.get()) != 0);
}

int main(int argc, char* argv[])
{
    test_or_operation_0d_false();
    test_or_operation_0d_true();

    test_or_operation_0d_lit_false();
    test_or_operation_0d_lit_true();

    test_or_operation_1d();
    test_or_operation_1d_lit();

    test_or_operation_2d();
    test_or_operation_2d_lit();

    return hpx::util::report_errors();
}

