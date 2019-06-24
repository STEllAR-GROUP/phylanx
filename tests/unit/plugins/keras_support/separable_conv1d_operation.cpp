// Copyright (c) 2019 Bita Hasheminezhad
// Copyright (c) 2019 Hartmut Kaiser
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/phylanx.hpp>

#include <hpx/hpx_main.hpp>
#include <hpx/include/lcos.hpp>
#include <hpx/util/lightweight_test.hpp>

#include <string>
#include <utility>

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
void test_separable_conv1d_operation(std::string const& code,
    std::string const& expected_str)
{
    HPX_TEST_EQ(compile_and_run(code), compile_and_run(expected_str));
}

///////////////////////////////////////////////////////////////////////////////
int main(int argc, char* argv[])
{

    test_separable_conv1d_operation(
        R"(separable_conv1d([[[1,2,3],[4,5,6],[7,8,9]],[[-1,-2,-3],[-4,-5,-6],
        [-7,-8,-9]],[[11,12,13],[14,15,16],[17,18,19]],[[-11,-12,-13],
        [-14,-15,-16] ,[17,18,19]]], [[[1,-1,0,0],[3,-3,4,-4],[1,0,13,0]],
        [[2,-42,0,5], [2,-2,10,-10],[2,2,12,3]]], [[[1,1],[1,2],[-2,3],[1,4],
        [1,6],[1,0],[-4,0],[11,6],[0,0],[10,0],[1,8],[12,33]]], "valid"))",
        "[[[ -563.,   981.], [-1055.,  1527.]],[[  563.,  -981.],"
        "[ 1055., -1527.]], [[-2203.,  2801.], [-2695.,  3347.]],"
        "[[ 2203., -2801.],[-1311.,   227.]]]");
    test_separable_conv1d_operation(
        R"(separable_conv1d([[[1,2,3],[4,5,6],[7,8,9]],[[-1,-2,-3],[-4,-5,-6],
        [-7,-8,-9]],[[11,12,13],[14,15,16],[17,18,19]],[[-11,-12,-13],
        [-14,-15,-16] ,[17,18,19]]], [[[1,-1,0,0],[3,-3,4,-4],[1,0,13,0]],
        [[2,-42,0,5], [2,-2,10,-10],[2,2,12,3]]], [[[1,1],[1,2],[-2,3],[1,4],
        [1,6],[1,0],[-4,0],[11,6],[0,0],[10,0],[1,8],[12,33]]], "valid", 3))",
        "[[[ -563.,   981.]],[[  563.,  -981.]],"
        "[[-2203.,  2801.]],[[ 2203., -2801.]]]");
    test_separable_conv1d_operation(
        R"(separable_conv1d([[[1,2,3],[4,5,6],[7,8,9]],[[-1,-2,-3],[-4,-5,-6],
        [-7,-8,-9]],[[11,12,13],[14,15,16],[17,18,19]],[[-11,-12,-13],
        [-14,-15,-16] ,[17,18,19]]], [[[1,-1,0,0],[3,-3,4,-4],[1,0,13,0]],
        [[2,-42,0,5], [2,-2,10,-10],[2,2,12,3]]], [[[1,1],[1,2],[-2,3],[1,4],
        [1,6],[1,0],[-4,0],[11,6],[0,0],[10,0],[1,8],[12,33]]], "valid", 1, 2))",
        "[[[ -914.,  1236.]], [[  914., -1236.]],"
        "[[-2554.,  3056.]], [[-1452.,   518.]]]");

    return hpx::util::report_errors();
}
