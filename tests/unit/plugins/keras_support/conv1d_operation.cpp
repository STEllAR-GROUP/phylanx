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
void test_conv1d_operation(std::string const& code,
    std::string const& expected_str)
{
    HPX_TEST_EQ(compile_and_run(code), compile_and_run(expected_str));
}

///////////////////////////////////////////////////////////////////////////////
int main(int argc, char* argv[])
{
    test_conv1d_operation("conv1d([4, 33, 13, 42, 6, 5,-3], [ 1, -2, 3])",
        "[-23., 133., -53.,  45., -13.]");
    test_conv1d_operation(
        R"(conv1d([4, 33, 13, 42, 6, 5,-3], [ 1, -2, 3], "same"))",
        "[ 91., -23., 133., -53.,  45., -13.,  11.]");
    test_conv1d_operation(
        R"(conv1d([4, 33, 13, 42, 6, 5,-3], [ 1, -2, 3, -1], "same"))",
        "[ 78., -65., 127., -58.,  48., -13.,  11.]");
    test_conv1d_operation(
        R"(conv1d([4, 33, 13, 42, 6, 5,-3], [ 1, -2, 3], "causal"))",
        "[ 12.,  91., -23., 133., -53.,  45., -13.]");
    test_conv1d_operation(
        R"(conv1d([4, 33, 13, 42, 0, 1, -1, 2, 6, 5,-3], [ 1, -2, 3], "valid",
        2))",
        "[-23., -71.,  -5.,  13., -13.]");
    test_conv1d_operation(
        R"(conv1d([4, 33, 13, 42, 0, 1, -1, 2, 6, 5,-3], [ 1, -2, 3], "same",
        2))",
        "[ 91., 133.,  45.,   9.,   5.,  11.]");
    test_conv1d_operation(
        R"(conv1d([4, 33, 13, 42, 0, 1, -1, 2, 6, 5], [ 1, -2, 3], "same",
        2))",
        "[-23., -71.,  -5.,  13.,  -4.]");
    test_conv1d_operation(
        R"(conv1d([4, 33, 13, 42, 0, 1, -1, 2, 6, 5], [ 1, -2, 3], "causal",
        2))",
        "[ 12., -23., -71.,  -5.,  13.]");
    test_conv1d_operation(
        R"(conv1d([4, 33, 13, 42, 0, 1, -1, 2, 6, 5,-3], [ 1, -2, 3], "valid",
        3))",
        "[-23.,  45.,  13.]");
    test_conv1d_operation(
        R"(conv1d([4, 33, 13, 42, 0, 1, -1, 2, 6, 5, -3, 1], [ 1, -2, 3],
        "same", 3))",
        "[-23.,  45.,  13.,  14.]");
    test_conv1d_operation(
        R"(conv1d([4, 33, 13, 42, 0, 1, -1, 2, 6, 5, -3, 1], [ 1, -2, 3],
        "causal",  3))",
        "[ 12., 133.,  -5.,   5.]");
    test_conv1d_operation(
        R"(conv1d([4, 33, 13, 42, 0, 1, -1, 2, 6, 5, -3, 1], [ 1, -2, 3],
        "valid",  1, 2))",
        "[-22., -48.,  10.,  46.,  20.,  12., -22.,  -5.]");
    test_conv1d_operation(
        R"(conv1d([4, 33, 13, 42, 0, 1, -1, 2, 6, 5, -3, 1], [ 1, -2, 3],
        "same",  1, 2))",
        "[ 31.,  60., -22., -48.,  10.,  46.,  20.,  12., -22.,  -5.,  12., "
        "3.]");
    test_conv1d_operation(
        R"(conv1d([4, 33, 13, 42, 0, 1, -1, 2, 6, 5, -3, 1], [ 1, -2, 3],
        "causal",  1, 2))",
        "[ 12.,  99.,  31.,  60., -22., -48.,  10.,  46.,  20.,  12., -22.,"
        " -5.]");
    test_conv1d_operation(
        R"(conv1d([4, 33, 13, 42, 0, 1, -1, 2, 6, 5, -3, 1], [ 1, -2, 3],
        "valid",  1, 3))",
        "[-83.,  39.,  29.,  59., -13.,  -8.]");
    test_conv1d_operation(
        R"(conv1d([4, 33, 13, 42, 0, 1, -1, 2, 6, 5, -3, 1], [ 1, -2, 3],
        "same",  1, 3))",
        "[118., -66., -23., -83.,  39.,  29.,  59., -13., -8.,-11., 8., 4.]");
    test_conv1d_operation(
        R"(conv1d([4, 33, 13, 42, 0, 1, -1, 2, 6, 5, -3, 1], [ 1, -2, 3],
        "causal",  1, 3))",
        "[ 12.,  99.,  39., 118., -66., -23., -83., 39., 29., 59., -13.,-8.]");
    test_conv1d_operation(
        R"(conv1d([4, 33, 13, 42, 0, 1, -1, 2, 6, 5, -3, 1], [ 1, -2, 3],
        "causal",  1, 4))",
        "[ 12., 99., 39., 126., -8., -63., -29., -78.,  22., 46., 6., 41.]");

    return hpx::util::report_errors();
}
