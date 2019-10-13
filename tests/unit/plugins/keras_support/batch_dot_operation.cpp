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

phylanx::execution_tree::primitive_argument_type compile_and_run(
    std::string const& codestr)
{
    phylanx::execution_tree::compiler::function_list snippets;
    phylanx::execution_tree::compiler::environment env =
        phylanx::execution_tree::compiler::default_environment();

    auto const& code = phylanx::execution_tree::compile(codestr, snippets, env);
    return code.run().arg_;
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
        "batch_dot([[1, 2], [4, 5], [3, 6]], [[-5,-6],[-1, 1],[-7,-8]])",
        "[[-17],[  1],[-69]]");
    test_batch_dot_operation(
        "batch_dot([[1, 2], [3, 4]], [[-5,-6],[-7,-8]], 1)", "[[-17],[-53]]");
    test_batch_dot_operation(
        "batch_dot([[1, 2], [3, 4]], [[-5,-6],[ 7, 8]], make_list(1, 1))",
        "[[-17],[53]]");

    test_batch_dot_operation(
        "batch_dot([[1, 2, 3], [4, 5, 6]], [[[-5,-6, 1, 0],[-7,-8, 1, 0]"
        ",[-1,-2, 13, 2]],[[ 5, 6,-1, 5],[7, 1, 0, 8],[1, 1,-2, 2]]])",
        "[[-22, -28,  42,   6], [ 61,  35, -16,  72]]");
    test_batch_dot_operation(
        "batch_dot([[1, 2, 3], [4, 5, 6]], [[[-5,-6],[-7,-8],[-1,-2]], "
        "[[5, 6],[7, 8],[1, 2]]], 1)",
        "[[-22, -28],[ 61,  76]]");
    test_batch_dot_operation(
        "batch_dot([[1, 2, 3], [4, 5, 6]], [[[-5,-6],[-7,-8],[-1,-2]], "
        "[[5, 6],[7, 8],[1, 2]]], make_list(1))",
        "[[-22, -28],[ 61,  76]]");
    test_batch_dot_operation(
        "batch_dot([[1, 2, 3], [4, 5, 6]], [[[-5, 0,-6],[0, -7, -8], "
        "[-1,-2, 0]],[[5, 6, -1],[2, 3, 8],[1, 4, 2]]], make_list(1,1))",
        "[[ -8, -20, -22],[ 36,  63,  48]]");
    test_batch_dot_operation(
        "batch_dot([[1, 2, 3], [4, 5, 6]], [[[-5, 0,-6],[0, -7, -8], "
        "[-1,-2, 0]],[[5, 6, -1],[2, 3, 8],[1, 4, 2]]], make_list(1,2))",
        "[[-23, -38,  -5],[ 44,  71,  36]]");


    test_batch_dot_operation(
        "batch_dot([[[-5, -6, 1, 0], [-7, -8, 1, 0], [-1, -2, 1, 0]],"
        "[[5, 6, 1, 0], [7, 1, 0, 8], [1, 1, 0, 2]]],[[1, 2, 3, 4], "
        "[5, 6, 7, 8]])",
        "[[-14, -20,  -2],[ 68, 105,  27]]");
    test_batch_dot_operation(
        "batch_dot([[[-5, -6, 1, 0], [-7, -8, 1, 0], [-1, -2, 1, 0]],"
        "[[5, 6, 1, 0], [7, 1, 0, 8], [1, 1, 0, 2]]],[[1, 2, 3, 4], "
        "[5, 6, 7, 8]], make_list(2,-1))",
        "[[-14, -20,  -2],[ 68, 105,  27]]");
    test_batch_dot_operation(
        "batch_dot([[[-5, -6, -4, -3],[13, 42, -7,-8],[-1,-2, 0, 0]], "
        "[[5, 6, 3, 5],[7, 6, 9, 8],[1, -1, -1, 2]]],[[1, 2, 1], [3, 4, 6]], "
        "make_list(1))",
        "[[ 20,  76, -18, -19], [ 49,  36,  39,  59]]");
    test_batch_dot_operation(
        "batch_dot([[[-5, -6, -4, -3],[13, 42, -7,-8],[-1,-2, 0, 0]], "
        "[[5, 6, 3, 5],[7, 6, 9, 8],[1, -1, -1, 2]]],[[1, 2, 1], [3, 4, 6]], "
        "make_list(-2,-1))",
        "[[ 20,  76, -18, -19], [ 49,  36,  39,  59]]");

    test_batch_dot_operation(
        "batch_dot([[[1, 2, 3], [2, 3, 4]], [[1, 5, 6], [7, 0, 8]]], "
        "[[[13, 0, 42, 1], [1, 0, 0, 33], [5, 0, 0, 1]], [[1, 0, 65, 0],"
        " [3, 1, 0, 1], [-2, 0, 1, 1]]])",
        "[[[ 30,   0,  42,  70],[ 49,   0,  84, 105]],[[  4,  5,  71,  11]  "
        ",[ -9,   0, 463,   8]]]");
    test_batch_dot_operation(
        "batch_dot([[[1, 2, 3], [2, 3, 4]]], "
        "[[[13, 0, 42, 1], [1, 0, 0, 33], [5, 0, 0, 1]]], make_list(2,-2))",
        "[[[ 30,   0,  42,  70],[ 49,   0,  84, 105]]]");
    test_batch_dot_operation(
        "batch_dot([[[1, 2, 33], [2, 13, 4], [-1,-1,-2]], [[1, 5, 6], [7, 0, 8]"
        ", [2, 3,-42]]], [[[0, 0, 10,-2], [1, 0, -20, 0], [0, 0, 1, 6]], "
        "[[5, 2, 0, 0], [13, 0, 12, 1], [0, 1, 2, 1]]], make_list(-2))",
        "[[[  2,  0, -31, -8], [ 13,  0, -241, -10],[ 4,  0,  248, -78]]"
        ",[[96, 4,  88, 9],[ 25, 13, 6, 3] ,[ 134, -30,  12, -34]]]");
    test_batch_dot_operation(
        "batch_dot([[[1, 2, 33], [2, 13, 4]]],  "
        "[[[0, 2, 10,-2], [42, 4, 1, 6]]], make_list(1, -2))",
        "[[[ 84,  10,  12,  10],[546,  56,  33,  74],[168,  82, 334, -42]]]");
    test_batch_dot_operation(
        "batch_dot([[[1, 2, 3], [1,-2, 4]], [[-1, 5, 6], [7, 0, 8]]],  "
        "[[[13, 33, 42], [5, 65, 105], [-5, -65, -105]], [[1, 2, 0], [0,-2,-3]"
        ", [-13, -42, -33]]], -1)",
        "[[[ 205,  450, -450],[ 115,  295, -295]],[[ 9,  -28, -395], "
        " [ 7,  -24, -355]]]");
    test_batch_dot_operation(
        "batch_dot([[[-5, -6, 1], [-7, 1, 0], [-1, 1, 0]] "
        ",[[42, 33, 13], [65, 105, 5],[107, 23, 4]]],  "
        "[[[1, 2, 4], [3, 5, 8]],[[-1, -2, -4], [-3, -5,-8]]] "
        ", make_list(1,-1))",
        "[[[ -23,  -58],[  0,  -5],[ 1, 3]],[[ -600, -1307],[ -335,  -808],"
        "[-39, -96]]]");

    return hpx::util::report_errors();
}
