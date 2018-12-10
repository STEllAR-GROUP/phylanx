// Copyright (c) 2018 Bita Hasheminezhad
// Copyright (c) 2018 Hartmut Kaiser
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/phylanx.hpp>

#include <hpx/hpx_main.hpp>
#include <hpx/include/lcos.hpp>
#include <hpx/util/lightweight_test.hpp>

#include <cstdint>
#include <iostream>
#include <string>
#include <utility>
#include <vector>

//////////////////////////////////////////////////////////////////////////
void test_softmax_operation_0d()
{
    phylanx::execution_tree::primitive first =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(42.0));

    phylanx::execution_tree::primitive softmax =
        phylanx::execution_tree::primitives::create_softmax_operation(
            hpx::find_here(),
            phylanx::execution_tree::primitive_arguments_type{
                std::move(first)});

    hpx::future<phylanx::execution_tree::primitive_argument_type> f =
        softmax.eval();

    HPX_TEST_EQ(
        1.0, phylanx::execution_tree::extract_numeric_value(f.get())[0]);
}



int main(int argc, char* argv[])
{
    test_softmax_operation_0d();
    //test_softmax_operation_1d();
    //test_softmax_operation_2d();
    //test_softmax_operation_2d_column();
    //test_softmax_operation_2d_row();

    return hpx::util::report_errors();
}
