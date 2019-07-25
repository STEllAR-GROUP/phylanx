// Copyright (c) 2018 Bibek Wagle
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/phylanx.hpp>

#include <hpx/hpx_main.hpp>
#include <hpx/include/lcos.hpp>
#include <hpx/testing.hpp>

#include <utility>
#include <vector>

void test_gradient_operation_1d()
{
    blaze::DynamicVector<double> v1{1.0, 2.0, 4.0, 7.0, 11.0, 16.0};

    phylanx::execution_tree::primitive vec =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(v1));

    phylanx::execution_tree::primitive gradient =
        phylanx::execution_tree::primitives::create_gradient_operation(
            hpx::find_here(),
            phylanx::execution_tree::primitive_arguments_type{
                std::move(vec)});

    hpx::future<phylanx::execution_tree::primitive_argument_type> f =
        gradient.eval();

    blaze::DynamicVector<double> expected{1., 1.5, 2.5, 3.5, 4.5, 5.};

    HPX_TEST_EQ(phylanx::ir::node_data<double>(std::move(expected)),
        phylanx::execution_tree::extract_numeric_value(f.get()));
}

void test_gradient_operation_2d_axis_zero()
{
    blaze::DynamicMatrix<double> m1{{90, 54, 57, 60, 63}, {51, 96, 57, 60, 63},
        {51, 54, 102, 60, 63}, {51, 54, 57, 108, 63}, {51, 54, 57, 60, 114}};

    phylanx::execution_tree::primitive mat =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(m1));

    phylanx::execution_tree::primitive k =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(0.0));

    phylanx::execution_tree::primitive gradient =
        phylanx::execution_tree::primitives::create_gradient_operation(
            hpx::find_here(),
            phylanx::execution_tree::primitive_arguments_type{
                std::move(mat), std::move(k)});

    hpx::future<phylanx::execution_tree::primitive_argument_type> f =
        gradient.eval();

    blaze::DynamicMatrix<double> expected{{-39., 42., 0., 0., 0.},
        {-19.5, 0., 22.5, 0., 0.}, {0., -21., 0., 24., 0.},
        {0., 0., -22.5, 0., 25.5}, {0., 0., 0., -48., 51.}};

    HPX_TEST_EQ(phylanx::ir::node_data<double>(std::move(expected)),
        phylanx::execution_tree::extract_numeric_value(f.get()));
}

void test_gradient_operation_2d_axis_one()
{
    blaze::DynamicMatrix<double> m1{{90, 54, 57, 60, 63}, {51, 96, 57, 60, 63},
        {51, 54, 102, 60, 63}, {51, 54, 57, 108, 63}, {51, 54, 57, 60, 114}};

    phylanx::execution_tree::primitive mat =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(m1));

    phylanx::execution_tree::primitive k =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(1.0));

    phylanx::execution_tree::primitive gradient =
        phylanx::execution_tree::primitives::create_gradient_operation(
            hpx::find_here(),
            phylanx::execution_tree::primitive_arguments_type{
                std::move(mat), std::move(k)});

    hpx::future<phylanx::execution_tree::primitive_argument_type> f =
        gradient.eval();

    blaze::DynamicMatrix<double> expected{{-36., -16.5, 3., 3., 3.},
        {45., 3., -18., 3., 3.}, {3., 25.5, 3., -19.5, 3.},
        {3., 3., 27., 3., -45.}, {3., 3., 3., 28.5, 54.}};

    HPX_TEST_EQ(phylanx::ir::node_data<double>(std::move(expected)),
        phylanx::execution_tree::extract_numeric_value(f.get()));
}

void test_gradient_operation_2d_no_axis_given()
{
    blaze::DynamicMatrix<double> m1{{90, 54, 57, 60, 63}, {51, 96, 57, 60, 63},
        {51, 54, 102, 60, 63}, {51, 54, 57, 108, 63}, {51, 54, 57, 60, 114}};

    phylanx::execution_tree::primitive mat =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(m1));

    phylanx::execution_tree::primitive gradient =
        phylanx::execution_tree::primitives::create_gradient_operation(
            hpx::find_here(),
            phylanx::execution_tree::primitive_arguments_type{
                std::move(mat)});

    hpx::future<phylanx::execution_tree::primitive_argument_type> f =
        gradient.eval();

    blaze::DynamicMatrix<double> expected_0{{-39., 42., 0., 0., 0.},
        {-19.5, 0., 22.5, 0., 0.}, {0., -21., 0., 24., 0.},
        {0., 0., -22.5, 0., 25.5}, {0., 0., 0., -48., 51.}};

    blaze::DynamicMatrix<double> expected_1{{-36., -16.5, 3., 3., 3.},
        {45., 3., -18., 3., 3.}, {3., 25.5, 3., -19.5, 3.},
        {3., 3., 27., 3., -45.}, {3., 3., 3., 28.5, 54.}};

    auto result = phylanx::execution_tree::extract_list_value(f.get());

    auto it = result.begin();
    HPX_TEST_EQ(phylanx::ir::node_data<double>{expected_0}, *it++);
    HPX_TEST_EQ(phylanx::ir::node_data<double>{expected_1}, *it);
}

int main(int argc, char* argv[])
{
    test_gradient_operation_1d();
    test_gradient_operation_2d_axis_zero();
    test_gradient_operation_2d_axis_one();

    test_gradient_operation_2d_no_axis_given();

    return hpx::util::report_errors();
}
