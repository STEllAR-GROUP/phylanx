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

int main(int argc, char* argv[])
{
  test_slicing_operation_0d();

  return hpx::util::report_errors();
}
