// Copyright (c) 2018 Shahrzad Shirzad
// Copyright (c) 2018 Parsa Amini
// Copyright (c) 2018 Hartmut Kaiser
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/phylanx.hpp>

#include <hpx/hpx_main.hpp>
#include <hpx/include/lcos.hpp>
#include <hpx/util/lightweight_test.hpp>

#include <cstddef>
#include <cstdint>
#include <utility>
#include <vector>

#include <blaze/Math.h>

void test_unique_0d()
{
    blaze::DynamicVector<double> expected{10.0};

    phylanx::execution_tree::primitive lhs =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(10.0));

    phylanx::execution_tree::primitive p =
        phylanx::execution_tree::primitives::create_unique(hpx::find_here(),
            phylanx::execution_tree::primitive_arguments_type{std::move(lhs)});

    hpx::future<phylanx::execution_tree::primitive_argument_type> f = p.eval();

    HPX_TEST_EQ(phylanx::ir::node_data<double>(expected),
        phylanx::execution_tree::extract_numeric_value(f.get()));
}

void test_unique_1d()
{
    blaze::DynamicVector<double> v1{2.0, 1.0, 4.0, 1.0, 3.0, 1.0, 3.0};

    blaze::DynamicVector<double> expected{1.0, 2.0, 3.0, 4.0};

    phylanx::execution_tree::primitive lhs =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(v1));

    phylanx::execution_tree::primitive p =
        phylanx::execution_tree::primitives::create_unique(hpx::find_here(),
            phylanx::execution_tree::primitive_arguments_type{std::move(lhs)});

    hpx::future<phylanx::execution_tree::primitive_argument_type> f = p.eval();

    HPX_TEST_EQ(phylanx::ir::node_data<double>(expected),
        phylanx::execution_tree::extract_numeric_value(f.get()));
}

void test_unique_2d()
{
    blaze::DynamicMatrix<double> m1{
        {1.0, 4.0, -1.0, 0.0, 5.0}, {4.0, 5.0, 0.0, -1.0, 2.0}};

    blaze::DynamicVector<double> expected{-1.0, 0.0, 1.0, 2.0, 4.0, 5.0};

    phylanx::execution_tree::primitive lhs =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(m1));

    phylanx::execution_tree::primitive p =
        phylanx::execution_tree::primitives::create_unique(hpx::find_here(),
            phylanx::execution_tree::primitive_arguments_type{std::move(lhs)});

    hpx::future<phylanx::execution_tree::primitive_argument_type> f = p.eval();

    HPX_TEST_EQ(phylanx::ir::node_data<double>(expected),
        phylanx::execution_tree::extract_numeric_value(f.get()));
}

void test_unique_2d_x_axis()
{
    using arg_type = phylanx::execution_tree::primitive_argument_type;

    blaze::DynamicMatrix<double> m1{
        {2.0, 15.0, 0.0, 1.0}, {2.0, 9.0, 8.0, 3.0}, {-2.0, 4.0, 3.0, 2.0}};

    phylanx::ir::node_data<double> expected(blaze::DynamicMatrix<double>{
        {-2.0, 4.0, 3.0, 2.0}, {2.0, 9.0, 8.0, 3.0}, {2.0, 15.0, 0.0, 1.0}});

    phylanx::execution_tree::primitive lhs =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(m1));

    phylanx::execution_tree::primitive rhs =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(0));

    phylanx::execution_tree::primitive p =
        phylanx::execution_tree::primitives::create_unique(hpx::find_here(),
            phylanx::execution_tree::primitive_arguments_type{
                std::move(lhs), std::move(rhs)});

    hpx::future<arg_type> f = p.eval();

    auto actual = phylanx::execution_tree::extract_numeric_value(f.get());

    HPX_TEST_EQ(expected, actual);
}

void test_unique_2d_y_axis()
{
    using arg_type = phylanx::execution_tree::primitive_argument_type;

    blaze::DynamicMatrix<double> m1{
        {2.0, 15.0, 0.0, 1.0}, {2.0, 9.0, 8.0, 3.0}, {-2.0, 4.0, 3.0, 2.0}};

    phylanx::ir::node_data<double> expected(blaze::DynamicMatrix<double>{
        {0.0, 1.0, 2.0, 15.0}, {8.0, 3.0, 2.0, 9.0}, {3.0, 2.0, -2.0, 4.0}});

    phylanx::execution_tree::primitive lhs =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(m1));

    phylanx::execution_tree::primitive rhs =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(1));

    phylanx::execution_tree::primitive p =
        phylanx::execution_tree::primitives::create_unique(hpx::find_here(),
            phylanx::execution_tree::primitive_arguments_type{
                std::move(lhs), std::move(rhs)});

    hpx::future<arg_type> f = p.eval();

    auto actual = phylanx::execution_tree::extract_numeric_value(f.get());

    HPX_TEST_EQ(expected, actual);
}

int main(int argc, char* argv[])
{
    test_unique_0d();
    test_unique_1d();
    test_unique_2d();
    test_unique_2d_x_axis();
    test_unique_2d_y_axis();

    return hpx::util::report_errors();
}
