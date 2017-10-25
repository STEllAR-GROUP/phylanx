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

void test_and_operation_0d_false()
{
    phylanx::execution_tree::primitive and_ =
        hpx::new_<phylanx::execution_tree::primitives::and_operation>(
            hpx::find_here(),
            std::vector<phylanx::execution_tree::primitive_argument_type>{
        true, false
    });

    hpx::future<phylanx::execution_tree::primitive_result_type> f =
        and_.eval();

    HPX_TEST(!phylanx::execution_tree::extract_boolean_value(f.get()));
}

void test_and_operation_0d_true()
{
    phylanx::execution_tree::primitive and_ =
        hpx::new_<phylanx::execution_tree::primitives::and_operation>(
            hpx::find_here(),
            std::vector<phylanx::execution_tree::primitive_argument_type>{
        true, true
    });

    hpx::future<phylanx::execution_tree::primitive_result_type> f =
        and_.eval();

    HPX_TEST(phylanx::execution_tree::extract_boolean_value(f.get()));
}

void test_and_operation_0d_lit_false()
{
    phylanx::ir::node_data<double> lhs(0.0);

    phylanx::execution_tree::primitive rhs =
        hpx::new_<phylanx::execution_tree::primitives::variable>(
            hpx::find_here(), phylanx::ir::node_data<double>(1.0));

    phylanx::execution_tree::primitive and_ =
        hpx::new_<phylanx::execution_tree::primitives::and_operation>(
            hpx::find_here(),
            std::vector<phylanx::execution_tree::primitive_argument_type>{
        std::move(lhs), std::move(rhs)
    });

    hpx::future<phylanx::execution_tree::primitive_result_type> f =
        and_.eval();

    HPX_TEST(!phylanx::execution_tree::extract_boolean_value(f.get()));
}

void test_and_operation_0d_lit_true()
{
    phylanx::ir::node_data<double> lhs(41.0);

    phylanx::execution_tree::primitive rhs =
        hpx::new_<phylanx::execution_tree::primitives::variable>(
            hpx::find_here(), phylanx::ir::node_data<double>(1.0));

    phylanx::execution_tree::primitive and_ =
        hpx::new_<phylanx::execution_tree::primitives::and_operation>(
            hpx::find_here(),
            std::vector<phylanx::execution_tree::primitive_argument_type>{
        std::move(lhs), std::move(rhs)
    });

    hpx::future<phylanx::execution_tree::primitive_result_type> f =
        and_.eval();

    HPX_TEST(phylanx::execution_tree::extract_boolean_value(f.get()));
}

void test_and_operation_1d()
{
    blaze::Rand<blaze::DynamicVector<double, blaze::rowVector>> gen{};
    blaze::DynamicVector<double, blaze::rowVector> v1 = gen.generate(1007UL);
    blaze::DynamicVector<double, blaze::rowVector> v2 = gen.generate(1007UL);

    phylanx::execution_tree::primitive lhs =
        hpx::new_<phylanx::execution_tree::primitives::variable>(
            hpx::find_here(), phylanx::ir::node_data<double>(v1));

    phylanx::execution_tree::primitive rhs =
        hpx::new_<phylanx::execution_tree::primitives::variable>(
            hpx::find_here(), phylanx::ir::node_data<double>(v2));

    phylanx::execution_tree::primitive and_ =
        hpx::new_<phylanx::execution_tree::primitives::and_operation>(
            hpx::find_here(),
            std::vector<phylanx::execution_tree::primitive_argument_type>{
        std::move(lhs), std::move(rhs)
    });

    hpx::future<phylanx::execution_tree::primitive_result_type> f =
        and_.eval();

    HPX_TEST_EQ(
        (!blaze::isZero(v1)) && (!blaze::isZero(v2)),
        phylanx::execution_tree::extract_boolean_value(f.get()) != 0);
}

void test_and_operation_1d_lit()
{
    blaze::Rand<blaze::DynamicVector<double, blaze::rowVector>> gen;
    blaze::DynamicVector<double, blaze::rowVector> v1 = gen.generate(1007UL);
    blaze::DynamicVector<double, blaze::rowVector> v2 = gen.generate(1007UL);

    phylanx::ir::node_data<double> lhs(v1);

    phylanx::execution_tree::primitive rhs =
        hpx::new_<phylanx::execution_tree::primitives::variable>(
            hpx::find_here(), phylanx::ir::node_data<double>(v2));

    phylanx::execution_tree::primitive and_ =
        hpx::new_<phylanx::execution_tree::primitives::and_operation>(
            hpx::find_here(),
            std::vector<phylanx::execution_tree::primitive_argument_type>{
        std::move(lhs), std::move(rhs)
    });

    hpx::future<phylanx::execution_tree::primitive_result_type> f =
        and_.eval();

    HPX_TEST_EQ(
        (!blaze::isZero(v1)) && (!blaze::isZero(v2)),
        phylanx::execution_tree::extract_boolean_value(f.get()) != 0);
}

void test_and_operation_2d()
{
    blaze::Rand<blaze::DynamicMatrix<double>> gen;
    blaze::DynamicMatrix<double> m1 = gen.generate(101UL, 101UL);
    blaze::DynamicMatrix<double> m2 = gen.generate(101UL, 101UL);

    phylanx::execution_tree::primitive lhs =
        hpx::new_<phylanx::execution_tree::primitives::variable>(
            hpx::find_here(), phylanx::ir::node_data<double>(m1));

    phylanx::execution_tree::primitive rhs =
        hpx::new_<phylanx::execution_tree::primitives::variable>(
            hpx::find_here(), phylanx::ir::node_data<double>(m2));

    phylanx::execution_tree::primitive and_ =
        hpx::new_<phylanx::execution_tree::primitives::and_operation>(
            hpx::find_here(),
            std::vector<phylanx::execution_tree::primitive_argument_type>{
        std::move(lhs), std::move(rhs)
    });

    hpx::future<phylanx::execution_tree::primitive_result_type> f =
        and_.eval();

    HPX_TEST_EQ(
        (!blaze::isZero(m1)) && (!blaze::isZero(m2)),
        phylanx::execution_tree::extract_boolean_value(f.get()) != 0);
}

void test_and_operation_2d_lit()
{
    blaze::Rand<blaze::DynamicMatrix<double>> gen;
    blaze::DynamicMatrix<double> m1 = gen.generate(101UL, 101UL);
    blaze::DynamicMatrix<double> m2 = gen.generate(101UL, 101UL);

    phylanx::ir::node_data<double> lhs(m1);

    phylanx::execution_tree::primitive rhs =
        hpx::new_<phylanx::execution_tree::primitives::variable>(
            hpx::find_here(), phylanx::ir::node_data<double>(m2));

    phylanx::execution_tree::primitive and_ =
        hpx::new_<phylanx::execution_tree::primitives::and_operation>(
            hpx::find_here(),
            std::vector<phylanx::execution_tree::primitive_argument_type>{
        std::move(lhs), std::move(rhs)
    });

    hpx::future<phylanx::execution_tree::primitive_result_type> f =
        and_.eval();

    HPX_TEST_EQ(
        (!blaze::isZero(m1)) && (!blaze::isZero(m2)),
        phylanx::execution_tree::extract_boolean_value(f.get()) != 0);
}

int main(int argc, char* argv[])
{
    test_and_operation_0d_false();
    test_and_operation_0d_true();

    test_and_operation_0d_lit_false();
    test_and_operation_0d_lit_true();

    test_and_operation_1d();
    test_and_operation_1d_lit();

    test_and_operation_2d();
    test_and_operation_2d_lit();

    return hpx::util::report_errors();
}
