//  Copyright (c) 2018 Bibek Wagle
//  Copyright (c) 2018 Hartmut Kaiser
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

void hstack_operation_empty()
{
    phylanx::execution_tree::primitive hstack =
        phylanx::execution_tree::primitives::create_hstack_operation(
            hpx::find_here(),
            phylanx::execution_tree::primitive_arguments_type{}, "hstack");

    hpx::future<phylanx::execution_tree::primitive_argument_type> f =
        hstack.eval();

    blaze::DynamicVector<double> expected(0);

    HPX_TEST_EQ(phylanx::ir::node_data<double>(std::move(expected)),
        phylanx::execution_tree::extract_numeric_value(f.get()));
}

void hstack_operation_0d()
{
    phylanx::execution_tree::primitive first =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(42.0));

    phylanx::execution_tree::primitive second =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(5.0));

    phylanx::execution_tree::primitive hstack =
        phylanx::execution_tree::primitives::create_hstack_operation(
            hpx::find_here(),
            phylanx::execution_tree::primitive_arguments_type{
                std::move(first), std::move(second)}, "hstack");

    hpx::future<phylanx::execution_tree::primitive_argument_type> f =
        hstack.eval();

    blaze::DynamicVector<double> expected{42.0, 5.0};

    HPX_TEST_EQ(phylanx::ir::node_data<double>(std::move(expected)),
        phylanx::execution_tree::extract_numeric_value(f.get()));
}

void hstack_operation_1d()
{
    blaze::DynamicVector<double> v1{1, 2, 3, 4, 5};
    blaze::DynamicVector<double> v2{11, 12, 13, 14, 15, 16};

    phylanx::execution_tree::primitive first =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(v1));

    phylanx::execution_tree::primitive second =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(v2));

    phylanx::execution_tree::primitive hstack =
        phylanx::execution_tree::primitives::create_hstack_operation(
            hpx::find_here(),
            phylanx::execution_tree::primitive_arguments_type{
                std::move(first), std::move(second)}, "hstack");

    hpx::future<phylanx::execution_tree::primitive_argument_type> f =
        hstack.eval();

    blaze::DynamicVector<double> expected{
        1, 2, 3, 4, 5, 11, 12, 13, 14, 15, 16};

    HPX_TEST_EQ(phylanx::ir::node_data<double>(std::move(expected)),
        phylanx::execution_tree::extract_numeric_value(f.get()));
}

void hstack_operation_2d()
{
    blaze::DynamicMatrix<double> m1{{1, 2, 3},
                                    {4, 5, 6}};
    blaze::DynamicMatrix<double> m2{{11, 22},
                                    {12, 13}};

    phylanx::execution_tree::primitive first =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(m1));

    phylanx::execution_tree::primitive second =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(m2));

    phylanx::execution_tree::primitive hstack =
        phylanx::execution_tree::primitives::create_hstack_operation(
            hpx::find_here(),
            phylanx::execution_tree::primitive_arguments_type{
                std::move(first), std::move(second)}, "hstack");

    blaze::DynamicMatrix<double> expected{{1, 2, 3, 11, 22},
                                          {4, 5, 6, 12, 13}};

    hpx::future<phylanx::execution_tree::primitive_argument_type> f =
        hstack.eval();

    HPX_TEST_EQ(phylanx::ir::node_data<double>(std::move(expected)),
        phylanx::execution_tree::extract_numeric_value(f.get()));
}

void hstack_operation_0d_1d_1d_0d()
{
    blaze::DynamicVector<double> v1{1, 2, 3, 4, 5};
    blaze::DynamicVector<double> v2{11, 12, 13, 14, 15, 16};

    phylanx::execution_tree::primitive first =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(42.0));

    phylanx::execution_tree::primitive second =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(5.0));

    phylanx::execution_tree::primitive firstvec =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(v1));

    phylanx::execution_tree::primitive secondvec =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(v2));

    phylanx::execution_tree::primitive hstack =
        phylanx::execution_tree::primitives::create_hstack_operation(
            hpx::find_here(),
            phylanx::execution_tree::primitive_arguments_type{
                std::move(first), std::move(firstvec), std::move(secondvec),
                std::move(second)}, "hstack");

    hpx::future<phylanx::execution_tree::primitive_argument_type> f =
        hstack.eval();

    blaze::DynamicVector<double> expected{
        42, 1, 2, 3, 4, 5, 11, 12, 13, 14, 15, 16, 5};

    HPX_TEST_EQ(phylanx::ir::node_data<double>(std::move(expected)),
                phylanx::execution_tree::extract_numeric_value(f.get()));
}

void hstack_operation_1d_0d_0d_1d()
{
    blaze::DynamicVector<double> v1{1, 2, 3, 4, 5};
    blaze::DynamicVector<double> v2{11, 12, 13, 14, 15, 16};

    phylanx::execution_tree::primitive first =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(42.0));

    phylanx::execution_tree::primitive second =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(5.0));

    phylanx::execution_tree::primitive firstvec =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(v1));

    phylanx::execution_tree::primitive secondvec =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(v2));

    phylanx::execution_tree::primitive hstack =
        phylanx::execution_tree::primitives::create_hstack_operation(
            hpx::find_here(),
            phylanx::execution_tree::primitive_arguments_type{
                std::move(firstvec), std::move(first), std::move(second),
                std::move(secondvec)}, "hstack");

    hpx::future<phylanx::execution_tree::primitive_argument_type> f =
        hstack.eval();

    blaze::DynamicVector<double> expected{
        1, 2, 3, 4, 5, 42, 5, 11, 12, 13, 14, 15, 16};

    HPX_TEST_EQ(phylanx::ir::node_data<double>(std::move(expected)),
                phylanx::execution_tree::extract_numeric_value(f.get()));
}

#if defined(PHYLANX_HAVE_BLAZE_TENSOR)
void hstack_operation_3d()
{
    blaze::DynamicTensor<double> t1{
        {{1, 2, 3}, {4, 5, 6}}, {{1, 2, 3}, {4, 5, 6}}};
    blaze::DynamicTensor<double> t2{
        {{11, 22, 33}, {12, 13, 14}}, {{11, 22, 33}, {12, 13, 14}}};

    phylanx::execution_tree::primitive first =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(t1));

    phylanx::execution_tree::primitive second =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(t2));

    phylanx::execution_tree::primitive hstack =
        phylanx::execution_tree::primitives::create_hstack_operation(
            hpx::find_here(),
            phylanx::execution_tree::primitive_arguments_type{
                std::move(first), std::move(second)}, "hstack");

    blaze::DynamicTensor<double> expected{
        {{1, 2, 3}, {4, 5, 6}, {11, 22, 33}, {12, 13, 14}},
        {{1, 2, 3}, {4, 5, 6}, {11, 22, 33}, {12, 13, 14}}};

    hpx::future<phylanx::execution_tree::primitive_argument_type> f =
        hstack.eval();

    HPX_TEST_EQ(phylanx::ir::node_data<double>(std::move(expected)),
        phylanx::execution_tree::extract_numeric_value(f.get()));
}
#endif

int main(int argc, char* argv[])
{
    hstack_operation_empty();

    hstack_operation_0d();
    hstack_operation_1d();
    hstack_operation_2d();

    hstack_operation_0d_1d_1d_0d();
    hstack_operation_1d_0d_0d_1d();

#if defined(PHYLANX_HAVE_BLAZE_TENSOR)
    hstack_operation_3d();
#endif

    return hpx::util::report_errors();
}
