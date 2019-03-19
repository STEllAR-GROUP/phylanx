// Copyright (c) 2018 Bita Hasheminezhad
// Copyright (c) 2018 Hartmut Kaiser
// Copyright (c) 2019 Jules Pénuchot
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/phylanx.hpp>

#include <hpx/hpx_main.hpp>
#include <hpx/include/lcos.hpp>
#include <hpx/util/lightweight_test.hpp>

#include <cstdint>
#include <iostream>
#include <string>
#include <utility>
#include <vector>

//////////////////////////////////////////////////////////////////////////
void test_elu_operation_0d()
{
    phylanx::execution_tree::primitive scal_0 =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(-3.0));

    phylanx::execution_tree::primitive alpha_0 =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(2.0));

    phylanx::execution_tree::primitive elu_0 =
        phylanx::execution_tree::primitives::create_elu_operation(
            hpx::find_here(),
            phylanx::execution_tree::primitive_arguments_type{
                std::move(scal_0), std::move(alpha_0)});

    hpx::future<phylanx::execution_tree::primitive_argument_type> f_0 =
        elu_0.eval();

    HPX_TEST_EQ(
        -1.9004258632642721,
        phylanx::execution_tree::extract_numeric_value(f_0.get())[0]);

    ////

    phylanx::execution_tree::primitive scal_1 =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(42.0));

    phylanx::execution_tree::primitive alpha_1 =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(1.0));

    phylanx::execution_tree::primitive elu_1 =
        phylanx::execution_tree::primitives::create_elu_operation(
            hpx::find_here(),
            phylanx::execution_tree::primitive_arguments_type{
                std::move(scal_1), std::move(alpha_1)});

    hpx::future<phylanx::execution_tree::primitive_argument_type> f_1 =
        elu_1.eval();

    HPX_TEST_EQ(
        42.0, phylanx::execution_tree::extract_numeric_value(f_1.get())[0]);

    ////

    phylanx::execution_tree::primitive scal_2 =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(-2.0));

    phylanx::execution_tree::primitive alpha_2 =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(1.0));

    phylanx::execution_tree::primitive elu_2 =
        phylanx::execution_tree::primitives::create_elu_operation(
            hpx::find_here(),
            phylanx::execution_tree::primitive_arguments_type{
                std::move(scal_2), std::move(alpha_2)});

    hpx::future<phylanx::execution_tree::primitive_argument_type> f_2 =
        elu_2.eval();

    HPX_TEST_EQ(
        -0.8646647167633873,
        phylanx::execution_tree::extract_numeric_value(f_2.get())[0]);
}

void test_elu_operation_1d()
{
    blaze::DynamicVector<double> subject{41., -4., 0.};

    phylanx::execution_tree::primitive arg =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(subject));

    phylanx::execution_tree::primitive alpha =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(2.0));

    phylanx::execution_tree::primitive elu =
        phylanx::execution_tree::primitives::create_elu_operation(
            hpx::find_here(),
            phylanx::execution_tree::primitive_arguments_type{std::move(arg),
                std::move(alpha)});

    hpx::future<phylanx::execution_tree::primitive_argument_type> f =
        elu.eval();

    blaze::DynamicVector<double> expected{41., -1.9633687222225316, 0.};

    HPX_TEST_EQ(phylanx::ir::node_data<double>(std::move(expected)),
        phylanx::execution_tree::extract_numeric_value(f.get()));
}

void test_elu_operation_2d()
{
    blaze::DynamicMatrix<double> subject{{-1., 2.,  3.},
                                         {-4., 1., -2.},
                                         {3. , 4., -1.}};

    phylanx::execution_tree::primitive mat =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(subject));

    phylanx::execution_tree::primitive alpha =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(2.0));

    phylanx::execution_tree::primitive elu =
        phylanx::execution_tree::primitives::create_elu_operation(
            hpx::find_here(),
            phylanx::execution_tree::primitive_arguments_type{std::move(mat),
                std::move(alpha)});

    hpx::future<phylanx::execution_tree::primitive_argument_type> f =
        elu.eval();

    blaze::DynamicMatrix<double> expected{
        {-1.2642411176571153, 2., 3                  },
        {-1.9633687222225316, 1., -1.7293294335267746},
        {3.                 , 4., -1.2642411176571153}};

    HPX_TEST_EQ(phylanx::ir::node_data<double>(std::move(expected)),
        phylanx::execution_tree::extract_numeric_value(f.get()));
}

#if defined(PHYLANX_HAVE_BLAZE_TENSOR)
void test_elu_operation_3d()
{
    blaze::DynamicTensor<double> subject{
        {{1.0,  2.0, 3.0},
        {-4.0, -1.0, 2.0},
        {-3.0, -4.0, 1.0}},

        {{3.0, 6.0,  2.0},
        {-2.0, 2.0,  0.0},
        { 1.0, 1.0, -3.0}}};

    phylanx::execution_tree::primitive arg =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(subject));

    phylanx::execution_tree::primitive alpha =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(2.0));

    phylanx::execution_tree::primitive elu =
        phylanx::execution_tree::primitives::create_elu_operation(
            hpx::find_here(),
            phylanx::execution_tree::primitive_arguments_type{std::move(arg),
                std::move(alpha)});

    hpx::future<phylanx::execution_tree::primitive_argument_type> f =
        elu.eval();

    blaze::DynamicTensor<double> expected{
        {{1.0               , 2.0                , 3.0},
        {-1.9633687222225316, -1.2642411176571153, 2.0},
        {-1.9004258632642721, -1.9633687222225316, 1.0}},

        {{3.0               , 6.0, 2.0                  },
        {-1.7293294335267746, 2.0, 0.0                  },
        { 1.0               , 1.0, -1.9004258632642721}}};

    HPX_TEST_EQ(phylanx::ir::node_data<double>(std::move(expected)),
        phylanx::execution_tree::extract_numeric_value(f.get()));
}
#endif

int main(int argc, char* argv[])
{
    test_elu_operation_0d();
    test_elu_operation_1d();
    test_elu_operation_2d();

#if defined(PHYLANX_HAVE_BLAZE_TENSOR)
    test_elu_operation_3d();
#endif

    return hpx::util::report_errors();
}
