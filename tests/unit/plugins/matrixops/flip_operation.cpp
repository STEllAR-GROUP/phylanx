// Copyright (c) 2018-2019 Bita Hasheminezhad
// Copyright (c) 2018-2019 Hartmut Kaiser
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/phylanx.hpp>

#include <hpx/hpx_main.hpp>
#include <hpx/testing.hpp>

#include <cstdint>
#include <string>
#include <utility>

///////////////////////////////////////////////////////////////////////////////
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
void test_flip_operation(std::string const& code,
    std::string const& expected_str)
{
    HPX_TEST_EQ(compile_and_run(code), compile_and_run(expected_str));
}

///////////////////////////////////////////////////////////////////////////////
int main(int argc, char* argv[])
{
    test_flip_operation("flip(42)", "42");
    test_flip_operation("flip(42,nil)", "42");

    test_flip_operation("flip([13, 42, 33])", "[33, 42, 13]");
    test_flip_operation("flip([13, 42, 33],nil)", "[33, 42, 13]");
    test_flip_operation("flip([13., 42., 33.], 0)", "[33., 42., 13.]");
    test_flip_operation("flip([13., 42., 33.]+0, 0)", "[33., 42., 13.]");
    test_flip_operation("flip([13., 42., 33.], -1)", "[33., 42., 13.]");
    test_flip_operation("flipud([13, 42, 33])", "[33, 42, 13]");

    test_flip_operation("flip([[13, 42, 33],[101, 12, 65]])",
        "[[ 65,  12, 101], [ 33,  42,  13]]");
    test_flip_operation("flip([[13, 42, 33],[101, 12, 65]],nil)",
        "[[ 65,  12, 101], [ 33,  42,  13]]");
    test_flip_operation("flip([[13., 42., 33.],[101., 12., 65.]],  0)",
        "[[101., 12., 65.], [13., 42., 33.]]");
    test_flip_operation("flip([[13., 42.],[22., 43.],[54., 41.]], -2)",
        "[[54., 41.], [22., 43.], [13., 42.]]");
    test_flip_operation("flip([[13., 42., 33.],[101., 12., 65.]],  1)",
        "[[33., 42., 13.], [65., 12., 101.]]");
    test_flip_operation("flip([[13., 42.],[22., 43.],[54., 41.]], -1)",
        "[[42., 13.], [43., 22.], [41., 54.]]");
    test_flip_operation("flip([[13, 42, 33],[101, 12, 65]], list(0,1))",
        "[[ 65,  12, 101], [ 33,  42,  13]]");
    test_flip_operation("flip([[13, 42, 33],[101, 12, 65]]+0, list(1,0))",
        "[[ 65,  12, 101], [ 33,  42,  13]]");
    test_flip_operation("flipud([[13, 42, 33],[101, 12, 65]])",
        "[[101,  12,  65],[ 13,  42,  33]]");
    test_flip_operation("fliplr([[13, 42, 33],[101, 12, 65]])",
        "[[ 33,  42,  13],[ 65,  12, 101]]");

    test_flip_operation(
        "flip([[[13, 42, 33],[101, 12, 65]],[[3, 4, 31],[10, 2, 5]]])",
        "[[[  5,  2,  10],[ 31,  4,   3]],[[ 65, 12, 101],[ 33,  42,  13]]]");
    test_flip_operation(
        "flip([[[13, 42, 33],[101, 12, 65]],[[3, 4, 31],[10, 2, 5]]], nil)",
        "[[[  5,  2,  10],[ 31,  4,   3]],[[ 65, 12, 101],[ 33,  42,  13]]]");
    test_flip_operation("flip([[[13., 42., 33.],[101., 12., 65.]]]+0, -3)",
        "[[[ 13.,  42.,  33.], [101.,  12.,  65.]]]");
    test_flip_operation("flip([[[13., 42., 33.],[101., 12., 65.]]], 0)",
        "[[[ 13.,  42.,  33.], [101.,  12.,  65.]]]");
    test_flip_operation("flip([[[13., 42., 33.],[101., 12., 65.]]]+0, 1)",
        "[[[101.,  12.,  65.], [ 13.,  42.,  33.]]]");
    test_flip_operation("flip([[[13., 42., 33.],[101., 12., 65.]]], -2)",
        "[[[101.,  12.,  65.], [ 13.,  42.,  33.]]]");
    test_flip_operation("flip([[[13., 42., 33.],[101., 12., 65.]]]+0, 2)",
        "[[[ 33.,  42.,  13.], [ 65.,  12., 101.]]]");
    test_flip_operation("flip([[[13., 42., 33.],[101., 12., 65.]]], -1)",
        "[[[ 33.,  42.,  13.], [ 65.,  12., 101.]]]");
    test_flip_operation(
        "flip([[[13., 42., 33.],[101., 12., 65.]]]+0, list(0,1))",
        "[[[101.,  12.,  65.],[ 13.,  42.,  33.]]]");
    test_flip_operation(
        "flip([[[13., 42., 33.],[101., 12., 65.]]], list(1,-3))",
        "[[[101.,  12.,  65.],[ 13.,  42.,  33.]]]");
    test_flip_operation(
        "flip([[[13., 42., 33.],[101., 12., 65.]]]+0, list(-3,-1))",
        "[[[ 33.,  42.,  13.], [ 65.,  12., 101.]]]");
    test_flip_operation(
        "flip([[[13., 42., 33.],[101., 12., 65.]]], list(2,0))",
        "[[[ 33.,  42.,  13.], [ 65.,  12., 101.]]]");
    test_flip_operation(
        "flip([[[13., 42., 33.],[101., 12., 65.]]], list(2,-2))",
        "[[[ 65.,  12., 101.], [ 33.,  42.,  13.]]]");
    test_flip_operation(
        "flip([[[13., 42., 33.],[101., 12., 65.]]], list(-1,1))",
        "[[[ 65.,  12., 101.], [ 33.,  42.,  13.]]]");
    test_flip_operation(
        "flip([[[13., 42., 33.],[101., 12., 65.]]], list(-3,-2,-1))",
        "[[[ 65.,  12., 101.], [ 33.,  42.,  13.]]]");
    test_flip_operation(
        "flipud([[[13, 42, 33],[101, 12, 65]],[[3, 4, 31],[10, 2, 5]]])",
        "[[[ 3,  4,  31],[ 10,  2,  5]],[[ 13,  42,  33],[101,  12,  65]]]");
    test_flip_operation(
        "fliplr([[[13, 42, 33],[101, 12, 65]],[[3, 4, 31],[10, 2, 5]]])",
        "[[[101,  12,  65],[ 13,  42,  33]],[[ 10,  2,  5],[ 3,  4,  31]]]");

    return hpx::util::report_errors();
}
