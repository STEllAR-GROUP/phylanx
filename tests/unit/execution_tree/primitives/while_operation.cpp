//   Copyright (c) 2017 Hartmut Kaiser
//
//   Distributed under the Boost Software License, Version 1.0. (See accompanying
//   file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef NOMINMAX
#define NOMINMAX
#endif

#include <phylanx/phylanx.hpp>

#include <hpx/hpx_main.hpp>
#include <hpx/include/lcos.hpp>
#include <hpx/util/lightweight_test.hpp>

#include <vector>

// condition is false, no iteration is performed
void test_while_operation_false()
{
    phylanx::execution_tree::primitive cond =
        hpx::new_<phylanx::execution_tree::primitives::variable>(
            hpx::find_here(), false);
    phylanx::execution_tree::primitive body =
        hpx::new_<phylanx::execution_tree::primitives::variable>(
            hpx::find_here(), phylanx::ir::node_data<double>{42.0});

    phylanx::execution_tree::primitive while_ =
        hpx::new_<phylanx::execution_tree::primitives::while_operation>(
            hpx::find_here(),
            std::vector<phylanx::execution_tree::primitive_argument_type>{
                std::move(cond), std::move(body)
            });

    hpx::future<phylanx::execution_tree::primitive_result_type> f =
        while_.eval();

    HPX_TEST(!phylanx::execution_tree::valid(f.get()));
}

// condition is set to false in first iteration
void test_while_operation_true()
{
    phylanx::execution_tree::primitive cond =
        hpx::new_<phylanx::execution_tree::primitives::variable>(
            hpx::find_here(), true);
    phylanx::execution_tree::primitive body =
        hpx::new_<phylanx::execution_tree::primitives::store_operation>(
            hpx::find_here(),
            std::vector<phylanx::execution_tree::primitive_argument_type>{
                cond, false
            });

    phylanx::execution_tree::primitive while_ =
        hpx::new_<phylanx::execution_tree::primitives::while_operation>(
            hpx::find_here(),
            std::vector<phylanx::execution_tree::primitive_argument_type>{
                std::move(cond), std::move(body)
            });

    hpx::future<phylanx::execution_tree::primitive_result_type> f =
        while_.eval();

    HPX_TEST(!phylanx::execution_tree::extract_boolean_value(f.get()));
}

// condition is set to false in first iteration
void test_while_operation_true_return()
{
    phylanx::execution_tree::primitive cond =
        hpx::new_<phylanx::execution_tree::primitives::variable>(
            hpx::find_here(), true);
    phylanx::execution_tree::primitive store =
        hpx::new_<phylanx::execution_tree::primitives::store_operation>(
            hpx::find_here(),
            std::vector<phylanx::execution_tree::primitive_argument_type>{
                cond, false
            });
    phylanx::execution_tree::primitive body =
        hpx::new_<phylanx::execution_tree::primitives::block_operation>(
            hpx::find_here(),
            std::vector<phylanx::execution_tree::primitive_argument_type>{
                std::move(store), true
            });

    phylanx::execution_tree::primitive while_ =
        hpx::new_<phylanx::execution_tree::primitives::while_operation>(
            hpx::find_here(),
            std::vector<phylanx::execution_tree::primitive_argument_type>{
                std::move(cond), std::move(body)
            });

    hpx::future<phylanx::execution_tree::primitive_result_type> f =
        while_.eval();

    HPX_TEST(phylanx::execution_tree::extract_boolean_value(f.get()));
}

int main(int argc, char* argv[])
{
    test_while_operation_false();
    test_while_operation_true();
    test_while_operation_true_return();

    return hpx::util::report_errors();
}


