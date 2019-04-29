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
void test_conv2d_transpose_operation(std::string const& code,
    std::string const& expected_str)
{
    HPX_TEST_EQ(compile_and_run(code), compile_and_run(expected_str));
}

///////////////////////////////////////////////////////////////////////////////
int main(int argc, char* argv[])
{
    test_conv2d_transpose_operation(
        R"(conv2d_transpose([[ 42, 13],[ 33, -5]], [[1,2,3],[4,5,6],[7,8,9]],
        list(4,4)))",
        "[[  42.,   97.,  152.,   39.], [ 201.,  323.,  406.,   63.],"
        "[ 426.,  572.,  655.,   87.], [ 231.,  229.,  257.,  -45.]]");
    test_conv2d_transpose_operation(
        R"(conv2d_transpose([[ 42, 13],[ 33, -5]], [[1,2,3],[4,5,6],[7,8,9]],
        list(2,2), "same"))",
        "[[ 323.,  406.], [ 572.,  655.]]");
    test_conv2d_transpose_operation(
        R"(conv2d_transpose([[1,2,3],[4,5,6]],
        [[1, 3, 2, 4],[10,20,30,40],[-1,-3, -4, -2]], list(4,6), "valid"))",
        "[[ 1.,  5.,  11.,  17.,  14.,  12.],"
        "[ 14., 57.,  129.,  204.,  202.,  144.],"
        "[  39.,  125.,  267.,  411.,  364.,  234.],"
        "[  -4.,  -17.,  -37.,  -46.,  -34.,  -12.]]");
    test_conv2d_transpose_operation(
        R"(conv2d_transpose([[1,2,3],[4,5,6]],
        [[1, 3, 2, 4],[10,20,30,40],[-1,-3, -4, -2]], list(2,3), "same"))",
        "[[  57.,  129.,  204.],  [ 125.,  267.,  411.]]");
    test_conv2d_transpose_operation(
        R"(conv2d_transpose([[1,2,3,4],[5,6,7,8]],
        [[42,-33],[13,22]], list(4,6), "valid", list(1,1),list(2,2)))",
        "[[  42.,   84.,   93.,  102.,  -99., -132.],"
        "[ 210.,  252.,  129.,  138., -231., -264.],"
        "[  13.,   26.,   61.,   96.,   66.,   88.],"
        "[  65.,   78.,  201.,  236.,  154.,  176.]]");
    test_conv2d_transpose_operation(
        R"(conv2d_transpose([[1,2,3,4],[5,6,7,8]],
        [[42,-33],[13,22]], list(2,4), "valid", list(1,1),list(2,2)))",
        "[[ 252.,  129.,  138., -231.], [  26.,   61.,   96.,   66.]]");

    return hpx::util::report_errors();
}
