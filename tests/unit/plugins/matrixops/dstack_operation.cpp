//  Copyright (c) 2018 Hartmut Kaiser
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/phylanx.hpp>

#include <hpx/hpx_main.hpp>
#include <hpx/include/lcos.hpp>
#include <hpx/util/lightweight_test.hpp>

#if defined(PHYLANX_HAVE_BLAZE_TENSOR)

#include <iostream>
#include <utility>
#include <vector>

void dstack_operation_empty()
{
    phylanx::execution_tree::primitive dstack =
        phylanx::execution_tree::primitives::create_dstack_operation(
            hpx::find_here(),
            phylanx::execution_tree::primitive_arguments_type{}, "dstack");

    hpx::future<phylanx::execution_tree::primitive_argument_type> f =
        dstack.eval();

    blaze::DynamicTensor<double> expected(0, 0, 0);

    HPX_TEST_EQ(phylanx::ir::node_data<double>(std::move(expected)),
        phylanx::execution_tree::extract_numeric_value(f.get()));
}

void dstack_operation_0d()
{
    phylanx::execution_tree::primitive first =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(),
            phylanx::execution_tree::primitive_argument_type{42.0});

    phylanx::execution_tree::primitive second =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(),
            phylanx::execution_tree::primitive_argument_type{5.0});

    phylanx::execution_tree::primitive dstack =
        phylanx::execution_tree::primitives::create_dstack_operation(
            hpx::find_here(),
            phylanx::execution_tree::primitive_arguments_type{
                phylanx::execution_tree::primitive_argument_type{std::move(first)},
                phylanx::execution_tree::primitive_argument_type{std::move(second)}
            }, "dstack");

    hpx::future<phylanx::execution_tree::primitive_argument_type> f =
        dstack.eval();

    blaze::DynamicTensor<double> expected{{{42.0, 5.0}}};

    HPX_TEST_EQ(phylanx::ir::node_data<double>(std::move(expected)),
        phylanx::execution_tree::extract_numeric_value(f.get()));
}

void dstack_operation_1d()
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

    phylanx::execution_tree::primitive dstack =
        phylanx::execution_tree::primitives::create_dstack_operation(
            hpx::find_here(),
            phylanx::execution_tree::primitive_arguments_type{
                phylanx::execution_tree::primitive_argument_type{std::move(first)},
                phylanx::execution_tree::primitive_argument_type{std::move(second)}
            }, "dstack");

    hpx::future<phylanx::execution_tree::primitive_argument_type> f =
        dstack.eval();

    blaze::DynamicTensor<double> expected{
        {{1, 11}, {2, 12}, {3, 13}, {4, 14}, {5, 15}}};

    HPX_TEST_EQ(phylanx::ir::node_data<double>(std::move(expected)),
        phylanx::execution_tree::extract_numeric_value(f.get()));
}

void dstack_operation_2d()
{
    blaze::DynamicMatrix<double> m1{{1, 2, 3}, {4, 5, 6}};
    blaze::DynamicMatrix<double> m2{{11, 12, 13}, {14, 15, 16}};

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

    phylanx::execution_tree::primitive dstack =
        phylanx::execution_tree::primitives::create_dstack_operation(
            hpx::find_here(),
            phylanx::execution_tree::primitive_arguments_type{
                phylanx::execution_tree::primitive_argument_type{std::move(first)},
                phylanx::execution_tree::primitive_argument_type{std::move(second)}
            }, "dstack");

    blaze::DynamicTensor<double> expected{
        {{1, 11}, {2, 12}, {3, 13}}, {{4, 14}, {5, 15}, {6, 16}}};

    hpx::future<phylanx::execution_tree::primitive_argument_type> f =
        dstack.eval();

    HPX_TEST_EQ(phylanx::ir::node_data<double>(std::move(expected)),
                phylanx::execution_tree::extract_numeric_value(f.get()));
}

void dstack_operation_2d_3d_mix()
{
    blaze::DynamicTensor<double> t1{
        {{1, 2}, {3, 4}, {5, 6}}, {{1, 2}, {3, 4}, {5, 6}}};
    blaze::DynamicTensor<double> t2{
        {{11, 12, 13}, {14, 15, 16}, {17, 18, 19}},
        {{21, 22, 23}, {24, 25, 26}, {27, 28, 29}}};

    blaze::DynamicMatrix<double> m1{{1, 2, 3}, {4, 5, 6}};
    blaze::DynamicMatrix<double> m2{{11, 12, 13}, {14, 15, 16}};

    phylanx::execution_tree::primitive first =
        phylanx::execution_tree::primitives::create_variable(hpx::find_here(),
            phylanx::execution_tree::primitive_argument_type{
                phylanx::ir::node_data<double>(t1)});

    phylanx::execution_tree::primitive second =
        phylanx::execution_tree::primitives::create_variable(hpx::find_here(),
            phylanx::execution_tree::primitive_argument_type{
                phylanx::ir::node_data<double>(t2)});

    phylanx::execution_tree::primitive firstvec =
        phylanx::execution_tree::primitives::create_variable(hpx::find_here(),
            phylanx::execution_tree::primitive_argument_type{
                phylanx::ir::node_data<double>(m1)});

    phylanx::execution_tree::primitive secondvec =
        phylanx::execution_tree::primitives::create_variable(hpx::find_here(),
            phylanx::execution_tree::primitive_argument_type{
                phylanx::ir::node_data<double>(m2)});

    phylanx::execution_tree::primitive dstack =
        phylanx::execution_tree::primitives::create_dstack_operation(
            hpx::find_here(),
            phylanx::execution_tree::primitive_arguments_type{
                phylanx::execution_tree::primitive_argument_type{
                    std::move(first)},
                phylanx::execution_tree::primitive_argument_type{
                    std::move(firstvec)},
                phylanx::execution_tree::primitive_argument_type{
                    std::move(secondvec)},
                phylanx::execution_tree::primitive_argument_type{
                    std::move(second)}
            }, "dstack");

    blaze::DynamicTensor<double> expected{
        {{1, 2, 1, 11, 11, 12, 13}, {3, 4, 2, 12, 14, 15, 16},
            {5, 6, 3, 13, 17, 18, 19}},
        {{1, 2, 4, 14, 21, 22, 23}, {3, 4, 5, 15, 24, 25, 26},
            {5, 6, 6, 16, 27, 28, 29}}};

    hpx::future<phylanx::execution_tree::primitive_argument_type> f =
        dstack.eval();

    HPX_TEST_EQ(phylanx::ir::node_data<double>(std::move(expected)),
        phylanx::execution_tree::extract_numeric_value(f.get()));
}
#endif

int main(int argc, char* argv[])
{
#if defined(PHYLANX_HAVE_BLAZE_TENSOR)
    dstack_operation_empty();
    dstack_operation_0d();
    dstack_operation_1d();
    dstack_operation_2d();
    dstack_operation_2d_3d_mix();
#endif

    return hpx::util::report_errors();
}

