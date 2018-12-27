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
void test_softmax_operation_0d()
{
    phylanx::execution_tree::primitive arg =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(42.0));

    phylanx::execution_tree::primitive softmax =
        phylanx::execution_tree::primitives::create_softmax_operation(
            hpx::find_here(),
            phylanx::execution_tree::primitive_arguments_type{
                std::move(arg)});

    hpx::future<phylanx::execution_tree::primitive_argument_type> f =
        softmax.eval();

    HPX_TEST_EQ(
        1.0, phylanx::execution_tree::extract_numeric_value(f.get())[0]);
}

void test_softmax_operation_1d()
{
    blaze::DynamicVector<double> subject{41., 42., 43.};
    phylanx::execution_tree::primitive arg =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(subject));

    phylanx::execution_tree::primitive softmax =
        phylanx::execution_tree::primitives::create_softmax_operation(
            hpx::find_here(),
            phylanx::execution_tree::primitive_arguments_type{std::move(arg)});

    hpx::future<phylanx::execution_tree::primitive_argument_type> f =
        softmax.eval();

    blaze::DynamicVector<double> expected{0.09003057, 0.24472847, 0.66524096};

    HPX_TEST_EQ(phylanx::ir::node_data<double>(std::move(expected)),
        phylanx::execution_tree::extract_numeric_value(f.get()));
}

void test_softmax_operation_2d()
{
    blaze::DynamicMatrix<std::int64_t> subject{{1, 2, 3},
                                               {4, 1, 2},
                                               {3, 4, 1}};
    phylanx::execution_tree::primitive arg =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<std::int64_t>(subject));

    phylanx::execution_tree::primitive softmax =
        phylanx::execution_tree::primitives::create_softmax_operation(
            hpx::find_here(),
            phylanx::execution_tree::primitive_arguments_type{std::move(arg)});

    hpx::future<phylanx::execution_tree::primitive_argument_type> f =
        softmax.eval();

    blaze::DynamicMatrix<double> expected{{0.09003057,  0.24472847,  0.66524096},
                                          {0.84379473,  0.04201007,  0.1141952 },
                                          {0.25949646,  0.70538451,  0.03511903}};

    HPX_TEST_EQ(phylanx::ir::node_data<double>(std::move(expected)),
        phylanx::execution_tree::extract_numeric_value(f.get()));
}

void test_softmax_operation_2d_column()
{
    blaze::DynamicMatrix<std::int64_t> subject{{1, 2, 3},
                                               {3, 4, 1}};
    phylanx::execution_tree::primitive arg0 =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<std::int64_t>(subject));

    phylanx::execution_tree::primitive arg1 =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<std::int64_t>(1));

    phylanx::execution_tree::primitive softmax =
        phylanx::execution_tree::primitives::create_softmax_operation(
            hpx::find_here(),
            phylanx::execution_tree::primitive_arguments_type{
                std::move(arg0), std::move(arg1)});

    hpx::future<phylanx::execution_tree::primitive_argument_type> f =
        softmax.eval();

    blaze::DynamicMatrix<double> expected{{0.09003057,  0.24472847,  0.66524096},
                                          {0.25949646,  0.70538451,  0.03511903}};

    HPX_TEST_EQ(phylanx::ir::node_data<double>(std::move(expected)),
        phylanx::execution_tree::extract_numeric_value(f.get()));
}

void test_softmax_operation_2d_row()
{
    blaze::DynamicMatrix<std::int64_t> subject{{1, 2, 3},
                                               {3, 4, 1}};
    phylanx::execution_tree::primitive arg0 =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<std::int64_t>(subject));

    phylanx::execution_tree::primitive arg1 =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<std::int64_t>(0));

    phylanx::execution_tree::primitive softmax =
        phylanx::execution_tree::primitives::create_softmax_operation(
            hpx::find_here(),
            phylanx::execution_tree::primitive_arguments_type{
                std::move(arg0), std::move(arg1)});

    hpx::future<phylanx::execution_tree::primitive_argument_type> f =
        softmax.eval();

    blaze::DynamicMatrix<double> expected{{0.11920292, 0.11920292, 0.88079708},
                                          {0.88079708, 0.88079708, 0.11920292}};

    HPX_TEST_EQ(phylanx::ir::node_data<double>(std::move(expected)),
        phylanx::execution_tree::extract_numeric_value(f.get()));
}

int main(int argc, char* argv[])
{
    test_softmax_operation_0d();
    test_softmax_operation_1d();
    test_softmax_operation_2d();
    test_softmax_operation_2d_column();
    test_softmax_operation_2d_row();

    return hpx::util::report_errors();
}
