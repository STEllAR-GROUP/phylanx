//   Copyright (c) 2017 Hartmut Kaiser
//   Copyright (c) 2017 Alireza Kheirkhahan
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

void test_div_operation_0d()
{
    phylanx::execution_tree::primitive lhs =
        hpx::new_<phylanx::execution_tree::primitives::literal_value>(
            hpx::find_here(), phylanx::ir::node_data<double>(42.0));

    phylanx::execution_tree::primitive rhs =
        hpx::new_<phylanx::execution_tree::primitives::literal_value>(
            hpx::find_here(), phylanx::ir::node_data<double>(7.0));

    phylanx::execution_tree::primitive div =
        hpx::new_<phylanx::execution_tree::primitives::div_operation>(
            hpx::find_here(),
            std::vector<phylanx::execution_tree::primitive_argument_type>{
                std::move(lhs), std::move(rhs)
            });

    hpx::future<phylanx::execution_tree::primitive_result_type> f =
        div.eval();
    HPX_TEST_EQ(
        6.0, phylanx::execution_tree::extract_numeric_value(f.get())[0]);
}

void test_div_operation_0d_lit()
{
    phylanx::ir::node_data<double> lhs(42.0);

    phylanx::execution_tree::primitive rhs =
        hpx::new_<phylanx::execution_tree::primitives::literal_value>(
            hpx::find_here(), phylanx::ir::node_data<double>(7.0));

    phylanx::execution_tree::primitive div =
        hpx::new_<phylanx::execution_tree::primitives::div_operation>(
            hpx::find_here(),
            std::vector<phylanx::execution_tree::primitive_argument_type>{
                std::move(lhs), std::move(rhs)
            });

    hpx::future<phylanx::execution_tree::primitive_result_type> f =
        div.eval();
    HPX_TEST_EQ(
        6.0, phylanx::execution_tree::extract_numeric_value(f.get())[0]);
}

void test_div_operation_0d1d()
{
    Eigen::VectorXd v = Eigen::VectorXd::Random(1007);

    phylanx::execution_tree::primitive lhs =
        hpx::new_<phylanx::execution_tree::primitives::literal_value>(
            hpx::find_here(), phylanx::ir::node_data<double>(42.0));

    phylanx::execution_tree::primitive rhs =
        hpx::new_<phylanx::execution_tree::primitives::literal_value>(
            hpx::find_here(), phylanx::ir::node_data<double>(v));

    phylanx::execution_tree::primitive div =
        hpx::new_<phylanx::execution_tree::primitives::div_operation>(
            hpx::find_here(),
            std::vector<phylanx::execution_tree::primitive_argument_type>{
                std::move(lhs), std::move(rhs)
            });

    hpx::future<phylanx::execution_tree::primitive_result_type> f =
        div.eval();

    Eigen::VectorXd expected = 42.0 / v.array();
    HPX_TEST_EQ(
        phylanx::ir::node_data<double>(std::move(expected)),
        phylanx::execution_tree::extract_numeric_value(f.get()));
}

void test_div_operation_0d1d_lit()
{
    Eigen::VectorXd v = Eigen::VectorXd::Random(1007);

    phylanx::ir::node_data<double> lhs(42.0);

    phylanx::execution_tree::primitive rhs =
        hpx::new_<phylanx::execution_tree::primitives::literal_value>(
            hpx::find_here(), phylanx::ir::node_data<double>(v));

    phylanx::execution_tree::primitive div =
        hpx::new_<phylanx::execution_tree::primitives::div_operation>(
            hpx::find_here(),
            std::vector<phylanx::execution_tree::primitive_argument_type>{
                std::move(lhs), std::move(rhs)
            });

    hpx::future<phylanx::execution_tree::primitive_result_type> f =
        div.eval();

    Eigen::VectorXd expected = 42.0 / v.array();
    HPX_TEST_EQ(
        phylanx::ir::node_data<double>(std::move(expected)),
        phylanx::execution_tree::extract_numeric_value(f.get()));
}

void test_div_operation_0d2d()
{
    Eigen::MatrixXd m = Eigen::MatrixXd::Random(101, 101);

    phylanx::execution_tree::primitive lhs =
        hpx::new_<phylanx::execution_tree::primitives::literal_value>(
            hpx::find_here(), phylanx::ir::node_data<double>(6.0));

    phylanx::execution_tree::primitive rhs =
        hpx::new_<phylanx::execution_tree::primitives::literal_value>(
            hpx::find_here(), phylanx::ir::node_data<double>(m));

    phylanx::execution_tree::primitive div =
        hpx::new_<phylanx::execution_tree::primitives::div_operation>(
            hpx::find_here(),
            std::vector<phylanx::execution_tree::primitive_argument_type>{
                std::move(lhs), std::move(rhs)
            });

    hpx::future<phylanx::execution_tree::primitive_result_type> f =
        div.eval();

    Eigen::MatrixXd expected = 6.0 / m.array();
    HPX_TEST_EQ(
        phylanx::ir::node_data<double>(std::move(expected)),
        phylanx::execution_tree::extract_numeric_value(f.get()));
}

void test_div_operation_0d2d_lit()
{
    Eigen::MatrixXd m = Eigen::MatrixXd::Random(101, 101);

    phylanx::ir::node_data<double> lhs(6.0);

    phylanx::execution_tree::primitive rhs =
        hpx::new_<phylanx::execution_tree::primitives::literal_value>(
            hpx::find_here(), phylanx::ir::node_data<double>(m));

    phylanx::execution_tree::primitive div =
        hpx::new_<phylanx::execution_tree::primitives::div_operation>(
            hpx::find_here(),
            std::vector<phylanx::execution_tree::primitive_argument_type>{
                std::move(lhs), std::move(rhs)
            });

    hpx::future<phylanx::execution_tree::primitive_result_type> f =
        div.eval();

    Eigen::MatrixXd expected = 6.0 / m.array();
    HPX_TEST_EQ(
        phylanx::ir::node_data<double>(std::move(expected)),
        phylanx::execution_tree::extract_numeric_value(f.get()));
}

void test_div_operation_1d0d()
{
    Eigen::VectorXd v = Eigen::VectorXd::Random(1007);

    phylanx::execution_tree::primitive lhs =
        hpx::new_<phylanx::execution_tree::primitives::literal_value>(
            hpx::find_here(), phylanx::ir::node_data<double>(v));

    phylanx::execution_tree::primitive rhs =
        hpx::new_<phylanx::execution_tree::primitives::literal_value>(
            hpx::find_here(), phylanx::ir::node_data<double>(6.0));

    phylanx::execution_tree::primitive div =
        hpx::new_<phylanx::execution_tree::primitives::div_operation>(
            hpx::find_here(),
            std::vector<phylanx::execution_tree::primitive_argument_type>{
                std::move(lhs), std::move(rhs)
            });

    hpx::future<phylanx::execution_tree::primitive_result_type> f =
        div.eval();

    Eigen::VectorXd expected = v.array() / 6.0;
    HPX_TEST_EQ(
        phylanx::ir::node_data<double>(std::move(expected)),
        phylanx::execution_tree::extract_numeric_value(f.get()));
}

void test_div_operation_1d0d_lit()
{
    Eigen::VectorXd v = Eigen::VectorXd::Random(1007);

    phylanx::ir::node_data<double> lhs(v);

    phylanx::execution_tree::primitive rhs =
        hpx::new_<phylanx::execution_tree::primitives::literal_value>(
            hpx::find_here(), phylanx::ir::node_data<double>(6.0));

    phylanx::execution_tree::primitive div =
        hpx::new_<phylanx::execution_tree::primitives::div_operation>(
            hpx::find_here(),
            std::vector<phylanx::execution_tree::primitive_argument_type>{
                std::move(lhs), std::move(rhs)
            });

    hpx::future<phylanx::execution_tree::primitive_result_type> f =
        div.eval();

    Eigen::VectorXd expected = v.array() / 6.0;
    HPX_TEST_EQ(
        phylanx::ir::node_data<double>(std::move(expected)),
        phylanx::execution_tree::extract_numeric_value(f.get()));
}

void test_div_operation_2d0d()
{
    Eigen::MatrixXd m = Eigen::MatrixXd::Random(42, 42);

    phylanx::execution_tree::primitive lhs =
        hpx::new_<phylanx::execution_tree::primitives::literal_value>(
            hpx::find_here(), phylanx::ir::node_data<double>(m));

    phylanx::execution_tree::primitive rhs =
        hpx::new_<phylanx::execution_tree::primitives::literal_value>(
            hpx::find_here(), phylanx::ir::node_data<double>(6.0));

    phylanx::execution_tree::primitive div =
        hpx::new_<phylanx::execution_tree::primitives::div_operation>(
            hpx::find_here(),
            std::vector<phylanx::execution_tree::primitive_argument_type>{
                std::move(lhs), std::move(rhs)
            });

    hpx::future<phylanx::execution_tree::primitive_result_type> f =
        div.eval();

    Eigen::MatrixXd expected = m.array() / 6.0;
    HPX_TEST_EQ(
        phylanx::ir::node_data<double>(std::move(expected)),
        phylanx::execution_tree::extract_numeric_value(f.get()));
}

void test_div_operation_2d0d_lit()
{
    Eigen::MatrixXd m = Eigen::MatrixXd::Random(42, 42);

    phylanx::ir::node_data<double> lhs(m);

    phylanx::execution_tree::primitive rhs =
        hpx::new_<phylanx::execution_tree::primitives::literal_value>(
            hpx::find_here(), phylanx::ir::node_data<double>(6.0));

    phylanx::execution_tree::primitive div =
        hpx::new_<phylanx::execution_tree::primitives::div_operation>(
            hpx::find_here(),
            std::vector<phylanx::execution_tree::primitive_argument_type>{
                std::move(lhs), std::move(rhs)
            });

    hpx::future<phylanx::execution_tree::primitive_result_type> f =
        div.eval();

    Eigen::MatrixXd expected = m.array() / 6.0;
    HPX_TEST_EQ(
        phylanx::ir::node_data<double>(std::move(expected)),
        phylanx::execution_tree::extract_numeric_value(f.get()));
}

void test_div_operation_2d()
{
    Eigen::MatrixXd m1 = Eigen::MatrixXd::Random(42, 42);
    Eigen::MatrixXd m2 = Eigen::MatrixXd::Random(42, 42);

    phylanx::execution_tree::primitive lhs =
        hpx::new_<phylanx::execution_tree::primitives::literal_value>(
            hpx::find_here(), phylanx::ir::node_data<double>(m1));

    phylanx::execution_tree::primitive rhs =
        hpx::new_<phylanx::execution_tree::primitives::literal_value>(
            hpx::find_here(), phylanx::ir::node_data<double>(m2));

    phylanx::execution_tree::primitive div =
        hpx::new_<phylanx::execution_tree::primitives::div_operation>(
            hpx::find_here(),
            std::vector<phylanx::execution_tree::primitive_argument_type>{
                std::move(lhs), std::move(rhs)
            });

    hpx::future<phylanx::execution_tree::primitive_result_type> f =
        div.eval();

    Eigen::MatrixXd expected = m1.array() / m2.array();
    HPX_TEST_EQ(
        phylanx::ir::node_data<double>(std::move(expected)),
        phylanx::execution_tree::extract_numeric_value(f.get()));
}

void test_div_operation_2d_lit()
{
    Eigen::MatrixXd m1 = Eigen::MatrixXd::Random(42, 42);
    Eigen::MatrixXd m2 = Eigen::MatrixXd::Random(42, 42);

    phylanx::ir::node_data<double> lhs(m1);

    phylanx::execution_tree::primitive rhs =
        hpx::new_<phylanx::execution_tree::primitives::literal_value>(
            hpx::find_here(), phylanx::ir::node_data<double>(m2));

    phylanx::execution_tree::primitive div =
        hpx::new_<phylanx::execution_tree::primitives::div_operation>(
            hpx::find_here(),
            std::vector<phylanx::execution_tree::primitive_argument_type>{
                std::move(lhs), std::move(rhs)
            });

    hpx::future<phylanx::execution_tree::primitive_result_type> f =
        div.eval();

    Eigen::MatrixXd expected = m1.array() / m2.array();
    HPX_TEST_EQ(
        phylanx::ir::node_data<double>(std::move(expected)),
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

    test_div_operation_2d0d();
    test_div_operation_2d0d_lit();

    test_div_operation_2d();
    test_div_operation_2d_lit();

    return hpx::util::report_errors();
}

