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

void test_column_set_operation_1d()
{
    blaze::DynamicVector<double> v1{0.052, 0.95, 0.55, 0.17, 0.85};
    blaze::DynamicVector<double> v2{0.152, 0.195};

    phylanx::execution_tree::primitive input_vector =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(v1));

    phylanx::execution_tree::primitive col_start =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(1.0));

    phylanx::execution_tree::primitive col_stop =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(3.0));

    phylanx::execution_tree::primitive step_col =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(1.0));

    phylanx::execution_tree::primitive column_set_vector =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(v2));

    phylanx::execution_tree::primitive column_set =
        phylanx::execution_tree::primitives::create_column_set_operation(
            hpx::find_here(),
            std::vector<phylanx::execution_tree::primitive_argument_type>{
                input_vector, std::move(col_start), std::move(col_stop),
                std::move(step_col), std::move(column_set_vector)});

    blaze::DynamicVector<double> expected{0.052, 0.152, 0.195, 0.17, 0.85};

    hpx::future<phylanx::execution_tree::primitive_argument_type> f =
        column_set.eval();

    f.get();

    auto result = phylanx::execution_tree::extract_numeric_value(
        input_vector.eval().get());

    HPX_TEST_EQ(phylanx::ir::node_data<double>(expected), result);
}

void test_column_set_operation_1d_negative_step()
{
    blaze::DynamicVector<double> v1{0.052, 0.95, 0.55, 0.17, 0.85};
    blaze::DynamicVector<double> v2{0.152, 0.195};

    phylanx::execution_tree::primitive input_vector =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(v1));

    phylanx::execution_tree::primitive col_start =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(3.0));

    phylanx::execution_tree::primitive col_stop =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(1.0));

    phylanx::execution_tree::primitive step_col =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(-1.0));

    phylanx::execution_tree::primitive column_set_vector =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(v2));

    phylanx::execution_tree::primitive column_set =
        phylanx::execution_tree::primitives::create_column_set_operation(
            hpx::find_here(),
            std::vector<phylanx::execution_tree::primitive_argument_type>{
                input_vector, std::move(col_start), std::move(col_stop),
                std::move(step_col), std::move(column_set_vector)});

    blaze::DynamicVector<double> expected{0.052, 0.95, 0.195, 0.152, 0.85};

    hpx::future<phylanx::execution_tree::primitive_argument_type> f =
        column_set.eval();

    f.get();

    auto result = phylanx::execution_tree::extract_numeric_value(
        input_vector.eval().get());

    HPX_TEST_EQ(phylanx::ir::node_data<double>(expected), result);
}

void test_column_set_operation_2d()
{
    blaze::DynamicMatrix<double> m1{
        {0.05286532, 0.95232529, 0.55222064, 0.17133773, 0.85641998},
        {0.84212087, 0.69646313, 0.18924143, 0.61812872, 0.48111144},
        {0.04567072, 0.15471737, 0.77637891, 0.84232174, 0.54772486},
        {0.84430163, 0.22872386, 0.38010922, 0.93930709, 0.82180563},
        {0.63714445, 0.06884843, 0.9002559, 0.14518178, 0.5056477}};

    blaze::DynamicMatrix<double> m2{{0.42, 0.84}, {0.34, 0.69}, {0.412, 0.184},
        {0.314, 0.619}, {0.69, 0.96}};

    phylanx::execution_tree::primitive input_matrix =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(m1));

    phylanx::execution_tree::primitive col_start =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(1.0));

    phylanx::execution_tree::primitive col_stop =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(4.0));

    phylanx::execution_tree::primitive step_col =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(2.0));

    phylanx::execution_tree::primitive column_set_matrix =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(m2));

    phylanx::execution_tree::primitive column_set =
        phylanx::execution_tree::primitives::create_column_set_operation(
            hpx::find_here(),
            std::vector<phylanx::execution_tree::primitive_argument_type>{
                input_matrix, std::move(col_start), std::move(col_stop),
                std::move(step_col), std::move(column_set_matrix)});

    blaze::DynamicMatrix<double> expected{
        {0.05286532, 0.42, 0.55222064, 0.84, 0.85641998},
        {0.84212087, 0.34, 0.18924143, 0.69, 0.48111144},
        {0.04567072, 0.412, 0.77637891, 0.184, 0.54772486},
        {0.84430163, 0.314, 0.38010922, 0.619, 0.82180563},
        {0.63714445, 0.69, 0.9002559, 0.96, 0.5056477}};

    hpx::future<phylanx::execution_tree::primitive_argument_type> f =
        column_set.eval();

    f.get();

    auto result = phylanx::execution_tree::extract_numeric_value(
        input_matrix.eval().get());

    HPX_TEST_EQ(phylanx::ir::node_data<double>(expected), result);
}

void test_column_set_operation_2d_vector_input()
{
    blaze::DynamicMatrix<double> m1{
        {0.05286532, 0.95232529, 0.55222064, 0.17133773, 0.85641998},
        {0.84212087, 0.69646313, 0.18924143, 0.61812872, 0.48111144},
        {0.04567072, 0.15471737, 0.77637891, 0.84232174, 0.54772486},
        {0.84430163, 0.22872386, 0.38010922, 0.93930709, 0.82180563},
        {0.63714445, 0.06884843, 0.9002559, 0.14518178, 0.5056477}};

    blaze::DynamicVector<double> v2{0.42, 0.84};

    phylanx::execution_tree::primitive input_matrix =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(m1));

    phylanx::execution_tree::primitive col_start =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(1.0));

    phylanx::execution_tree::primitive col_stop =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(4.0));

    phylanx::execution_tree::primitive step_col =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(2.0));

    phylanx::execution_tree::primitive column_set_vector =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(v2));

    phylanx::execution_tree::primitive column_set =
        phylanx::execution_tree::primitives::create_column_set_operation(
            hpx::find_here(),
            std::vector<phylanx::execution_tree::primitive_argument_type>{
                input_matrix, std::move(col_start), std::move(col_stop),
                std::move(step_col), std::move(column_set_vector)});

    blaze::DynamicMatrix<double> expected{
        {0.05286532, 0.42, 0.55222064, 0.84, 0.85641998},
        {0.84212087, 0.42, 0.18924143, 0.84, 0.48111144},
        {0.04567072, 0.42, 0.77637891, 0.84, 0.54772486},
        {0.84430163, 0.42, 0.38010922, 0.84, 0.82180563},
        {0.63714445, 0.42, 0.9002559, 0.84, 0.5056477}};

    hpx::future<phylanx::execution_tree::primitive_argument_type> f =
        column_set.eval();

    f.get();

    auto result = phylanx::execution_tree::extract_numeric_value(
        input_matrix.eval().get());

    HPX_TEST_EQ(phylanx::ir::node_data<double>(expected), result);
}

void test_column_set_operation_2d_negetive_step()
{
    blaze::DynamicMatrix<double> m1{
        {0.05286532, 0.95232529, 0.55222064, 0.17133773, 0.85641998},
        {0.84212087, 0.69646313, 0.18924143, 0.61812872, 0.48111144},
        {0.04567072, 0.15471737, 0.77637891, 0.84232174, 0.54772486},
        {0.84430163, 0.22872386, 0.38010922, 0.93930709, 0.82180563},
        {0.63714445, 0.06884843, 0.9002559, 0.14518178, 0.5056477}};

    blaze::DynamicMatrix<double> m2{{0.42, 0.84}, {0.34, 0.69}, {0.412, 0.184},
        {0.314, 0.619}, {0.69, 0.96}};

    phylanx::execution_tree::primitive input_matrix =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(m1));

    phylanx::execution_tree::primitive col_start =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(4.0));

    phylanx::execution_tree::primitive col_stop =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(1.0));

    phylanx::execution_tree::primitive step_col =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(-2.0));

    phylanx::execution_tree::primitive column_set_matrix =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(m2));

    phylanx::execution_tree::primitive column_set =
        phylanx::execution_tree::primitives::create_column_set_operation(
            hpx::find_here(),
            std::vector<phylanx::execution_tree::primitive_argument_type>{
                input_matrix, std::move(col_start), std::move(col_stop),
                std::move(step_col), std::move(column_set_matrix)});

    blaze::DynamicMatrix<double> expected{
        {0.05286532, 0.95232529, 0.84, 0.17133773, 0.42},
        {0.84212087, 0.69646313, 0.69, 0.61812872, 0.34},
        {0.04567072, 0.15471737, 0.184, 0.84232174, 0.412},
        {0.84430163, 0.22872386, 0.619, 0.93930709, 0.314},
        {0.63714445, 0.06884843, 0.96, 0.14518178, 0.69}};

    hpx::future<phylanx::execution_tree::primitive_argument_type> f =
        column_set.eval();

    f.get();

    auto result = phylanx::execution_tree::extract_numeric_value(
        input_matrix.eval().get());

    HPX_TEST_EQ(phylanx::ir::node_data<double>(expected), result);
}

int main(int argc, char* argv[])
{
    test_column_set_operation_1d();
    test_column_set_operation_1d_negative_step();

    test_column_set_operation_2d();
    test_column_set_operation_2d_vector_input();
    test_column_set_operation_2d_negetive_step();

    return hpx::util::report_errors();
}
