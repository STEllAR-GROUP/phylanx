// Copyright (c) 2019 Bita Hasheminezhad
// Copyright (c) 2019 Hartmut Kaiser
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/phylanx.hpp>

#include <hpx/hpx_main.hpp>
#include <hpx/util/lightweight_test.hpp>

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
    return code.run();
}

///////////////////////////////////////////////////////////////////////////////
void test_max_pool2d_operation(std::string const& code,
    std::string const& expected_str)
{
    HPX_TEST_EQ(compile_and_run(code), compile_and_run(expected_str));
}

///////////////////////////////////////////////////////////////////////////////
int main(int argc, char* argv[])
{
    test_max_pool2d_operation(
        "max_pool2d([[1,2,3,0],[6,5,17,0],[13,2,0,0]], make_list(3,2))",
        "[[ 13.,  17.,  17.]]");
    test_max_pool2d_operation(
        R"(max_pool2d([[1., 2., 3., 4.],[5, 6., 7., 8.],[9.,10.,11.,12.]],
        make_list(2,2), "valid"))",
        "[[ 6.,  7.,  8.],[10., 11., 12.]]");
    test_max_pool2d_operation(
        R"(max_pool2d([[11,2,3,0],[6,5,17,0],[13,2,0,10]], make_list(3,2),
        "same"))",
        "[[ 11., 17., 17., 0.],[ 13., 17., 17., 10.],[ 13., 17., 17., 10.]]");
    test_max_pool2d_operation(
        R"(max_pool2d([[1, 42, 3, 0],[6, 5, -17, 0],[13, 2, 0, 33]],
         make_list(2,2), "valid", make_list(1,1)))",
        "[[ 42.,  42.,   3.],[ 13.,   5.,  33.]]");
    test_max_pool2d_operation(
        R"(max_pool2d([[1, 42, 3, 0],[6, 5, -17, 0],[13, 2, 0, 33]],
         make_list(2,2), "valid", make_list(2,2)))",
        "[[ 42., 3.]]");
    test_max_pool2d_operation(
        R"(max_pool2d([[1, 42, 3, 0],[6, 5, -17, 0],[13, 2, 0, 33]],
         make_list(2,2), "same", make_list(2,2)))",
        "[[ 42., 3.],[13., 33.]]");
    test_max_pool2d_operation(
        R"(max_pool2d([[11,2,3,0,-2],[6,5,-4,17,5],[-5,13,2,0,10],
         [33,-3,2,65,8]], make_list(3,2), "same", make_list(3,4)))",
        "[[ 11.,   5.], [ 33.,  10.]]");

    return hpx::util::report_errors();
}
