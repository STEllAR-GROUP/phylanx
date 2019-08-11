// Copyright (c) 2018 Bita Hasheminezhad
// Copyright (c) 2018 Hartmut Kaiser
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/phylanx.hpp>

#include <hpx/hpx_main.hpp>
#include <hpx/include/lcos.hpp>
#include <hpx/testing.hpp>

#include <cstdint>
#include <iostream>
#include <string>
#include <utility>
#include <vector>

//////////////////////////////////////////////////////////////////////////
void test_hard_sigmoid_operation_0d()
{
    phylanx::execution_tree::primitive arg =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(42.0));

    phylanx::execution_tree::primitive hard_sigmoid =
        phylanx::execution_tree::primitives::create_hard_sigmoid_operation(
            hpx::find_here(),
            phylanx::execution_tree::primitive_arguments_type{
                std::move(arg)});

    hpx::future<phylanx::execution_tree::primitive_argument_type> f =
        hard_sigmoid.eval();

    auto result = phylanx::execution_tree::extract_numeric_value(f.get());
    HPX_TEST(allclose(phylanx::ir::node_data<double>(1.0), result));
}

void test_hard_sigmoid_operation_1d()
{
    blaze::DynamicVector<double> subject{1., 2., 3.};

    phylanx::execution_tree::primitive arg =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(subject));

    phylanx::execution_tree::primitive hard_sigmoid =
        phylanx::execution_tree::primitives::create_hard_sigmoid_operation(
            hpx::find_here(),
            phylanx::execution_tree::primitive_arguments_type{std::move(arg)});

    hpx::future<phylanx::execution_tree::primitive_argument_type> f =
        hard_sigmoid.eval();

    blaze::DynamicVector<double> expected{0.7, 0.9, 1.};

    auto result = phylanx::execution_tree::extract_numeric_value(f.get());
    HPX_TEST(
        allclose(phylanx::ir::node_data<double>(std::move(expected)), result));
}

void test_hard_sigmoid_operation_2d()
{
    blaze::DynamicMatrix<std::int64_t> subject{{1, 2, 3}, {4, 1, 2}, {3, 4, 1}};

    phylanx::execution_tree::primitive arg =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<std::int64_t>(subject));

    phylanx::execution_tree::primitive hard_sigmoid =
        phylanx::execution_tree::primitives::create_hard_sigmoid_operation(
            hpx::find_here(),
            phylanx::execution_tree::primitive_arguments_type{std::move(arg)});

    hpx::future<phylanx::execution_tree::primitive_argument_type> f =
        hard_sigmoid.eval();

    blaze::DynamicMatrix<double> expected{
        {0.7, 0.9, 1.}, {1., 0.7, 0.9}, {1., 1., 0.7}};

    auto result = phylanx::execution_tree::extract_numeric_value(f.get());
    HPX_TEST(
        allclose(phylanx::ir::node_data<double>(std::move(expected)), result));
}

void test_hard_sigmoid_operation_3d()
{
    blaze::DynamicTensor<double> subject{
        {{1.0, 2.0, 3.0}, {4.0, 1.0, 2.0}, {3.0, 4.0, 1.0}},
        {{3.0, 6.0, 2.0}, {-2.0, 2.0, 0.0}, {1.0, 1.0, 3.0}}};

    phylanx::execution_tree::primitive arg =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<std::int64_t>(subject));

    phylanx::execution_tree::primitive hard_sigmoid =
        phylanx::execution_tree::primitives::create_hard_sigmoid_operation(
            hpx::find_here(),
            phylanx::execution_tree::primitive_arguments_type{std::move(arg)});

    hpx::future<phylanx::execution_tree::primitive_argument_type> f =
        hard_sigmoid.eval();

    blaze::DynamicTensor<double> expected{
        {{0.7, .9, 1.}, {1., 0.7, 0.9}, {1., 1., 0.7}},
        {{1., 1., 0.9}, {0.1, 0.9, 0.5}, {0.7, 0.7, 1.}}};

    auto result = phylanx::execution_tree::extract_numeric_value(f.get());
    HPX_TEST(
        allclose(phylanx::ir::node_data<double>(std::move(expected)), result));
}

int main(int argc, char* argv[])
{
    test_hard_sigmoid_operation_0d();
    test_hard_sigmoid_operation_1d();
    test_hard_sigmoid_operation_2d();
    test_hard_sigmoid_operation_3d();

    return hpx::util::report_errors();
}
