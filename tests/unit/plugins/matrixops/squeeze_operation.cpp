// Copyright (c) 2018 Hartmut Kaiser
// Copyright (c) 2018 Bita Hasheminezhad
// Copyright (c) 2018 Parsa Amini
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
void test_squeeze_operation_0d()
{
    phylanx::execution_tree::primitive first =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(42.0));

    phylanx::execution_tree::primitive squeeze =
        phylanx::execution_tree::primitives::create_squeeze_operation(
            hpx::find_here(),
            phylanx::execution_tree::primitive_arguments_type{
                std::move(first)});

    hpx::future<phylanx::execution_tree::primitive_argument_type> f =
        squeeze.eval();

    HPX_TEST_EQ(
        42.0, phylanx::execution_tree::extract_numeric_value(f.get())[0]);
}

void test_squeeze_operation_0d_int()
{
    phylanx::execution_tree::primitive first =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<std::int64_t>(42));
    std::int64_t expected = 42;

    phylanx::execution_tree::primitive squeeze =
        phylanx::execution_tree::primitives::create_squeeze_operation(
            hpx::find_here(),
            phylanx::execution_tree::primitive_arguments_type{
                std::move(first) });

    hpx::future<phylanx::execution_tree::primitive_argument_type> f =
        squeeze.eval();

    HPX_TEST_EQ(
        expected, phylanx::execution_tree::extract_numeric_value(f.get())[0]);
}

void test_squeeze_operation_1d()
{
    blaze::DynamicVector<std::int64_t> v1{42};

    std::int64_t expected = 42;

    phylanx::execution_tree::primitive first =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<std::int64_t>(v1));

    phylanx::execution_tree::primitive p =
        phylanx::execution_tree::primitives::create_squeeze_operation(
            hpx::find_here(),
            phylanx::execution_tree::primitive_arguments_type{
                std::move(first)});

    hpx::future<phylanx::execution_tree::primitive_argument_type> f = p.eval();

    HPX_TEST_EQ(phylanx::ir::node_data<std::int64_t>(expected),
        phylanx::execution_tree::extract_integer_value(f.get()));
}

void test_squeeze_operation_1d_nochange()
{
    blaze::DynamicVector<double> v1{42.0, 4.0};

    blaze::DynamicVector<double> expected{42.0, 4.0};

    phylanx::execution_tree::primitive first =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(v1));

    phylanx::execution_tree::primitive p =
        phylanx::execution_tree::primitives::create_squeeze_operation(
            hpx::find_here(),
            phylanx::execution_tree::primitive_arguments_type{
                std::move(first)});

    hpx::future<phylanx::execution_tree::primitive_argument_type> f = p.eval();

    HPX_TEST_EQ(phylanx::ir::node_data<double>(expected),
        phylanx::execution_tree::extract_numeric_value(f.get()));
}

void test_squeeze_operation_2d_nochange()
{
    blaze::DynamicMatrix<double> m1{{6., 9.}, {13., 42.}, {33., 33.}};

    blaze::DynamicMatrix<double> expected{{6., 9.}, {13., 42.}, {33., 33.}};

    phylanx::execution_tree::primitive first =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(m1));

    phylanx::execution_tree::primitive p =
        phylanx::execution_tree::primitives::create_squeeze_operation(
            hpx::find_here(),
            phylanx::execution_tree::primitive_arguments_type{
                std::move(first)});

    hpx::future<phylanx::execution_tree::primitive_argument_type> f = p.eval();

    HPX_TEST_EQ(phylanx::ir::node_data<double>(expected),
        phylanx::execution_tree::extract_numeric_value(f.get()));
}

void test_squeeze_operation_2d()
{
    blaze::DynamicMatrix<std::int64_t> m1{{42}};

    std::int64_t expected = 42;

    phylanx::execution_tree::primitive first =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(m1));

    phylanx::execution_tree::primitive p =
        phylanx::execution_tree::primitives::create_squeeze_operation(
            hpx::find_here(),
            phylanx::execution_tree::primitive_arguments_type{
                std::move(first)});

    hpx::future<phylanx::execution_tree::primitive_argument_type> f = p.eval();

    HPX_TEST_EQ(phylanx::ir::node_data<double>(expected),
        phylanx::execution_tree::extract_numeric_value(f.get()));
}

void test_squeeze_operation_2d_column()
{
    blaze::DynamicMatrix<double> m1{{42.}, {13.}};

    blaze::DynamicVector<double> expected{42.0, 13.0};

    phylanx::execution_tree::primitive first =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(m1));

    phylanx::execution_tree::primitive p =
        phylanx::execution_tree::primitives::create_squeeze_operation(
            hpx::find_here(),
            phylanx::execution_tree::primitive_arguments_type{
                std::move(first)});

    hpx::future<phylanx::execution_tree::primitive_argument_type> f = p.eval();

    HPX_TEST_EQ(phylanx::ir::node_data<double>(expected),
        phylanx::execution_tree::extract_numeric_value(f.get()));
}

void test_squeeze_operation_2d_row()
{
    blaze::DynamicMatrix<std::int64_t> m1{{42, 13}};

    blaze::DynamicVector<std::int64_t> expected{42, 13};

    phylanx::execution_tree::primitive first =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<std::int64_t>(m1));

    phylanx::execution_tree::primitive p =
        phylanx::execution_tree::primitives::create_squeeze_operation(
            hpx::find_here(),
            phylanx::execution_tree::primitive_arguments_type{
                std::move(first)});

    hpx::future<phylanx::execution_tree::primitive_argument_type> f = p.eval();

    HPX_TEST_EQ(phylanx::ir::node_data<std::int64_t>(expected),
        phylanx::execution_tree::extract_integer_value(f.get()));
}

int main(int argc, char* argv[])
{
    test_squeeze_operation_0d();
    test_squeeze_operation_0d_int();
    test_squeeze_operation_1d();
    test_squeeze_operation_1d_nochange();
    test_squeeze_operation_2d_nochange();
    test_squeeze_operation_2d();
    test_squeeze_operation_2d_column();
    test_squeeze_operation_2d_row();

    return hpx::util::report_errors();
}
