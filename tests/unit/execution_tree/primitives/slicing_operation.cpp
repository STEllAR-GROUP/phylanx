// Copyright (c) 2017-2018 Bibek Wagle
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

void test_slicing_operation_0d()
{
    phylanx::execution_tree::primitive first =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(42.0));

    phylanx::execution_tree::primitive second =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(5.0));

    phylanx::execution_tree::primitive third =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(47.0));

    phylanx::execution_tree::primitive fourth =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(5.0));

    phylanx::execution_tree::primitive fifth =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(15.0));

    phylanx::execution_tree::primitive slice =
        phylanx::execution_tree::primitives::create_slicing_operation(
            hpx::find_here(),
            std::vector<phylanx::execution_tree::primitive_argument_type>{
                std::move(first), std::move(second), std::move(third),
                std::move(fourth), std::move(fifth)});

    hpx::future<phylanx::execution_tree::primitive_argument_type> f =
        slice.eval();

    HPX_TEST_EQ(
        42.0, phylanx::execution_tree::extract_numeric_value(f.get())[0]);
}

void test_slicing_operation_1d()
{
    blaze::Rand<blaze::DynamicVector<double>> gen{};
    blaze::DynamicVector<double> v1 = gen.generate(1007UL);

    phylanx::execution_tree::primitive input_vector =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(v1));

    phylanx::execution_tree::primitive row_start =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(5.0));

    phylanx::execution_tree::primitive row_stop =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(15.0));

    phylanx::execution_tree::primitive col_start =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(0.0));

    phylanx::execution_tree::primitive col_stop =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(0.0));

    phylanx::execution_tree::primitive slice =
        phylanx::execution_tree::primitives::create_slicing_operation(
            hpx::find_here(),
            std::vector<phylanx::execution_tree::primitive_argument_type>{
                std::move(input_vector), std::move(row_start),
                std::move(row_stop), std::move(col_start),
                std::move(col_stop)});

    auto sm = blaze::subvector(v1, 5, 10);
    auto expected = sm;

    hpx::future<phylanx::execution_tree::primitive_argument_type> f =
        slice.eval();

    HPX_TEST_EQ(phylanx::ir::node_data<double>(std::move(expected)),
        phylanx::execution_tree::extract_numeric_value(f.get()));
}

void test_slicing_operation_1d_zero_start()
{
    blaze::Rand<blaze::DynamicVector<double>> gen{};
    blaze::DynamicVector<double> v1 = gen.generate(1007UL);

    phylanx::execution_tree::primitive input_vector =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(v1));

    phylanx::execution_tree::primitive row_start =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(0.0));

    phylanx::execution_tree::primitive row_stop =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(15.0));

    phylanx::execution_tree::primitive col_start =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(0.0));

    phylanx::execution_tree::primitive col_stop =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(0.0));

    phylanx::execution_tree::primitive slice =
        phylanx::execution_tree::primitives::create_slicing_operation(
            hpx::find_here(),
            std::vector<phylanx::execution_tree::primitive_argument_type>{
                std::move(input_vector), std::move(row_start),
                std::move(row_stop), std::move(col_start),
                std::move(col_stop)});

    auto sm = blaze::subvector(v1, 0, 15);
    auto expected = sm;

    hpx::future<phylanx::execution_tree::primitive_argument_type> f =
        slice.eval();

    HPX_TEST_EQ(phylanx::ir::node_data<double>(std::move(expected)),
        phylanx::execution_tree::extract_numeric_value(f.get()));
}

void test_slicing_operation_2d()
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
            hpx::find_here(), phylanx::ir::node_data<double>(1.0));

    phylanx::execution_tree::primitive row_stop =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(3.0));

    phylanx::execution_tree::primitive col_start =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(2.0));

    phylanx::execution_tree::primitive col_stop =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(4.0));

    phylanx::execution_tree::primitive slice =
        phylanx::execution_tree::primitives::create_slicing_operation(
            hpx::find_here(),
            std::vector<phylanx::execution_tree::primitive_argument_type>{
                std::move(input_matrix), std::move(row_start),
                std::move(row_stop), std::move(col_start),
                std::move(col_stop)});

    blaze::DynamicMatrix<double> expected{
        {0.18924143, 0.61812872}, {0.77637891, 0.84232174}};

    hpx::future<phylanx::execution_tree::primitive_argument_type> f =
        slice.eval();

    HPX_TEST_EQ(phylanx::ir::node_data<double>(std::move(expected)),
        phylanx::execution_tree::extract_numeric_value(f.get()));
}

void test_slicing_operation_2d_zero_start()
{
    blaze::Rand<blaze::DynamicMatrix<double>> gen{};
    blaze::DynamicMatrix<double> m1 = gen.generate(101UL, 101UL);

    phylanx::execution_tree::primitive input_matrix =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(m1));

    phylanx::execution_tree::primitive row_start =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(0.0));

    phylanx::execution_tree::primitive row_stop =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(47.0));

    phylanx::execution_tree::primitive col_start =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(0.0));

    phylanx::execution_tree::primitive col_stop =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(15.0));

    phylanx::execution_tree::primitive slice =
        phylanx::execution_tree::primitives::create_slicing_operation(
            hpx::find_here(),
            std::vector<phylanx::execution_tree::primitive_argument_type>{
                std::move(input_matrix), std::move(row_start),
                std::move(row_stop), std::move(col_start),
                std::move(col_stop)});

    auto sm = blaze::submatrix(m1, 0, 0, 47, 15);
    auto expected = sm;

    hpx::future<phylanx::execution_tree::primitive_argument_type> f =
        slice.eval();

    HPX_TEST_EQ(phylanx::ir::node_data<double>(std::move(expected)),
        phylanx::execution_tree::extract_numeric_value(f.get()));
}

void test_col_slicing_operation_from_end()
{
    blaze::Rand<blaze::DynamicMatrix<double>> gen{};
    blaze::DynamicMatrix<double> m1 = gen.generate(101UL, 101UL);

    phylanx::execution_tree::primitive input_matrix =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(m1));

    phylanx::execution_tree::primitive row_start =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(0.0));

    phylanx::execution_tree::primitive row_stop =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(47.0));

    phylanx::execution_tree::primitive col_start =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(0.0));

    phylanx::execution_tree::primitive col_stop =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(-2.0));

    phylanx::execution_tree::primitive slice =
        phylanx::execution_tree::primitives::create_slicing_operation(
            hpx::find_here(),
            std::vector<phylanx::execution_tree::primitive_argument_type>{
                std::move(input_matrix), std::move(row_start),
                std::move(row_stop), std::move(col_start),
                std::move(col_stop)});

    auto sm = blaze::submatrix(m1, 0, 0, 47, 99);
    auto expected = sm;

    hpx::future<phylanx::execution_tree::primitive_argument_type> f =
        slice.eval();

    HPX_TEST_EQ(phylanx::ir::node_data<double>(std::move(expected)),
        phylanx::execution_tree::extract_numeric_value(f.get()));
}

void test_row_slicing_operation_from_end()
{
    blaze::Rand<blaze::DynamicMatrix<double>> gen{};
    blaze::DynamicMatrix<double> m1 = gen.generate(101UL, 101UL);

    phylanx::execution_tree::primitive input_matrix =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(m1));

    phylanx::execution_tree::primitive row_start =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(0.0));

    phylanx::execution_tree::primitive row_stop =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(-5.0));

    phylanx::execution_tree::primitive col_start =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(6.0));

    phylanx::execution_tree::primitive col_stop =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(16.0));

    phylanx::execution_tree::primitive slice =
        phylanx::execution_tree::primitives::create_slicing_operation(
            hpx::find_here(),
            std::vector<phylanx::execution_tree::primitive_argument_type>{
                std::move(input_matrix), std::move(row_start),
                std::move(row_stop), std::move(col_start),
                std::move(col_stop)});

    auto sm = blaze::submatrix(m1, 0, 6, 96, 10);
    auto expected = sm;

    hpx::future<phylanx::execution_tree::primitive_argument_type> f =
        slice.eval();

    HPX_TEST_EQ(phylanx::ir::node_data<double>(std::move(expected)),
        phylanx::execution_tree::extract_numeric_value(f.get()));
}

void test_col_slicing_operation_from_end_negative_start_stop()
{
    blaze::Rand<blaze::DynamicMatrix<double>> gen{};
    blaze::DynamicMatrix<double> m1 = gen.generate(101UL, 101UL);

    phylanx::execution_tree::primitive input_matrix =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(m1));

    phylanx::execution_tree::primitive row_start =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(0.0));

    phylanx::execution_tree::primitive row_stop =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(47.0));

    phylanx::execution_tree::primitive col_start =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(-5.0));

    phylanx::execution_tree::primitive col_stop =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(-2.0));

    phylanx::execution_tree::primitive slice =
        phylanx::execution_tree::primitives::create_slicing_operation(
            hpx::find_here(),
            std::vector<phylanx::execution_tree::primitive_argument_type>{
                std::move(input_matrix), std::move(row_start),
                std::move(row_stop), std::move(col_start),
                std::move(col_stop)});

    auto sm = blaze::submatrix(m1, 0, 96, 47, 3);
    auto expected = sm;

    hpx::future<phylanx::execution_tree::primitive_argument_type> f =
        slice.eval();

    HPX_TEST_EQ(phylanx::ir::node_data<double>(std::move(expected)),
        phylanx::execution_tree::extract_numeric_value(f.get()));
}

void test_row_slicing_operation_from_end_negative_start_stop()
{
    blaze::Rand<blaze::DynamicMatrix<double>> gen{};
    blaze::DynamicMatrix<double> m1 = gen.generate(101UL, 101UL);

    phylanx::execution_tree::primitive input_matrix =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(m1));

    phylanx::execution_tree::primitive row_start =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(-5.0));

    phylanx::execution_tree::primitive row_stop =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(-2.0));

    phylanx::execution_tree::primitive col_start =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(6.0));

    phylanx::execution_tree::primitive col_stop =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(16.0));

    phylanx::execution_tree::primitive slice =
        phylanx::execution_tree::primitives::create_slicing_operation(
            hpx::find_here(),
            std::vector<phylanx::execution_tree::primitive_argument_type>{
                std::move(input_matrix), std::move(row_start),
                std::move(row_stop), std::move(col_start),
                std::move(col_stop)});

    auto sm = blaze::submatrix(m1, 96, 6, 3, 10);
    auto expected = sm;

    hpx::future<phylanx::execution_tree::primitive_argument_type> f =
        slice.eval();

    HPX_TEST_EQ(phylanx::ir::node_data<double>(std::move(expected)),
        phylanx::execution_tree::extract_numeric_value(f.get()));
}

void test_row_and_column_slicing_from_end_negative_start_stop()
{
    blaze::Rand<blaze::DynamicMatrix<double>> gen{};
    blaze::DynamicMatrix<double> m1 = gen.generate(101UL, 101UL);

    phylanx::execution_tree::primitive input_matrix =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(m1));

    phylanx::execution_tree::primitive row_start =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(-5.0));

    phylanx::execution_tree::primitive row_stop =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(-2.0));

    phylanx::execution_tree::primitive col_start =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(-5.0));

    phylanx::execution_tree::primitive col_stop =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(-3.0));

    phylanx::execution_tree::primitive slice =
        phylanx::execution_tree::primitives::create_slicing_operation(
            hpx::find_here(),
            std::vector<phylanx::execution_tree::primitive_argument_type>{
                std::move(input_matrix), std::move(row_start),
                std::move(row_stop), std::move(col_start),
                std::move(col_stop)});

    auto sm = blaze::submatrix(m1, 96, 96, 3, 2);
    auto expected = sm;

    hpx::future<phylanx::execution_tree::primitive_argument_type> f =
        slice.eval();

    HPX_TEST_EQ(phylanx::ir::node_data<double>(std::move(expected)),
        phylanx::execution_tree::extract_numeric_value(f.get()));
}

void test_col_slicing_operation_from_end_single_column()
{
    blaze::Rand<blaze::DynamicMatrix<double>> gen{};
    blaze::DynamicMatrix<double> m1 = gen.generate(101UL, 101UL);

    phylanx::execution_tree::primitive input_matrix =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(m1));

    phylanx::execution_tree::primitive row_start =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(0.0));

    phylanx::execution_tree::primitive row_stop =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(47.0));

    phylanx::execution_tree::primitive col_start =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(-5.0));

    phylanx::execution_tree::primitive col_stop =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(-4.0));

    phylanx::execution_tree::primitive slice =
        phylanx::execution_tree::primitives::create_slicing_operation(
            hpx::find_here(),
            std::vector<phylanx::execution_tree::primitive_argument_type>{
                std::move(input_matrix), std::move(row_start),
                std::move(row_stop), std::move(col_start),
                std::move(col_stop)});

    auto sm = blaze::submatrix(m1, 0, 96, 47, 1);
    auto expected = blaze::column(sm, 0);

    hpx::future<phylanx::execution_tree::primitive_argument_type> f =
        slice.eval();

    HPX_TEST_EQ(phylanx::ir::node_data<double>(std::move(expected)),
        phylanx::execution_tree::extract_numeric_value(f.get()));
}

void test_row_slicing_operation_from_end_single_row()
{
    blaze::Rand<blaze::DynamicMatrix<double>> gen{};
    blaze::DynamicMatrix<double> m1 = gen.generate(101UL, 101UL);

    phylanx::execution_tree::primitive input_matrix =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(m1));

    phylanx::execution_tree::primitive row_start =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(-5.0));

    phylanx::execution_tree::primitive row_stop =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(-4.0));

    phylanx::execution_tree::primitive col_start =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(6.0));

    phylanx::execution_tree::primitive col_stop =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(16.0));

    phylanx::execution_tree::primitive slice =
        phylanx::execution_tree::primitives::create_slicing_operation(
            hpx::find_here(),
            std::vector<phylanx::execution_tree::primitive_argument_type>{
                std::move(input_matrix), std::move(row_start),
                std::move(row_stop), std::move(col_start),
                std::move(col_stop)});

    auto sm = blaze::submatrix(m1, 96, 6, 1, 10);
    auto expected = blaze::trans(blaze::row(sm, 0));

    hpx::future<phylanx::execution_tree::primitive_argument_type> f =
        slice.eval();

    HPX_TEST_EQ(phylanx::ir::node_data<double>(std::move(expected)),
        phylanx::execution_tree::extract_numeric_value(f.get()));
}

void test_single_element_slicing_operation_from_end_row()
{
    blaze::Rand<blaze::DynamicMatrix<double>> gen{};
    blaze::DynamicMatrix<double> m1 = gen.generate(101UL, 101UL);

    phylanx::execution_tree::primitive input_matrix =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(m1));

    phylanx::execution_tree::primitive row_start =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(-5.0));

    phylanx::execution_tree::primitive row_stop =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(-4.0));

    phylanx::execution_tree::primitive col_start =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(6.0));

    phylanx::execution_tree::primitive col_stop =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(7.0));

    phylanx::execution_tree::primitive slice =
        phylanx::execution_tree::primitives::create_slicing_operation(
            hpx::find_here(),
            std::vector<phylanx::execution_tree::primitive_argument_type>{
                std::move(input_matrix), std::move(row_start),
                std::move(row_stop), std::move(col_start),
                std::move(col_stop)});

    auto sm = blaze::submatrix(m1, 96, 6, 1, 1);
    auto expected = blaze::trans(blaze::row(sm, 0))[0];

    hpx::future<phylanx::execution_tree::primitive_argument_type> f =
        slice.eval();

    HPX_TEST_EQ(phylanx::ir::node_data<double>(std::move(expected)),
        phylanx::execution_tree::extract_numeric_value(f.get()));
}

void test_single_element_slicing_operation_from_end_column()
{
    blaze::Rand<blaze::DynamicMatrix<double>> gen{};
    blaze::DynamicMatrix<double> m1 = gen.generate(101UL, 101UL);

    phylanx::execution_tree::primitive input_matrix =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(m1));

    phylanx::execution_tree::primitive row_start =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(0.0));

    phylanx::execution_tree::primitive row_stop =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(1.0));

    phylanx::execution_tree::primitive col_start =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(-5.0));

    phylanx::execution_tree::primitive col_stop =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(-4.0));

    phylanx::execution_tree::primitive slice =
        phylanx::execution_tree::primitives::create_slicing_operation(
            hpx::find_here(),
            std::vector<phylanx::execution_tree::primitive_argument_type>{
                std::move(input_matrix), std::move(row_start),
                std::move(row_stop), std::move(col_start),
                std::move(col_stop)});

    auto sm = blaze::submatrix(m1, 0, 96, 1, 1);
    auto expected = blaze::column(sm, 0)[0];

    hpx::future<phylanx::execution_tree::primitive_argument_type> f =
        slice.eval();

    HPX_TEST_EQ(phylanx::ir::node_data<double>(std::move(expected)),
        phylanx::execution_tree::extract_numeric_value(f.get()));
}

void test_single_element_slicing_operation()
{
    blaze::Rand<blaze::DynamicMatrix<double>> gen{};
    blaze::DynamicMatrix<double> m1 = gen.generate(101UL, 101UL);

    phylanx::execution_tree::primitive input_matrix =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(m1));

    phylanx::execution_tree::primitive row_start =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(10.0));

    phylanx::execution_tree::primitive row_stop =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(11.0));

    phylanx::execution_tree::primitive col_start =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(14.0));

    phylanx::execution_tree::primitive col_stop =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(15.0));

    phylanx::execution_tree::primitive slice =
        phylanx::execution_tree::primitives::create_slicing_operation(
            hpx::find_here(),
            std::vector<phylanx::execution_tree::primitive_argument_type>{
                std::move(input_matrix), std::move(row_start),
                std::move(row_stop), std::move(col_start),
                std::move(col_stop)});

    auto sm = blaze::submatrix(m1, 10, 14, 1, 1);
    auto expected = sm(0, 0);

    hpx::future<phylanx::execution_tree::primitive_argument_type> f =
        slice.eval();

    HPX_TEST_EQ(phylanx::ir::node_data<double>(std::move(expected)),
        phylanx::execution_tree::extract_numeric_value(f.get()));
}

void test_slicing_operation_2d_negative_index_neg_step()
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
            hpx::find_here(), phylanx::ir::node_data<double>(-1.0));

    phylanx::execution_tree::primitive row_stop =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(-4.0));

    phylanx::execution_tree::primitive step_row =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(-2.0));

    phylanx::execution_tree::primitive col_start =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(-1.0));

    phylanx::execution_tree::primitive col_stop =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(-4.0));

    phylanx::execution_tree::primitive step_col =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(-2.0));

    phylanx::execution_tree::primitive slice =
        phylanx::execution_tree::primitives::create_slicing_operation(
            hpx::find_here(),
            std::vector<phylanx::execution_tree::primitive_argument_type>{
                std::move(input_matrix), std::move(row_start),
                std::move(row_stop), std::move(step_row), std::move(col_start),
                std::move(col_stop), std::move(step_col)});

    blaze::DynamicMatrix<double> expected{
        {0.5056477, 0.9002559}, {0.54772486, 0.77637891}};

    hpx::future<phylanx::execution_tree::primitive_argument_type> f =
        slice.eval();

    HPX_TEST_EQ(phylanx::ir::node_data<double>(std::move(expected)),
        phylanx::execution_tree::extract_numeric_value(f.get()));
}

void test_slicing_operation_2d_negative_index_pos_step()
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
            hpx::find_here(), phylanx::ir::node_data<double>(-4.0));

    phylanx::execution_tree::primitive row_stop =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(-1.0));

    phylanx::execution_tree::primitive step_row =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(2.0));

    phylanx::execution_tree::primitive col_start =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(-4.0));

    phylanx::execution_tree::primitive col_stop =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(-1.0));

    phylanx::execution_tree::primitive step_col =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(2.0));

    phylanx::execution_tree::primitive slice =
        phylanx::execution_tree::primitives::create_slicing_operation(
            hpx::find_here(),
            std::vector<phylanx::execution_tree::primitive_argument_type>{
                std::move(input_matrix), std::move(row_start),
                std::move(row_stop), std::move(step_row), std::move(col_start),
                std::move(col_stop), std::move(step_col)});

    blaze::DynamicMatrix<double> expected{
        {0.69646313, 0.61812872}, {0.22872386, 0.93930709}};

    hpx::future<phylanx::execution_tree::primitive_argument_type> f =
        slice.eval();

    HPX_TEST_EQ(phylanx::ir::node_data<double>(std::move(expected)),
        phylanx::execution_tree::extract_numeric_value(f.get()));
}

void test_slicing_operation_2d_pos_index_pos_step()
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

    phylanx::execution_tree::primitive slice =
        phylanx::execution_tree::primitives::create_slicing_operation(
            hpx::find_here(),
            std::vector<phylanx::execution_tree::primitive_argument_type>{
                std::move(input_matrix), std::move(row_start),
                std::move(row_stop), std::move(step_row), std::move(col_start),
                std::move(col_stop), std::move(step_col)});

    blaze::DynamicMatrix<double> expected{
        {0.69646313, 0.61812872}, {0.22872386, 0.93930709}};

    hpx::future<phylanx::execution_tree::primitive_argument_type> f =
        slice.eval();

    HPX_TEST_EQ(phylanx::ir::node_data<double>(std::move(expected)),
        phylanx::execution_tree::extract_numeric_value(f.get()));
}

int main(int argc, char* argv[])
{
    test_slicing_operation_0d();

    test_slicing_operation_1d();
    test_slicing_operation_1d_zero_start();

    test_slicing_operation_2d();
    test_slicing_operation_2d_zero_start();

    test_col_slicing_operation_from_end();
    test_row_slicing_operation_from_end();

    test_col_slicing_operation_from_end_negative_start_stop();
    test_row_slicing_operation_from_end_negative_start_stop();

    test_row_and_column_slicing_from_end_negative_start_stop();

    test_col_slicing_operation_from_end_single_column();
    test_row_slicing_operation_from_end_single_row();

    test_single_element_slicing_operation_from_end_row();
    test_single_element_slicing_operation_from_end_column();

    test_single_element_slicing_operation();

    test_slicing_operation_2d_negative_index_neg_step();
    test_slicing_operation_2d_negative_index_pos_step();

    return hpx::util::report_errors();
}
