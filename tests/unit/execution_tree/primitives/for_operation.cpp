// Copyright (c) 2017 Bibek Wagle
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)


#include <phylanx/phylanx.hpp>

#include <hpx/hpx_main.hpp>
#include <hpx/include/lcos.hpp>
#include <hpx/util/lightweight_test.hpp>

#include <vector>

// condition is false, no iteration is performed
void test_for_operation_false()
{
    phylanx::execution_tree::primitive init =
        hpx::new_<phylanx::execution_tree::primitives::variable>(
            hpx::find_here(), phylanx::ir::node_data<double>{0.0});

    phylanx::execution_tree::primitive cond =
            hpx::new_<phylanx::execution_tree::primitives::variable>(
                    hpx::find_here(), false);

    phylanx::execution_tree::primitive incr =
            hpx::new_<phylanx::execution_tree::primitives::variable>(
                    hpx::find_here(), phylanx::ir::node_data<double>{1.0});

    phylanx::execution_tree::primitive body =
        hpx::new_<phylanx::execution_tree::primitives::variable>(
            hpx::find_here(), phylanx::ir::node_data<double>{42.0});

    phylanx::execution_tree::primitive for_ =
        hpx::new_<phylanx::execution_tree::primitives::for_operation>(
            hpx::find_here(),
            std::vector<phylanx::execution_tree::primitive_argument_type>{
                std::move(init), std::move(cond), std::move(incr), std::move(body)
            });

    hpx::future<phylanx::execution_tree::primitive_result_type> f =
        for_.eval();

    HPX_TEST(!phylanx::execution_tree::valid(f.get()));
}

int main(int argc, char* argv[])
{
    test_for_operation_false();
    return hpx::util::report_errors();
}


