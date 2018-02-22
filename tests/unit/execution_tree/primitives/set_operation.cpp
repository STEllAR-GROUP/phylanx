// Copyright (c) 2018 Bibek Wagle
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

void test_set_operation_1d()
{
    blaze::DynamicVector<double> v1{0.052, 0.95, 0.55, 0.17, 0.85};
    blaze::DynamicVector<double> v2{0.152, 0.195};

    phylanx::execution_tree::primitive input_vector =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(v1));

    phylanx::execution_tree::primitive row_start =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(1.0));

    phylanx::execution_tree::primitive row_stop =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(4.0));

    phylanx::execution_tree::primitive step_row =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(2.0));

    phylanx::execution_tree::primitive col_start =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(0.0));

    phylanx::execution_tree::primitive col_stop =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(0.0));

    phylanx::execution_tree::primitive step_col =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(0.0));

    phylanx::execution_tree::primitive set_vector =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(v2));

    phylanx::execution_tree::primitive set =
        phylanx::execution_tree::primitives::create_set_operation(
            hpx::find_here(),
            std::vector<phylanx::execution_tree::primitive_argument_type>{
                input_vector, std::move(row_start), std::move(row_stop),
                std::move(step_row), std::move(col_start), std::move(col_stop),
                std::move(step_col), std::move(set_vector)});

    blaze::DynamicVector<double> expected{0.052, 0.152, 0.55, 0.195, 0.85};

    hpx::future<phylanx::execution_tree::primitive_argument_type> f =
        set.eval();

    f.get();

    auto result = phylanx::execution_tree::extract_numeric_value(
        input_vector.eval().get());

    HPX_TEST_EQ(phylanx::ir::node_data<double>(expected), result);
}

void test_set_operation_1d_single_step()
{
    blaze::DynamicVector<double> v1{0.052, 0.95, 0.55, 0.17, 0.85};
    blaze::DynamicVector<double> v2{0.152, 0.195};

    phylanx::execution_tree::primitive input_vector =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(v1));

    phylanx::execution_tree::primitive row_start =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(1.0));

    phylanx::execution_tree::primitive row_stop =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(3.0));

    phylanx::execution_tree::primitive step_row =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(1.0));

    phylanx::execution_tree::primitive col_start =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(0.0));

    phylanx::execution_tree::primitive col_stop =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(0.0));

    phylanx::execution_tree::primitive step_col =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(0.0));

    phylanx::execution_tree::primitive set_vector =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(v2));

    phylanx::execution_tree::primitive set =
        phylanx::execution_tree::primitives::create_set_operation(
            hpx::find_here(),
            std::vector<phylanx::execution_tree::primitive_argument_type>{
                input_vector, std::move(row_start), std::move(row_stop),
                std::move(step_row), std::move(col_start), std::move(col_stop),
                std::move(step_col), std::move(set_vector)});

    blaze::DynamicVector<double> expected{0.052, 0.152, 0.195, 0.17, 0.85};

    hpx::future<phylanx::execution_tree::primitive_argument_type> f =
        set.eval();

    f.get();

    auto result = phylanx::execution_tree::extract_numeric_value(
        input_vector.eval().get());

    HPX_TEST_EQ(phylanx::ir::node_data<double>(expected), result);
}

void test_set_operation_1d_single_negative_step()
{
    blaze::DynamicVector<double> v1{0.052, 0.95, 0.55, 0.17, 0.85};
    blaze::DynamicVector<double> v2{0.152, 0.195};

    phylanx::execution_tree::primitive input_vector =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(v1));

    phylanx::execution_tree::primitive row_start =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(3.0));

    phylanx::execution_tree::primitive row_stop =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(1.0));

    phylanx::execution_tree::primitive step_row =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(-1.0));

    phylanx::execution_tree::primitive col_start =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(0.0));

    phylanx::execution_tree::primitive col_stop =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(0.0));

    phylanx::execution_tree::primitive step_col =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(0.0));

    phylanx::execution_tree::primitive set_vector =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(v2));

    phylanx::execution_tree::primitive set =
        phylanx::execution_tree::primitives::create_set_operation(
            hpx::find_here(),
            std::vector<phylanx::execution_tree::primitive_argument_type>{
                input_vector, std::move(row_start), std::move(row_stop),
                std::move(step_row), std::move(col_start), std::move(col_stop),
                std::move(step_col), std::move(set_vector)});

    blaze::DynamicVector<double> expected{0.052, 0.95, 0.195, 0.152, 0.85};

    hpx::future<phylanx::execution_tree::primitive_argument_type> f =
        set.eval();

    f.get();

    auto result = phylanx::execution_tree::extract_numeric_value(
        input_vector.eval().get());

    HPX_TEST_EQ(phylanx::ir::node_data<double>(expected), result);
}

void test_set_operation_2d()
{
    blaze::DynamicMatrix<double> m1{
        {0.05286532, 0.95232529, 0.55222064, 0.17133773, 0.85641998},
        {0.84212087, 0.69646313, 0.18924143, 0.61812872, 0.48111144},
        {0.04567072, 0.15471737, 0.77637891, 0.84232174, 0.54772486},
        {0.84430163, 0.22872386, 0.38010922, 0.93930709, 0.82180563},
        {0.63714445, 0.06884843, 0.9002559, 0.14518178, 0.5056477}};

    blaze::DynamicMatrix<double> m2{{0.42, 0.84}, {0.34, 0.69}};

    phylanx::execution_tree::primitive input_matrix =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(m1));

    phylanx::execution_tree::primitive row_start =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(1.0));

    phylanx::execution_tree::primitive row_stop =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(4.0));

    phylanx::execution_tree::primitive step_row =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(2.0));

    phylanx::execution_tree::primitive col_start =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(1.0));

    phylanx::execution_tree::primitive col_stop =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(4.0));

    phylanx::execution_tree::primitive step_col =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(2.0));

    phylanx::execution_tree::primitive set_matrix =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(m2));

    phylanx::execution_tree::primitive set =
        phylanx::execution_tree::primitives::create_set_operation(
            hpx::find_here(),
            std::vector<phylanx::execution_tree::primitive_argument_type>{
                input_matrix, std::move(row_start), std::move(row_stop),
                std::move(step_row), std::move(col_start), std::move(col_stop),
                std::move(step_col), std::move(set_matrix)});

    blaze::DynamicMatrix<double> expected{
        {0.05286532, 0.95232529, 0.55222064, 0.17133773, 0.85641998},
        {0.84212087, 0.42, 0.18924143, 0.84, 0.48111144},
        {0.04567072, 0.15471737, 0.77637891, 0.84232174, 0.54772486},
        {0.84430163, 0.34, 0.38010922, 0.69, 0.82180563},
        {0.63714445, 0.06884843, 0.9002559, 0.14518178, 0.5056477}};

    hpx::future<phylanx::execution_tree::primitive_argument_type> f =
        set.eval();

    f.get();

    auto result = phylanx::execution_tree::extract_numeric_value(
        input_matrix.eval().get());

    HPX_TEST_EQ(phylanx::ir::node_data<double>(expected), result);
}

void test_set_operation_2d_vector_input()
{
    blaze::DynamicMatrix<double> m1{
        {0.05286532, 0.95232529, 0.55222064, 0.17133773, 0.85641998},
        {0.84212087, 0.69646313, 0.18924143, 0.61812872, 0.48111144},
        {0.04567072, 0.15471737, 0.77637891, 0.84232174, 0.54772486},
        {0.84430163, 0.22872386, 0.38010922, 0.93930709, 0.82180563},
        {0.63714445, 0.06884843, 0.9002559, 0.14518178, 0.5056477}};

    blaze::DynamicVector<double> m2{0.34, 0.69};

    phylanx::execution_tree::primitive input_matrix =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(m1));

    phylanx::execution_tree::primitive row_start =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(1.0));

    phylanx::execution_tree::primitive row_stop =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(4.0));

    phylanx::execution_tree::primitive step_row =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(2.0));

    phylanx::execution_tree::primitive col_start =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(1.0));

    phylanx::execution_tree::primitive col_stop =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(4.0));

    phylanx::execution_tree::primitive step_col =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(2.0));

    phylanx::execution_tree::primitive set_matrix =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(m2));

    phylanx::execution_tree::primitive set =
        phylanx::execution_tree::primitives::create_set_operation(
            hpx::find_here(),
            std::vector<phylanx::execution_tree::primitive_argument_type>{
                input_matrix, std::move(row_start), std::move(row_stop),
                std::move(step_row), std::move(col_start), std::move(col_stop),
                std::move(step_col), std::move(set_matrix)});

    blaze::DynamicMatrix<double> expected{
        {0.05286532, 0.95232529, 0.55222064, 0.17133773, 0.85641998},
        {0.84212087, 0.34, 0.18924143, 0.69, 0.48111144},
        {0.04567072, 0.15471737, 0.77637891, 0.84232174, 0.54772486},
        {0.84430163, 0.34, 0.38010922, 0.69, 0.82180563},
        {0.63714445, 0.06884843, 0.9002559, 0.14518178, 0.5056477}};

    hpx::future<phylanx::execution_tree::primitive_argument_type> f =
        set.eval();

    f.get();

    auto result = phylanx::execution_tree::extract_numeric_value(
        input_matrix.eval().get());

    HPX_TEST_EQ(phylanx::ir::node_data<double>(expected), result);
}

void test_set_operation_2d_vector_input_single_step()
{
    blaze::DynamicMatrix<double> m1{
        {0.05286532, 0.95232529, 0.55222064, 0.17133773, 0.85641998},
        {0.84212087, 0.69646313, 0.18924143, 0.61812872, 0.48111144},
        {0.04567072, 0.15471737, 0.77637891, 0.84232174, 0.54772486},
        {0.84430163, 0.22872386, 0.38010922, 0.93930709, 0.82180563},
        {0.63714445, 0.06884843, 0.9002559, 0.14518178, 0.5056477}};

    blaze::DynamicVector<double> m2{0.34, 0.97, 0.69};

    phylanx::execution_tree::primitive input_matrix =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(m1));

    phylanx::execution_tree::primitive row_start =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(1.0));

    phylanx::execution_tree::primitive row_stop =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(4.0));

    phylanx::execution_tree::primitive step_row =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(1.0));

    phylanx::execution_tree::primitive col_start =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(1.0));

    phylanx::execution_tree::primitive col_stop =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(4.0));

    phylanx::execution_tree::primitive step_col =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(1.0));

    phylanx::execution_tree::primitive set_matrix =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(m2));

    phylanx::execution_tree::primitive set =
        phylanx::execution_tree::primitives::create_set_operation(
            hpx::find_here(),
            std::vector<phylanx::execution_tree::primitive_argument_type>{
                input_matrix, std::move(row_start), std::move(row_stop),
                std::move(step_row), std::move(col_start), std::move(col_stop),
                std::move(step_col), std::move(set_matrix)});

    blaze::DynamicMatrix<double> expected{
        {0.05286532, 0.95232529, 0.55222064, 0.17133773, 0.85641998},
        {0.84212087, 0.34, 0.97, 0.69, 0.48111144},
        {0.04567072, 0.34, 0.97, 0.69, 0.54772486},
        {0.84430163, 0.34, 0.97, 0.69, 0.82180563},
        {0.63714445, 0.06884843, 0.9002559, 0.14518178, 0.5056477}};

    hpx::future<phylanx::execution_tree::primitive_argument_type> f =
        set.eval();

    f.get();

    auto result = phylanx::execution_tree::extract_numeric_value(
        input_matrix.eval().get());

    HPX_TEST_EQ(phylanx::ir::node_data<double>(expected), result);
}

void test_set_operation_2d_vector_input_negative_single_step()
{
    blaze::DynamicMatrix<double> m1{
        {0.05286532, 0.95232529, 0.55222064, 0.17133773, 0.85641998},
        {0.84212087, 0.69646313, 0.18924143, 0.61812872, 0.48111144},
        {0.04567072, 0.15471737, 0.77637891, 0.84232174, 0.54772486},
        {0.84430163, 0.22872386, 0.38010922, 0.93930709, 0.82180563},
        {0.63714445, 0.06884843, 0.9002559, 0.14518178, 0.5056477}};

    blaze::DynamicVector<double> m2{0.69, 0.97, 0.34};

    phylanx::execution_tree::primitive input_matrix =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(m1));

    phylanx::execution_tree::primitive row_start =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(4.0));

    phylanx::execution_tree::primitive row_stop =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(1.0));

    phylanx::execution_tree::primitive step_row =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(-1.0));

    phylanx::execution_tree::primitive col_start =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(4.0));

    phylanx::execution_tree::primitive col_stop =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(1.0));

    phylanx::execution_tree::primitive step_col =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(-1.0));

    phylanx::execution_tree::primitive set_matrix =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(m2));

    phylanx::execution_tree::primitive set =
        phylanx::execution_tree::primitives::create_set_operation(
            hpx::find_here(),
            std::vector<phylanx::execution_tree::primitive_argument_type>{
                input_matrix, std::move(row_start), std::move(row_stop),
                std::move(step_row), std::move(col_start), std::move(col_stop),
                std::move(step_col), std::move(set_matrix)});

    blaze::DynamicMatrix<double> expected{
        {0.05286532, 0.95232529, 0.55222064, 0.17133773, 0.85641998},
        {0.84212087, 0.69646313, 0.18924143, 0.61812872, 0.48111144},
        {0.04567072, 0.15471737, 0.34, 0.97, 0.69},
        {0.84430163, 0.22872386, 0.34, 0.97, 0.69},
        {0.63714445, 0.06884843, 0.34, 0.97, 0.69}};

    hpx::future<phylanx::execution_tree::primitive_argument_type> f =
        set.eval();

    f.get();

    auto result = phylanx::execution_tree::extract_numeric_value(
        input_matrix.eval().get());

    HPX_TEST_EQ(phylanx::ir::node_data<double>(expected), result);
}

void test_set_operation_2d_negetive_step()
{
    blaze::DynamicMatrix<double> m1{{1.0, 2.0, 3.0, 4.0, 5.0},
        {1.0, 2.0, 3.0, 4.0, 5.0}, {1.0, 2.0, 3.0, 4.0, 5.0},
        {1.0, 2.0, 3.0, 4.0, 5.0}};

    blaze::DynamicMatrix<double> m2{{0.42, 0.84}, {0.34, 0.69}};

    phylanx::execution_tree::primitive input_matrix =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(m1));

    phylanx::execution_tree::primitive row_start =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(3.0));

    phylanx::execution_tree::primitive row_stop =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(1.0));

    phylanx::execution_tree::primitive step_row =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(-1.0));

    phylanx::execution_tree::primitive col_start =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(3.0));

    phylanx::execution_tree::primitive col_stop =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(1.0));

    phylanx::execution_tree::primitive step_col =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(-1.0));

    phylanx::execution_tree::primitive set_matrix =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(m2));

    phylanx::execution_tree::primitive set =
        phylanx::execution_tree::primitives::create_set_operation(
            hpx::find_here(),
            std::vector<phylanx::execution_tree::primitive_argument_type>{
                input_matrix, std::move(row_start), std::move(row_stop),
                std::move(step_row), std::move(col_start), std::move(col_stop),
                std::move(step_col), std::move(set_matrix)});

    blaze::DynamicMatrix<double> expected{{1.0, 2.0, 3.0, 4.0, 5.0},
        {1.0, 2.0, 3.0, 4.0, 5.0}, {1.0, 2.0, 0.69, 0.34, 5.0},
        {1.0, 2.0, 0.84, 0.42, 5.0}};

    hpx::future<phylanx::execution_tree::primitive_argument_type> f =
        set.eval();

    f.get();

    auto result = phylanx::execution_tree::extract_numeric_value(
        input_matrix.eval().get());

    HPX_TEST_EQ(phylanx::ir::node_data<double>(expected), result);
}

void test_set_operation_2d_single_element_input()
{
  blaze::DynamicMatrix<double> m1{
      {0.05286532, 0.95232529, 0.55222064, 0.17133773, 0.85641998},
      {0.84212087, 0.69646313, 0.18924143, 0.61812872, 0.48111144},
      {0.04567072, 0.15471737, 0.77637891, 0.84232174, 0.54772486},
      {0.84430163, 0.22872386, 0.38010922, 0.93930709, 0.82180563},
      {0.63714445, 0.06884843, 0.9002559, 0.14518178, 0.5056477}};

  phylanx::execution_tree::primitive input_matrix =
      phylanx::execution_tree::primitives::create_variable(
          hpx::find_here(), phylanx::ir::node_data<double>(m1));

  phylanx::execution_tree::primitive row_start =
      phylanx::execution_tree::primitives::create_variable(
          hpx::find_here(), phylanx::ir::node_data<double>(4.0));

  phylanx::execution_tree::primitive row_stop =
      phylanx::execution_tree::primitives::create_variable(
          hpx::find_here(), phylanx::ir::node_data<double>(1.0));

  phylanx::execution_tree::primitive step_row =
      phylanx::execution_tree::primitives::create_variable(
          hpx::find_here(), phylanx::ir::node_data<double>(-1.0));

  phylanx::execution_tree::primitive col_start =
      phylanx::execution_tree::primitives::create_variable(
          hpx::find_here(), phylanx::ir::node_data<double>(4.0));

  phylanx::execution_tree::primitive col_stop =
      phylanx::execution_tree::primitives::create_variable(
          hpx::find_here(), phylanx::ir::node_data<double>(1.0));

  phylanx::execution_tree::primitive step_col =
      phylanx::execution_tree::primitives::create_variable(
          hpx::find_here(), phylanx::ir::node_data<double>(-1.0));

  phylanx::execution_tree::primitive set_element =
      phylanx::execution_tree::primitives::create_variable(
          hpx::find_here(), phylanx::ir::node_data<double>(0.42));

  phylanx::execution_tree::primitive set =
      phylanx::execution_tree::primitives::create_set_operation(
          hpx::find_here(),
          std::vector<phylanx::execution_tree::primitive_argument_type>{
              input_matrix, std::move(row_start), std::move(row_stop),
              std::move(step_row), std::move(col_start), std::move(col_stop),
              std::move(step_col), std::move(set_element)});

  blaze::DynamicMatrix<double> expected{
      {0.05286532, 0.95232529, 0.55222064, 0.17133773, 0.85641998},
      {0.84212087, 0.69646313, 0.18924143, 0.61812872, 0.48111144},
      {0.04567072, 0.15471737, 0.42, 0.42, 0.42},
      {0.84430163, 0.22872386, 0.42, 0.42, 0.42},
      {0.63714445, 0.06884843, 0.42, 0.42, 0.42}};

  hpx::future<phylanx::execution_tree::primitive_argument_type> f =
      set.eval();

  f.get();

  auto result = phylanx::execution_tree::extract_numeric_value(
      input_matrix.eval().get());

  HPX_TEST_EQ(phylanx::ir::node_data<double>(expected), result);
}

int main(int argc, char* argv[])
{
    test_set_operation_1d();
    test_set_operation_1d_single_step();
    test_set_operation_1d_single_negative_step();

    test_set_operation_2d_single_element_input();
    test_set_operation_2d();
    test_set_operation_2d_vector_input();
    test_set_operation_2d_vector_input_single_step();
    test_set_operation_2d_vector_input_negative_single_step();
    test_set_operation_2d_negetive_step();

    return hpx::util::report_errors();
}
