// Copyright (c) 2018 Bita Hasheminezhad
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
void test_softplus_operation_0d()
{
    phylanx::execution_tree::primitive arg =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(42.0));

    phylanx::execution_tree::primitive softplus =
        phylanx::execution_tree::primitives::create_softplus_operation(
            hpx::find_here(),
            phylanx::execution_tree::primitive_arguments_type{
                std::move(arg)});

    hpx::future<phylanx::execution_tree::primitive_argument_type> f =
        softplus.eval();

    HPX_TEST(allclose(phylanx::ir::node_data<double>(42.0),
        phylanx::execution_tree::extract_numeric_value(f.get())));
}

void test_softplus_operation_1d()
{
    blaze::DynamicVector<double> subject{1., 2., 3.};

    phylanx::execution_tree::primitive arg =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(subject));

    phylanx::execution_tree::primitive softplus =
        phylanx::execution_tree::primitives::create_softplus_operation(
            hpx::find_here(),
            phylanx::execution_tree::primitive_arguments_type{std::move(arg)});

    hpx::future<phylanx::execution_tree::primitive_argument_type> f =
        softplus.eval();

    blaze::DynamicVector<double> expected{1.31326, 2.12693, 3.04859};

    HPX_TEST(allclose(phylanx::ir::node_data<double>(std::move(expected)),
        phylanx::execution_tree::extract_numeric_value(f.get())));
}

void test_softplus_operation_2d()
{
    blaze::DynamicMatrix<std::int64_t> subject{{1, 2, 3}, {4, 1, 2}, {3, 4, 1}};

    phylanx::execution_tree::primitive arg =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<std::int64_t>(subject));

    phylanx::execution_tree::primitive softplus =
        phylanx::execution_tree::primitives::create_softplus_operation(
            hpx::find_here(),
            phylanx::execution_tree::primitive_arguments_type{std::move(arg)});

    hpx::future<phylanx::execution_tree::primitive_argument_type> f =
        softplus.eval();

    blaze::DynamicMatrix<double> expected{{1.31326, 2.12693, 3.04859},
        {4.01815, 1.31326, 2.12693}, {3.04859, 4.01815, 1.31326}};

    HPX_TEST(allclose(phylanx::ir::node_data<double>(std::move(expected)),
        phylanx::execution_tree::extract_numeric_value(f.get())));
}

#if defined(PHYLANX_HAVE_BLAZE_TENSOR)
void test_softplus_operation_3d()
{
    blaze::DynamicTensor<double> subject{
        {{1.0, 2.0, 3.0}, {4.0, 1.0, 2.0}, {3.0, 4.0, 1.0}},
        {{3.0, 6.0, 2.0}, {-2.0, 2.0, 0.0}, {1.0, 1.0, 3.0}}};

    phylanx::execution_tree::primitive arg =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<std::int64_t>(subject));

    phylanx::execution_tree::primitive softplus =
        phylanx::execution_tree::primitives::create_softplus_operation(
            hpx::find_here(),
            phylanx::execution_tree::primitive_arguments_type{std::move(arg)});

    hpx::future<phylanx::execution_tree::primitive_argument_type> f =
        softplus.eval();

    blaze::DynamicTensor<double> expected{
        {{1.31326, 2.12693, 3.04859}, {4.01815, 1.31326, 2.12693},
            {3.04859, 4.01815, 1.31326}},
        {{3.04859, 6.00248, 2.12693}, {0.126928, 2.12693, 0.693147},
            {1.31326, 1.31326, 3.04859}}};

    HPX_TEST(allclose(phylanx::ir::node_data<double>(std::move(expected)),
        phylanx::execution_tree::extract_numeric_value(f.get())));
}
#endif

int main(int argc, char* argv[])
{
    test_softplus_operation_0d();
    test_softplus_operation_1d();
    test_softplus_operation_2d();

#if defined(PHYLANX_HAVE_BLAZE_TENSOR)
    test_softplus_operation_3d();
#endif

    return hpx::util::report_errors();
}
