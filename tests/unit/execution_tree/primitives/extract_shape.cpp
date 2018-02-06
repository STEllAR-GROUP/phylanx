//   Copyright (c) 2017 Bibek Wagle
//
//   Distributed under the Boost Software License, Version 1.0. (See accompanying
//   file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/phylanx.hpp>

#include <hpx/hpx_main.hpp>
#include <hpx/include/lcos.hpp>
#include <hpx/util/lightweight_test.hpp>

#include <utility>
#include <vector>

void test_block_operation()
{
    phylanx::execution_tree::primitive arg1 =
        hpx::new_<phylanx::execution_tree::primitives::variable>(
            hpx::find_here(), phylanx::ir::node_data<double>(5.0));
    phylanx::execution_tree::primitive arg2 =
        hpx::new_<phylanx::execution_tree::primitives::variable>(
            hpx::find_here(), phylanx::ir::node_data<double>(7.0));
    phylanx::execution_tree::primitive arg3 =
        hpx::new_<phylanx::execution_tree::primitives::variable>(
            hpx::find_here(), phylanx::ir::node_data<double>(9.0));

    phylanx::execution_tree::primitive block =
        hpx::new_<phylanx::execution_tree::primitives::block_operation>(
            hpx::find_here(),
            std::vector<phylanx::execution_tree::primitive_argument_type>{
                std::move(arg1), std::move(arg2), std::move(arg3)
            });

    hpx::future<phylanx::execution_tree::primitive_argument_type> f =
        block.eval();

    HPX_TEST_EQ(
        9.0, phylanx::execution_tree::extract_numeric_value(f.get())[0]);
}

int main(int argc, char* argv[])
{
    test_block_operation();

    return hpx::util::report_errors();
}


