//  Copyright (c) 2018 Maxwell Reeser
//  Copyright (c) 2017-2018 Bibek Wagle
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/phylanx.hpp>

#include <hpx/hpx_main.hpp>
#include <hpx/include/lcos.hpp>
#include <hpx/modules/testing.hpp>

#include <iostream>
#include <utility>
#include <vector>

void hsplit_operation_scalar_blocks()
{
    blaze::DynamicMatrix<double> m1{{1.0, 2.0, 3.0}, {4.0, 5.0, 6.0},
        {7.0, 8.0, 9.0}, {10.0, 11.0, 12.0}, {13.0, 14.0, 15.0}};

    phylanx::execution_tree::primitive matrix =
        phylanx::execution_tree::primitives::create_variable(hpx::find_here(),
            phylanx::execution_tree::primitive_argument_type{
                phylanx::ir::node_data<double>(m1)});

    phylanx::execution_tree::primitive num_blocks =
        phylanx::execution_tree::primitives::create_variable(hpx::find_here(),
            phylanx::execution_tree::primitive_argument_type{3.0});

    phylanx::execution_tree::primitive hsplit =
        phylanx::execution_tree::primitives::create_hsplit_operation(
            hpx::find_here(),
            phylanx::execution_tree::primitive_arguments_type{
                phylanx::execution_tree::primitive_argument_type{
                    std::move(matrix)},
                phylanx::execution_tree::primitive_argument_type{
                    std::move(num_blocks)}});

    hpx::future<phylanx::execution_tree::primitive_argument_type> f =
        hsplit.eval();

    blaze::DynamicMatrix<double> expected_first{
        {1.0}, {4.0}, {7.0}, {10.0}, {13.0}};
    blaze::DynamicMatrix<double> expected_second{
        {2.0}, {5.0}, {8.0}, {11.0}, {14.0}};
    blaze::DynamicMatrix<double> expected_third{
        {3.0}, {6.0}, {9.0}, {12.0}, {15.0}};

    phylanx::ir::node_data<double> data_expected_first(
        std::move(expected_first));
    phylanx::ir::node_data<double> data_expected_second(
        std::move(expected_second));
    phylanx::ir::node_data<double> data_expected_third(
        std::move(expected_third));

    phylanx::execution_tree::primitive_argument_type result = f.get();

    auto list_result = extract_list_value(result);

    HPX_TEST_EQ(3, list_result.size());

    auto it = list_result.begin();

    HPX_TEST_EQ(data_expected_first,
        phylanx::execution_tree::extract_numeric_value(*it++));
    HPX_TEST_EQ(data_expected_second,
        phylanx::execution_tree::extract_numeric_value(*it++));
    HPX_TEST_EQ(data_expected_third,
        phylanx::execution_tree::extract_numeric_value(*it));
}

void hsplit_operation_range_blocks()
{
    blaze::DynamicMatrix<double> m1{{1.0, 2.0, 3.0}, {4.0, 5.0, 6.0},
        {7.0, 8.0, 9.0}, {10.0, 11.0, 12.0}, {13.0, 14.0, 15.0}};

    blaze::DynamicVector<double> blocks{0.0, 1.0, 4.0, 2.0, 3.0};

    phylanx::execution_tree::primitive matrix =
        phylanx::execution_tree::primitives::create_variable(hpx::find_here(),
            phylanx::execution_tree::primitive_argument_type{
                phylanx::ir::node_data<double>(m1)});

    phylanx::execution_tree::primitive block_range =
        phylanx::execution_tree::primitives::create_variable(hpx::find_here(),
            phylanx::execution_tree::primitive_argument_type{
                phylanx::ir::node_data<double>(blocks)});

    phylanx::execution_tree::primitive hsplit =
        phylanx::execution_tree::primitives::create_hsplit_operation(
            hpx::find_here(),
            phylanx::execution_tree::primitive_arguments_type{
                phylanx::execution_tree::primitive_argument_type{
                    std::move(matrix)},
                phylanx::execution_tree::primitive_argument_type{
                    std::move(block_range)}});

    hpx::future<phylanx::execution_tree::primitive_argument_type> f =
        hsplit.eval();

    blaze::DynamicMatrix<double> expected_zero(5, 0);
    blaze::DynamicMatrix<double> expected_first{
        {1.0}, {4.0}, {7.0}, {10.0}, {13.0}};
    blaze::DynamicMatrix<double> expected_second{
        {2.0, 3.0}, {5.0, 6.0}, {8.0, 9.0}, {11.0, 12.0}, {14.0, 15.0}};
    blaze::DynamicMatrix<double> expected_third(5, 0);
    blaze::DynamicMatrix<double> expected_fourth{
        {3.0}, {6.0}, {9.0}, {12.0}, {15.0}};
    blaze::DynamicMatrix<double> expected_fifth(5, 0);

    phylanx::ir::node_data<double> data_expected_zero(std::move(expected_zero));
    phylanx::ir::node_data<double> data_expected_first(
        std::move(expected_first));
    phylanx::ir::node_data<double> data_expected_second(
        std::move(expected_second));
    phylanx::ir::node_data<double> data_expected_third(
        std::move(expected_third));
    phylanx::ir::node_data<double> data_expected_fourth(
        std::move(expected_fourth));
    phylanx::ir::node_data<double> data_expected_fifth(
        std::move(expected_fifth));

    phylanx::execution_tree::primitive_argument_type result = f.get();

    auto list_result = extract_list_value(result);

    HPX_TEST_EQ(6, list_result.size());

    auto it = list_result.begin();

    HPX_TEST_EQ(data_expected_zero,
        phylanx::execution_tree::extract_numeric_value(*it++));
    HPX_TEST_EQ(data_expected_first,
        phylanx::execution_tree::extract_numeric_value(*it++));
    HPX_TEST_EQ(data_expected_second,
        phylanx::execution_tree::extract_numeric_value(*it++));
    HPX_TEST_EQ(data_expected_third,
        phylanx::execution_tree::extract_numeric_value(*it++));
    HPX_TEST_EQ(data_expected_fourth,
        phylanx::execution_tree::extract_numeric_value(*it++));
    HPX_TEST_EQ(data_expected_fifth,
        phylanx::execution_tree::extract_numeric_value(*it));
}

///////////////////////////////////////////////////////////////////////////////

int main(int argc, char* argv[])
{
    hsplit_operation_scalar_blocks();
    hsplit_operation_range_blocks();

    return hpx::util::report_errors();
}
