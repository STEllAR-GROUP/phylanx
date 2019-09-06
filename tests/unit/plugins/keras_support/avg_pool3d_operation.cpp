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
void test_avg_pool3d_operation(std::string const& code,
    std::string const& expected_str)
{
    HPX_TEST_EQ(compile_and_run(code), compile_and_run(expected_str));
}

///////////////////////////////////////////////////////////////////////////////
int main(int argc, char* argv[])
{
    test_avg_pool3d_operation(
        "avg_pool3d([[[1, 2, 3],[6, 5, 17],[13, 2, 0]],[[-33, 2, 3],"
        "[16, 5, -7],[13, 2, 0]]], make_list(1,2,2))",
        "[[[ 3.5 ,  6.75],[ 6.5 ,  6.  ]],[[-2.5 ,  0.75],[ 9.  ,  0.  ]]]");
    test_avg_pool3d_operation(
        R"(avg_pool3d([[[1, 2, 3],[6, 5, 17],[13, 2, 0]],[[-33, 2, 3],
          [16, 5, -7],[13, 2, 0]]], make_list(2,2,2),"same"))",
        "[[[ 0.5 ,  3.75,  4.  ],[ 7.75,  3.  ,  2.5 ],[ 7.5 ,  1.  ,  0.  ]],"
        "[[-2.5 ,  0.75, -2.  ],[ 9.  ,  0.  , -3.5 ],[ 7.5 ,  1.  ,  0.  ]]]");
    test_avg_pool3d_operation(
        R"(avg_pool3d([[[1, 2, 3],[6, 5, 17],[13, 2, 0]],[[-33, 2, 3],
          [16, 5, -7],[13, 2, 0]]], make_list(2,2,2),"same", make_list(2,2,2)))",
        "[[[ 0.5,  4. ],[ 7.5,  0. ]]]");
    test_avg_pool3d_operation(
        R"(avg_pool3d([[[1, 2, 3],[6, 5, 17],[13, 2, 0]],[[-33, 2, 3],
          [16, 5, -7],[13, 4, 0]]], make_list(2,2,2),"valid", make_list(2,1,2)))",
        "[[[ 0.5],[ 8. ]]]");
    test_avg_pool3d_operation(
        R"(avg_pool3d([[[1, 2, 3],[6, 5, 17],[13, 2, 0]],[[-33, 2, 3],
          [16, 5, -7],[13, 4, 0]]], make_list(2,3,2),"valid", make_list(1,1,1)))",
        "[[[ 3., 3.]]]");

    return hpx::util::report_errors();
}
