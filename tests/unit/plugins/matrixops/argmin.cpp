// Copyright (c) 2018 Parsa Amini
// Copyright (c) 2018-2019 Hartmut Kaiser
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
#if defined(PHYLANX_HAVE_BLAZE_TENSOR)
#include <blaze_tensor/Math.h>
#endif

///////////////////////////////////////////////////////////////////////////////
void test_argmin_0d()
{
    int s1 = 10;

    std::int64_t expected = 0ul;

    phylanx::execution_tree::primitive lhs =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(s1));

    phylanx::execution_tree::primitive p =
        phylanx::execution_tree::primitives::create_argmin(
            hpx::find_here(),
            phylanx::execution_tree::primitive_arguments_type{
        std::move(lhs)
    });

    hpx::future<phylanx::execution_tree::primitive_argument_type> f =
        p.eval();


    HPX_TEST_EQ(phylanx::ir::node_data<std::int64_t>(expected),
        phylanx::execution_tree::extract_integer_value_strict(f.get()));
}

///////////////////////////////////////////////////////////////////////////////
void test_argmin_1d()
{
    blaze::DynamicVector<double> v1{1.0, 2.0, 3.0, -1.0};

    std::int64_t expected = 3;

    phylanx::execution_tree::primitive lhs =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(v1));

    phylanx::execution_tree::primitive p =
        phylanx::execution_tree::primitives::create_argmin(
            hpx::find_here(),
            phylanx::execution_tree::primitive_arguments_type{
        std::move(lhs)
    });

    hpx::future<phylanx::execution_tree::primitive_argument_type> f =
        p.eval();


    HPX_TEST_EQ(phylanx::ir::node_data<std::int64_t>(expected),
        phylanx::execution_tree::extract_integer_value_strict(f.get()));
}

///////////////////////////////////////////////////////////////////////////////
void test_argmin_2d_flat()
{
    blaze::DynamicMatrix<double> m1{{1.0, 2.0, 3.0}, {4.0, 5.0, -6.0}};

    std::size_t expected = 5;

    phylanx::execution_tree::primitive lhs =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(m1));

    phylanx::execution_tree::primitive p =
        phylanx::execution_tree::primitives::create_argmin(
            hpx::find_here(),
            phylanx::execution_tree::primitive_arguments_type{
        std::move(lhs)
    });

    hpx::future<phylanx::execution_tree::primitive_argument_type> f =
        p.eval();


    HPX_TEST_EQ(phylanx::ir::node_data<std::int64_t>(expected),
        phylanx::execution_tree::extract_integer_value_strict(f.get()));
}

void test_argmin_2d_x_axis()
{
    using arg_type = phylanx::execution_tree::primitive_argument_type;

    blaze::DynamicMatrix<double> m1{{1.0, 2.0, -3.0}, {4.0, 5.0, -6.0}};

    phylanx::ir::node_data<std::int64_t> expected(
        blaze::DynamicVector<std::int64_t>{0, 0, 1});

    phylanx::execution_tree::primitive lhs =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(m1));

    phylanx::execution_tree::primitive rhs =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<std::int64_t>(0));

    phylanx::execution_tree::primitive p =
        phylanx::execution_tree::primitives::create_argmin(hpx::find_here(),
            phylanx::execution_tree::primitive_arguments_type{
                std::move(lhs), std::move(rhs)});

    hpx::future<arg_type> f = p.eval();

    auto actual =
        phylanx::execution_tree::extract_integer_value_strict(f.get());

    HPX_TEST_EQ(expected, actual);
}

void test_argmin_2d_y_axis()
{
    using arg_type = phylanx::execution_tree::primitive_argument_type;

    blaze::DynamicMatrix<double> m1{{1.0, 2.0, 3.0}, {4.0, 5.0, 6.0}};

    phylanx::ir::node_data<std::int64_t> expected(
        blaze::DynamicVector<std::int64_t>{0, 0});

    phylanx::execution_tree::primitive lhs =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(m1));

    phylanx::execution_tree::primitive rhs =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<std::int64_t>(1));

    phylanx::execution_tree::primitive p =
        phylanx::execution_tree::primitives::create_argmin(hpx::find_here(),
            phylanx::execution_tree::primitive_arguments_type{
                std::move(lhs), std::move(rhs)});

    hpx::future<arg_type> f = p.eval();

    auto actual =
        phylanx::execution_tree::extract_integer_value_strict(f.get());

    HPX_TEST_EQ(expected, actual);
}

#if defined(PHYLANX_HAVE_BLAZE_TENSOR)
void test_argmin_3d_flat()
{
    blaze::DynamicTensor<double> t1{
        {{1.0, 20.0, 3.0}, {40.0, 5.0, 60.0}},
        {{10.0, 2.0, 30.0}, {4.0, 50.0, 6.0}}};

    std::int64_t expected = 0;

    phylanx::execution_tree::primitive lhs =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(t1));

    phylanx::execution_tree::primitive p =
        phylanx::execution_tree::primitives::create_argmin(
            hpx::find_here(),
            phylanx::execution_tree::primitive_arguments_type{
        std::move(lhs)
    });

    hpx::future<phylanx::execution_tree::primitive_argument_type> f =
        p.eval();


    HPX_TEST_EQ(phylanx::ir::node_data<std::int64_t>(expected),
        phylanx::execution_tree::extract_integer_value_strict(f.get()));
}

void test_argmin_3d_0_axis()
{
    using arg_type = phylanx::execution_tree::primitive_argument_type;

    blaze::DynamicTensor<double> t1{
        {{1.0, 20.0, 3.0}, {40.0, 5.0, 60.0}},
        {{10.0, 2.0, 30.0}, {4.0, 50.0, 6.0}}};

    phylanx::ir::node_data<std::int64_t> expected(
        blaze::DynamicMatrix<std::int64_t>{{0, 1, 0}, {1, 0, 1}});

    phylanx::execution_tree::primitive lhs =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(t1));

    phylanx::execution_tree::primitive rhs =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<std::int64_t>(0));

    phylanx::execution_tree::primitive p =
        phylanx::execution_tree::primitives::create_argmin(hpx::find_here(),
            phylanx::execution_tree::primitive_arguments_type{
                std::move(lhs), std::move(rhs)});

    hpx::future<arg_type> f = p.eval();

    auto actual =
        phylanx::execution_tree::extract_integer_value_strict(f.get());

    HPX_TEST_EQ(expected, actual);
}

void test_argmin_3d_1_axis()
{
    using arg_type = phylanx::execution_tree::primitive_argument_type;

    blaze::DynamicTensor<double> t1{
        {{1.0, 20.0, 3.0}, {40.0, 5.0, 60.0}},
        {{10.0, 2.0, 30.0}, {4.0, 50.0, 6.0}}};

    phylanx::ir::node_data<std::int64_t> expected(
        blaze::DynamicMatrix<std::int64_t>{{0, 1, 0}, {1, 0, 1}});

    phylanx::execution_tree::primitive lhs =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(t1));

    phylanx::execution_tree::primitive rhs =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<std::int64_t>(1));

    phylanx::execution_tree::primitive p =
        phylanx::execution_tree::primitives::create_argmin(hpx::find_here(),
            phylanx::execution_tree::primitive_arguments_type{
                std::move(lhs), std::move(rhs)});

    hpx::future<arg_type> f = p.eval();

    auto actual =
        phylanx::execution_tree::extract_integer_value_strict(f.get());

    HPX_TEST_EQ(expected, actual);
}

void test_argmin_3d_2_axis()
{
    using arg_type = phylanx::execution_tree::primitive_argument_type;

    blaze::DynamicTensor<double> t1{
        {{1.0, 20.0, 3.0}, {40.0, 5.0, 60.0}},
        {{10.0, 2.0, 30.0}, {4.0, 50.0, 6.0}}};

    phylanx::ir::node_data<std::int64_t> expected(
        blaze::DynamicMatrix<std::int64_t>({{0, 1}, {1, 0}}));

    phylanx::execution_tree::primitive lhs =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(t1));

    phylanx::execution_tree::primitive rhs =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<std::int64_t>(2));

    phylanx::execution_tree::primitive p =
        phylanx::execution_tree::primitives::create_argmin(hpx::find_here(),
            phylanx::execution_tree::primitive_arguments_type{
                std::move(lhs), std::move(rhs)});

    hpx::future<arg_type> f = p.eval();

    auto actual =
        phylanx::execution_tree::extract_integer_value_strict(f.get());

    HPX_TEST_EQ(expected, actual);
}
#endif

int main(int argc, char* argv[])
{
    test_argmin_0d();
    test_argmin_1d();
    test_argmin_2d_flat();
    test_argmin_2d_x_axis();
    test_argmin_2d_y_axis();

#if defined(PHYLANX_HAVE_BLAZE_TENSOR)
    test_argmin_3d_flat();
    test_argmin_3d_0_axis();
    test_argmin_3d_1_axis();
    test_argmin_3d_2_axis();
#endif

    return hpx::util::report_errors();
}
