// Copyright (c) 2019 Bita Hasheminezhad
// Copyright (c) 2019 Hartmut Kaiser
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/phylanx.hpp>

#include <hpx/hpx_main.hpp>
#include <hpx/testing.hpp>

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
void test_max_pool3d_operation(std::string const& code,
    std::string const& expected_str)
{
    HPX_TEST_EQ(compile_and_run(code), compile_and_run(expected_str));
}

///////////////////////////////////////////////////////////////////////////////
int main(int argc, char* argv[])
{
    test_max_pool3d_operation(
        "max_pool3d([[[1, 42, 3],[6, 5, -17],[1, 1, 23]], "
        "[[33, 2, 3],[-16, 5,-7],[13, 2, 0]]], make_list(1,2,2))",
        "[[[ 42.,  42.],[  6.,  23.]],[[ 33.,   5.],[ 13.,   5.]]]");
    test_max_pool3d_operation(
        R"(max_pool3d([[[1, 42, 3],[6, 5, -17],[1, 1, 23]], [[33, 2, 3],
          [-16, 5,-7],[13, 2, 0]]], make_list(1,2,2), "same"))",
        "[[[42., 42.,  3.],[ 6., 23., 23.],[ 1., 23., 23.]],[[33.,  5.,  3.],"
        "[13.,  5., 0.], [13.,  2.,  0.]]]");
    test_max_pool3d_operation(
        R"(max_pool3d([[[1, 42, 3],[6, 5, -17],[1, 1, 23]], [[33, 2, 3],
          [-16, 5,-7],[13, 2, 0]]], make_list(2,2,1), "valid"))",
        "[[[33., 42.,  3.],[13.,  5., 23.]]]");
    test_max_pool3d_operation(
        R"(max_pool3d([[[1, 42, 3],[6, 5, -17],[1, 1, 23]], [[33, 2, 3],
          [-16, 5,-7] ,[13, 2, 0]]], make_list(2,2,1), "valid",
          make_list(3,2,1)))",
        "[[[33., 42.,  3.]]]");
    test_max_pool3d_operation(
        R"(max_pool3d([[[1, 42, 3],[6, 5, -17],[1, 1, 23]], [[33, 2, 3],
          [-16, 5,-7] ,[13, 2, 0]]], make_list(2,3,2), "same",
          make_list(3,2,1)))",
        "[[[42., 42.,  3.], [13., 23., 23.]]]");

    return hpx::util::report_errors();
}
