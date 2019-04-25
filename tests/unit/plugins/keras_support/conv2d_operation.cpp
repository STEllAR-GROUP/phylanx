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
void test_conv2d_operation(std::string const& code,
    std::string const& expected_str)
{
    HPX_TEST_EQ(compile_and_run(code), compile_and_run(expected_str));
}

///////////////////////////////////////////////////////////////////////////////
int main(int argc, char* argv[])
{
    test_conv2d_operation(
        "conv2d([[42, 3, 1, 0, 2],[2, 1, 0, 1, 33],[1, 0, 13, 1, -1], "
        "[0, 1, 0, 2, -2]], [[ 1, 2],[-1,-2],[-3,-4]])",
        "[[ 41., -48., -44., -62.], [ -1., -28., -21.,  70.]]");
    test_conv2d_operation(
        R"(conv2d([[42, 3, 1, 0, 2],[2, 1, 0, 1, 33],[1, 0, 13, 1, -1],
        [0, 1, 0, 2, -2]], [[ 1, 2],[-1,-2],[-3,-4]], "same"))",
        "[[ -58., -8., -5., -139., -101.],[  41., -48., -44., -62., -28.], "
        "[ -1.,  -28.,  -21.,  70.,  40.],[  -1.,  25.,  11.,   1.,   1.]]");
    test_conv2d_operation(
        R"(conv2d([[42, 3, 1, 0, 2],[2, 1, 0, 1, 33],[1, 0, 13, 1, -1],
        [0, 1, 0, 2, -2]], [[ 1, 2],[-1,-2],[-3,-4]], "valid", list(1,2)))",
        "[[ 41., -44.],[ -1., -21.]]");
    test_conv2d_operation(
        R"(conv2d([[42, 3, 1, 0, 2],[2, 1, 0, 1, 33],[1, 0, 13, 1, -1],
        [0, 1, 0, 2, -2]], [[ 1, 2],[-1,-2],[-3,-4]], "same", list(1,2)))",
        "[[ -58.,   -5., -101.],[  41.,  -44.,  -28.],[  -1.,  -21.,   40.],"
        "[  -1.,   11.,    1.]]");
    test_conv2d_operation(
        R"(conv2d([[42, 3, 1, 0, 2],[2, 1, 0, 1, 33],[1, 0, 13, 1, -1],
        [0, 1, 0, 2, -2]], [[ 1, 2],[-1,-2],[-3,-4]], "same", list(2,2)))",
        "[[ 41., -44., -28.], [ -1.,  11.,   1.]]");
    test_conv2d_operation(
        R"(conv2d([[42, 3, 1, 0, 2],[2, 1, 0, 1, 33],[1, 0, 13, 1, -1],
        [0, 1, 0, 2, -2]], [[ 1, 2],[-1,-2],[-3,-4]], "same", list(2,1)))",
        "[[ 41., -48., -44., -62., -28.],[ -1.,  25.,  11.,   1.,   1.]]");
    test_conv2d_operation(
        R"(conv2d([[42, 3, 1, 0, 2],[2, 1, 0, 1, 33],[1, 0, 13, 1, -1],
        [0, 1, 0, 2, -2]], [[ 1, 2],[-1,-2],[-3,-4]], "valid", list(1,1),
        list(1,2)))",
        "[[-13.,  -4., -96.], [-25., -10.,  63.]]");
    test_conv2d_operation(
        R"(conv2d([[42, 3, 1, 0, 2],[2, 1, 0, 1, 33],[1, 0, 13, 1, -1],
        [0, 1, 0, 2, -2]], [[ 1, 2],[-1,-2],[-3,-4]], "same", list(1,1),
        list(1,2)))",
        "[[ -10., -50., -10., -137.,  -3.], [   4., -13.,  -4.,  -96.,  -4.],"
        "[ -2.,  -25., -10.,  63.,  -6.], [  -2.,   27., -3.,   15.,  -1.]]");
    test_conv2d_operation(
        R"(conv2d([[42, 3, 1, 0, 2],[2, 1, 0, 1, 33],[1, 0, 13, 1, -1],
        [0, 1, 0, 2, -2]], [[ 1, 2],[-1,-2],[-3,-4]], "same", list(1,1),
        list(2,2)))",
        "[[ -6., -99.,  -7., -40.,  -3.], [   -6.,  -2., -14., -58.,  -7.],"
        "[ 6.,  17.,   1.,  -6.,  -1.], [  0.,   2.,  -2.,  70.,  -1.]]");
    test_conv2d_operation(
        R"(conv2d([[42, 3, 1, 0, 2],[2, 1, 0, 1, 33],[1, 0, 13, 1, -1],
        [0, 1, 0, 2, -2]], [[ 1, 2],[-1,-2],[-3,-4]], "same", list(1,1),
        list(2,1)))",
        "[[-51., -57., -44.,  -3.,   1.], [ -8.,  -4., -10., -65., -27.],"
        "[ 47., -21., -14.,   5.,   3.], [  2.,   0.,  -2.,  69.,  35.]]");
    test_conv2d_operation(
        R"(conv2d([[42, 3, 1, 0, 2],[2, 1, 0, 1, 33],[1, 0, 13, 1, -1],
        [0, 1, 0, 2, -2]], [[ 1, 2],[-1,-2],[-3,-4]], "same", list(1,1),
        list(5,6)))",
        "[[  0.,  -4.,   0., -42.,  -3.], [ -2., -66.,   0.,  -2.,  -1.],"
        "[ -2.,   2.,   0.,  -1.,   0.], [ -4.,   4.,   0.,   0.,  -1.]]");
    test_conv2d_operation(
        R"(conv2d([[ 1, 2],[-1,-2],[-3,-4]], [[42, 3, 1, 0, 2],[2, 1, 0, 1, 33]
        ,[1, 0, 13, 1, -1], [0, 1, 0, 2, -2]],  "same", list(1,1),
        list(2,3)))",
        "[[-13., -26.], [-39., -52.], [  0.,   0.]]");
    test_conv2d_operation(
        R"(conv2d([[ 1, 2 ,3],[-2,-3,-4]], [[42, 3, 1, 0, 2],[2, 1, 0, 1, 33]
        ,[1, 0, 13, 1, -1], [0, 1, 0, 2, -2]],  "same", list(1,1),
        list(2,3)))",
        "[[-26., -39., -52.], [  0.,   0.,   0.]]");

    return hpx::util::report_errors();
}
