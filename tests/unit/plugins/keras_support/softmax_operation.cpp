// Copyright (c) 2018 Bita Hasheminezhad
// Copyright (c) 2018 Hartmut Kaiser
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/phylanx.hpp>

#include <hpx/hpx_main.hpp>
#include <hpx/include/lcos.hpp>
#include <hpx/testing.hpp>

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

void test_softmax_operation_3d()
{
    blaze::DynamicTensor<double> subject{
        {{1.0, 2.0, 3.0}, {4.0, 1.0, 2.0}, {3.0, 4.0, 1.0}},
        {{3.0, 6.0, 2.0}, {-2.0, 2.0, 0.0}, {1.0, 1.0, 3.0}}};

    phylanx::execution_tree::primitive arg =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<std::int64_t>(subject));

    phylanx::execution_tree::primitive softmax =
        phylanx::execution_tree::primitives::create_softmax_operation(
            hpx::find_here(),
            phylanx::execution_tree::primitive_arguments_type{std::move(arg)});

    hpx::future<phylanx::execution_tree::primitive_argument_type> f =
        softmax.eval();

    blaze::DynamicTensor<double> expected{
        {{0.09003057, 0.24472847, 0.66524096},
            {0.84379473, 0.04201007, 0.1141952},
            {0.25949646, 0.70538451, 0.03511903}},
        {{0.04661262, 0.93623955, 0.01714783},
            {0.01587624, 0.86681333, 0.11731043},
            {0.10650698, 0.10650698, 0.78698604}}};

    HPX_TEST_EQ(phylanx::ir::node_data<double>(std::move(expected)),
        phylanx::execution_tree::extract_numeric_value(f.get()));
}

void test_softmax_operation_3d_page()
{
    blaze::DynamicTensor<std::int64_t> subject{
        {{1, 2}, {4, 1}, {3, 4}}, {{3, 6}, {-2, 2}, {1, 1}}};

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

    blaze::DynamicTensor<double> expected{
        {{0.11920292, 0.01798621}, {0.99752738, 0.26894142},
            {0.88079708, 0.95257413}},
        {{0.88079708, 0.98201379}, {0.00247262, 0.73105858},
            {0.11920292, 0.04742587}}};

    HPX_TEST_EQ(phylanx::ir::node_data<double>(std::move(expected)),
        phylanx::execution_tree::extract_numeric_value(f.get()));
}

void test_softmax_operation_3d_row()
{
    blaze::DynamicTensor<double> subject{
        {{1.0, 2.0, 3.0}, {4.0, 1.0, 2.0}, {3.0, 4.0, 1.0}},
        {{3.0, 6.0, 2.0}, {-2.0, 2.0, 0.0}, {1.0, 1.0, 3.0}}};

    phylanx::execution_tree::primitive arg0 =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<std::int64_t>(subject));

    phylanx::execution_tree::primitive arg1 =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<std::int64_t>(-2));

    phylanx::execution_tree::primitive softmax =
        phylanx::execution_tree::primitives::create_softmax_operation(
            hpx::find_here(),
            phylanx::execution_tree::primitive_arguments_type{
                std::move(arg0), std::move(arg1)});

    hpx::future<phylanx::execution_tree::primitive_argument_type> f =
        softmax.eval();

    blaze::DynamicTensor<double> expected{
        {{0.03511903, 0.1141952, 0.66524096},
            {0.70538451, 0.04201007, 0.24472847},
            {0.25949646, 0.84379473, 0.09003057}},
        {{0.8756006, 0.97555875, 0.25949646},
            {0.00589975, 0.01786798, 0.03511903},
            {0.11849965, 0.00657326, 0.70538451}}};

    HPX_TEST_EQ(phylanx::ir::node_data<double>(std::move(expected)),
        phylanx::execution_tree::extract_numeric_value(f.get()));
}

void test_softmax_operation_3d_column()
{
    blaze::DynamicTensor<std::int64_t> subject{
        {{1, 2}, {4, 1}, {3, 4}}, {{3, 6}, {-2, 2}, {1, 1}}};

    phylanx::execution_tree::primitive arg0 =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<std::int64_t>(subject));

    phylanx::execution_tree::primitive arg1 =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<std::int64_t>(-1));

    phylanx::execution_tree::primitive softmax =
        phylanx::execution_tree::primitives::create_softmax_operation(
            hpx::find_here(),
            phylanx::execution_tree::primitive_arguments_type{
                std::move(arg0), std::move(arg1)});

    hpx::future<phylanx::execution_tree::primitive_argument_type> f =
        softmax.eval();

    blaze::DynamicTensor<double> expected{
        {{0.26894142, 0.73105858}, {0.95257413, 0.04742587},
            {0.26894142, 0.73105858}},
        {{0.04742587, 0.95257413}, {0.01798621, 0.98201379}, {0.5, 0.5}}};

    HPX_TEST_EQ(phylanx::ir::node_data<double>(std::move(expected)),
        phylanx::execution_tree::extract_numeric_value(f.get()));
}


void test_softmax_operation_4d()
{
    blaze::DynamicArray<4UL, std::int64_t> subject{
        {{{{1, 2}, {4, 1}, {3, 4}}, {{3, 6}, {-2, 2}, {1, 1}}},
            {{{1, 2}, {4, 1}, {3, 4}}, {{3, 6}, {-2, 2}, {1, 1}}}}};

    phylanx::execution_tree::primitive arg =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<std::int64_t>(subject));

    phylanx::execution_tree::primitive softmax =
        phylanx::execution_tree::primitives::create_softmax_operation(
            hpx::find_here(),
            phylanx::execution_tree::primitive_arguments_type{std::move(arg)});

    hpx::future<phylanx::execution_tree::primitive_argument_type> f =
        softmax.eval();

    blaze::DynamicArray<4UL, double> expected{
        {{{{0.26894143, 0.7310586}, {0.95257413, 0.04742587},
              {0.26894143, 0.7310586}},
             {{0.04742587, 0.95257413}, {0.01798621, 0.98201376}, {0.5, 0.5}}},
            {{{0.26894143, 0.7310586}, {0.95257413, 0.04742587},
                 {0.26894143, 0.7310586}},
                {{0.04742587, 0.95257413}, {0.01798621, 0.98201376},
                    {0.5, 0.5}}}}};

    HPX_TEST(allclose(phylanx::ir::node_data<double>(std::move(expected)),
        phylanx::execution_tree::extract_numeric_value(std::move(f.get()))));
}

void test_softmax_operation_4d_axis0()
{
    blaze::DynamicArray<4UL, double> subject{
        {{{{1.0, 2.0, 3.0}, {0.0, 1.0, 2.0}, {3.0, 4.0, 1.0}},
             {{3.0, 1.0, 2.0}, {-2.0, 2.0, 0.0}, {1.0, 1.0, 3.0}}},
            {{{1.0, 1.0, 3.0}, {-4.0, 1.0, 2.0}, {3.0, 1.0, 4.0}},
                {{3.0, -1.0, 4.0}, {-2.0, 2.0, 3.0}, {1.0, 1.0, 4.0}}}}};

    phylanx::execution_tree::primitive arg0 =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(subject));

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

    blaze::DynamicArray<4UL, double> expected{
        {{{{0.5, 0.7310586, 0.5}, {0.98201376, 0.5, 0.5},
              {0.5, 0.95257413, 0.04742587}},

             {{0.5, 0.880797, 0.11920291}, {0.5, 0.5, 0.04742587},
                 {0.5, 0.5, 0.26894143}}},

            {{{0.5, 0.26894143, 0.5}, {0.01798621, 0.5, 0.5},
                 {0.5, 0.04742587, 0.95257413}},

                {{0.5, 0.11920291, 0.880797}, {0.5, 0.5, 0.95257413},
                    {0.5, 0.5, 0.7310586}}}}};

    HPX_TEST(allclose(phylanx::ir::node_data<double>(std::move(expected)),
        phylanx::execution_tree::extract_numeric_value(std::move(f.get()))));
}

void test_softmax_operation_4d_axis1()
{
    blaze::DynamicArray<4UL, std::int64_t> subject{
        {{{{1, 2}, {4, 1}, {3, 4}}, {{3, 6}, {-2, 2}, {4, 5}}},
            {{{1, 2}, {-1, 1}, {3, 4}}, {{3, 6}, {-2, 2}, {1, 1}}}}};

    phylanx::execution_tree::primitive arg0 =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<std::int64_t>(subject));

    phylanx::execution_tree::primitive arg1 =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<std::int64_t>(-3));

    phylanx::execution_tree::primitive softmax =
        phylanx::execution_tree::primitives::create_softmax_operation(
            hpx::find_here(),
            phylanx::execution_tree::primitive_arguments_type{
                std::move(arg0), std::move(arg1)});

    hpx::future<phylanx::execution_tree::primitive_argument_type> f =
        softmax.eval();

    blaze::DynamicArray<4UL, double> expected{
        {{{{0.11920291, 0.01798621}, {0.99752742, 0.26894143},
              {0.26894143, 0.26894143}},
             {{0.88079703, 0.98201376}, {0.00247262, 0.7310586},
                 {0.7310586, 0.7310586}}},
            {{{0.11920291, 0.01798621}, {0.7310586, 0.26894143},
                 {0.88079703, 0.95257413}},
                {{0.88079703, 0.98201376}, {0.26894143, 0.7310586},
                    {0.11920291, 0.04742587}}}}};

    HPX_TEST(allclose(phylanx::ir::node_data<double>(std::move(expected)),
        phylanx::execution_tree::extract_numeric_value(f.get())));
}

void test_softmax_operation_4d_axis2()
{
    blaze::DynamicArray<4UL, std::int64_t> subject{
        {{{{1, 2}, {4, 1}, {3, 4}}, {{3, 6}, {-2, 2}, {1, 1}}},
            {{{1, -2}, {4, 1}, {3, 4}}, {{-3, 6}, {-2, 2}, {1, 1}}}}};

    phylanx::execution_tree::primitive arg0 =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<std::int64_t>(subject));

    phylanx::execution_tree::primitive arg1 =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<std::int64_t>(-2));

    phylanx::execution_tree::primitive softmax =
        phylanx::execution_tree::primitives::create_softmax_operation(
            hpx::find_here(),
            phylanx::execution_tree::primitive_arguments_type{
                std::move(arg0), std::move(arg1)});

    hpx::future<phylanx::execution_tree::primitive_argument_type> f =
        softmax.eval();

    blaze::DynamicArray<4UL, double> expected{
        {{{{0.03511903, 0.11419519}, {0.70538455, 0.04201007},
              {0.25949648, 0.8437947}},
             {{0.87560058, 0.9755587}, {0.00589975, 0.01786798},
                 {0.11849965, 0.00657326}}},
            {{{0.03511903, 0.00235563}, {0.70538455, 0.04731416},
                 {0.25949648, 0.95033026}},
                {{0.01714783, 0.9755587}, {0.04661262, 0.01786798},
                    {0.93623954, 0.00657326}}}}};

    HPX_TEST(allclose(phylanx::ir::node_data<double>(std::move(expected)),
        phylanx::execution_tree::extract_numeric_value(f.get())));
}

void test_softmax_operation_4d_axis3()
{
    blaze::DynamicArray<4UL, double> subject{
        {{{{1.0, 2.0, 3.0}, {4.0, 1.0, 2.0}, {3.0, 4.0, 1.0}},
             {{3.0, 6.0, 2.0}, {-2.0, 2.0, 0.0}, {1.0, 1.0, 3.0}}},
            {{{1.0, 1.0, 3.0}, {-4.0, 1.0, 2.0}, {3.0, 1.0, 4.0}},
                {{3.0, -1.0, 4.0}, {-2.0, 2.0, 3.0}, {1.0, 1.0, 4.0}}}}};

    phylanx::execution_tree::primitive arg0 =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(subject));

    phylanx::execution_tree::primitive arg1 =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<std::int64_t>(3));

    phylanx::execution_tree::primitive softmax =
        phylanx::execution_tree::primitives::create_softmax_operation(
            hpx::find_here(),
            phylanx::execution_tree::primitive_arguments_type{
                std::move(arg0), std::move(arg1)});

    hpx::future<phylanx::execution_tree::primitive_argument_type> f =
        softmax.eval();

    blaze::DynamicArray<4UL, double> expected{
        {{{{0.09003057, 0.24472848, 0.66524094},
              {0.8437947, 0.04201007, 0.11419519},
              {0.25949648, 0.70538455, 0.03511903}},
             {{0.04661262, 0.93623954, 0.01714783},
                 {0.01587624, 0.8668133, 0.11731042},
                 {0.10650698, 0.10650698, 0.78698605}}},
            {{{0.10650698, 0.10650698, 0.78698605},
                 {0.00180884, 0.26845497, 0.72973621},
                 {0.25949645, 0.03511902, 0.70538449}},
                {{0.26762316, 0.00490169, 0.72747517},
                    {0.00490169, 0.26762316, 0.72747517},
                    {0.0452785, 0.0452785, 0.90944302}}}}};

    auto rhs = f.get();
    HPX_TEST(allclose(phylanx::ir::node_data<double>(std::move(expected)),
        phylanx::execution_tree::extract_numeric_value(std::move(rhs))));
}

int main(int argc, char* argv[])
{
    test_softmax_operation_0d();
    test_softmax_operation_1d();

    test_softmax_operation_2d();
    test_softmax_operation_2d_column();
    test_softmax_operation_2d_row();

    test_softmax_operation_3d();
    test_softmax_operation_3d_page();
    test_softmax_operation_3d_row();
    test_softmax_operation_3d_column();

    test_softmax_operation_4d();
    test_softmax_operation_4d_axis0();
    test_softmax_operation_4d_axis1();
    test_softmax_operation_4d_axis2();
    test_softmax_operation_4d_axis3();

    return hpx::util::report_errors();
}
