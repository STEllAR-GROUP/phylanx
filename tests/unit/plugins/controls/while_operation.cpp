//   Copyright (c) 2017 Hartmut Kaiser
//
//   Distributed under the Boost Software License, Version 1.0. (See accompanying
//   file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/phylanx.hpp>

#include <hpx/hpx_main.hpp>
#include <hpx/include/lcos.hpp>
#include <hpx/testing.hpp>

#include <cstdint>
#include <utility>
#include <vector>

// condition is false, no iteration is performed
void test_while_operation_false()
{
    phylanx::execution_tree::primitive cond =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<std::uint8_t>(false));
    phylanx::execution_tree::primitive body =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(),
            phylanx::execution_tree::primitive_argument_type{42.0});

    phylanx::execution_tree::primitive while_ =
        phylanx::execution_tree::primitives::create_while_operation(
            hpx::find_here(),
            phylanx::execution_tree::primitive_arguments_type{
                phylanx::execution_tree::primitive_argument_type{std::move(cond)},
                phylanx::execution_tree::primitive_argument_type{std::move(body)}
            });

    hpx::future<phylanx::execution_tree::primitive_argument_type> f =
        while_.eval();

    HPX_TEST(!phylanx::execution_tree::valid(f.get()));
}

// condition is set to false in first iteration
void test_while_operation_true()
{
    phylanx::execution_tree::primitive cond =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<std::uint8_t>(true));
    phylanx::execution_tree::primitive body =
        phylanx::execution_tree::primitives::create_store_operation(
            hpx::find_here(),
            phylanx::execution_tree::primitive_arguments_type{
                phylanx::execution_tree::primitive_argument_type{cond},
                phylanx::execution_tree::primitive_argument_type{false}
            });

    phylanx::execution_tree::primitive while_ =
        phylanx::execution_tree::primitives::create_while_operation(
            hpx::find_here(),
            phylanx::execution_tree::primitive_arguments_type{
                phylanx::execution_tree::primitive_argument_type{std::move(cond)},
                phylanx::execution_tree::primitive_argument_type{std::move(body)}
            });

    hpx::future<phylanx::execution_tree::primitive_argument_type> f =
        while_.eval();

    HPX_TEST(!phylanx::execution_tree::extract_scalar_boolean_value(f.get()));
}

// condition is set to false in first iteration
void test_while_operation_true_return()
{
    phylanx::execution_tree::primitive cond =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<std::uint8_t>(true));
    phylanx::execution_tree::primitive store =
        phylanx::execution_tree::primitives::create_store_operation(
            hpx::find_here(),
            phylanx::execution_tree::primitive_arguments_type{
                phylanx::execution_tree::primitive_argument_type{cond},
                phylanx::execution_tree::primitive_argument_type{false}
            });
    phylanx::execution_tree::primitive body =
        phylanx::execution_tree::primitives::create_block_operation(
            hpx::find_here(),
            phylanx::execution_tree::primitive_arguments_type{
                phylanx::execution_tree::primitive_argument_type{std::move(store)},
                phylanx::execution_tree::primitive_argument_type{true}
            });

    phylanx::execution_tree::primitive while_ =
        phylanx::execution_tree::primitives::create_while_operation(
            hpx::find_here(),
            phylanx::execution_tree::primitive_arguments_type{
                phylanx::execution_tree::primitive_argument_type{std::move(cond)},
                phylanx::execution_tree::primitive_argument_type{std::move(body)}
            });

    hpx::future<phylanx::execution_tree::primitive_argument_type> f =
        while_.eval();

    HPX_TEST(phylanx::execution_tree::extract_scalar_boolean_value(f.get()));
}

int main(int argc, char* argv[])
{
    test_while_operation_false();
    test_while_operation_true();
    test_while_operation_true_return();

    return hpx::util::report_errors();
}


