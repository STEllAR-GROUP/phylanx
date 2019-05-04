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
void test_pool_operation(std::string const& code,
    std::string const& expected_str)
{
    HPX_TEST_EQ(compile_and_run(code), compile_and_run(expected_str));
}

///////////////////////////////////////////////////////////////////////////////
int main(int argc, char* argv[])
{
    // 2d, max
    test_pool_operation(
        "max_pool([[1, 2, 3, 4],[5, 6, 7, 8],[9,10,11,12]], make_list(1,2))",
        "[[ 2,  3,  4],[ 6,  7,  8],[10, 11, 12]]");
    test_pool_operation(
        R"(max_pool([[1, 2, 3, 4],[5, 6, 7, 8],[9,10,11,12]], make_list(2,2),
        "valid"))",
        "[[ 6,  7,  8],[10, 11, 12]]");
    test_pool_operation(
        R"(max_pool([[1, 2, 3, 4],[5, 6, 7, 8],[9,10,11,12]], make_list(1,2),
        "same"))",
        "[[ 2,  3,  4,  4],[ 6,  7,  8,  8],[10, 11, 12, 12]]");
    test_pool_operation(
        R"(max_pool([[1, 42, 3, 0],[6, 5, -17, 0],[13, 2, 0, 33]],
         make_list(2,2), "valid", make_list(1,1)))",
        "[[ 42,  42,   3],[ 13,   5,  33]]");
    test_pool_operation(
        R"(max_pool([[1, 42, 3, 0],[6, 5, -17, 0],[13, 2, 0, 33]],
         make_list(2,2), "valid", make_list(2,2)))",
        "[[ 42, 3]]");
    test_pool_operation(
        R"(max_pool([[1, 42, 3, 0],[6, 5, -17, 0],[13, 2, 0, 33]],
         make_list(2,2), "same", make_list(2,2)))",
        "[[ 42, 3],[13, 33]]");
    test_pool_operation(
        R"(max_pool([[1., 42., 3., 0.],[6., 5., -17., 0.],[13., 2., 0., 33.]],
         make_list(2,2), "same", make_list(2,1)))",
        "[[ 42.,  42.,   3.,   0.],[ 13.,   2.,  33.,  33.]]");

    // 2d, average
    test_pool_operation(
        "avg_pool([[1, 2, 3, 4],[5, 6, 7, 8],[9,10,11,12]], make_list(1,2))",
        "[[  1.5,   2.5,   3.5],[  5.5,   6.5,   7.5],[  9.5,  10.5,  11.5]]");
    test_pool_operation(
        R"(avg_pool([[1, 2, 3, 4],[5, 6, 7, 8],[9,10,11,12]], make_list(2,2),
        "valid"))",
        "[[ 3.5,  4.5,  5.5], [ 7.5,  8.5,  9.5]]");
    test_pool_operation(
        R"(avg_pool([[1, 2, 3, 4],[5, 6, 7, 8],[9,10,11,12]], make_list(2,2),
        "same"))",
        "[[  3.5,   4.5,   5.5,   6. ],[  7.5,   8.5,   9.5,  10. ],  "
        "[  9.5, 10.5,  11.5,  12. ]]");
    test_pool_operation(
        R"(avg_pool([[1, 42, 3],[6, 5,-2],[13, 2, 0],[1, -1, 33]],
        make_list(2,2), "valid", make_list(1,1)))",
        "[[ 13.5 ,  12.  ],[  6.5 ,   1.25],[  3.75,   8.5 ]]");
    test_pool_operation(
        R"(avg_pool([[1, 42, 3],[6, 5,-2],[13, 2, 0],[1, -1, 33]],
        make_list(2,2), "same", make_list(1,1)))",
        "[[ 13.5 ,  12. ,  0.5 ],[ 6.5 ,  1.25,  -1. ],[ 3.75,   8.5 , 16.5 ],"
        "[  0.  ,  16.  ,  33.  ]]");
    test_pool_operation(
        R"(avg_pool([[1, 42, 3],[6, 5,-2],[13, 2, 0],[1, -1, 33]],
        make_list(2,2), "same", make_list(2,2)))",
        "[[ 13.5 ,   0.5 ],[  3.75,  16.5 ]]");

#if defined(PHYLANX_HAVE_BLAZE_TENSOR)
    // 3d, max
    test_pool_operation(
        "max_pool([[[1, 42, 3],[6, 5, -17],[1, 1, 23]], "
        "[[33, 2, 3],[-16, 5,-7],[13, 2, 0]]], make_list(1,2,2))",
        "[[[ 42,  42],[  6,  23]],[[ 33,   5],[ 13,   5]]]");
    test_pool_operation(
        R"(max_pool([[[1, 42, 3],[6, 5, -17],[1, 1, 23]], [[33, 2, 3],
          [-16, 5,-7],[13, 2, 0]]], make_list(1,2,2), "same"))",
        "[[[42, 42,  3],[ 6, 23, 23],[ 1, 23, 23]],[[33,  5,  3],[13,  5, 0],"
        "[13,  2,  0]]]");
    test_pool_operation(
        R"(max_pool([[[1, 42, 3],[6, 5, -17],[1, 1, 23]], [[33, 2, 3],
          [-16, 5,-7],[13, 2, 0]]], make_list(2,2,1), "valid"))",
        "[[[33, 42,  3],[13,  5, 23]]]");
    test_pool_operation(
        R"(max_pool([[[1, 42, 3],[6, 5, -17],[1, 1, 23]], [[33, 2, 3],
          [-16, 5,-7] ,[13, 2, 0]]], make_list(2,2,1), "valid",
          make_list(3,2,1)))",
        "[[[33, 42,  3]]]");
    test_pool_operation(
        R"(max_pool([[[1, 42, 3],[6, 5, -17],[1, 1, 23]], [[33, 2, 3],
          [-16, 5,-7] ,[13, 2, 0]]], make_list(2,2,1), "same",
          make_list(3,2,1)))",
        "[[[33, 42,  3], [13,  2, 23]]]");

    // 3d, avg
    test_pool_operation(
        "avg_pool([[[1, 2, 3],[6, 5, 17],[13, 2, 0]],[[-33, 2, 3],"
        "[16, 5, -7],[13, 2, 0]]], make_list(1,2,2))",
        "[[[ 3.5 ,  6.75],[ 6.5 ,  6.  ]],[[-2.5 ,  0.75],[ 9.  ,  0.  ]]]");
    test_pool_operation(
        R"(avg_pool([[[1, 2, 3],[6, 5, 17],[13, 2, 0]],[[-33, 2, 3],
          [16, 5, -7],[13, 2, 0]]], make_list(2,2,2),"same"))",
        "[[[ 0.5 ,  3.75,  4.  ],[ 7.75,  3.  ,  2.5 ],[ 7.5 ,  1.  ,  0.  ]],"
        "[[-2.5 ,  0.75, -2.  ],[ 9.  ,  0.  , -3.5 ],[ 7.5 ,  1.  ,  0.  ]]]");
    test_pool_operation(
        R"(avg_pool([[[1, 2, 3],[6, 5, 17],[13, 2, 0]],[[-33, 2, 3],
          [16, 5, -7],[13, 2, 0]]], make_list(2,2,2),"same", make_list(2,2,2)))",
        "[[[ 0.5,  4. ],[ 7.5,  0. ]]]");
    test_pool_operation(
        R"(avg_pool([[[1, 2, 3],[6, 5, 17],[13, 2, 0]],[[-33, 2, 3],
          [16, 5, -7],[13, 4, 0]]], make_list(2,2,2),"valid", make_list(2,1,2)))",
        "[[[ 0.5],[ 8. ]]]");
    test_pool_operation(
        R"(avg_pool([[[1, 2, 3],[6, 5, 17],[13, 2, 0]],[[-33, 2, 3],
          [16, 5, -7],[13, 4, 0]]], make_list(2,3,2),"valid", make_list(1,1,1)))",
        "[[[ 3., 3.]]]");

#endif

    return hpx::util::report_errors();
}
