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
    return code.run();
}

///////////////////////////////////////////////////////////////////////////////
void test_avg_pool2d_operation(std::string const& code,
    std::string const& expected_str)
{
    HPX_TEST_EQ(compile_and_run(code), compile_and_run(expected_str));
}

///////////////////////////////////////////////////////////////////////////////
int main(int argc, char* argv[])
{
    test_avg_pool2d_operation(
        "avg_pool2d([[1, 2, 3, 4],[5, 6, 7, 8],[9,10,11,12]], make_list(1,2))",
        "[[  1.5,   2.5,   3.5],[  5.5,   6.5,   7.5],[  9.5,  10.5,  11.5]]");
    test_avg_pool2d_operation(
        R"(avg_pool2d([[1, 2, 3, 4],[5, 6, 7, 8],[9,10,11,12]], make_list(2,2),
        "valid"))",
        "[[ 3.5,  4.5,  5.5], [ 7.5,  8.5,  9.5]]");
    test_avg_pool2d_operation(
        R"(avg_pool2d([[1, 2, 3, 4],[5, 6, 7, 8],[9,10,11,12]], make_list(2,2),
        "same"))",
        "[[  3.5,   4.5,   5.5,   6. ],[  7.5,   8.5,   9.5,  10. ],  "
        "[  9.5, 10.5,  11.5,  12. ]]");
    test_avg_pool2d_operation(
        R"(avg_pool2d([[1, 42, 3],[6, 5,-2],[13, 2, 0],[1, -1, 33]],
        make_list(2,2), "valid", make_list(1,1)))",
        "[[ 13.5 ,  12.  ],[  6.5 ,   1.25],[  3.75,   8.5 ]]");
    test_avg_pool2d_operation(
        R"(avg_pool2d([[1, 42, 3],[6, 5,-2],[13, 2, 0],[1, -1, 33]],
        make_list(2,2), "same", make_list(1,1)))",
        "[[ 13.5 ,  12. ,  0.5 ],[ 6.5 ,  1.25,  -1. ],[ 3.75,   8.5 , 16.5 ],"
        "[  0.  ,  16.  ,  33.  ]]");
    test_avg_pool2d_operation(
        R"(avg_pool2d([[1, 42, 3],[6, 5,-2],[13, 2, 0],[1, -1, 33]],
        make_list(2,2), "same", make_list(2,2)))",
        "[[ 13.5 ,   0.5 ],[  3.75,  16.5 ]]");

    return hpx::util::report_errors();
}
