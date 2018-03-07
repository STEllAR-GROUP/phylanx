// Copyright (c) 2017-2018 Monil, Mohammad Alaul Haque
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/phylanx.hpp>

#include <hpx/hpx_main.hpp>
#include <hpx/include/lcos.hpp>
#include <hpx/util/lightweight_test.hpp>

#include <iostream>
#include <utility>
#include <vector>

void test_mean_operation_0d()
{
    phylanx::execution_tree::primitive first =
            phylanx::execution_tree::primitives::create_variable(
                    hpx::find_here(), phylanx::ir::node_data<double>(42.0));

    phylanx::execution_tree::primitive mean =
            phylanx::execution_tree::primitives::create_mean_operation(
                    hpx::find_here(),
                    std::vector<phylanx::execution_tree::primitive_argument_type>{
                            std::move(first)
                    });

    hpx::future<phylanx::execution_tree::primitive_argument_type> f =
            mean.eval();

    HPX_TEST_EQ(
            42.0, phylanx::execution_tree::extract_numeric_value(f.get())[0]);
}

void test_mean_operation_1d()
{
    blaze::DynamicVector<double> v1{ 1.0, 2.0, 3.0, 4.0, 5.0 };

    double expected = 3.0;

    phylanx::execution_tree::primitive first  =
            phylanx::execution_tree::primitives::create_variable(
                    hpx::find_here(), phylanx::ir::node_data<double>(v1));


    phylanx::execution_tree::primitive p =
            phylanx::execution_tree::primitives::create_mean_operation(
                    hpx::find_here(),
                    std::vector<phylanx::execution_tree::primitive_argument_type>{
                            std::move(first)
                    });

    hpx::future<phylanx::execution_tree::primitive_argument_type> f =
            p.eval();


    HPX_TEST_EQ(phylanx::ir::node_data<double>(expected),
                phylanx::execution_tree::extract_numeric_value(f.get()));
}

void test_mean_operation_2d_flat()
{
    blaze::DynamicMatrix<double> matrix_1{ { 1.0, 2.0, 6.0 },{ 4.0, 5.0, 6.0 } };

    double expected = 4.0;

    phylanx::execution_tree::primitive first =
            phylanx::execution_tree::primitives::create_variable(
                    hpx::find_here(), phylanx::ir::node_data<double>(matrix_1));

    phylanx::execution_tree::primitive p =
            phylanx::execution_tree::primitives::create_mean_operation(
                    hpx::find_here(),
                    std::vector<phylanx::execution_tree::primitive_argument_type>{
                            std::move(first)
                    });

    hpx::future<phylanx::execution_tree::primitive_argument_type> f = p.eval();


    HPX_TEST_EQ(phylanx::ir::node_data<double>(expected),
                phylanx::execution_tree::extract_numeric_value(f.get()));

}

void test_mean_operation_2d_x_axis()
{
    using arg_type = phylanx::execution_tree::primitive_argument_type;

    blaze::DynamicMatrix<double> matrix_1{ { 1.0, 2.0, 6.0 },{ 4.0, 5.0, 6.0 } };

    std::vector<arg_type> expected{
            arg_type{static_cast<double>(3.0)},
            arg_type{static_cast<double>(5.0)}};

    phylanx::execution_tree::primitive first =
            phylanx::execution_tree::primitives::create_variable(
                    hpx::find_here(), phylanx::ir::node_data<double>(matrix_1));

    phylanx::execution_tree::primitive second =
            phylanx::execution_tree::primitives::create_variable(
                    hpx::find_here(), phylanx::ir::node_data<double>(0));


    phylanx::execution_tree::primitive p =
            phylanx::execution_tree::primitives::create_mean_operation(
                    hpx::find_here(),
                    std::vector<phylanx::execution_tree::primitive_argument_type>{
                            std::move(first), std::move(second)
                    });

    hpx::future<phylanx::execution_tree::primitive_argument_type> f =
            p.eval();


    auto actual = phylanx::execution_tree::extract_list_value(f.get());

    HPX_TEST_EQ(expected, actual);
}


void test_mean_operation_2d_y_axis()
{
    using arg_type = phylanx::execution_tree::primitive_argument_type;
    blaze::DynamicMatrix<double> matrix_1{ { 1.0, 2.0, 6.0 },{ 4.0, 5.0, 6.0 } };

    std::vector<arg_type> expected{
            arg_type{static_cast<double>(2.5)},
            arg_type{static_cast<double>(3.5)},
            arg_type{static_cast<double>(6.0)}};

    phylanx::execution_tree::primitive first =
            phylanx::execution_tree::primitives::create_variable(
                    hpx::find_here(), phylanx::ir::node_data<double>(matrix_1));

    phylanx::execution_tree::primitive second =
            phylanx::execution_tree::primitives::create_variable(
                    hpx::find_here(), phylanx::ir::node_data<double>(1));


    phylanx::execution_tree::primitive p =
            phylanx::execution_tree::primitives::create_mean_operation(
                    hpx::find_here(),
                    std::vector<phylanx::execution_tree::primitive_argument_type>{
                            std::move(first), std::move(second)
                    });

    hpx::future<phylanx::execution_tree::primitive_argument_type> f =
            p.eval();


    auto actual = phylanx::execution_tree::extract_list_value(f.get());

    HPX_TEST_EQ(expected, actual);
}

int main(int argc, char* argv[])
{
    test_mean_operation_0d();
    test_mean_operation_1d();
    test_mean_operation_2d_flat();
    test_mean_operation_2d_x_axis();
    test_mean_operation_2d_y_axis();

    return hpx::util::report_errors();
}