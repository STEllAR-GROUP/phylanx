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
    blaze::Rand<blaze::DynamicVector<double, blaze::rowVector>> gen{};
    blaze::DynamicVector<double, blaze::rowVector> v1 = gen.generate(1007UL);

    phylanx::execution_tree::primitive input_vector =
            hpx::new_<phylanx::execution_tree::primitives::variable>(
                    hpx::find_here(), phylanx::ir::node_data<double>(v1));

    phylanx::execution_tree::primitive row_start =
            hpx::new_<phylanx::execution_tree::primitives::variable>(
                    hpx::find_here(), phylanx::ir::node_data<double>(0));

    phylanx::execution_tree::primitive row_stop =
            hpx::new_<phylanx::execution_tree::primitives::variable>(
                    hpx::find_here(), phylanx::ir::node_data<double>(0));

    phylanx::execution_tree::primitive col_start =
            hpx::new_<phylanx::execution_tree::primitives::variable>(
                    hpx::find_here(), phylanx::ir::node_data<double>(5.0));

    phylanx::execution_tree::primitive col_stop =
            hpx::new_<phylanx::execution_tree::primitives::variable>(
                    hpx::find_here(), phylanx::ir::node_data<double>(15.0));

    // row_start an row_stop does not have any effect on the output.
    // row_start and row_stop is set to 0 and 1 repectively internally.
    // any user input for these parameters is ignored internally.
    
    phylanx::execution_tree::primitive slice =
            hpx::new_<phylanx::execution_tree::primitives::slicing_operation>(
                    hpx::find_here(),
                    std::vector<phylanx::execution_tree::primitive_argument_type>{
                            std::move(input_vector), std::move(row_start)
                            , std::move(row_stop), std::move(col_start)
                            , std::move(col_stop)
                    });

    auto sm = blaze::subvector(v1,5,15);
    auto expected = sm;

    hpx::future<phylanx::execution_tree::primitive_result_type> f =
            slice.eval();

    HPX_TEST_EQ(phylanx::ir::node_data<double>(std::move(expected)),
                phylanx::execution_tree::extract_numeric_value(f.get()));
}

void test_slicing_operation_2d()
{
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
                          std::move(input_matrix), std::move(row_start)
                          , std::move(row_stop), std::move(col_start)
                          , std::move(col_stop)
                  });

    auto sm = blaze::submatrix(m1,5,5,47,15);
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

  return hpx::util::report_errors();
}
