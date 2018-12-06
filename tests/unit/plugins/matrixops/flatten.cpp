// Copyright (c) 2018 Shahrzad Shirzad
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
#include <string>
#include <utility>
#include <vector>

#include <blaze/Math.h>

void test_flatten_0d()
{
    phylanx::execution_tree::primitive lhs =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(10.0));

    phylanx::execution_tree::primitive p =
        phylanx::execution_tree::primitives::create_flatten(hpx::find_here(),
            phylanx::execution_tree::primitive_arguments_type{std::move(lhs)});

    hpx::future<phylanx::execution_tree::primitive_argument_type> f = p.eval();

    blaze::DynamicVector<double> expected{10.0};

    HPX_TEST_EQ(phylanx::ir::node_data<double>(expected),
        phylanx::execution_tree::extract_numeric_value(f.get()));
}

void test_flatten_1d()
{
    blaze::DynamicVector<double> v1{1.0, 2.0, 3.0, 1.0};

    blaze::DynamicVector<double> expected{1.0, 2.0, 3.0, 1.0};

    phylanx::execution_tree::primitive lhs =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(v1));

    phylanx::execution_tree::primitive p =
        phylanx::execution_tree::primitives::create_flatten(hpx::find_here(),
            phylanx::execution_tree::primitive_arguments_type{std::move(lhs)});

    hpx::future<phylanx::execution_tree::primitive_argument_type> f = p.eval();

    HPX_TEST_EQ(phylanx::ir::node_data<double>(expected),
        phylanx::execution_tree::extract_numeric_value(f.get()));
}

void test_flatten_2d()
{
    blaze::DynamicMatrix<double> m1{{3.0, 4.0, 6.0}, {1.0, 2.0, 9.0}};
    blaze::DynamicVector<double> expected{3.0, 4.0, 6.0, 1.0, 2.0, 9.0};

    phylanx::execution_tree::primitive lhs =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(m1));

    phylanx::execution_tree::primitive p =
        phylanx::execution_tree::primitives::create_flatten(hpx::find_here(),
            phylanx::execution_tree::primitive_arguments_type{std::move(lhs)});

    hpx::future<phylanx::execution_tree::primitive_argument_type> f = p.eval();

    HPX_TEST_EQ(phylanx::ir::node_data<double>(expected),
        phylanx::execution_tree::extract_numeric_value(f.get()));
}

void test_flatten_2d_column()
{
    blaze::DynamicMatrix<double> m1{{3.0, 4.0, 6.0}, {1.0, 2.0, 9.0}};
    blaze::DynamicVector<double> expected{3.0, 1.0, 4.0, 2.0, 6.0, 9.0};

    phylanx::execution_tree::primitive order =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), std::string("F"));

    phylanx::execution_tree::primitive lhs =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(m1));

    phylanx::execution_tree::primitive p =
        phylanx::execution_tree::primitives::create_flatten(hpx::find_here(),
            phylanx::execution_tree::primitive_arguments_type{
                std::move(lhs), std::move(order)});

    hpx::future<phylanx::execution_tree::primitive_argument_type> f = p.eval();

    HPX_TEST_EQ(phylanx::ir::node_data<double>(expected),
        phylanx::execution_tree::extract_numeric_value(f.get()));
}

void test_flatten_2d_row()
{
    blaze::DynamicMatrix<double> m1{{3.0, 4.0, 6.0}, {1.0, 2.0, 9.0}};
    blaze::DynamicVector<double> expected{3.0, 4.0, 6.0, 1.0, 2.0, 9.0};

    phylanx::execution_tree::primitive order =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), std::string("C"));

    phylanx::execution_tree::primitive lhs =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(m1));

    phylanx::execution_tree::primitive p =
        phylanx::execution_tree::primitives::create_flatten(hpx::find_here(),
            phylanx::execution_tree::primitive_arguments_type{std::move(lhs)});

    hpx::future<phylanx::execution_tree::primitive_argument_type> f = p.eval();

    HPX_TEST_EQ(phylanx::ir::node_data<double>(expected),
        phylanx::execution_tree::extract_numeric_value(f.get()));
}

int main(int argc, char* argv[])
{
    test_flatten_0d();
    test_flatten_1d();
    test_flatten_2d();
    test_flatten_2d_column();
    test_flatten_2d_row();

    return hpx::util::report_errors();
}
