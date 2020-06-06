// Copyright (c) 2018 Bita Hasheminezhad
// Copyright (c) 2018 Hartmut Kaiser
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/phylanx.hpp>

#include <hpx/hpx_main.hpp>
#include <hpx/include/lcos.hpp>
#include <hpx/modules/testing.hpp>

#include <cstdint>
#include <iostream>
#include <string>
#include <utility>
#include <vector>

//////////////////////////////////////////////////////////////////////////
void test_sigmoid_operation_0d()
{
    phylanx::execution_tree::primitive arg =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(42.0));

    phylanx::execution_tree::primitive sigmoid =
        phylanx::execution_tree::primitives::create_sigmoid_operation(
            hpx::find_here(),
            phylanx::execution_tree::primitive_arguments_type{
                std::move(arg)});

    hpx::future<phylanx::execution_tree::primitive_argument_type> f =
        sigmoid.eval();

    auto result = phylanx::execution_tree::extract_numeric_value(f.get());
    HPX_TEST(allclose(phylanx::ir::node_data<double>(1.0), result));
}

void test_sigmoid_operation_1d()
{
    blaze::DynamicVector<double> subject{1., 2., 3.};

    phylanx::execution_tree::primitive arg =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(subject));

    phylanx::execution_tree::primitive sigmoid =
        phylanx::execution_tree::primitives::create_sigmoid_operation(
            hpx::find_here(),
            phylanx::execution_tree::primitive_arguments_type{std::move(arg)});

    hpx::future<phylanx::execution_tree::primitive_argument_type> f =
        sigmoid.eval();

    blaze::DynamicVector<double> expected{0.731059, 0.880797, 0.952574};

    auto result = phylanx::execution_tree::extract_numeric_value(f.get());
    HPX_TEST(
        allclose(phylanx::ir::node_data<double>(std::move(expected)), result));
}

void test_sigmoid_operation_2d()
{
    blaze::DynamicMatrix<std::int64_t> subject{{1, 2, 3}, {4, 1, 2}, {3, 4, 1}};

    phylanx::execution_tree::primitive arg =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<std::int64_t>(subject));

    phylanx::execution_tree::primitive sigmoid =
        phylanx::execution_tree::primitives::create_sigmoid_operation(
            hpx::find_here(),
            phylanx::execution_tree::primitive_arguments_type{std::move(arg)});

    hpx::future<phylanx::execution_tree::primitive_argument_type> f =
        sigmoid.eval();

    blaze::DynamicMatrix<double> expected{{0.731059, 0.880797, 0.952574},
        {0.982014, 0.731059, 0.880797}, {0.952574, 0.982014, 0.731059}};

    auto result = phylanx::execution_tree::extract_numeric_value(f.get());
    HPX_TEST(
        allclose(phylanx::ir::node_data<double>(std::move(expected)), result));
}

void test_sigmoid_operation_3d()
{
    blaze::DynamicTensor<double> subject{
        {{1.0, 2.0, 3.0}, {4.0, 1.0, 2.0}, {3.0, 4.0, 1.0}},
        {{3.0, 6.0, 2.0}, {-2.0, 2.0, 0.0}, {1.0, 1.0, 3.0}}};

    phylanx::execution_tree::primitive arg =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<std::int64_t>(subject));

    phylanx::execution_tree::primitive sigmoid =
        phylanx::execution_tree::primitives::create_sigmoid_operation(
            hpx::find_here(),
            phylanx::execution_tree::primitive_arguments_type{std::move(arg)});

    hpx::future<phylanx::execution_tree::primitive_argument_type> f =
        sigmoid.eval();

    blaze::DynamicTensor<double> expected{
        {{0.731059, 0.880797, 0.952574}, {0.982014, 0.731059, 0.880797},
            {0.952574, 0.982014, 0.731059}},
        {{0.952574, 0.997527, 0.880797}, {0.119203, 0.880797, 0.5},
            {0.731059, 0.731059, 0.952574}}};

    auto result = phylanx::execution_tree::extract_numeric_value(f.get());
    HPX_TEST(
        allclose(phylanx::ir::node_data<double>(std::move(expected)), result));
}

int main(int argc, char* argv[])
{
    test_sigmoid_operation_0d();
    test_sigmoid_operation_1d();
    test_sigmoid_operation_2d();
    test_sigmoid_operation_3d();

    return hpx::util::report_errors();
}
