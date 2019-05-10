// Copyright (c) 2019 Bita Hasheminezhad
// Copyright (c) 2019 Hartmut Kaiser
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
void test_l2_normalize_operation_0d()
{
    phylanx::execution_tree::primitive arg =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<std::int64_t>(42));

    phylanx::execution_tree::primitive l2_normalize =
        phylanx::execution_tree::primitives::create_l2_normalize_operation(
            hpx::find_here(),
            phylanx::execution_tree::primitive_arguments_type{
                std::move(arg)});

    hpx::future<phylanx::execution_tree::primitive_argument_type> f =
        l2_normalize.eval();

    HPX_TEST_EQ(
        1.0, phylanx::execution_tree::extract_numeric_value(f.get())[0]);
}

void test_l2_normalize_operation_1d()
{
    blaze::DynamicVector<double> subject{13., 42., 33.};
    phylanx::execution_tree::primitive arg =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(subject));

    phylanx::execution_tree::primitive l2_normalize =
        phylanx::execution_tree::primitives::create_l2_normalize_operation(
            hpx::find_here(),
            phylanx::execution_tree::primitive_arguments_type{std::move(arg)});

    hpx::future<phylanx::execution_tree::primitive_argument_type> f =
        l2_normalize.eval();

    blaze::DynamicVector<double> expected{0.23648092, 0.7640153 , 0.60029775};

    HPX_TEST(allclose(phylanx::ir::node_data<double>(std::move(expected)),
        phylanx::execution_tree::extract_numeric_value(f.get())));
}

void test_l2_normalize_operation_2d()
{
    blaze::DynamicMatrix<std::int64_t> subject{{1, 2, 3},
                                               {4, 5, 6}};
    phylanx::execution_tree::primitive arg =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<std::int64_t>(subject));

    phylanx::execution_tree::primitive l2_normalize =
        phylanx::execution_tree::primitives::create_l2_normalize_operation(
            hpx::find_here(),
            phylanx::execution_tree::primitive_arguments_type{std::move(arg)});

    hpx::future<phylanx::execution_tree::primitive_argument_type> f =
        l2_normalize.eval();

    blaze::DynamicMatrix<double> expected{{0.10482848, 0.20965695, 0.31448543},
                                          {0.4193139, 0.5241424, 0.62897086}};

    HPX_TEST(allclose(phylanx::ir::node_data<double>(std::move(expected)),
        phylanx::execution_tree::extract_numeric_value(f.get())));
}

void test_l2_normalize_operation_2d_nil()
{
    blaze::DynamicMatrix<std::int64_t> subject{{1, 2, 3},
                                               {3, 4, 1}};
    phylanx::execution_tree::primitive arg0 =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<std::int64_t>(subject));

    phylanx::execution_tree::primitive arg1 =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ast::nil{});

    phylanx::execution_tree::primitive l2_normalize =
        phylanx::execution_tree::primitives::create_l2_normalize_operation(
            hpx::find_here(),
            phylanx::execution_tree::primitive_arguments_type{
                std::move(arg0), std::move(arg1)});

    hpx::future<phylanx::execution_tree::primitive_argument_type> f =
        l2_normalize.eval();

    blaze::DynamicMatrix<double> expected{{0.15811388, 0.31622776, 0.47434163},
                                          {0.47434163, 0.6324555 , 0.15811388}};

    HPX_TEST(allclose(phylanx::ir::node_data<double>(std::move(expected)),
        phylanx::execution_tree::extract_numeric_value(f.get())));
}

void test_l2_normalize_operation_2d_axis1()
{
    blaze::DynamicMatrix<std::int64_t> subject{{1, 2, 3},
                                               {3, 4, 1}};
    phylanx::execution_tree::primitive arg0 =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<std::int64_t>(subject));

    phylanx::execution_tree::primitive arg1 =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<std::int64_t>(1));

    phylanx::execution_tree::primitive l2_normalize =
        phylanx::execution_tree::primitives::create_l2_normalize_operation(
            hpx::find_here(),
            phylanx::execution_tree::primitive_arguments_type{
                std::move(arg0), std::move(arg1)});

    hpx::future<phylanx::execution_tree::primitive_argument_type> f =
        l2_normalize.eval();

    blaze::DynamicMatrix<double> expected{{0.26726124, 0.5345225, 0.8017837},
                                          {0.5883484, 0.78446454, 0.19611613}};

    HPX_TEST(allclose(phylanx::ir::node_data<double>(std::move(expected)),
        phylanx::execution_tree::extract_numeric_value(f.get())));
}

void test_l2_normalize_operation_2d_axis0()
{
    blaze::DynamicMatrix<std::int64_t> subject{{1, 2, 3},
                                               {3, 4, 1}};
    phylanx::execution_tree::primitive arg0 =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<std::int64_t>(subject));

    phylanx::execution_tree::primitive arg1 =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<std::int64_t>(0));

    phylanx::execution_tree::primitive l2_normalize =
        phylanx::execution_tree::primitives::create_l2_normalize_operation(
            hpx::find_here(),
            phylanx::execution_tree::primitive_arguments_type{
                std::move(arg0), std::move(arg1)});

    hpx::future<phylanx::execution_tree::primitive_argument_type> f =
        l2_normalize.eval();

    blaze::DynamicMatrix<double> expected{{0.31622776, 0.4472136 , 0.94868326},
                                          {0.94868326, 0.8944272 , 0.31622776}};

    HPX_TEST(allclose(phylanx::ir::node_data<double>(std::move(expected)),
        phylanx::execution_tree::extract_numeric_value(f.get())));
}

#if defined(PHYLANX_HAVE_BLAZE_TENSOR)
void test_l2_normalize_operation_3d()
{
    blaze::DynamicTensor<double> subject{
        {{1.0, 2.0, 3.0}, {4.0, 1.0, 2.0}, {3.0, 4.0, 1.0}},
        {{3.0, 6.0, 2.0}, {-2.0, 2.0, 0.0}, {1.0, 1.0, 3.0}}};

    phylanx::execution_tree::primitive arg =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<std::int64_t>(subject));

    phylanx::execution_tree::primitive l2_normalize =
        phylanx::execution_tree::primitives::create_l2_normalize_operation(
            hpx::find_here(),
            phylanx::execution_tree::primitive_arguments_type{std::move(arg)});

    hpx::future<phylanx::execution_tree::primitive_argument_type> f =
        l2_normalize.eval();

    blaze::DynamicTensor<double> expected{
        {{0.08804509, 0.17609018, 0.26413527},
            {0.35218036, 0.08804509, 0.17609018},
            {0.26413527, 0.35218036, 0.08804509}},
        {{0.26413527, 0.52827054, 0.17609018},
            {-0.17609018, 0.17609018, 0.},
            {0.08804509, 0.08804509, 0.26413527}}};

    HPX_TEST(allclose(phylanx::ir::node_data<double>(std::move(expected)),
        phylanx::execution_tree::extract_numeric_value(f.get())));
}

void test_l2_normalize_operation_3d_axis0()
{
    blaze::DynamicTensor<double> subject{
        {{1.0, 2.0, 3.0}, {4.0, 1.0, 2.0}, {3.0, 4.0, 1.0}},
        {{3.0, 6.0, 2.0}, {-2.0, 2.0, 0.0}, {1.0, 1.0, 3.0}} };

    phylanx::execution_tree::primitive arg0 =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<std::int64_t>(subject));

    phylanx::execution_tree::primitive arg1 =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<std::int64_t>(-3));

    phylanx::execution_tree::primitive l2_normalize =
        phylanx::execution_tree::primitives::create_l2_normalize_operation(
            hpx::find_here(),
            phylanx::execution_tree::primitive_arguments_type{
                std::move(arg0), std::move(arg1) });

    hpx::future<phylanx::execution_tree::primitive_argument_type> f =
        l2_normalize.eval();

    blaze::DynamicTensor<double> expected {
        {{0.31622776, 0.31622776, 0.8320502},
            {0.89442718, 0.44721359, 0.99999994},
            {0.94868326, 0.97014242, 0.31622776}},
        {{0.94868326, 0.94868326, 0.55470014},
            {-0.44721359, 0.89442718, 0.},
            {0.31622776, 0.24253561, 0.94868326}}};

    HPX_TEST(allclose(phylanx::ir::node_data<double>(std::move(expected)),
        phylanx::execution_tree::extract_numeric_value(f.get())));
}

void test_l2_normalize_operation_3d_axis1()
{
    blaze::DynamicTensor<double> subject{
        {{1.0, 2.0, 3.0}, {4.0, 1.0, 2.0}, {3.0, 4.0, 1.0}},
        {{3.0, 6.0, 2.0}, {-2.0, 2.0, 0.0}, {1.0, 1.0, 3.0}}};

    phylanx::execution_tree::primitive arg0 =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<std::int64_t>(subject));

    phylanx::execution_tree::primitive arg1 =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<std::int64_t>(1));

    phylanx::execution_tree::primitive l2_normalize =
        phylanx::execution_tree::primitives::create_l2_normalize_operation(
            hpx::find_here(),
            phylanx::execution_tree::primitive_arguments_type{
                std::move(arg0), std::move(arg1)});

    hpx::future<phylanx::execution_tree::primitive_argument_type> f =
        l2_normalize.eval();

    blaze::DynamicTensor<double> expected{
        {{0.1961161, 0.43643573, 0.80178368},
            {0.78446442, 0.21821786, 0.53452247},
            {0.58834833, 0.87287146, 0.26726124}},
        {{0.80178368, 0.93704259, 0.5547002},
            {-0.53452247, 0.31234753, 0.},
            {0.26726124, 0.15617377, 0.83205032}}};

    HPX_TEST(allclose(phylanx::ir::node_data<double>(std::move(expected)),
        phylanx::execution_tree::extract_numeric_value(f.get())));
}

void test_l2_normalize_operation_3d_axis2()
{
    blaze::DynamicTensor<double> subject{
        {{1.0, 2.0, 3.0}, {4.0, 1.0, 2.0}, {3.0, 4.0, 1.0}},
        {{3.0, 6.0, 2.0}, {-2.0, 2.0, 0.0}, {1.0, 1.0, 3.0}}};

    phylanx::execution_tree::primitive arg0 =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<std::int64_t>(subject));

    phylanx::execution_tree::primitive arg1 =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<std::int64_t>(-1));

    phylanx::execution_tree::primitive l2_normalize =
        phylanx::execution_tree::primitives::create_l2_normalize_operation(
            hpx::find_here(),
            phylanx::execution_tree::primitive_arguments_type{
                std::move(arg0), std::move(arg1)});

    hpx::future<phylanx::execution_tree::primitive_argument_type> f =
        l2_normalize.eval();

    blaze::DynamicTensor<double> expected{
        {{0.26726124, 0.53452247, 0.80178368},
            {0.87287146, 0.21821786, 0.43643573},
            {0.58834833, 0.78446442, 0.1961161}},
        {{0.4285714, 0.85714281, 0.28571427},
            {-0.70710677, 0.70710677, 0.},
            {0.30151135, 0.30151135, 0.90453404}}};

    HPX_TEST(allclose(phylanx::ir::node_data<double>(std::move(expected)),
        phylanx::execution_tree::extract_numeric_value(f.get())));
}
#endif

int main(int argc, char* argv[])
{
    test_l2_normalize_operation_0d();
    test_l2_normalize_operation_1d();
    test_l2_normalize_operation_2d();
    test_l2_normalize_operation_2d_nil();
    test_l2_normalize_operation_2d_axis1();
    test_l2_normalize_operation_2d_axis0();

#if defined(PHYLANX_HAVE_BLAZE_TENSOR)
    test_l2_normalize_operation_3d();
    test_l2_normalize_operation_3d_axis0();
    test_l2_normalize_operation_3d_axis1();
    test_l2_normalize_operation_3d_axis2();
#endif

    return hpx::util::report_errors();
}
