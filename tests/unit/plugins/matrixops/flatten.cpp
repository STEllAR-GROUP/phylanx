// Copyright (c) 2018 Shahrzad Shirzad
// Copyright (c) 2018 Hartmut Kaiser
// Copyright (c) 2019 Bita Hasheminezhad
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
            phylanx::execution_tree::primitive_arguments_type{
                std::move(lhs), std::move(order)});

    hpx::future<phylanx::execution_tree::primitive_argument_type> f = p.eval();

    HPX_TEST_EQ(phylanx::ir::node_data<double>(expected),
        phylanx::execution_tree::extract_numeric_value(f.get()));
}

#if defined(PHYLANX_HAVE_BLAZE_TENSOR)
void test_flatten_3d()
{
    blaze::DynamicTensor<std::int64_t> t{{{3, 2, 15}, {1, 10, 2}, {3, 2, 15}},
        {{4, 20, 5}, {-1, 1, -2}, {40, 12, 5}}};
    blaze::DynamicVector<std::int64_t> expected{
        3, 2, 15, 1, 10, 2, 3, 2, 15, 4, 20, 5, -1, 1, -2, 40, 12, 5};

    phylanx::execution_tree::primitive lhs =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(t));

    phylanx::execution_tree::primitive p =
        phylanx::execution_tree::primitives::create_flatten(hpx::find_here(),
            phylanx::execution_tree::primitive_arguments_type{std::move(lhs)});

    hpx::future<phylanx::execution_tree::primitive_argument_type> f = p.eval();

    HPX_TEST_EQ(phylanx::ir::node_data<std::int64_t>(expected),
        phylanx::execution_tree::extract_integer_value(f.get()));
}

void test_flatten_3d_C()
{
    blaze::DynamicTensor<std::int64_t> t{{{3, 2, 15}, {1, 10, 2}, {3, 2, 15}},
        {{4, 20, 5}, {-1, 1, -2}, {40, 12, 5}}};
    blaze::DynamicVector<std::int64_t> expected{
        3, 2, 15, 1, 10, 2, 3, 2, 15, 4, 20, 5, -1, 1, -2, 40, 12, 5};

    phylanx::execution_tree::primitive order =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), std::string("C"));

    phylanx::execution_tree::primitive lhs =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(t));

    phylanx::execution_tree::primitive p =
        phylanx::execution_tree::primitives::create_flatten(hpx::find_here(),
            phylanx::execution_tree::primitive_arguments_type{
                std::move(lhs), std::move(order)});

    hpx::future<phylanx::execution_tree::primitive_argument_type> f = p.eval();

    HPX_TEST_EQ(phylanx::ir::node_data<std::int64_t>(expected),
        phylanx::execution_tree::extract_integer_value(f.get()));
}

void test_flatten_3d_F()
{
    blaze::DynamicTensor<double> t{{{3, 2, 15}, {1, 10, 2}, {3, 2, 15}},
        {{4, 20, 5}, {-1, 1, -2}, {40, 12, 5}}};
    blaze::DynamicVector<double> expected{3., 4., 1., -1., 3., 40., 2., 20.,
        10., 1., 2., 12., 15., 5., 2., -2., 15., 5.};

    phylanx::execution_tree::primitive order =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), std::string("F"));

    phylanx::execution_tree::primitive lhs =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(t));

    phylanx::execution_tree::primitive p =
        phylanx::execution_tree::primitives::create_flatten(hpx::find_here(),
            phylanx::execution_tree::primitive_arguments_type{
                std::move(lhs), std::move(order)});

    hpx::future<phylanx::execution_tree::primitive_argument_type> f = p.eval();

    HPX_TEST_EQ(phylanx::ir::node_data<double>(expected),
        phylanx::execution_tree::extract_numeric_value(f.get()));
}
#endif

int main(int argc, char* argv[])
{
    test_flatten_0d();
    test_flatten_1d();
    test_flatten_2d();
    test_flatten_2d_column();
    test_flatten_2d_row();

#if defined(PHYLANX_HAVE_BLAZE_TENSOR)
    test_flatten_3d();
    test_flatten_3d_C();
    test_flatten_3d_F();
#endif
    return hpx::util::report_errors();
}
