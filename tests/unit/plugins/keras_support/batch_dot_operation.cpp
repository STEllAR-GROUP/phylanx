// Copyright (c) 2019 Bita Hasheminezhad
// Copyright (c) 2019 Hartmut Kaiser
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

#include <blaze/Math.h>

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
void test_batch_dot_operation(std::string const& code,
    std::string const& expected_str)
{
    HPX_TEST_EQ(compile_and_run(code), compile_and_run(expected_str));
}

///////////////////////////////////////////////////////////////////////////////
int main(int argc, char* argv[])
{
    test_batch_dot_operation(
        "batch_dot([[1, 2, 3], [4, 5, 6]], [[7, 8],[9, 10],[11, 12]])",
        "[[ 58],[154]]");
    test_batch_dot_operation(
        "batch_dot([[1, 2], [1 , 0]], [[33],[42]])", "[[117], [ 33]]");
    test_batch_dot_operation(
        "batch_dot([[1, 42, 3]], [[7, 8, 1],[9, 10, 1],[0, 13, 1]])",
        "[[385], [467], [ 46]]");
    test_batch_dot_operation(
        "batch_dot([[13, 42, 0]], [[1],[0],[33]])", "[[13]]");

#if defined(PHYLANX_HAVE_BLAZE_TENSOR)
    test_batch_dot_operation(
        "batch_dot([[1, 2, 3], [4, 5, 6]], [[[-5,-6, 1, 0],[-7,-8, 1, 0]"
        ",[-1,-2, 13, 2]],[[ 5, 6,-1, 5],[7, 1, 0, 8],[1, 1,-2, 2]]])",
        "[[-22, -28,  42,   6], [ 61,  35, -16,  72]]");
#endif

    return hpx::util::report_errors();
}
