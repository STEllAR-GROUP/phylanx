// Copyright (c) 2019 Bita Hasheminezhad
// Copyright (c) 2019 Hartmut Kaiser
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/phylanx.hpp>

#include <hpx/hpx_main.hpp>
#include <hpx/include/lcos.hpp>
#include <hpx/testing.hpp>

#include <iostream>
#include <string>
#include <utility>
#include <vector>

#include <blaze/Math.h>

///////////////////////////////////////////////////////////////////////////////
phylanx::execution_tree::primitive_argument_type compile_and_run(
    std::string const& codestr)
{
    phylanx::execution_tree::compiler::function_list snippets;
    phylanx::execution_tree::compiler::environment env =
        phylanx::execution_tree::compiler::default_environment();

    auto const& code = phylanx::execution_tree::compile(codestr, snippets, env);
    return code.run();
}

///////////////////////////////////////////////////////////////////////////////
void test_bias_add_operation(std::string const& code,
    std::string const& expected_str)
{
    HPX_TEST_EQ(compile_and_run(code), compile_and_run(expected_str));
}

///////////////////////////////////////////////////////////////////////////////
int main(int argc, char* argv[])
{
    // 3d
    test_bias_add_operation(
        "bias_add([[[1,2,3],[4,5,6]],[[7,8,9],[10,11,12]]],[-1,-2,-3])",
        "[[[0., 0., 0.],[3., 3., 3.]],[[6., 6., 6.],[9., 9., 9.]]]");
    test_bias_add_operation(
        "bias_add([[[1,2,3],[4,5,6]],[[7,8,9],[10,11,12]]], "
        "[[-1,-2,-3],[-3,-2,-1]])",
        "[[[ 0., 0., 0.],[ 1., 3., 5.]],[[ 6., 6., 6.],[ 7., 9., 11.]]]");

    return hpx::util::report_errors();
}
