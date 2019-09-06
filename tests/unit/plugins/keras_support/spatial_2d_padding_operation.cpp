// Copyright (c) 2019 Bita Hasheminezhad
// Copyright (c) 2019 Hartmut Kaiser
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
void test_spatial_2d_padding_operation(std::string const& code,
    std::string const& expected_str)
{
    HPX_TEST_EQ(compile_and_run(code), compile_and_run(expected_str));
}

///////////////////////////////////////////////////////////////////////////////
int main(int argc, char* argv[])
{
    test_spatial_2d_padding_operation(
        "spatial_2d_padding([[[[1,2,3],[4,5,6]],[[7,8,9],[10,11,12]]],"
        "[[[-1,-2,-3],[-4,-5,-6]],[[-7,-8,-9],[-10,-11,-12]]]])",
        "[[[[  0.,   0.,   0.], [  0.,   0.,   0.],   [  0.,   0.,   0.],"
        "[ 0.,   0.,   0.]], [[  0.,   0.,   0.],   [  1.,   2.,   3.], "
        "[  4.,   5.,   6.], [  0.,   0.,   0.]], [[  0.,   0.,   0.], "
        "[  7.,   8.,   9.], [ 10.,  11.,  12.], [  0.,   0.,   0.]],"
        "[[  0.,   0.,   0.], [  0.,   0.,   0.], [  0.,   0.,   0.], "
        "[  0.,   0.,   0.]]], [[[  0.,   0.,   0.],  [  0.,   0.,   0.],  "
        "[  0.,   0.,   0.], [  0.,   0.,   0.]], [[  0.,   0.,   0.], "
        "[ -1.,  -2.,  -3.], [ -4.,  -5.,  -6.],   [  0.,   0.,   0.]], "
        "[[  0.,   0.,   0.],[ -7.,  -8.,  -9.], [-10., -11., -12.], "
        "[  0.,   0.,   0.]], [[  0.,   0.,   0.], [  0.,   0.,   0.], "
        "[  0.,   0.,   0.], [  0.,   0.,   0.]]]]");
    test_spatial_2d_padding_operation(
        "spatial_2d_padding([[[[1,2,3],[4,5,6]],[[7,8,9],[10,11,12]]],"
        "[[[-1,-2,-3],[-4,-5,-6]],[[-7,-8,-9],[-10,-11,-12]]]],"
        "list(list(1,0),list(2,3)))",
        "[[[[  0.,   0.,   0.],  [  0.,   0.,   0.],  [  0.,   0.,   0.], "
        "  [  0.,   0.,   0.],  [  0.,   0.,   0.],  [  0.,   0.,   0.],  "
        "  [  0.,   0.,   0.]], [[  0.,   0.,   0.],  [  0.,   0.,   0.], "
        "  [  1.,   2.,   3.],  [  4.,   5.,   6.],  [  0.,   0.,   0.],  "
        "  [  0.,   0.,   0.],  [  0.,   0.,   0.]], [[  0.,   0.,   0.], "
        "  [  0.,   0.,   0.],  [  7.,   8.,   9.],  [ 10.,  11.,  12.],  "
        "  [  0.,   0.,   0.],  [  0.,   0.,   0.],  [  0.,   0.,   0.]]],"
        "[[[  0.,   0.,   0.],  [  0.,   0.,   0.],  [  0.,   0.,   0.],  "
        "  [  0.,   0.,   0.],  [  0.,   0.,   0.],  [  0.,   0.,   0.],  "
        "  [  0.,   0.,   0.]], [[  0.,   0.,   0.],  [  0.,   0.,   0.], "
        "  [ -1.,  -2.,  -3.],  [ -4.,  -5.,  -6.],  [  0.,   0.,   0.],  "
        "  [  0.,   0.,   0.],  [  0.,   0.,   0.]], [[  0.,   0.,   0.], "
        "  [  0.,   0.,   0.],  [ -7.,  -8.,  -9.],  [-10., -11., -12.],  "
        "  [  0.,   0.,   0.],  [  0.,   0.,   0.],  [  0.,   0.,   0.]]]]");

    return hpx::util::report_errors();
}
