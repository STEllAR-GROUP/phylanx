// Copyright (c) 2018 Bita Hasheminezhad
// Copyright (c) 2018 Parsa Amini
// Copyright (c) 2018 Hartmut Kaiser
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/phylanx.hpp>

#include <hpx/hpx_main.hpp>
#include <hpx/include/lcos.hpp>
#include <hpx/util/lightweight_test.hpp>

#include <cstdint>
#include <utility>
#include <vector>

//////////////////////////////////////////////////////////////////////////
void test_tile_operation_0d_vector()
{
    phylanx::execution_tree::primitive arr =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(42.0));

    phylanx::execution_tree::primitive v1 =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<std::int64_t>(3));

    phylanx::execution_tree::primitive tile =
        phylanx::execution_tree::primitives::create_tile_operation(
            hpx::find_here(),
            phylanx::execution_tree::primitive_arguments_type{
                phylanx::execution_tree::primitive_argument_type{
                    std::move(arr)},
                phylanx::execution_tree::primitive_argument_type{
                    std::move(v1)} });

    hpx::future<phylanx::execution_tree::primitive_argument_type> f =
        tile.eval();

    blaze::DynamicVector<double> expected{42., 42., 42.};

    HPX_TEST_EQ(phylanx::ir::node_data<double>(std::move(expected)),
        phylanx::execution_tree::extract_numeric_value(f.get()));
}

void test_tile_operation_0d_matrix()
{
    phylanx::execution_tree::primitive arr =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(42.0));

    phylanx::execution_tree::primitive tile =
        phylanx::execution_tree::primitives::create_tile_operation(
            hpx::find_here(),
            phylanx::execution_tree::primitive_arguments_type{std::move(arr),
                phylanx::execution_tree::primitive_argument_type{
                    phylanx::execution_tree::primitive_arguments_type{
                        phylanx::ir::node_data<std::int64_t>(3),
                        phylanx::ir::node_data<std::int64_t>(2)}}});

    hpx::future<phylanx::execution_tree::primitive_argument_type> f =
        tile.eval();

    blaze::DynamicMatrix<double> expected{{42., 42.}, {42., 42.}, {42., 42.}};

    HPX_TEST_EQ(phylanx::ir::node_data<double>(std::move(expected)),
        phylanx::execution_tree::extract_numeric_value(f.get()));
}

void test_tile_operation_1d_vector()
{
    blaze::DynamicVector<std::int64_t> vec{42, 13, 33};
    phylanx::execution_tree::primitive arr =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<std::int64_t>(vec));

    phylanx::execution_tree::primitive v1 =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<std::int64_t>(3));

    phylanx::execution_tree::primitive tile =
        phylanx::execution_tree::primitives::create_tile_operation(
            hpx::find_here(),
            phylanx::execution_tree::primitive_arguments_type{
                phylanx::execution_tree::primitive_argument_type{
                    std::move(arr)},
                phylanx::execution_tree::primitive_argument_type{
                    std::move(v1)}});

    hpx::future<phylanx::execution_tree::primitive_argument_type> f =
        tile.eval();

    blaze::DynamicVector<std::int64_t> expected{
        42, 13, 33, 42, 13, 33, 42, 13, 33};

    HPX_TEST_EQ(phylanx::ir::node_data<std::int64_t>(std::move(expected)),
        phylanx::execution_tree::extract_integer_value(f.get()));
}

void test_tile_operation_1d_matrix()
{
    blaze::DynamicVector<double> vec{42., 13., 33.};

    phylanx::execution_tree::primitive arr =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(vec));

    phylanx::execution_tree::primitive tile =
        phylanx::execution_tree::primitives::create_tile_operation(
            hpx::find_here(),
            phylanx::execution_tree::primitive_arguments_type{ std::move(arr),
                phylanx::execution_tree::primitive_argument_type{
                    phylanx::execution_tree::primitive_arguments_type{
                        phylanx::ir::node_data<std::int64_t>(2),
                        phylanx::ir::node_data<std::int64_t>(3)}} });

    hpx::future<phylanx::execution_tree::primitive_argument_type> f =
        tile.eval();

    blaze::DynamicMatrix<double> expected{
        {42., 13., 33., 42., 13., 33., 42., 13., 33.},
        {42., 13., 33., 42., 13., 33., 42., 13., 33.}};

    HPX_TEST_EQ(phylanx::ir::node_data<double>(std::move(expected)),
        phylanx::execution_tree::extract_numeric_value(f.get()));
}

void test_tile_operation_2d_vector()
{
    blaze::DynamicMatrix<std::int64_t> mat{{42, 13}, {33, 5}, {6, 4}};

    phylanx::execution_tree::primitive arr =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<std::int64_t>(mat));

    phylanx::execution_tree::primitive v1 =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<std::int64_t>(3));

    phylanx::execution_tree::primitive tile =
        phylanx::execution_tree::primitives::create_tile_operation(
            hpx::find_here(),
            phylanx::execution_tree::primitive_arguments_type{
                phylanx::execution_tree::primitive_argument_type{
                    std::move(arr)},
                phylanx::execution_tree::primitive_argument_type{
                    std::move(v1)} });

    hpx::future<phylanx::execution_tree::primitive_argument_type> f =
        tile.eval();

    blaze::DynamicMatrix<std::int64_t> expected{
        {42, 13, 42, 13, 42, 13}, {33, 5, 33, 5, 33, 5}, {6, 4, 6, 4, 6, 4}};

    HPX_TEST_EQ(phylanx::ir::node_data<std::int64_t>(std::move(expected)),
        phylanx::execution_tree::extract_integer_value(f.get()));
}

void test_tile_operation_2d_matrix()
{
    blaze::DynamicMatrix<double> mat{{1., 2.}, {3., 4.}};

    phylanx::execution_tree::primitive arr =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(mat));

    phylanx::execution_tree::primitive tile =
        phylanx::execution_tree::primitives::create_tile_operation(
            hpx::find_here(),
            phylanx::execution_tree::primitive_arguments_type{ std::move(arr),
                phylanx::execution_tree::primitive_argument_type{
                    phylanx::execution_tree::primitive_arguments_type{
                        phylanx::ir::node_data<std::int64_t>(2),
                        phylanx::ir::node_data<std::int64_t>(2)}} });

    hpx::future<phylanx::execution_tree::primitive_argument_type> f =
        tile.eval();

    blaze::DynamicMatrix<double> expected{
        {1., 2., 1., 2.}, {3., 4., 3., 4.}, {1., 2., 1., 2.}, {3., 4., 3., 4.}};

    HPX_TEST_EQ(phylanx::ir::node_data<double>(std::move(expected)),
        phylanx::execution_tree::extract_numeric_value(f.get()));
}

int main(int argc, char* argv[])
{
    test_tile_operation_0d_vector();
    test_tile_operation_0d_matrix();
    test_tile_operation_1d_vector();
    test_tile_operation_1d_matrix();
    test_tile_operation_2d_vector();
    test_tile_operation_2d_matrix();
    return hpx::util::report_errors();
}
