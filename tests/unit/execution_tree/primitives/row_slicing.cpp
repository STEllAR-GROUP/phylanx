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

void test_row_slicing_operation_0d()
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
      hpx::new_<phylanx::execution_tree::primitives::row_slicing_operation>(
          hpx::find_here(),
          std::vector<phylanx::execution_tree::primitive_argument_type>{
              std::move(first), std::move(second), std::move(third)
          });

    hpx::future<phylanx::execution_tree::primitive_argument_type> f =
      slice.eval();

    HPX_TEST_EQ(42.0, phylanx::execution_tree::extract_numeric_value(f.get())[0]);
}

void test_row_slicing_operation_1d()
{
    blaze::Rand<blaze::DynamicVector<double>> gen{};
    blaze::DynamicVector<double> v1 = gen.generate(1007UL);

    phylanx::execution_tree::primitive input_vector =
        hpx::new_<phylanx::execution_tree::primitives::variable>(
            hpx::find_here(), phylanx::ir::node_data<double>(v1));

    phylanx::execution_tree::primitive second =
        hpx::new_<phylanx::execution_tree::primitives::variable>(
            hpx::find_here(), phylanx::ir::node_data<double>(5.0));

    phylanx::execution_tree::primitive third =
        hpx::new_<phylanx::execution_tree::primitives::variable>(
            hpx::find_here(), phylanx::ir::node_data<double>(47.0));

    phylanx::execution_tree::primitive slice =
        hpx::new_<phylanx::execution_tree::primitives::row_slicing_operation>(
            hpx::find_here(),
            std::vector<phylanx::execution_tree::primitive_argument_type>{
                std::move(input_vector), std::move(second), std::move(third)
            });

    hpx::future<phylanx::execution_tree::primitive_argument_type> f =
        slice.eval();

    HPX_TEST_EQ(phylanx::ir::node_data<double>(v1),
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
          hpx::new_<phylanx::execution_tree::primitives::variable>(
                  hpx::find_here(), phylanx::ir::node_data<double>(m1));

    phylanx::execution_tree::primitive row_start =
        hpx::new_<phylanx::execution_tree::primitives::variable>(
            hpx::find_here(), phylanx::ir::node_data<double>(5.0));

    phylanx::execution_tree::primitive row_stop =
        hpx::new_<phylanx::execution_tree::primitives::variable>(
            hpx::find_here(), phylanx::ir::node_data<double>(47.0));

    phylanx::execution_tree::primitive slice =
        hpx::new_<phylanx::execution_tree::primitives::row_slicing_operation>(
            hpx::find_here(),
            std::vector<phylanx::execution_tree::primitive_argument_type>{
                std::move(input_matrix), std::move(row_start),
                std::move(row_stop)
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

void test_row_slicing_operation_2d_negative_index()
{
    blaze::Rand<blaze::DynamicMatrix<double>> gen{};
    blaze::DynamicMatrix<double> m1 = gen.generate(90UL, 101UL);

    phylanx::execution_tree::primitive input_matrix =
            hpx::new_<phylanx::execution_tree::primitives::variable>(
                    hpx::find_here(), phylanx::ir::node_data<double>(m1));

    phylanx::execution_tree::primitive row_start =
            hpx::new_<phylanx::execution_tree::primitives::variable>(
                    hpx::find_here(), phylanx::ir::node_data<double>(-5.0));

    phylanx::execution_tree::primitive row_stop =
            hpx::new_<phylanx::execution_tree::primitives::variable>(
                    hpx::find_here(), phylanx::ir::node_data<double>(-2.0));

    phylanx::execution_tree::primitive slice =
            hpx::new_<phylanx::execution_tree::primitives::row_slicing_operation>(
                    hpx::find_here(),
                    std::vector<phylanx::execution_tree::primitive_argument_type>{
                            std::move(input_matrix), std::move(row_start),
                            std::move(row_stop)
                    });


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
            hpx::new_<phylanx::execution_tree::primitives::variable>(
                    hpx::find_here(), phylanx::ir::node_data<double>(m1));

    phylanx::execution_tree::primitive row_start =
            hpx::new_<phylanx::execution_tree::primitives::variable>(
                    hpx::find_here(), phylanx::ir::node_data<double>(-5.0));

    phylanx::execution_tree::primitive row_stop =
            hpx::new_<phylanx::execution_tree::primitives::variable>(
                    hpx::find_here(), phylanx::ir::node_data<double>(-4.0));

    phylanx::execution_tree::primitive slice =
            hpx::new_<phylanx::execution_tree::primitives::row_slicing_operation>(
                    hpx::find_here(),
                    std::vector<phylanx::execution_tree::primitive_argument_type>{
                            std::move(input_matrix), std::move(row_start),
                            std::move(row_stop)
                    });


    auto sm = blaze::submatrix(m1, 85, 0, 1, 101);
    auto expected = blaze::trans(blaze::row(sm, 0));

    hpx::future<phylanx::execution_tree::primitive_argument_type> f =
            slice.eval();

    HPX_TEST_EQ(phylanx::ir::node_data<double>(std::move(expected)),
                phylanx::execution_tree::extract_numeric_value(f.get()));
}


int main(int argc, char* argv[])
{
  test_row_slicing_operation_0d();
  test_row_slicing_operation_1d();
  test_row_slicing_operation_2d();

  test_row_slicing_operation_2d_negative_index();
  test_row_slicing_operation_2d_single_slice_negative_index();

  return hpx::util::report_errors();
}
