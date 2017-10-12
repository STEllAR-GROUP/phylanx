// Copyright (c) 2017 Bibek Wagle
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/phylanx.hpp>

#include <hpx/hpx_main.hpp>
#include <hpx/include/lcos.hpp>
#include <hpx/util/lightweight_test.hpp>

void test_for_operation()
{
  char const* const for_code = R"(
    block(
        while(
            (x < 10),
            (store(x, x + 1))
        ),
        x
    )
)";

  phylanx::execution_tree::variables variables = {
      {"x", phylanx::ir::node_data<double>{0.0}},
  };

  // generate an execution tree from the given code
  phylanx::execution_tree::primitive_argument_type p =
      phylanx::execution_tree::generate_tree(for_code, variables);

  // evaluate generated execution tree
  hpx::future<phylanx::ir::node_data<double>> f =
      phylanx::execution_tree::numeric_operand(p);

  std::cout << "Result: \n" << f.get() << std::endl;

}

int main(int argc, char* argv[])
{
  test_for_operation();
  return hpx::util::report_errors();
}