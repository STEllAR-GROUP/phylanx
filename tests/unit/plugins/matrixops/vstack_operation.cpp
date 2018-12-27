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

void vstack_operation_empty()
{
    phylanx::execution_tree::primitive vstack =
        phylanx::execution_tree::primitives::create_vstack_operation(
            hpx::find_here(),
            phylanx::execution_tree::primitive_arguments_type{}, "vstack");

    hpx::future<phylanx::execution_tree::primitive_argument_type> f =
        vstack.eval();

    blaze::DynamicMatrix<double> expected(0, 0);

    HPX_TEST_EQ(phylanx::ir::node_data<double>(std::move(expected)),
        phylanx::execution_tree::extract_numeric_value(f.get()));
}

void vstack_operation_0d()
{
    phylanx::execution_tree::primitive first =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(),
            phylanx::execution_tree::primitive_argument_type{42.0});

    phylanx::execution_tree::primitive second =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(),
            phylanx::execution_tree::primitive_argument_type{5.0});

    phylanx::execution_tree::primitive vstack =
        phylanx::execution_tree::primitives::create_vstack_operation(
            hpx::find_here(),
            phylanx::execution_tree::primitive_arguments_type{
                phylanx::execution_tree::primitive_argument_type{std::move(first)},
                phylanx::execution_tree::primitive_argument_type{std::move(second)}
            }, "vstack");

    hpx::future<phylanx::execution_tree::primitive_argument_type> f =
        vstack.eval();

    blaze::DynamicMatrix<double> expected{{42.0}, {5.0}};

    HPX_TEST_EQ(phylanx::ir::node_data<double>(std::move(expected)),
        phylanx::execution_tree::extract_numeric_value(f.get()));
}

void vstack_operation_1d()
{
    blaze::DynamicVector<double> v1{1, 2, 3, 4, 5};
    blaze::DynamicVector<double> v2{11, 12, 13, 14, 15};

    phylanx::execution_tree::primitive first =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(),
            phylanx::execution_tree::primitive_argument_type{
                phylanx::ir::node_data<double>(v1)});

    phylanx::execution_tree::primitive second =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(),
            phylanx::execution_tree::primitive_argument_type{
                phylanx::ir::node_data<double>(v2)});

    phylanx::execution_tree::primitive vstack =
        phylanx::execution_tree::primitives::create_vstack_operation(
            hpx::find_here(),
            phylanx::execution_tree::primitive_arguments_type{
                phylanx::execution_tree::primitive_argument_type{std::move(first)},
                phylanx::execution_tree::primitive_argument_type{std::move(second)}
            }, "vstack");

    hpx::future<phylanx::execution_tree::primitive_argument_type> f =
        vstack.eval();

    blaze::DynamicMatrix<double> expected{
        {1, 2, 3, 4, 5},
        {11, 12, 13, 14, 15}};

    HPX_TEST_EQ(phylanx::ir::node_data<double>(std::move(expected)),
        phylanx::execution_tree::extract_numeric_value(f.get()));
}

void vstack_operation_1d_2d_mix()
{
    blaze::DynamicMatrix<double> m1{{1, 2, 3}, {4, 5, 6}};
    blaze::DynamicMatrix<double> m2{{11, 22, 33}, {12, 13, 33}};
    blaze::DynamicVector<double> v1{1, 2, 3};
    blaze::DynamicVector<double> v2{13, 14, 15};

    phylanx::execution_tree::primitive first =
        phylanx::execution_tree::primitives::create_variable(hpx::find_here(),
            phylanx::execution_tree::primitive_argument_type{
                phylanx::ir::node_data<double>(m1)});

    phylanx::execution_tree::primitive second =
        phylanx::execution_tree::primitives::create_variable(hpx::find_here(),
            phylanx::execution_tree::primitive_argument_type{
                phylanx::ir::node_data<double>(m2)});

    phylanx::execution_tree::primitive firstvec =
        phylanx::execution_tree::primitives::create_variable(hpx::find_here(),
            phylanx::execution_tree::primitive_argument_type{
                phylanx::ir::node_data<double>(v1)});

    phylanx::execution_tree::primitive secondvec =
        phylanx::execution_tree::primitives::create_variable(hpx::find_here(),
            phylanx::execution_tree::primitive_argument_type{
                phylanx::ir::node_data<double>(v2)});

    phylanx::execution_tree::primitive vstack =
        phylanx::execution_tree::primitives::create_vstack_operation(
            hpx::find_here(),
            phylanx::execution_tree::primitive_arguments_type{
                phylanx::execution_tree::primitive_argument_type{
                    std::move(first)},
                phylanx::execution_tree::primitive_argument_type{
                    std::move(firstvec)},
                phylanx::execution_tree::primitive_argument_type{
                    std::move(secondvec)},
                phylanx::execution_tree::primitive_argument_type{
                    std::move(second)}}, "vstack");

    blaze::DynamicMatrix<double> expected{{1, 2, 3}, {4, 5, 6}, {1, 2, 3},
        {13, 14, 15}, {11, 22, 33}, {12, 13, 33}};

    hpx::future<phylanx::execution_tree::primitive_argument_type> f =
        vstack.eval();

    HPX_TEST_EQ(phylanx::ir::node_data<double>(std::move(expected)),
        phylanx::execution_tree::extract_numeric_value(f.get()));
}

void vstack_operation_2d()
{
    blaze::DynamicMatrix<double> m1{{1, 2, 3}, {4, 5, 6}};
    blaze::DynamicMatrix<double> m2{{11, 22, 33}, {12, 13, 33}};

    phylanx::execution_tree::primitive first =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(),
            phylanx::execution_tree::primitive_argument_type{
                phylanx::ir::node_data<double>(m1)});

    phylanx::execution_tree::primitive second =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(),
            phylanx::execution_tree::primitive_argument_type{
                phylanx::ir::node_data<double>(m2)});

    phylanx::execution_tree::primitive vstack =
        phylanx::execution_tree::primitives::create_vstack_operation(
            hpx::find_here(),
            phylanx::execution_tree::primitive_arguments_type{
                phylanx::execution_tree::primitive_argument_type{std::move(first)},
                phylanx::execution_tree::primitive_argument_type{std::move(second)}
            }, "vstack");

    blaze::DynamicMatrix<double> expected{
        {1, 2, 3}, {4, 5, 6}, {11, 22, 33}, {12, 13, 33}};

    hpx::future<phylanx::execution_tree::primitive_argument_type> f =
        vstack.eval();

    HPX_TEST_EQ(phylanx::ir::node_data<double>(std::move(expected)),
                phylanx::execution_tree::extract_numeric_value(f.get()));
}

#if defined(PHYLANX_HAVE_BLAZE_TENSOR)
void vstack_operation_3d()
{
    blaze::DynamicTensor<double> t1{
        {{1, 2, 3}, {4, 5, 6}}, {{1, 2, 3}, {4, 5, 6}}};
    blaze::DynamicTensor<double> t2{{{11, 22, 33}, {12, 13, 33}}};

    phylanx::execution_tree::primitive first =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(),
            phylanx::execution_tree::primitive_argument_type{
                phylanx::ir::node_data<double>(t1)});

    phylanx::execution_tree::primitive second =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(),
            phylanx::execution_tree::primitive_argument_type{
                phylanx::ir::node_data<double>(t2)});

    phylanx::execution_tree::primitive vstack =
        phylanx::execution_tree::primitives::create_vstack_operation(
            hpx::find_here(),
            phylanx::execution_tree::primitive_arguments_type{
                phylanx::execution_tree::primitive_argument_type{std::move(first)},
                phylanx::execution_tree::primitive_argument_type{std::move(second)}
            }, "vstack");

    blaze::DynamicTensor<double> expected{{{1, 2, 3}, {4, 5, 6}},
        {{1, 2, 3}, {4, 5, 6}}, {{11, 22, 33}, {12, 13, 33}}};

    hpx::future<phylanx::execution_tree::primitive_argument_type> f =
        vstack.eval();

    HPX_TEST_EQ(phylanx::ir::node_data<double>(std::move(expected)),
                phylanx::execution_tree::extract_numeric_value(f.get()));
}
#endif

int main(int argc, char* argv[])
{
    vstack_operation_empty();
    vstack_operation_0d();
    vstack_operation_1d();
    vstack_operation_1d_2d_mix();
    vstack_operation_2d();
#if defined(PHYLANX_HAVE_BLAZE_TENSOR)
    vstack_operation_3d();
#endif

    return hpx::util::report_errors();
}
