//   Copyright (c) 2017 Hartmut Kaiser
//   Copyright (c) 2017 Adrian Serio
//
//   Distributed under the Boost Software License, Version 1.0. (See accompanying
//   file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)


#include <phylanx/phylanx.hpp>

#include <hpx/hpx_main.hpp>
#include <hpx/include/lcos.hpp>
#include <hpx/util/lightweight_test.hpp>

#include <blaze/Math.h>

#include <iostream>
#include <utility>
#include <vector>

// Test 1
//  test literal conditional and literal cases
void test_if_conditional_t1()
{
    phylanx::execution_tree::primitive cond =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(1.0));

    phylanx::execution_tree::primitive true_case =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(1.0));

    phylanx::execution_tree::primitive false_case =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(0.0));

    phylanx::execution_tree::primitive if_prim =
        phylanx::execution_tree::primitives::create_if_conditional(
            hpx::find_here(),
            std::vector<phylanx::execution_tree::primitive_argument_type>{
                std::move(cond), std::move(true_case), std::move(false_case)});

    hpx::future<phylanx::execution_tree::primitive_argument_type> f =
        if_prim.eval();
    HPX_TEST_EQ(
        1.0, phylanx::execution_tree::extract_numeric_value(f.get())[0]);
}

// Test 2
//  test a literal conditional and two expressions
void test_if_conditional_t2()
{
    // Create addition expression
    phylanx::execution_tree::primitive add_lhs =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(41.0));

    phylanx::execution_tree::primitive add_rhs =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(1.0));

    phylanx::execution_tree::primitive add =
        phylanx::execution_tree::primitives::create_add_operation(
            hpx::find_here(),
            std::vector<phylanx::execution_tree::primitive_argument_type>{
                std::move(add_lhs), std::move(add_rhs)});

    // Create subtration expression
    //   What is six by nine?
    phylanx::execution_tree::primitive sub_lhs =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(58.0));

    phylanx::execution_tree::primitive sub_rhs =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(4.0));

    phylanx::execution_tree::primitive sub =
        phylanx::execution_tree::primitives::create_sub_operation(
            hpx::find_here(),
            std::vector<phylanx::execution_tree::primitive_argument_type>{
                std::move(sub_lhs), std::move(sub_rhs)});

    // Create conditional
    phylanx::execution_tree::primitive cond =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(1.0));

    phylanx::execution_tree::primitive if_prim =
        phylanx::execution_tree::primitives::create_if_conditional(
            hpx::find_here(),
            std::vector<phylanx::execution_tree::primitive_argument_type>{
                std::move(cond), std::move(add), std::move(sub)});

    hpx::future<phylanx::execution_tree::primitive_argument_type> f =
        if_prim.eval();

    HPX_TEST_EQ(
        42.0, phylanx::execution_tree::extract_numeric_value(f.get())[0]);
}

// Test 3
//  test if with expressions
void test_if_conditional_t3()
{
    // Create addition expression
    phylanx::execution_tree::primitive add_lhs =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(41.0));

    phylanx::execution_tree::primitive add_rhs =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(1.0));

    phylanx::execution_tree::primitive add =
        phylanx::execution_tree::primitives::create_add_operation(
            hpx::find_here(),
            std::vector<phylanx::execution_tree::primitive_argument_type>{
                std::move(add_lhs), std::move(add_rhs)});

    // Create subtration expression
    phylanx::execution_tree::primitive sub_lhs =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(58.0));

    phylanx::execution_tree::primitive sub_rhs =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(4.0));

    phylanx::execution_tree::primitive sub =
        phylanx::execution_tree::primitives::create_sub_operation(
            hpx::find_here(),
            std::vector<phylanx::execution_tree::primitive_argument_type>{
                std::move(sub_lhs), std::move(sub_rhs)});

    // Create conditional expression
    phylanx::execution_tree::primitive lhs =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(42.0));

    phylanx::execution_tree::primitive rhs =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(1.0));

    phylanx::execution_tree::primitive equal =
        phylanx::execution_tree::primitives::create_equal(hpx::find_here(),
            std::vector<phylanx::execution_tree::primitive_argument_type>{
                std::move(lhs), std::move(rhs)});

    phylanx::execution_tree::primitive if_prim =
        phylanx::execution_tree::primitives::create_if_conditional(
            hpx::find_here(),
            std::vector<phylanx::execution_tree::primitive_argument_type>{
                std::move(equal), std::move(add), std::move(sub)});

    hpx::future<phylanx::execution_tree::primitive_argument_type> f =
        if_prim.eval();

    HPX_TEST_EQ(
        54.0, phylanx::execution_tree::extract_numeric_value(f.get())[0]);
}

// Test 4
//  test two input if with literals
void test_if_conditional_t4()
{
    phylanx::execution_tree::primitive cond =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), true);

    phylanx::execution_tree::primitive true_case =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(42.0));

    phylanx::execution_tree::primitive if_prim =
        phylanx::execution_tree::primitives::create_if_conditional(
            hpx::find_here(),
            std::vector<phylanx::execution_tree::primitive_argument_type>{
                std::move(cond), std::move(true_case)});

    hpx::future<phylanx::execution_tree::primitive_argument_type> f =
        if_prim.eval();
    HPX_TEST_EQ(
        42.0, phylanx::execution_tree::extract_numeric_value(f.get())[0]);
}

// Test 5
//  test two input if with a literal condition and an expression
void test_if_conditional_t5()
{
    phylanx::execution_tree::primitive cond =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), true);

    // Create addition expression
    phylanx::execution_tree::primitive add_lhs =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(41.0));

    phylanx::execution_tree::primitive add_rhs =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(1.0));

    phylanx::execution_tree::primitive add =
        phylanx::execution_tree::primitives::create_add_operation(
            hpx::find_here(),
            std::vector<phylanx::execution_tree::primitive_argument_type>{
                std::move(add_lhs), std::move(add_rhs)});

    phylanx::execution_tree::primitive if_prim =
        phylanx::execution_tree::primitives::create_if_conditional(
            hpx::find_here(),
            std::vector<phylanx::execution_tree::primitive_argument_type>{
                std::move(cond), std::move(add)});

    hpx::future<phylanx::execution_tree::primitive_argument_type> f =
        if_prim.eval();
    HPX_TEST_EQ(
        42.0, phylanx::execution_tree::extract_numeric_value(f.get())[0]);
}

// Test 6
//  test two input if with a literal (false) condition and an expression
void test_if_conditional_t6()
{
    phylanx::execution_tree::primitive cond =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), false);

    // Create addition expression
    phylanx::execution_tree::primitive add_lhs =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(41.0));

    phylanx::execution_tree::primitive add_rhs =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(1.0));

    phylanx::execution_tree::primitive add =
        phylanx::execution_tree::primitives::create_add_operation(
            hpx::find_here(),
            std::vector<phylanx::execution_tree::primitive_argument_type>{
                std::move(add_lhs), std::move(add_rhs)});

    phylanx::execution_tree::primitive if_prim =
        phylanx::execution_tree::primitives::create_if_conditional(
            hpx::find_here(),
            std::vector<phylanx::execution_tree::primitive_argument_type>{
                std::move(cond), std::move(add)});

    hpx::future<phylanx::execution_tree::primitive_argument_type> f =
        if_prim.eval();

    HPX_TEST(!phylanx::execution_tree::valid(f.get()));
}

int main(int argc, char* argv[])
{
    test_if_conditional_t1();
    test_if_conditional_t2();
    test_if_conditional_t3();
    test_if_conditional_t4();
    test_if_conditional_t5();
    test_if_conditional_t6();

    return hpx::util::report_errors();
}
