//   Copyright (c) 2017 Parsa Amini
//
//   Distributed under the Boost Software License, Version 1.0. (See accompanying
//   file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/phylanx.hpp>

#include <hpx/hpx_main.hpp>
#include <hpx/include/lcos.hpp>
#include <hpx/testing.hpp>

#include <cstdint>
#include <iostream>
#include <utility>
#include <vector>

#include <blaze/Math.h>
#include <blaze_tensor/Math.h>

void test_power_operation_0d()
{
    phylanx::ir::node_data<double> lhs(2.0);

    phylanx::execution_tree::primitive rhs =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(),
            phylanx::execution_tree::primitive_argument_type{7.0});

    phylanx::execution_tree::primitive power =
        phylanx::execution_tree::primitives::create_power_operation(
            hpx::find_here(),
            phylanx::execution_tree::primitive_arguments_type{
                phylanx::execution_tree::primitive_argument_type{std::move(lhs)},
                phylanx::execution_tree::primitive_argument_type{std::move(rhs)}
            });

    hpx::future<phylanx::execution_tree::primitive_argument_type> f =
        power.eval();
    HPX_TEST_EQ(
        128.0, phylanx::execution_tree::extract_numeric_value(f.get())[0]);
}

void test_power_operation_1d()
{
    blaze::DynamicVector<double> v1{
        1.0, 2.0, 3.0, 4.0, 5.0, 6.0};

    phylanx::execution_tree::primitive lhs =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::execution_tree::primitive_argument_type{
                phylanx::ir::node_data<double>(v1)});

    phylanx::execution_tree::primitive rhs =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(),
            phylanx::execution_tree::primitive_argument_type{2.0});

    phylanx::execution_tree::primitive power =
        phylanx::execution_tree::primitives::create_power_operation(
            hpx::find_here(),
            phylanx::execution_tree::primitive_arguments_type{
                phylanx::execution_tree::primitive_argument_type{std::move(lhs)},
                phylanx::execution_tree::primitive_argument_type{std::move(rhs)}
            });

    hpx::future<phylanx::execution_tree::primitive_argument_type> f =
        power.eval();
    blaze::DynamicVector<double> expected = blaze::pow(v1, 2.0);

    HPX_TEST_EQ(phylanx::ir::node_data<double>(std::move(expected)),
        phylanx::execution_tree::extract_numeric_value(f.get()));
}

void test_power_operation_2d()
{
    blaze::Rand<blaze::DynamicMatrix<double>> gen{};
    blaze::DynamicMatrix<double> m1 = gen.generate(42UL, 42UL);

    phylanx::execution_tree::primitive lhs =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::execution_tree::primitive_argument_type{
                phylanx::ir::node_data<double>(m1)});

    phylanx::execution_tree::primitive rhs =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(),
            phylanx::execution_tree::primitive_argument_type{2.0});

    phylanx::execution_tree::primitive power =
        phylanx::execution_tree::primitives::create_power_operation(
            hpx::find_here(),
            phylanx::execution_tree::primitive_arguments_type{
                phylanx::execution_tree::primitive_argument_type{std::move(lhs)},
                phylanx::execution_tree::primitive_argument_type{std::move(rhs)}
            });

    hpx::future<phylanx::execution_tree::primitive_argument_type> f =
        power.eval();

    blaze::DynamicMatrix<double> expected = blaze::pow(m1, 2.0);

    HPX_TEST_EQ(phylanx::ir::node_data<double>(std::move(expected)),
        phylanx::execution_tree::extract_numeric_value(f.get()));
}

void test_power_operation_3d()
{
    blaze::Rand<blaze::DynamicTensor<double>> gen{};
    blaze::DynamicTensor<double> t = gen.generate(11UL, 42UL, 42UL);

    phylanx::execution_tree::primitive lhs =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::execution_tree::primitive_argument_type{
                phylanx::ir::node_data<double>(t)});

    phylanx::execution_tree::primitive rhs =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(),
            phylanx::execution_tree::primitive_argument_type{2.0});

    phylanx::execution_tree::primitive power =
        phylanx::execution_tree::primitives::create_power_operation(
            hpx::find_here(),
            phylanx::execution_tree::primitive_arguments_type{
                phylanx::execution_tree::primitive_argument_type{std::move(lhs)},
                phylanx::execution_tree::primitive_argument_type{std::move(rhs)}
            });

    hpx::future<phylanx::execution_tree::primitive_argument_type> f =
        power.eval();

    blaze::DynamicTensor<double> expected = blaze::pow(t, 2.0);

    HPX_TEST_EQ(phylanx::ir::node_data<double>(std::move(expected)),
        phylanx::execution_tree::extract_numeric_value(f.get()));
}

int main(int argc, char* argv[])
{
    test_power_operation_0d();
    test_power_operation_1d();
    test_power_operation_2d();
    test_power_operation_3d();

    return hpx::util::report_errors();
}

