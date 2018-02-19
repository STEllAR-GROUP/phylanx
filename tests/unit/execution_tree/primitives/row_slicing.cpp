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

void test_row_slicing_operation_0d()
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

    phylanx::execution_tree::primitive slice =
        phylanx::execution_tree::primitives::create_row_slicing_operation(
            hpx::find_here(),
            std::vector<phylanx::execution_tree::primitive_argument_type>{
                std::move(first), std::move(second), std::move(third)});

    hpx::future<phylanx::execution_tree::primitive_argument_type> f =
        slice.eval();

    HPX_TEST_EQ(
        42.0, phylanx::execution_tree::extract_numeric_value(f.get())[0]);
}

void test_row_slicing_operation_1d()
{
    // parameters required by phylanx to create a slice is as follows:
    // vector : v1 in this testcase
    // row_start The index of the first element to extract.
    // row_start The index of the last element to extract.

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

    phylanx::execution_tree::primitive slice =
        phylanx::execution_tree::primitives::create_row_slicing_operation(
            hpx::find_here(),
            std::vector<phylanx::execution_tree::primitive_argument_type>{
                std::move(input_vector), std::move(row_start),
                std::move(row_stop)});

    // parameters required by blaze to create a subvector is as follows:
    // vector The vector containing the subvector.
    // index The index of the first element of the subvector.
    // size The size of the subvector.

    // The following math is a result of converting the arguments
    // provided in slice primitive so that equivalent operation is
    // performed in blaze.
    // vector = v1
    // index = row_start
    // size = (row_stop - row_start)

    // Here, matrix = m1 , column = 5, n = 10
    auto sm = blaze::subvector(v1, 5, 10);
    auto expected = sm;

    hpx::future<phylanx::execution_tree::primitive_argument_type> f =
        slice.eval();

    HPX_TEST_EQ(phylanx::ir::node_data<double>(std::move(expected)),
        phylanx::execution_tree::extract_numeric_value(f.get()));
}

void test_row_slicing_operation_1d_step()
{
    blaze::DynamicVector<double> v1{
        1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15};

    phylanx::execution_tree::primitive input_vector =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(v1));

    phylanx::execution_tree::primitive row_start =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(2.0));

    phylanx::execution_tree::primitive row_stop =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(11.0));

    phylanx::execution_tree::primitive step =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(2.0));

    phylanx::execution_tree::primitive slice =
        phylanx::execution_tree::primitives::create_row_slicing_operation(
            hpx::find_here(),
            std::vector<phylanx::execution_tree::primitive_argument_type>{
                std::move(input_vector), std::move(row_start),
                std::move(row_stop), std::move(step)});

    blaze::DynamicVector<double> expected{3, 5, 7, 9, 11};

    hpx::future<phylanx::execution_tree::primitive_argument_type> f =
        slice.eval();

    HPX_TEST_EQ(phylanx::ir::node_data<double>(std::move(expected)),
        phylanx::execution_tree::extract_numeric_value(f.get()));
}

void test_row_slicing_operation_1d_neg_step()
{
    blaze::DynamicVector<double> v1{
        1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15};

    phylanx::execution_tree::primitive input_vector =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(v1));

    phylanx::execution_tree::primitive row_start =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(11.0));

    phylanx::execution_tree::primitive row_stop =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(2.0));

    phylanx::execution_tree::primitive step =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(-2.0));

    phylanx::execution_tree::primitive slice =
        phylanx::execution_tree::primitives::create_row_slicing_operation(
            hpx::find_here(),
            std::vector<phylanx::execution_tree::primitive_argument_type>{
                std::move(input_vector), std::move(row_start),
                std::move(row_stop), std::move(step)});

    blaze::DynamicVector<double> expected{12, 10, 8, 6, 4};

    hpx::future<phylanx::execution_tree::primitive_argument_type> f =
        slice.eval();

    HPX_TEST_EQ(phylanx::ir::node_data<double>(std::move(expected)),
        phylanx::execution_tree::extract_numeric_value(f.get()));
}

void test_row_slicing_operation_1d_negative_index()
{
    blaze::Rand<blaze::DynamicVector<double>> gen{};
    blaze::DynamicVector<double> v1 = gen.generate(1007UL);

    phylanx::execution_tree::primitive input_vector =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(v1));

    phylanx::execution_tree::primitive row_start =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(-5.0));

    phylanx::execution_tree::primitive row_stop =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(-2.0));

    phylanx::execution_tree::primitive slice =
        phylanx::execution_tree::primitives::create_row_slicing_operation(
            hpx::find_here(),
            std::vector<phylanx::execution_tree::primitive_argument_type>{
                std::move(input_vector), std::move(row_start),
                std::move(row_stop)});

    auto sm = blaze::subvector(v1, 1002, 3);
    auto expected = sm;

    hpx::future<phylanx::execution_tree::primitive_argument_type> f =
        slice.eval();

    HPX_TEST_EQ(phylanx::ir::node_data<double>(std::move(expected)),
        phylanx::execution_tree::extract_numeric_value(f.get()));
}

void test_row_slicing_operation_1d_single_slice_negative_index()
{
    blaze::Rand<blaze::DynamicVector<double>> gen{};
    blaze::DynamicVector<double> v1 = gen.generate(1007UL);

    phylanx::execution_tree::primitive input_vector =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(v1));

    phylanx::execution_tree::primitive row_start =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(-5.0));

    phylanx::execution_tree::primitive row_stop =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(-4.0));

    phylanx::execution_tree::primitive slice =
        phylanx::execution_tree::primitives::create_row_slicing_operation(
            hpx::find_here(),
            std::vector<phylanx::execution_tree::primitive_argument_type>{
                std::move(input_vector), std::move(row_start),
                std::move(row_stop)});

    auto sm = blaze::subvector(v1, 1002, 1);
    auto expected = sm[0];

    hpx::future<phylanx::execution_tree::primitive_argument_type> f =
        slice.eval();

    HPX_TEST_EQ(phylanx::ir::node_data<double>(std::move(expected)),
        phylanx::execution_tree::extract_numeric_value(f.get()));
}

void test_row_slicing_operation_1d_negative_index_zero_start()
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
            hpx::find_here(), phylanx::ir::node_data<double>(-1.0));

    phylanx::execution_tree::primitive slice =
        phylanx::execution_tree::primitives::create_row_slicing_operation(
            hpx::find_here(),
            std::vector<phylanx::execution_tree::primitive_argument_type>{
                std::move(input_vector), std::move(row_start),
                std::move(row_stop)});

    auto sm = blaze::subvector(v1, 0, 1006);
    auto expected = sm;

    hpx::future<phylanx::execution_tree::primitive_argument_type> f =
        slice.eval();

    HPX_TEST_EQ(phylanx::ir::node_data<double>(std::move(expected)),
        phylanx::execution_tree::extract_numeric_value(f.get()));
}

void test_row_slicing_operation_1d_negative_index_neg_step()
{
    blaze::DynamicVector<double> v1{
        1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15};

    phylanx::execution_tree::primitive input_vector =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(v1));

    phylanx::execution_tree::primitive row_start =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(-2.0));

    phylanx::execution_tree::primitive row_stop =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(-11.0));

    phylanx::execution_tree::primitive step =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(-2.0));

    phylanx::execution_tree::primitive slice =
        phylanx::execution_tree::primitives::create_row_slicing_operation(
            hpx::find_here(),
            std::vector<phylanx::execution_tree::primitive_argument_type>{
                std::move(input_vector), std::move(row_start),
                std::move(row_stop), std::move(step)});

    blaze::DynamicVector<double> expected{14, 12, 10, 8, 6};

    hpx::future<phylanx::execution_tree::primitive_argument_type> f =
        slice.eval();

    HPX_TEST_EQ(phylanx::ir::node_data<double>(std::move(expected)),
        phylanx::execution_tree::extract_numeric_value(f.get()));
}

void test_row_slicing_operation_2d()
{
    // parameters required by phylanx to create a slice is as follows:
    // matrix The matrix containing the submatrix.
    // row_start The index of the first row of the submatrix.
    // row_stop The index of the last row of the submatrix.

    blaze::Rand<blaze::DynamicMatrix<double>> gen{};
    blaze::DynamicMatrix<double> m1 = gen.generate(90UL, 101UL);

    phylanx::execution_tree::primitive input_matrix =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(m1));

    phylanx::execution_tree::primitive row_start =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(5.0));

    phylanx::execution_tree::primitive row_stop =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(47.0));

    phylanx::execution_tree::primitive slice =
        phylanx::execution_tree::primitives::create_row_slicing_operation(
            hpx::find_here(),
            std::vector<phylanx::execution_tree::primitive_argument_type>{
                std::move(input_matrix), std::move(row_start),
                std::move(row_stop)});

    // parameters required by blaze to create a submatrix is as follows:
    // matrix The matrix containing the submatrix.
    // row The index of the first row of the submatrix.
    // column The index of the first column of the submatrix.
    // m The number of rows of the submatrix.
    // n The number of columns of the submatrix.
    // return View on the specific submatrix of the matrix.
    // exception std::invalid_argument Invalid submatrix specification.

    // The following math is a result of converting the arguments
    // provided in slice primitive so that equivalent operation is
    // performed in  blaze.
    // matrix = matrix
    // row = row_start
    // m = (row_stop - row_start)
    // column and n are 0 and the number of columns in
    // the input matrix respectively
    // Here, matrix = m1 , row = 5, column = 0, m = 42, n = 101

    auto sm = blaze::submatrix(m1, 5, 0, 42, 101);
    auto expected = sm;

    hpx::future<phylanx::execution_tree::primitive_argument_type> f =
        slice.eval();

    HPX_TEST_EQ(phylanx::ir::node_data<double>(std::move(expected)),
        phylanx::execution_tree::extract_numeric_value(f.get()));
}

void test_row_slicing_operation_2d_step()
{
    blaze::DynamicMatrix<double> m1{
        {0.54769045, 0.91450975, 0.80695526, 0.84479593, 0.3487697},
        {0.16290767, 0.39112232, 0.65630879, 0.09022202, 0.51387922},
        {0.60897262, 0.36334488, 0.93994283, 0.95566919, 0.05137503},
        {0.60357895, 0.14664937, 0.11202065, 0.11757873, 0.79123244},
        {0.44672457, 0.38056627, 0.89120904, 0.26006523, 0.00738655}};

    phylanx::execution_tree::primitive input_matrix =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(m1));

    phylanx::execution_tree::primitive row_start =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(1.0));

    phylanx::execution_tree::primitive row_stop =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(4.0));

    phylanx::execution_tree::primitive step =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(2.0));

    phylanx::execution_tree::primitive slice =
        phylanx::execution_tree::primitives::create_row_slicing_operation(
            hpx::find_here(),
            std::vector<phylanx::execution_tree::primitive_argument_type>{
                std::move(input_matrix), std::move(row_start),
                std::move(row_stop), std::move(step)});

    blaze::DynamicMatrix<double> expected{
        {0.16290767, 0.39112232, 0.65630879, 0.09022202, 0.51387922},
        {0.60357895, 0.14664937, 0.11202065, 0.11757873, 0.79123244}};

    hpx::future<phylanx::execution_tree::primitive_argument_type> f =
        slice.eval();

    HPX_TEST_EQ(phylanx::ir::node_data<double>(std::move(expected)),
        phylanx::execution_tree::extract_numeric_value(f.get()));
}

void test_row_slicing_operation_2d_neg_step()
{
    blaze::DynamicMatrix<double> m1{
        {0.54769045, 0.91450975, 0.80695526, 0.84479593, 0.3487697},
        {0.16290767, 0.39112232, 0.65630879, 0.09022202, 0.51387922},
        {0.60897262, 0.36334488, 0.93994283, 0.95566919, 0.05137503},
        {0.60357895, 0.14664937, 0.11202065, 0.11757873, 0.79123244},
        {0.44672457, 0.38056627, 0.89120904, 0.26006523, 0.00738655}};

    phylanx::execution_tree::primitive input_matrix =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(m1));

    phylanx::execution_tree::primitive row_start =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(4.0));

    phylanx::execution_tree::primitive row_stop =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(1.0));

    phylanx::execution_tree::primitive step =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(-2.0));

    phylanx::execution_tree::primitive slice =
        phylanx::execution_tree::primitives::create_row_slicing_operation(
            hpx::find_here(),
            std::vector<phylanx::execution_tree::primitive_argument_type>{
                std::move(input_matrix), std::move(row_start),
                std::move(row_stop), std::move(step)});

    blaze::DynamicMatrix<double> expected{
        {0.44672457, 0.38056627, 0.89120904, 0.26006523, 0.00738655},
        {0.60897262, 0.36334488, 0.93994283, 0.95566919, 0.05137503}};

    hpx::future<phylanx::execution_tree::primitive_argument_type> f =
        slice.eval();

    HPX_TEST_EQ(phylanx::ir::node_data<double>(std::move(expected)),
        phylanx::execution_tree::extract_numeric_value(f.get()));
}

void test_row_slicing_operation_2d_negative_index()
{
    blaze::Rand<blaze::DynamicMatrix<double>> gen{};
    blaze::DynamicMatrix<double> m1 = gen.generate(90UL, 101UL);

    phylanx::execution_tree::primitive input_matrix =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(m1));

    phylanx::execution_tree::primitive row_start =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(-5.0));

    phylanx::execution_tree::primitive row_stop =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(-2.0));

    phylanx::execution_tree::primitive slice =
        phylanx::execution_tree::primitives::create_row_slicing_operation(
            hpx::find_here(),
            std::vector<phylanx::execution_tree::primitive_argument_type>{
                std::move(input_matrix), std::move(row_start),
                std::move(row_stop)});

    auto sm = blaze::submatrix(m1, 85, 0, 3, 101);
    auto expected = sm;

    hpx::future<phylanx::execution_tree::primitive_argument_type> f =
        slice.eval();

    HPX_TEST_EQ(phylanx::ir::node_data<double>(std::move(expected)),
        phylanx::execution_tree::extract_numeric_value(f.get()));
}

void test_row_slicing_operation_2d_single_slice_negative_index()
{
    blaze::Rand<blaze::DynamicMatrix<double>> gen{};
    blaze::DynamicMatrix<double> m1 = gen.generate(90UL, 101UL);

    phylanx::execution_tree::primitive input_matrix =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(m1));

    phylanx::execution_tree::primitive row_start =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(-5.0));

    phylanx::execution_tree::primitive row_stop =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(-4.0));

    phylanx::execution_tree::primitive slice =
        phylanx::execution_tree::primitives::create_row_slicing_operation(
            hpx::find_here(),
            std::vector<phylanx::execution_tree::primitive_argument_type>{
                std::move(input_matrix), std::move(row_start),
                std::move(row_stop)});

    auto sm = blaze::submatrix(m1, 85, 0, 1, 101);
    auto expected = blaze::trans(blaze::row(sm, 0));

    hpx::future<phylanx::execution_tree::primitive_argument_type> f =
        slice.eval();

    HPX_TEST_EQ(phylanx::ir::node_data<double>(std::move(expected)),
        phylanx::execution_tree::extract_numeric_value(f.get()));
}

void test_row_slicing_operation_2d_negative_index_zero_start()
{
    blaze::Rand<blaze::DynamicMatrix<double>> gen{};
    blaze::DynamicMatrix<double> m1 = gen.generate(90UL, 101UL);

    phylanx::execution_tree::primitive input_matrix =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(m1));

    phylanx::execution_tree::primitive row_start =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(0.0));

    phylanx::execution_tree::primitive row_stop =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(-2.0));

    phylanx::execution_tree::primitive slice =
        phylanx::execution_tree::primitives::create_row_slicing_operation(
            hpx::find_here(),
            std::vector<phylanx::execution_tree::primitive_argument_type>{
                std::move(input_matrix), std::move(row_start),
                std::move(row_stop)});

    auto sm = blaze::submatrix(m1, 0, 0, 88, 101);
    auto expected = sm;

    hpx::future<phylanx::execution_tree::primitive_argument_type> f =
        slice.eval();

    HPX_TEST_EQ(phylanx::ir::node_data<double>(std::move(expected)),
        phylanx::execution_tree::extract_numeric_value(f.get()));
}

void test_row_slicing_operation_2d_negative_index_neg_step()
{
    blaze::DynamicMatrix<double> m1{
        {0.54769045, 0.91450975, 0.80695526, 0.84479593, 0.3487697},
        {0.16290767, 0.39112232, 0.65630879, 0.09022202, 0.51387922},
        {0.60897262, 0.36334488, 0.93994283, 0.95566919, 0.05137503},
        {0.60357895, 0.14664937, 0.11202065, 0.11757873, 0.79123244},
        {0.44672457, 0.38056627, 0.89120904, 0.26006523, 0.00738655}};

    phylanx::execution_tree::primitive input_matrix =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(m1));

    phylanx::execution_tree::primitive row_start =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(-1.0));

    phylanx::execution_tree::primitive row_stop =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(-4.0));

    phylanx::execution_tree::primitive step =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(-2.0));

    phylanx::execution_tree::primitive slice =
        phylanx::execution_tree::primitives::create_row_slicing_operation(
            hpx::find_here(),
            std::vector<phylanx::execution_tree::primitive_argument_type>{
                std::move(input_matrix), std::move(row_start),
                std::move(row_stop), std::move(step)});

    blaze::DynamicMatrix<double> expected{
        {0.44672457, 0.38056627, 0.89120904, 0.26006523, 0.00738655},
        {0.60897262, 0.36334488, 0.93994283, 0.95566919, 0.05137503}};

    hpx::future<phylanx::execution_tree::primitive_argument_type> f =
        slice.eval();

    HPX_TEST_EQ(phylanx::ir::node_data<double>(std::move(expected)),
        phylanx::execution_tree::extract_numeric_value(f.get()));
}

int main(int argc, char* argv[])
{
    test_row_slicing_operation_0d();

    test_row_slicing_operation_1d();
    test_row_slicing_operation_1d_step();
    test_row_slicing_operation_1d_neg_step();

    test_row_slicing_operation_1d_negative_index();
    test_row_slicing_operation_1d_single_slice_negative_index();
    test_row_slicing_operation_1d_negative_index_zero_start();
    test_row_slicing_operation_1d_negative_index_neg_step();

    test_row_slicing_operation_2d();
    test_row_slicing_operation_2d_step();
    test_row_slicing_operation_2d_neg_step();

    test_row_slicing_operation_2d_negative_index();
    test_row_slicing_operation_2d_single_slice_negative_index();
    test_row_slicing_operation_2d_negative_index_zero_start();
    test_row_slicing_operation_2d_negative_index_neg_step();

    return hpx::util::report_errors();
}
