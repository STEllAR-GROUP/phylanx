// Copyright (c) 2017 Bibek Wagle
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
        hpx::new_<phylanx::execution_tree::primitives::variable>(
            hpx::find_here(), phylanx::ir::node_data<double>(42.0));

    phylanx::execution_tree::primitive second =
        hpx::new_<phylanx::execution_tree::primitives::variable>(
            hpx::find_here(), phylanx::ir::node_data<double>(5.0));

    phylanx::execution_tree::primitive third =
        hpx::new_<phylanx::execution_tree::primitives::variable>(
            hpx::find_here(), phylanx::ir::node_data<double>(47.0));

    phylanx::execution_tree::primitive fourth =
        hpx::new_<phylanx::execution_tree::primitives::variable>(
            hpx::find_here(), phylanx::ir::node_data<double>(5.0));

    phylanx::execution_tree::primitive fifth =
        hpx::new_<phylanx::execution_tree::primitives::variable>(
            hpx::find_here(), phylanx::ir::node_data<double>(15.0));

    phylanx::execution_tree::primitive slice =
        hpx::new_<phylanx::execution_tree::primitives::slicing_operation>(
            hpx::find_here(),
            std::vector<phylanx::execution_tree::primitive_argument_type>{
                std::move(first), std::move(second), std::move(third),
                std::move(fourth),std::move(fifth)
            });

    hpx::future<phylanx::execution_tree::primitive_result_type> f =
        slice.eval();

    HPX_TEST_EQ(42.0, phylanx::execution_tree::extract_numeric_value(f.get())[0]);
}

void test_slicing_operation_1d()
{
    // parameters required by phylanx to create a slice is as follows:
    // vector : v1 in this test case
    // row_start : set to zero internally
    // row_stop : set to 1 internally
    // col_start : The index of the first element to extract.
    // col_stop : The index of the last element(exclusive) to extract.

    blaze::Rand<blaze::DynamicVector<double>> gen{};
    blaze::DynamicVector<double> v1 = gen.generate(1007UL);

    phylanx::execution_tree::primitive input_vector =
        hpx::new_<phylanx::execution_tree::primitives::variable>(
            hpx::find_here(), phylanx::ir::node_data<double>(v1));

    phylanx::execution_tree::primitive row_start =
        hpx::new_<phylanx::execution_tree::primitives::variable>(
            hpx::find_here(), phylanx::ir::node_data<double>(0.0));

    phylanx::execution_tree::primitive row_stop =
        hpx::new_<phylanx::execution_tree::primitives::variable>(
            hpx::find_here(), phylanx::ir::node_data<double>(1.0));

    phylanx::execution_tree::primitive col_start =
        hpx::new_<phylanx::execution_tree::primitives::variable>(
            hpx::find_here(), phylanx::ir::node_data<double>(5.0));

    phylanx::execution_tree::primitive col_stop =
        hpx::new_<phylanx::execution_tree::primitives::variable>(
            hpx::find_here(), phylanx::ir::node_data<double>(15.0));

    // row_start an row_stop does not have any effect on the output.
    // row_start and row_stop is set to 0 and 1 respectively internally.
    // any user input for these parameters is ignored internally.

    phylanx::execution_tree::primitive slice =
        hpx::new_<phylanx::execution_tree::primitives::slicing_operation>(
            hpx::find_here(),
            std::vector<phylanx::execution_tree::primitive_argument_type>{
                 std::move(input_vector), std::move(row_start),
                 std::move(row_stop), std::move(col_start),
                 std::move(col_stop)
            });

    // parameters required by blaze to create a subvector is as follows:
    // vector The vector containing the subvector.
    // index The index of the first element of the subvector.
    // size The size of the subvector.

    // The following math is a result of converting the arguments
    // provided in slice primitive so that equivalent operation is
    // performed in blaze.
    // vector = v1
    // index = col_start
    // size = (col_stop - col_start)

    // Here, matrix = m1 , row = 5, column = 5, m = 43, n = 10
    auto sm = blaze::subvector(v1, 5, 10);
    auto expected = sm;

    hpx::future<phylanx::execution_tree::primitive_result_type> f =
        slice.eval();

    HPX_TEST_EQ(phylanx::ir::node_data<double>(std::move(expected)),
        phylanx::execution_tree::extract_numeric_value(f.get()));
}

void test_slicing_operation_1d_zero_start()
{
    // parameters required by phylanx to create a slice is as follows:
    // vector : v1 in this test case
    // row_start : set to zero internally
    // row_stop : set to 1 internally
    // col_start : The index of the first element to extract.
    // col_stop : The index of the last element(exclusive) to extract.

    blaze::Rand<blaze::DynamicVector<double>> gen{};
    blaze::DynamicVector<double> v1 = gen.generate(1007UL);

    phylanx::execution_tree::primitive input_vector =
        hpx::new_<phylanx::execution_tree::primitives::variable>(
            hpx::find_here(), phylanx::ir::node_data<double>(v1));

    phylanx::execution_tree::primitive row_start =
        hpx::new_<phylanx::execution_tree::primitives::variable>(
            hpx::find_here(), phylanx::ir::node_data<double>(0.0));

    phylanx::execution_tree::primitive row_stop =
        hpx::new_<phylanx::execution_tree::primitives::variable>(
            hpx::find_here(), phylanx::ir::node_data<double>(1.0));

    phylanx::execution_tree::primitive col_start =
        hpx::new_<phylanx::execution_tree::primitives::variable>(
            hpx::find_here(), phylanx::ir::node_data<double>(0.0));

    phylanx::execution_tree::primitive col_stop =
        hpx::new_<phylanx::execution_tree::primitives::variable>(
            hpx::find_here(), phylanx::ir::node_data<double>(15.0));

    // row_start an row_stop does not have any effect on the output.
    // row_start and row_stop is set to 0 and 1 respectively internally.
    // any user input for these parameters is ignored internally.

    phylanx::execution_tree::primitive slice =
        hpx::new_<phylanx::execution_tree::primitives::slicing_operation>(
            hpx::find_here(),
            std::vector<phylanx::execution_tree::primitive_argument_type>{
                std::move(input_vector), std::move(row_start),
                std::move(row_stop), std::move(col_start),
                std::move(col_stop)
            });

    // parameters required by blaze to create a subvector is as follows:
    // vector The vector containing the subvector.
    // index The index of the first element of the subvector.
    // size The size of the subvector.

    // The following math is a result of converting the arguments
    // provided in slice primitive so that equivalent operation is
    // performed in blaze.
    // vector = v1
    // index = col_start
    // size = (col_stop - col_start)

    // Here, matrix = m1 , row = 5, column = 5, m = 43, n = 10
    auto sm = blaze::subvector(v1, 0, 15);
    auto expected = sm;

    hpx::future<phylanx::execution_tree::primitive_result_type> f =
        slice.eval();

    HPX_TEST_EQ(phylanx::ir::node_data<double>(std::move(expected)),
        phylanx::execution_tree::extract_numeric_value(f.get()));
}

void test_slicing_operation_2d()
{
    // parameters required by phylanx to create a slice is as follows:
    // matrix The matrix containing the submatrix.
    // row_start The index of the first row of the submatrix.
    // row_stop The index of the last row(exclusive) of the submatrix.
    // col_start The index of the first column of the submatrix.
    // col_stop The index of the last column(exclusive) of the submatrix.

    blaze::Rand<blaze::DynamicMatrix<double>> gen{};
    blaze::DynamicMatrix<double> m1 = gen.generate(101UL, 101UL);

    phylanx::execution_tree::primitive input_matrix =
        hpx::new_<phylanx::execution_tree::primitives::variable>(
            hpx::find_here(), phylanx::ir::node_data<double>(m1));

    phylanx::execution_tree::primitive row_start =
        hpx::new_<phylanx::execution_tree::primitives::variable>(
            hpx::find_here(), phylanx::ir::node_data<double>(5.0));

    phylanx::execution_tree::primitive row_stop =
        hpx::new_<phylanx::execution_tree::primitives::variable>(
            hpx::find_here(), phylanx::ir::node_data<double>(47.0));

    phylanx::execution_tree::primitive col_start =
        hpx::new_<phylanx::execution_tree::primitives::variable>(
            hpx::find_here(), phylanx::ir::node_data<double>(5.0));

    phylanx::execution_tree::primitive col_stop =
        hpx::new_<phylanx::execution_tree::primitives::variable>(
            hpx::find_here(), phylanx::ir::node_data<double>(15.0));

    phylanx::execution_tree::primitive slice =
        hpx::new_<phylanx::execution_tree::primitives::slicing_operation>(
            hpx::find_here(),
            std::vector<phylanx::execution_tree::primitive_argument_type>{
                std::move(input_matrix), std::move(row_start),
                std::move(row_stop), std::move(col_start),
                std::move(col_stop)
            });

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
    // column = col_start
    // m = (row_stop - row_start)
    // n = (col_stop - col_start)

    // Here, matrix = m1 , row = 5, column = 5, m = 42, n = 10

    auto sm = blaze::submatrix(m1, 5, 5, 42, 10);
    auto expected = sm;

    hpx::future<phylanx::execution_tree::primitive_result_type> f =
        slice.eval();

    HPX_TEST_EQ(phylanx::ir::node_data<double>(std::move(expected)),
        phylanx::execution_tree::extract_numeric_value(f.get()));
}

void test_slicing_operation_2d_zero_start()
{
    // parameters required by phylanx to create a slice is as follows:
    // matrix The matrix containing the submatrix.
    // row_start The index of the first row of the submatrix.
    // row_stop The index of the last row(exclusive) of the submatrix.
    // col_start The index of the first column of the submatrix.
    // col_stop The index of the last column(exclusive) of the submatrix.

    blaze::Rand<blaze::DynamicMatrix<double>> gen{};
    blaze::DynamicMatrix<double> m1 = gen.generate(101UL, 101UL);

    phylanx::execution_tree::primitive input_matrix =
        hpx::new_<phylanx::execution_tree::primitives::variable>(
            hpx::find_here(), phylanx::ir::node_data<double>(m1));

    phylanx::execution_tree::primitive row_start =
        hpx::new_<phylanx::execution_tree::primitives::variable>(
            hpx::find_here(), phylanx::ir::node_data<double>(0.0));

    phylanx::execution_tree::primitive row_stop =
        hpx::new_<phylanx::execution_tree::primitives::variable>(
            hpx::find_here(), phylanx::ir::node_data<double>(47.0));

    phylanx::execution_tree::primitive col_start =
        hpx::new_<phylanx::execution_tree::primitives::variable>(
            hpx::find_here(), phylanx::ir::node_data<double>(0.0));

    phylanx::execution_tree::primitive col_stop =
        hpx::new_<phylanx::execution_tree::primitives::variable>(
            hpx::find_here(), phylanx::ir::node_data<double>(15.0));

    phylanx::execution_tree::primitive slice =
        hpx::new_<phylanx::execution_tree::primitives::slicing_operation>(
            hpx::find_here(),
            std::vector<phylanx::execution_tree::primitive_argument_type>{
                std::move(input_matrix), std::move(row_start),
                std::move(row_stop), std::move(col_start),
                std::move(col_stop)
            });

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
    // performed in blaze.
    // matrix = matrix
    // row = row_start
    // column = col_start
    // m = (row_stop - row_start)
    // n = (col_stop - col_start)

    // Here, matrix = m1 , row = 0, column = 0, m = 47, n = 15

    auto sm = blaze::submatrix(m1, 0, 0, 47, 15);
    auto expected = sm;

    hpx::future<phylanx::execution_tree::primitive_result_type> f =
        slice.eval();

    HPX_TEST_EQ(phylanx::ir::node_data<double>(std::move(expected)),
        phylanx::execution_tree::extract_numeric_value(f.get()));
}

void test_col_slicing_operation_from_end()
{
    blaze::Rand<blaze::DynamicMatrix<double>> gen{};
    blaze::DynamicMatrix<double> m1 = gen.generate(101UL, 101UL);

    phylanx::execution_tree::primitive input_matrix =
            hpx::new_<phylanx::execution_tree::primitives::variable>(
                    hpx::find_here(), phylanx::ir::node_data<double>(m1));

    phylanx::execution_tree::primitive row_start =
            hpx::new_<phylanx::execution_tree::primitives::variable>(
                    hpx::find_here(), phylanx::ir::node_data<double>(0.0));

    phylanx::execution_tree::primitive row_stop =
            hpx::new_<phylanx::execution_tree::primitives::variable>(
                    hpx::find_here(), phylanx::ir::node_data<double>(47.0));

    phylanx::execution_tree::primitive col_start =
            hpx::new_<phylanx::execution_tree::primitives::variable>(
                    hpx::find_here(), phylanx::ir::node_data<double>(-2.0));


    phylanx::execution_tree::primitive col_stop =
            hpx::new_<phylanx::execution_tree::primitives::variable>(
                    hpx::find_here(), phylanx::ir::node_data<double>(0.0));

    phylanx::execution_tree::primitive slice =
            hpx::new_<phylanx::execution_tree::primitives::slicing_operation>(
                    hpx::find_here(),
                    std::vector<phylanx::execution_tree::primitive_argument_type>{
                            std::move(input_matrix), std::move(row_start),
                            std::move(row_stop), std::move(col_start),
                            std::move(col_stop)
                    });


    auto sm = blaze::submatrix(m1, 0, 99, 47, 2);
    auto expected = sm;


    hpx::future<phylanx::execution_tree::primitive_result_type> f =
            slice.eval();


    HPX_TEST_EQ(phylanx::ir::node_data<double>(std::move(expected)),
                phylanx::execution_tree::extract_numeric_value(f.get()));
}

void test_row_slicing_operation_from_end()
{
    blaze::Rand<blaze::DynamicMatrix<double>> gen{};
    blaze::DynamicMatrix<double> m1 = gen.generate(101UL, 101UL);

    phylanx::execution_tree::primitive input_matrix =
            hpx::new_<phylanx::execution_tree::primitives::variable>(
                    hpx::find_here(), phylanx::ir::node_data<double>(m1));

    phylanx::execution_tree::primitive row_start =
            hpx::new_<phylanx::execution_tree::primitives::variable>(
                    hpx::find_here(), phylanx::ir::node_data<double>(-5.0));


    phylanx::execution_tree::primitive row_stop =
            hpx::new_<phylanx::execution_tree::primitives::variable>(
                    hpx::find_here(), phylanx::ir::node_data<double>(0.0));

    phylanx::execution_tree::primitive col_start =
            hpx::new_<phylanx::execution_tree::primitives::variable>(
                    hpx::find_here(), phylanx::ir::node_data<double>(6.0));

    phylanx::execution_tree::primitive col_stop =
            hpx::new_<phylanx::execution_tree::primitives::variable>(
                    hpx::find_here(), phylanx::ir::node_data<double>(16.0));

    phylanx::execution_tree::primitive slice =
            hpx::new_<phylanx::execution_tree::primitives::slicing_operation>(
                    hpx::find_here(),
                    std::vector<phylanx::execution_tree::primitive_argument_type>{
                            std::move(input_matrix), std::move(row_start),
                            std::move(row_stop), std::move(col_start),
                            std::move(col_stop)
                    });


    auto sm = blaze::submatrix(m1, 96, 6, 5, 10);
    auto expected = sm;

    hpx::future<phylanx::execution_tree::primitive_result_type> f =
            slice.eval();

    HPX_TEST_EQ(phylanx::ir::node_data<double>(std::move(expected)),
                phylanx::execution_tree::extract_numeric_value(f.get()));
}


void test_col_slicing_operation_from_end_negative_start_stop()
{
    blaze::Rand<blaze::DynamicMatrix<double>> gen{};
    blaze::DynamicMatrix<double> m1 = gen.generate(101UL, 101UL);

    phylanx::execution_tree::primitive input_matrix =
            hpx::new_<phylanx::execution_tree::primitives::variable>(
                    hpx::find_here(), phylanx::ir::node_data<double>(m1));

    phylanx::execution_tree::primitive row_start =
            hpx::new_<phylanx::execution_tree::primitives::variable>(
                    hpx::find_here(), phylanx::ir::node_data<double>(0.0));

    phylanx::execution_tree::primitive row_stop =
            hpx::new_<phylanx::execution_tree::primitives::variable>(
                    hpx::find_here(), phylanx::ir::node_data<double>(47.0));

    phylanx::execution_tree::primitive col_start =
            hpx::new_<phylanx::execution_tree::primitives::variable>(
                    hpx::find_here(), phylanx::ir::node_data<double>(-5.0));

    phylanx::execution_tree::primitive col_stop =
            hpx::new_<phylanx::execution_tree::primitives::variable>(
                    hpx::find_here(), phylanx::ir::node_data<double>(-2.0));

    phylanx::execution_tree::primitive slice =
            hpx::new_<phylanx::execution_tree::primitives::slicing_operation>(
                    hpx::find_here(),
                    std::vector<phylanx::execution_tree::primitive_argument_type>{
                            std::move(input_matrix), std::move(row_start),
                            std::move(row_stop), std::move(col_start),
                            std::move(col_stop)
                    });


    auto sm = blaze::submatrix(m1, 0, 96, 47, 3);
    auto expected = sm;


    hpx::future<phylanx::execution_tree::primitive_result_type> f =
            slice.eval();


    HPX_TEST_EQ(phylanx::ir::node_data<double>(std::move(expected)),
                phylanx::execution_tree::extract_numeric_value(f.get()));
}

void test_row_slicing_operation_from_end_negative_start_stop()
{
    blaze::Rand<blaze::DynamicMatrix<double>> gen{};
    blaze::DynamicMatrix<double> m1 = gen.generate(101UL, 101UL);

    phylanx::execution_tree::primitive input_matrix =
            hpx::new_<phylanx::execution_tree::primitives::variable>(
                    hpx::find_here(), phylanx::ir::node_data<double>(m1));

    phylanx::execution_tree::primitive row_start =
            hpx::new_<phylanx::execution_tree::primitives::variable>(
                    hpx::find_here(), phylanx::ir::node_data<double>(-5.0));

    phylanx::execution_tree::primitive row_stop =
            hpx::new_<phylanx::execution_tree::primitives::variable>(
                    hpx::find_here(), phylanx::ir::node_data<double>(-2.0));

    phylanx::execution_tree::primitive col_start =
            hpx::new_<phylanx::execution_tree::primitives::variable>(
                    hpx::find_here(), phylanx::ir::node_data<double>(6.0));

    phylanx::execution_tree::primitive col_stop =
            hpx::new_<phylanx::execution_tree::primitives::variable>(
                    hpx::find_here(), phylanx::ir::node_data<double>(16.0));

    phylanx::execution_tree::primitive slice =
            hpx::new_<phylanx::execution_tree::primitives::slicing_operation>(
                    hpx::find_here(),
                    std::vector<phylanx::execution_tree::primitive_argument_type>{
                            std::move(input_matrix), std::move(row_start),
                            std::move(row_stop), std::move(col_start),
                            std::move(col_stop)
                    });


    auto sm = blaze::submatrix(m1, 96, 6, 3, 10);
    auto expected = sm;

    hpx::future<phylanx::execution_tree::primitive_result_type> f =
            slice.eval();

    HPX_TEST_EQ(phylanx::ir::node_data<double>(std::move(expected)),
                phylanx::execution_tree::extract_numeric_value(f.get()));
}

void test_row_and_column_slicing_from_end_negative_start_stop()
{
    blaze::Rand<blaze::DynamicMatrix<double>> gen{};
    blaze::DynamicMatrix<double> m1 = gen.generate(101UL, 101UL);

    phylanx::execution_tree::primitive input_matrix =
            hpx::new_<phylanx::execution_tree::primitives::variable>(
                    hpx::find_here(), phylanx::ir::node_data<double>(m1));

    phylanx::execution_tree::primitive row_start =
            hpx::new_<phylanx::execution_tree::primitives::variable>(
                    hpx::find_here(), phylanx::ir::node_data<double>(-5.0));

    phylanx::execution_tree::primitive row_stop =
            hpx::new_<phylanx::execution_tree::primitives::variable>(
                    hpx::find_here(), phylanx::ir::node_data<double>(-2.0));

    phylanx::execution_tree::primitive col_start =
            hpx::new_<phylanx::execution_tree::primitives::variable>(
                    hpx::find_here(), phylanx::ir::node_data<double>(-5.0));

    phylanx::execution_tree::primitive col_stop =
            hpx::new_<phylanx::execution_tree::primitives::variable>(
                    hpx::find_here(), phylanx::ir::node_data<double>(-3.0));

    phylanx::execution_tree::primitive slice =
            hpx::new_<phylanx::execution_tree::primitives::slicing_operation>(
                    hpx::find_here(),
                    std::vector<phylanx::execution_tree::primitive_argument_type>{
                            std::move(input_matrix), std::move(row_start),
                            std::move(row_stop), std::move(col_start),
                            std::move(col_stop)
                    });


    auto sm = blaze::submatrix(m1, 96, 96, 3, 2);
    auto expected = sm;

    hpx::future<phylanx::execution_tree::primitive_result_type> f =
            slice.eval();

    HPX_TEST_EQ(phylanx::ir::node_data<double>(std::move(expected)),
                phylanx::execution_tree::extract_numeric_value(f.get()));
}

int main(int argc, char* argv[])
{
    test_slicing_operation_0d();
    test_slicing_operation_1d();
    test_slicing_operation_2d();

    test_slicing_operation_1d_zero_start();
    test_slicing_operation_2d_zero_start();

    test_col_slicing_operation_from_end();
    test_row_slicing_operation_from_end();

    test_col_slicing_operation_from_end_negative_start_stop();
    test_row_slicing_operation_from_end_negative_start_stop();

    test_row_and_column_slicing_from_end_negative_start_stop();

    return hpx::util::report_errors();
}
