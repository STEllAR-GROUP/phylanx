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

void test_column_slicing_operation_0d()
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

    phylanx::execution_tree::primitive slice =
      hpx::new_<phylanx::execution_tree::primitives::column_slicing_operation>(
          hpx::find_here(),
          std::vector<phylanx::execution_tree::primitive_argument_type>{
              std::move(first), std::move(second), std::move(third)
          });

    hpx::future<phylanx::execution_tree::primitive_result_type> f =
      slice.eval();

    HPX_TEST_EQ(42.0, phylanx::execution_tree::extract_numeric_value(f.get())[0]);
}

void test_column_slicing_operation_1d()
{
    // parameters required by phylanx to create a slice is as follows:
    // vector : v1 in this testcase
    // col_start The index of the first element to extract.
    // col_start The index of the last element to extract.

    blaze::Rand<blaze::DynamicVector<double>> gen{};
    blaze::DynamicVector<double> v1 = gen.generate(1007UL);

    phylanx::execution_tree::primitive input_vector =
        hpx::new_<phylanx::execution_tree::primitives::variable>(
            hpx::find_here(), phylanx::ir::node_data<double>(v1));

    phylanx::execution_tree::primitive col_start =
        hpx::new_<phylanx::execution_tree::primitives::variable>(
            hpx::find_here(), phylanx::ir::node_data<double>(5.0));

    phylanx::execution_tree::primitive col_stop =
        hpx::new_<phylanx::execution_tree::primitives::variable>(
            hpx::find_here(), phylanx::ir::node_data<double>(15.0));

    phylanx::execution_tree::primitive slice =
        hpx::new_<phylanx::execution_tree::primitives::column_slicing_operation>(
            hpx::find_here(),
            std::vector<phylanx::execution_tree::primitive_argument_type>{
                std::move(input_vector), std::move(col_start),
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
    // size = (col_stop - col_start)+1

    // Here, matrix = m1 , column = 5, n = 11
    auto sm = blaze::subvector(v1, 5, 11);
    auto expected = sm;

    hpx::future<phylanx::execution_tree::primitive_result_type> f =
        slice.eval();

    HPX_TEST_EQ(phylanx::ir::node_data<double>(std::move(expected)),
        phylanx::execution_tree::extract_numeric_value(f.get()));
}

void test_column_slicing_operation_2d()
{
    // parameters required by phylanx to create a slice is as follows:
    // matrix The matrix containing the submatrix.
    // col_start The index of the first row of the submatrix.
    // col_stop The index of the last row of the submatrix.

    blaze::Rand<blaze::DynamicMatrix<double>> gen{};
    blaze::DynamicMatrix<double> m1 = gen.generate(90UL, 101UL);

    phylanx::execution_tree::primitive input_matrix =
        hpx::new_<phylanx::execution_tree::primitives::variable>(
            hpx::find_here(), phylanx::ir::node_data<double>(m1));

    phylanx::execution_tree::primitive col_start =
        hpx::new_<phylanx::execution_tree::primitives::variable>(
            hpx::find_here(), phylanx::ir::node_data<double>(5.0));

    phylanx::execution_tree::primitive col_stop =
        hpx::new_<phylanx::execution_tree::primitives::variable>(
            hpx::find_here(), phylanx::ir::node_data<double>(47.0));

    phylanx::execution_tree::primitive slice =
        hpx::new_<phylanx::execution_tree::primitives::column_slicing_operation>(
            hpx::find_here(),
            std::vector<phylanx::execution_tree::primitive_argument_type>{
                std::move(input_matrix), std::move(col_start),
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
    // row = 0
    // column = col_start
    // m = number of rows in the input matrix
    // n = (col_stop - col_start)+1
    // the input matrix respectively
    // Here, matrix = m1 , column = 5, n = 43

    auto sm = blaze::submatrix(m1,0,5,90,43);
    auto expected = sm;

    hpx::future<phylanx::execution_tree::primitive_result_type> f =
          slice.eval();

    HPX_TEST_EQ(phylanx::ir::node_data<double>(std::move(expected)),
              phylanx::execution_tree::extract_numeric_value(f.get()));
}

int main(int argc, char* argv[])
{
  test_column_slicing_operation_0d();
  test_column_slicing_operation_1d();
  test_column_slicing_operation_2d();

  return hpx::util::report_errors();
}
