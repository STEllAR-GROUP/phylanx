//   Copyright (c) 2017 Bibek Wagle
//
//   Distributed under the Boost Software License, Version 1.0. (See accompanying
//   file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/phylanx.hpp>

#include <hpx/hpx_main.hpp>
#include <hpx/include/lcos.hpp>
#include <hpx/util/lightweight_test.hpp>


void test_exponential_operation_0d()
{
  phylanx::execution_tree::primitive lhs =
      hpx::new_<phylanx::execution_tree::primitives::literal_value>(
          hpx::find_here(), phylanx::ir::node_data<double>(5.0));

  phylanx::execution_tree::primitive exponential =
      hpx::new_<phylanx::execution_tree::primitives::exponential_operation>(
          hpx::find_here(),std::vector<phylanx::execution_tree::primitive_argument_type
          >{std::move(lhs)}
          );

  hpx::future<phylanx::ir::node_data<double>> f = exponential.eval();
  HPX_TEST_EQ(std::exp(5.0), f.get()[0]);
}

void test_exponential_operation_0d_lit()
{
  phylanx::ir::node_data<double> lhs(5.0);

  phylanx::execution_tree::primitive exponential =
      hpx::new_<phylanx::execution_tree::primitives::exponential_operation>(
          hpx::find_here(), std::vector<phylanx::execution_tree::primitive_argument_type
          >{std::move(lhs)}
      );

  hpx::future<phylanx::ir::node_data<double>> f = exponential.eval();
  HPX_TEST_EQ(std::exp(5.0), f.get()[0]);
}

int main(int argc, char* argv[])
{
  test_exponential_operation_0d();
  test_exponential_operation_0d_lit();

  return hpx::util::report_errors();
}


