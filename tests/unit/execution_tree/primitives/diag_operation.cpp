// Copyright (c) 2018 Bibek Wagle
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)


#include <phylanx/phylanx.hpp>

#include <hpx/hpx_main.hpp>
#include <hpx/include/lcos.hpp>
#include <hpx/util/lightweight_test.hpp>

#include <vector>
#include <utility>

void test_diag_operation_0d()
{
    phylanx::execution_tree::primitive data =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(5.0));

    phylanx::execution_tree::primitive k =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(0.0));

    phylanx::execution_tree::primitive diag =
        phylanx::execution_tree::primitives::create_diag_operation(
            hpx::find_here(),
            std::vector<phylanx::execution_tree::primitive_argument_type>{
                std::move(data), std::move(k)});

    hpx::future<phylanx::execution_tree::primitive_argument_type> f =
        diag.eval();

    HPX_TEST_EQ(5.0,
                phylanx::execution_tree::extract_numeric_value(f.get())[0]);
}

void test_diag_operation_1d()
{
    blaze::DynamicVector<double> v1{1, 2, 3, 4, 5};

    phylanx::execution_tree::primitive vec =
            phylanx::execution_tree::primitives::create_variable(
                    hpx::find_here(), phylanx::ir::node_data<double>(v1));

    phylanx::execution_tree::primitive k =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(0.0));

    phylanx::execution_tree::primitive diag =
        phylanx::execution_tree::primitives::create_diag_operation(
            hpx::find_here(),
            std::vector<phylanx::execution_tree::primitive_argument_type>{
                std::move(vec), std::move(k)
            });


    hpx::future<phylanx::execution_tree::primitive_argument_type> f =
        diag.eval();

    blaze::DynamicMatrix<double> expected{{1, 0, 0, 0, 0},
                                          {0, 2, 0, 0, 0},
                                          {0, 0, 3, 0, 0},
                                          {0, 0, 0, 4, 0},
                                          {0, 0, 0, 0, 5}};

    HPX_TEST_EQ(phylanx::ir::node_data<double>(std::move(expected)),
                phylanx::execution_tree::extract_numeric_value(f.get()));
}

void test_diag_operation_1d_plus_one()
{
    blaze::DynamicVector<double> v1{1, 2, 3, 4, 5};

    phylanx::execution_tree::primitive vec =
            phylanx::execution_tree::primitives::create_variable(
                    hpx::find_here(), phylanx::ir::node_data<double>(v1));

    phylanx::execution_tree::primitive k =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(1.0));

    phylanx::execution_tree::primitive diag =
        phylanx::execution_tree::primitives::create_diag_operation(
            hpx::find_here(),
            std::vector<phylanx::execution_tree::primitive_argument_type>{
                std::move(vec), std::move(k)
            });


    hpx::future<phylanx::execution_tree::primitive_argument_type> f =
        diag.eval();

    blaze::DynamicMatrix<double> expected{{0, 1, 0, 0, 0, 0},
                                          {0, 0, 2, 0, 0, 0},
                                          {0, 0, 0, 3, 0, 0},
                                          {0, 0, 0, 0, 4, 0},
                                          {0, 0, 0, 0, 0, 5},
                                          {0, 0, 0, 0, 0, 0}};

    HPX_TEST_EQ(phylanx::ir::node_data<double>(std::move(expected)),
                phylanx::execution_tree::extract_numeric_value(f.get()));
}

void test_diag_operation_1d_minus_one()
{
    blaze::DynamicVector<double> v1{1, 2, 3, 4, 5};

    phylanx::execution_tree::primitive vec =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(v1));

    phylanx::execution_tree::primitive k =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(-1.0));

    phylanx::execution_tree::primitive diag =
        phylanx::execution_tree::primitives::create_diag_operation(
            hpx::find_here(),
            std::vector<phylanx::execution_tree::primitive_argument_type>{
                std::move(vec), std::move(k)
            });


    hpx::future<phylanx::execution_tree::primitive_argument_type> f =
        diag.eval();

    blaze::DynamicMatrix<double> expected{{ 0, 0, 0, 0, 0, 0},
                                          { 1, 0, 0, 0, 0, 0},
                                          { 0, 2, 0, 0, 0, 0},
                                          { 0, 0, 3, 0, 0, 0},
                                          { 0, 0, 0, 4, 0, 0},
                                          { 0, 0, 0, 0, 5, 0}};

    HPX_TEST_EQ(phylanx::ir::node_data<double>(std::move(expected)),
                phylanx::execution_tree::extract_numeric_value(f.get()));
}

void test_diag_operation_2d()
{
    blaze::DynamicMatrix<double> m1{{ 90,  54,  57,  60,  63},
                                    { 51,  96,  57,  60,  63},
                                    { 51,  54, 102,  60,  63},
                                    { 51,  54,  57, 108,  63},
                                    { 51,  54,  57,  60, 114}};

    phylanx::execution_tree::primitive mat =
            phylanx::execution_tree::primitives::create_variable(
                    hpx::find_here(), phylanx::ir::node_data<double>(m1));

    phylanx::execution_tree::primitive k =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(0.0));

    phylanx::execution_tree::primitive diag =
        phylanx::execution_tree::primitives::create_diag_operation(
            hpx::find_here(),
            std::vector<phylanx::execution_tree::primitive_argument_type>{
                std::move(mat), std::move(k)
            });

    hpx::future<phylanx::execution_tree::primitive_argument_type> f =
        diag.eval();

    blaze::DynamicVector<double> expected{90,  96, 102, 108, 114};

    HPX_TEST_EQ(
        phylanx::ir::node_data<double>(std::move(expected)),
        phylanx::execution_tree::extract_numeric_value(f.get()));
}

void test_diag_operation_2d_plus_one()
{
    blaze::DynamicMatrix<double> m1{{ 90,  54,  57,  60,  63},
                                    { 51,  96,  57,  60,  63},
                                    { 51,  54, 102,  60,  63},
                                    { 51,  54,  57, 108,  63},
                                    { 51,  54,  57,  60, 114}};

    phylanx::execution_tree::primitive mat =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(m1));

    phylanx::execution_tree::primitive k =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(1.0));

    phylanx::execution_tree::primitive diag =
        phylanx::execution_tree::primitives::create_diag_operation(
            hpx::find_here(),
            std::vector<phylanx::execution_tree::primitive_argument_type>{
                std::move(mat), std::move(k)
            });

    hpx::future<phylanx::execution_tree::primitive_argument_type> f =
        diag.eval();

    blaze::DynamicVector<double> expected{54, 57, 60, 63};

    HPX_TEST_EQ(
        phylanx::ir::node_data<double>(std::move(expected)),
        phylanx::execution_tree::extract_numeric_value(f.get()));
}

void test_diag_operation_2d_minus_one()
{
    blaze::DynamicMatrix<double> m1{{ 90,  54,  57,  60,  63},
                                    { 51,  96,  57,  60,  63},
                                    { 51,  54, 102,  60,  63},
                                    { 51,  54,  57, 108,  63},
                                    { 51,  54,  57,  60, 114}};

    phylanx::execution_tree::primitive mat =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(m1));

    phylanx::execution_tree::primitive k =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(-1.0));

    phylanx::execution_tree::primitive diag =
        phylanx::execution_tree::primitives::create_diag_operation(
            hpx::find_here(),
            std::vector<phylanx::execution_tree::primitive_argument_type>{
                std::move(mat), std::move(k)
            });

    hpx::future<phylanx::execution_tree::primitive_argument_type> f =
        diag.eval();

    blaze::DynamicVector<double> expected{51, 54, 57, 60};

    HPX_TEST_EQ(
        phylanx::ir::node_data<double>(std::move(expected)),
        phylanx::execution_tree::extract_numeric_value(f.get()));
}

int main(int argc, char* argv[])
{
    test_diag_operation_0d();

    test_diag_operation_1d();
    test_diag_operation_1d_minus_one();
    test_diag_operation_1d_plus_one();

    test_diag_operation_2d();
    test_diag_operation_2d_minus_one();
    test_diag_operation_2d_plus_one();

    return hpx::util::report_errors();
}
