// Copyright (c) 2018 Bita Hasheminezhad
// Copyright (c) 2018 Shahrzad Shirzad
// Copyright (c) 2018 Hartmut Kaiser
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
void test_reshape_operation_0d_vector()
{
    phylanx::execution_tree::primitive arr =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(42.0));

    phylanx::execution_tree::primitive v1 =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<std::int64_t>(1));

    phylanx::execution_tree::primitive reshape =
        phylanx::execution_tree::primitives::create_reshape_operation(
            hpx::find_here(),
            phylanx::execution_tree::primitive_arguments_type{
                phylanx::execution_tree::primitive_argument_type{
                    std::move(arr)},
                phylanx::execution_tree::primitive_argument_type{
                    std::move(v1)}});

    hpx::future<phylanx::execution_tree::primitive_argument_type> f =
        reshape.eval();

    blaze::DynamicVector<double> expected{42.};

    HPX_TEST_EQ(phylanx::ir::node_data<double>(std::move(expected)),
        phylanx::execution_tree::extract_numeric_value(f.get()));
}


void test_reshape_operation_0d_matrix()
{
    phylanx::execution_tree::primitive arr =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(42.0));

    phylanx::execution_tree::primitive reshape =
        phylanx::execution_tree::primitives::create_reshape_operation(
            hpx::find_here(),
            phylanx::execution_tree::primitive_arguments_type{
                std::move(arr),
                phylanx::execution_tree::primitive_argument_type{
                    phylanx::execution_tree::primitive_arguments_type{
                        phylanx::ir::node_data<std::int64_t>(1),
                        phylanx::ir::node_data<std::int64_t>(1)}
                    } });

    hpx::future<phylanx::execution_tree::primitive_argument_type> f =
        reshape.eval();

    blaze::DynamicMatrix<double> expected{{42.}};

    HPX_TEST_EQ(phylanx::ir::node_data<double>(std::move(expected)),
        phylanx::execution_tree::extract_numeric_value(f.get()));
}

void test_reshape_operation_1d_vector()
{
    blaze::DynamicVector<std::int64_t> vec{42, 13, 33};
    phylanx::execution_tree::primitive arr =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<std::int64_t>(vec));

    phylanx::execution_tree::primitive v1 =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<std::int64_t>(-1));

    phylanx::execution_tree::primitive reshape =
        phylanx::execution_tree::primitives::create_reshape_operation(
            hpx::find_here(),
            phylanx::execution_tree::primitive_arguments_type{
                phylanx::execution_tree::primitive_argument_type{
                    std::move(arr)},
                phylanx::execution_tree::primitive_argument_type{
                    std::move(v1)}});

    hpx::future<phylanx::execution_tree::primitive_argument_type> f =
        reshape.eval();

    blaze::DynamicVector<std::int64_t> expected{42, 13, 33};

    HPX_TEST_EQ(phylanx::ir::node_data<std::int64_t>(std::move(expected)),
        phylanx::execution_tree::extract_integer_value(f.get()));
}

void test_reshape_operation_1d_matrix()
{
    blaze::DynamicVector<std::int64_t> vec{42, 13, 33, 1, 2, 5};
    phylanx::execution_tree::primitive arr =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<std::int64_t>(vec));

    phylanx::execution_tree::primitive reshape =
        phylanx::execution_tree::primitives::create_reshape_operation(
            hpx::find_here(),
            phylanx::execution_tree::primitive_arguments_type{
                std::move(arr),
                phylanx::execution_tree::primitive_argument_type{
                    phylanx::execution_tree::primitive_arguments_type{
                        phylanx::ir::node_data<std::int64_t>(3),
                        phylanx::ir::node_data<std::int64_t>(2)}
                    } });

    hpx::future<phylanx::execution_tree::primitive_argument_type> f =
        reshape.eval();

    blaze::DynamicMatrix<std::int64_t> expected{{42, 13}, {33, 1}, {2, 5}};

    HPX_TEST_EQ(phylanx::ir::node_data<std::int64_t>(std::move(expected)),
        phylanx::execution_tree::extract_integer_value(f.get()));
}

void test_reshape_operation_2d_vector()
{
    blaze::DynamicMatrix<double> mat{{42., 13.}, {33., 1.}, {2., 5.}};
    phylanx::execution_tree::primitive arr =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<std::int64_t>(mat));

    phylanx::execution_tree::primitive v1 =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<std::int64_t>(6));

    phylanx::execution_tree::primitive reshape =
        phylanx::execution_tree::primitives::create_reshape_operation(
            hpx::find_here(),
            phylanx::execution_tree::primitive_arguments_type{
                phylanx::execution_tree::primitive_argument_type{
                    std::move(arr)},
                phylanx::execution_tree::primitive_argument_type{
                    std::move(v1)}});

    hpx::future<phylanx::execution_tree::primitive_argument_type> f =
        reshape.eval();

    blaze::DynamicVector<double> expected{42., 13., 33., 1., 2., 5.};

    HPX_TEST_EQ(phylanx::ir::node_data<double>(std::move(expected)),
        phylanx::execution_tree::extract_numeric_value(f.get()));
}

void test_reshape_operation_2d_matrix()
{
    blaze::DynamicMatrix<std::int64_t> mat{{42, 13}, {33, 1}, {2, 5}, {12, 15}};
    phylanx::execution_tree::primitive arr =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<std::int64_t>(mat));

    phylanx::execution_tree::primitive reshape =
        phylanx::execution_tree::primitives::create_reshape_operation(
            hpx::find_here(),
            phylanx::execution_tree::primitive_arguments_type{
                std::move(arr),
                phylanx::execution_tree::primitive_argument_type{
                    phylanx::execution_tree::primitive_arguments_type{
                        phylanx::ir::node_data<std::int64_t>(2),
                        phylanx::ir::node_data<std::int64_t>(4)}
                    } });

    hpx::future<phylanx::execution_tree::primitive_argument_type> f =
        reshape.eval();

    blaze::DynamicMatrix<std::int64_t> expected{
        {42, 13, 33, 1}, {2, 5, 12, 15}};

    HPX_TEST_EQ(phylanx::ir::node_data<std::int64_t>(std::move(expected)),
        phylanx::execution_tree::extract_integer_value(f.get()));
}

int main(int argc, char* argv[])
{
//     test_reshape_operation_0d_vector();
//     test_reshape_operation_0d_matrix();
//     test_reshape_operation_1d_vector();
    test_reshape_operation_1d_matrix();
    test_reshape_operation_2d_vector();
    test_reshape_operation_2d_matrix();
    return hpx::util::report_errors();
}
