// Copyright (c) 2018 Parsa Amini
// Copyright (c) 2018 Hartmut Kaiser
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/phylanx.hpp>

#include <hpx/hpx_main.hpp>
#include <hpx/testing.hpp>

#include <cstdint>
#include <string>

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
void test_where_operation(std::string const& code,
    std::string const& expected_str)
{
    HPX_TEST_EQ(compile_and_run(code), compile_and_run(expected_str));
}

///////////////////////////////////////////////////////////////////////////////
void test_one_argument_where()
{
    // test 0d data (scalars)
    test_where_operation("where(false)",
        R"(list(hstack(list(), __arg(dtype, "int"))))");
    test_where_operation("where(0)",
        R"(list(hstack(list(), __arg(dtype, "int"))))");
    test_where_operation("where(0.)",
        R"(list(hstack(list(), __arg(dtype, "int"))))");
    test_where_operation("where(true)", "list([0])");
    test_where_operation("where(1)", "list([0])");
    test_where_operation("where(1.)", "list([0])");

    // test 1d data (vectors)
    test_where_operation("where([])",
        R"(list(hstack(list(), __arg(dtype, "int"))))");

    test_where_operation("where(hstack(list(false)))",
        R"(list(hstack(list(), __arg(dtype, "int"))))");
    test_where_operation("where([0])",
        R"(list(hstack(list(), __arg(dtype, "int"))))");
    test_where_operation("where([0.])",
        R"(list(hstack(list(), __arg(dtype, "int"))))");

    test_where_operation("where(hstack(list(true)))", "list([0])");
    test_where_operation("where([1])", "list([0])");
    test_where_operation("where([1.])", "list([0])");

    test_where_operation(
        "where(hstack(list(false, true, false, true)))", "list([1, 3])");
    test_where_operation("where([0, 1, 2, 0])", "list([1, 2])");
    test_where_operation(
        "where([1., 0., 42., 43.])", "list([0, 2, 3])");

    // test 2d data (matrix)
    test_where_operation(
        "where([[]])",
        R"(list(hstack(list(), __arg(dtype, "int")),
                hstack(list(), __arg(dtype, "int"))))");
    test_where_operation(
        "where([[0, 0], [0, 0]])",
        R"(list(hstack(list(), __arg(dtype, "int")),
                hstack(list(), __arg(dtype, "int"))))");

    test_where_operation(
        "where([[0, 1], [2, 0]])",
        "list([0, 1], [1, 0])");
    test_where_operation(
        "where([[0., 1.], [2., 0.]])",
        "list([0, 1], [1, 0])");
}

void test_three_argument_where_0d()
{
    ///////////////////////////////////////////////////////////////////////////
    // test scalar result
    test_where_operation("where(false, 1, 2)", "2");
    test_where_operation("where(true, 1, 2)", "1");

    ///////////////////////////////////////////////////////////////////////////
    // test vector result
    test_where_operation("where(false, [1], [2])", "[2]");
    test_where_operation("where(true, [1], [2])", "[1]");

    test_where_operation(
        "where(false, [1, 2], [2, 3])", "[2, 3]");
    test_where_operation(
        "where(true, [1, 2], [2, 3])", "[1, 2]");

    // test vector result with broadcasting a scalar
    test_where_operation("where(false, [1], 2)", "[2]");
    test_where_operation("where(true, [1], 2)", "[1]");
    test_where_operation("where(false, 1, [2])", "[2]");
    test_where_operation("where(true, 1, [2])", "[1]");

    test_where_operation("where(false, [1, 2], 2)", "[2, 2]");
    test_where_operation("where(true, [1, 2], 2)", "[1, 2]");
    test_where_operation("where(false, 1, [2, 3])", "[2, 3]");
    test_where_operation("where(true, 1, [2, 3])", "[1, 1]");

    // test vector result with broadcasting a single element vector
    test_where_operation(
        "where(false, [1, 2], [2])", "[2, 2]");
    test_where_operation(
        "where(true, [1, 2], [2])", "[1, 2]");
    test_where_operation(
        "where(false, [2], [2, 3])", "[2, 3]");
    test_where_operation(
        "where(true, [2], [2, 3])", "[2, 2]");

    ///////////////////////////////////////////////////////////////////////////
    // test matrix result
    test_where_operation(
        R"(where(false,
            [[1], [2]], [[2], [1]]
          ))", "[[2], [1]]");
    test_where_operation(
        R"(where(true,
            [[1], [2]], [[2], [1]]
          ))", "[[1], [2]]");

    // test matrix result with broadcasting a scalar
    test_where_operation(
        R"(where(false,
            [[1], [2]], 3
          ))", "[[3], [3]]");
    test_where_operation(
        R"(where(true,
            [[1], [2]], 3
          ))", "[[1], [2]]");
    test_where_operation(
        R"(where(false,
            3, [[2], [1]]
          ))", "[[2], [1]]");
    test_where_operation(
        R"(where(true,
            3, [[2], [1]]
          ))", "[[3], [3]]");

    test_where_operation(
        R"(where(false,
            [[1, 2], [2, 3]], 3
          ))", "[[3, 3], [3, 3]]");
    test_where_operation(
        R"(where(true,
            [[1, 2], [2, 3]], 3
          ))", "[[1, 2], [2, 3]]");
    test_where_operation(
        R"(where(false,
            3, [[1, 2], [2, 3]]
          ))", "[[1, 2], [2, 3]]");
    test_where_operation(
        R"(where(true,
            3, [[1, 2], [2, 3]]
          ))", "[[3, 3], [3, 3]]");

    // test matrix result with broadcasting a single element vector
    test_where_operation(
        R"(where(false,
            [[1], [2]], [3]
          ))", "[[3], [3]]");
    test_where_operation(
        R"(where(true,
            [[1], [2]], [3]
          ))", "[[1], [2]]");
    test_where_operation(
        R"(where(false,
            [3], [[2], [1]]
          ))", "[[2], [1]]");
    test_where_operation(
        R"(where(true,
            [3], [[2], [1]]
          ))", "[[3], [3]]");

    test_where_operation(
        R"(where(false,
            [[1, 2], [2, 3]], [3]
          ))", "[[3, 3], [3, 3]]");
    test_where_operation(
        R"(where(true,
            [[1, 2], [2, 3]], [3]
          ))", "[[1, 2], [2, 3]]");
    test_where_operation(
        R"(where(false,
            [3], [[1, 2], [2, 3]]
          ))", "[[1, 2], [2, 3]]");
    test_where_operation(
        R"(where(true,
            [3], [[1, 2], [2, 3]]
          ))", "[[3, 3], [3, 3]]");

    // test matrix result with broadcasting a vector
    test_where_operation(
        R"(where(false,
            [[1], [2]], [3, 4]
          ))", "[[3, 4], [3, 4]]");
    test_where_operation(
        R"(where(true,
            [[1], [2]], [3, 4]
          ))", "[[1, 1], [2, 2]]");
    test_where_operation(
        R"(where(false,
            [3, 4], [[2], [1]]
          ))", "[[2, 2], [1, 1]]");
    test_where_operation(
        R"(where(true,
            [3, 4], [[2], [1]]
          ))", "[[3, 4], [3, 4]]");

    test_where_operation(
        R"(where(false,
            [[1, 2]], [3, 4]
          ))", "[[3, 4]]");
    test_where_operation(
        R"(where(true,
            [[1, 2]], [3, 4]
          ))", "[[1, 2]]");
    test_where_operation(
        R"(where(false,
            [3, 4], [[2, 1]]
          ))", "[[2, 1]]");
    test_where_operation(
        R"(where(true,
            [3, 4], [[2, 1]]
          ))", "[[3, 4]]");

    // test matrix result with broadcasting a matrix
    test_where_operation(
        R"(where(false,
            [[1, 2]], [[3]]
          ))", "[[3, 3]]");
    test_where_operation(
        R"(where(true,
            [[1, 2]], [[3]]
          ))", "[[1, 2]]");
    test_where_operation(
        R"(where(false,
            [[3]], [[2, 1]]
          ))", "[[2, 1]]");
    test_where_operation(
        R"(where(true,
            [[3]], [[2, 1]]
          ))", "[[3, 3]]");

    test_where_operation(
        R"(where(false,
            [[1, 2]], [[3, 4]]
          ))", "[[3, 4]]");
    test_where_operation(
        R"(where(true,
            [[1, 2]], [[3, 4]]
          ))", "[[1, 2]]");
    test_where_operation(
        R"(where(false,
            [[3, 4]], [[2, 1]]
          ))", "[[2, 1]]");
    test_where_operation(
        R"(where(true,
            [[3, 4]], [[2, 1]]
          ))", "[[3, 4]]");

    test_where_operation(
        R"(where(false,
            [[1, 2], [3, 4]], [[5, 6]]
          ))", "[[5, 6], [5, 6]]");
    test_where_operation(
        R"(where(true,
            [[1, 2], [3, 4]], [[5, 6]]
          ))", "[[1, 2], [3, 4]]");
    test_where_operation(
        R"(where(false,
            [[5, 6]], [[1, 2], [3, 4]]
          ))", "[[1, 2], [3, 4]]");
    test_where_operation(
        R"(where(true,
            [[5, 6]], [[1, 2], [3, 4]]
          ))", "[[5, 6], [5, 6]]");

    test_where_operation(
        R"(where(false,
            [[1, 2], [3, 4]], [[5], [6]]
          ))", "[[5, 5], [6, 6]]");
    test_where_operation(
        R"(where(true,
            [[1, 2], [3, 4]], [[5], [6]]
          ))", "[[1, 2], [3, 4]]");
    test_where_operation(
        R"(where(false,
            [[5], [6]], [[1, 2], [3, 4]]
          ))", "[[1, 2], [3, 4]]");
    test_where_operation(
        R"(where(true,
            [[5], [6]], [[1, 2], [3, 4]]
          ))", "[[5, 5], [6, 6]]");
}

void test_three_argument_where_1d()
{
    ///////////////////////////////////////////////////////////////////////////
    // test scalar right hand sides
    test_where_operation("where([0, 0], 1, 2)", "[2, 2]");
    test_where_operation("where([0, 1], 1, 2)", "[2, 1]");
    test_where_operation("where([1, 0], 1, 2)", "[1, 2]");
    test_where_operation("where([1, 1], 1, 2)", "[1, 1]");

    test_where_operation("where([0, 0], [1], 2)", "[2, 2]");
    test_where_operation("where([0, 1], [1], 2)", "[2, 1]");
    test_where_operation("where([1, 0], 1, [2])", "[1, 2]");
    test_where_operation("where([1, 1], 1, [2])", "[1, 1]");

    test_where_operation("where([0, 0], [1, 2], 3)", "[3, 3]");
    test_where_operation("where([0, 1], [1, 2], 3)", "[3, 2]");
    test_where_operation("where([1, 0], [1, 2], 3)", "[1, 3]");
    test_where_operation("where([1, 1], [1, 2], 3)", "[1, 2]");

    test_where_operation("where([0, 0], 1, [2, 3])", "[2, 3]");
    test_where_operation("where([0, 1], 1, [2, 3])", "[2, 1]");
    test_where_operation("where([1, 0], 1, [2, 3])", "[1, 3]");
    test_where_operation("where([1, 1], 1, [2, 3])", "[1, 1]");

    ///////////////////////////////////////////////////////////////////////////
    // test vector right hand sides
    test_where_operation(
        "where([0, 0], [1], [2])", "[2, 2]");
    test_where_operation(
        "where([0, 1], [1], [2])", "[2, 1]");
    test_where_operation(
        "where([1, 0], [1], [2])", "[1, 2]");
    test_where_operation(
        "where([1, 1], [1], [2])", "[1, 1]");

    test_where_operation(
        "where([0, 0], [1, 2], [3])", "[3, 3]");
    test_where_operation(
        "where([0, 1], [1, 2], [3])", "[3, 2]");
    test_where_operation(
        "where([1, 0], [1, 2], [3])", "[1, 3]");
    test_where_operation(
        "where([1, 1], [1, 2], [3])", "[1, 2]");

    test_where_operation(
        "where([0, 0], [1], [2, 3])", "[2, 3]");
    test_where_operation(
        "where([0, 1], [1], [2, 3])", "[2, 1]");
    test_where_operation(
        "where([1, 0], [1], [2, 3])", "[1, 3]");
    test_where_operation(
        "where([1, 1], [1], [2, 3])", "[1, 1]");

    test_where_operation(
        "where([0, 0], [1, 2], [3, 4])", "[3, 4]");
    test_where_operation(
        "where([0, 1], [1, 2], [3, 4])", "[3, 2]");
    test_where_operation(
        "where([1, 0], [1, 2], [3, 4])", "[1, 4]");
    test_where_operation(
        "where([1, 1], [1, 2], [3, 4])", "[1, 2]");

    test_where_operation(
        "where([0, 0], [1, 4], [2, 3])", "[2, 3]");
    test_where_operation(
        "where([0, 1], [1, 4], [2, 3])", "[2, 4]");
    test_where_operation(
        "where([1, 0], [1, 4], [2, 3])", "[1, 3]");
    test_where_operation(
        "where([1, 1], [1, 4], [2, 3])", "[1, 4]");

    ///////////////////////////////////////////////////////////////////////////
    // test matrix right hand sides
    test_where_operation(
        "where([0, 0], [[1]], 2)",
        "[[2, 2]]");
    test_where_operation(
        "where([0, 1], [[1]], 2)",
        "[[2, 1]]");
    test_where_operation(
        "where([1, 0], [[1]], 2)",
        "[[1, 2]]");
    test_where_operation(
        "where([1, 1], [[1]], 2)",
        "[[1, 1]]");

    test_where_operation(
        "where([0, 0], 2, [[1]])",
        "[[1, 1]]");
    test_where_operation(
        "where([0, 1], 2, [[1]])",
        "[[1, 2]]");
    test_where_operation(
        "where([1, 0], 2, [[1]])",
        "[[2, 1]]");
    test_where_operation(
        "where([1, 1], 2, [[1]])",
        "[[2, 2]]");

    test_where_operation(
        "where([0, 0], [[1]], [2])",
        "[[2, 2]]");
    test_where_operation(
        "where([0, 1], [[1]], [2])",
        "[[2, 1]]");
    test_where_operation(
        "where([1, 0], [[1]], [2])",
        "[[1, 2]]");
    test_where_operation(
        "where([1, 1], [[1]], [2])",
        "[[1, 1]]");
    test_where_operation(
        "where([0], [[-1,-2,-5],[-1,-3,-4]], [1, 2, 3])",
        "[[1, 2, 3],[1, 2, 3]]");

    test_where_operation(
        "where([0, 0], [2], [[1]])",
        "[[1, 1]]");
    test_where_operation(
        "where([0, 1], [2], [[1]])",
        "[[1, 2]]");
    test_where_operation(
        "where([1, 0], [2], [[1]])",
        "[[2, 1]]");
    test_where_operation(
        "where([1, 1], [2], [[1]])",
        "[[2, 2]]");

    test_where_operation(
        "where([0, 0], [[1]], [[2]])",
        "[[2, 2]]");
    test_where_operation(
        "where([0, 1], [[1]], [[2]])",
        "[[2, 1]]");
    test_where_operation(
        "where([1, 0], [[1]], [[2]])",
        "[[1, 2]]");
    test_where_operation(
        "where([1, 1], [[1]], [[2]])",
        "[[1, 1]]");

    test_where_operation(
        "where([0, 0], [[2]], [[1]])",
        "[[1, 1]]");
    test_where_operation(
        "where([0, 1], [[2]], [[1]])",
        "[[1, 2]]");
    test_where_operation(
        "where([1, 0], [[2]], [[1]])",
        "[[2, 1]]");
    test_where_operation(
        "where([1, 1], [[2]], [[1]])",
        "[[2, 2]]");

    test_where_operation(
        "where([0, 0], [[1], [2]], [[3]])",
        "[[3, 3], [3, 3]]");
    test_where_operation(
        "where([0, 1], [[1], [2]], [[3]])",
        "[[3, 1], [3, 2]]");
    test_where_operation(
        "where([1, 0], [[1], [2]], [[3]])",
        "[[1, 3], [2, 3]]");
    test_where_operation(
        "where([1, 1], [[1], [2]], [[3]])",
        "[[1, 1], [2, 2]]");

    test_where_operation(
        "where([0, 0], [[1, 2]], [[3]])",
        "[[3, 3]]");
    test_where_operation(
        "where([0, 1], [[1, 2]], [[3]])",
        "[[3, 2]]");
    test_where_operation(
        "where([1, 0], [[1, 2]], [[3]])",
        "[[1, 3]]");
    test_where_operation(
        "where([1, 1], [[1, 2]], [[3]])",
        "[[1, 2]]");
}

void test_three_argument_where_2d()
{
    ///////////////////////////////////////////////////////////////////////////
    // test scalar right hand sides
    test_where_operation("where([[0, 0], [0, 0]], 1, 2)",
        "[[2, 2], [2, 2]]");
    test_where_operation("where([[0, 1], [1, 0]], 1, 2)",
        "[[2, 1], [1, 2]]");
    test_where_operation("where([[1, 0], [0, 1]], 1, 2)",
        "[[1, 2], [2, 1]]");
    test_where_operation("where([[1, 1], [1, 1]], 1, 2)",
        "[[1, 1], [1, 1]]");

    test_where_operation(
        "where([[0, 0], [0, 0]], [1], 2)",
        "[[2, 2], [2, 2]]");
    test_where_operation(
        "where([[0, 1], [1, 0]], [1], 2)",
        "[[2, 1], [1, 2]]");
    test_where_operation(
        "where([[1, 0], [0, 1]], [1], 2)",
        "[[1, 2], [2, 1]]");
    test_where_operation(
        "where([[1, 1], [1, 1]], [1], 2)",
        "[[1, 1], [1, 1]]");
    test_where_operation(
        "where([[0]], [1, 2, 3, 4], -4)",
        "[[-4, -4, -4, -4]]");

    test_where_operation(
        "where([[0, 0], [0, 0]], 1, [2])",
        "[[2, 2], [2, 2]]");
    test_where_operation(
        "where([[0, 1], [1, 0]], 1, [2])",
        "[[2, 1], [1, 2]]");
    test_where_operation(
        "where([[1, 0], [0, 1]], 1, [2])",
        "[[1, 2], [2, 1]]");
    test_where_operation(
        "where([[1, 1], [1, 1]], 1, [2])",
        "[[1, 1], [1, 1]]");
    test_where_operation(
        "where([[0, 0, 0], [1, 1, 1]], [1, 2, 3], -4)",
        "[[-4, -4, -4],[ 1,  2,  3]]");

    test_where_operation(
        "where([[0, 0], [0, 0]], [[1]], 2)",
        "[[2, 2], [2, 2]]");
    test_where_operation(
        "where([[0, 1], [1, 0]], [[1]], 2)",
        "[[2, 1], [1, 2]]");
    test_where_operation(
        "where([[1, 0], [0, 1]], [[1]], 2)",
        "[[1, 2], [2, 1]]");
    test_where_operation(
        "where([[1, 1], [1, 1]], [[1]], 2)",
        "[[1, 1], [1, 1]]");

    test_where_operation(
        "where([[0, 0], [0, 0]], 1, [[2]])",
        "[[2, 2], [2, 2]]");
    test_where_operation(
        "where([[0, 1], [1, 0]], 1, [[2]])",
        "[[2, 1], [1, 2]]");
    test_where_operation(
        "where([[1, 0], [0, 1]], 1, [[2]])",
        "[[1, 2], [2, 1]]");
    test_where_operation(
        "where([[1, 1], [1, 1]], 1, [[2]])",
        "[[1, 1], [1, 1]]");
    test_where_operation(
        "where([[0, 0, 0]], 1, [[-1,-2,-3],[-4,-5,-6],[-7,-8,-9]])",
        "[[-1, -2, -3],[-4, -5, -6],[-7, -8, -9]]");

    ///////////////////////////////////////////////////////////////////////////
    // test vector right hand sides
    test_where_operation(
        "where([[0, 0], [0, 0]], [1], [2])",
        "[[2, 2], [2, 2]]");
    test_where_operation(
        "where([[0, 1], [1, 0]], [1], [2])",
        "[[2, 1], [1, 2]]");
    test_where_operation(
        "where([[1, 0], [0, 1]], [1], [2])",
        "[[1, 2], [2, 1]]");
    test_where_operation(
        "where([[1, 1], [1, 1]], [1], [2])",
        "[[1, 1], [1, 1]]");

    test_where_operation(
        "where([[0, 0], [0, 0]], [1, 3], [2])",
        "[[2, 2], [2, 2]]");
    test_where_operation(
        "where([[0, 1], [1, 0]], [1, 3], [2])",
        "[[2, 3], [1, 2]]");
    test_where_operation(
        "where([[1, 0], [0, 1]], [1, 3], [2])",
        "[[1, 2], [2, 3]]");
    test_where_operation(
        "where([[1, 1], [1, 1]], [1, 3], [2])",
        "[[1, 3], [1, 3]]");

    test_where_operation(
        "where([[0, 0], [0, 0]], [1], [2, 3])",
        "[[2, 3], [2, 3]]");
    test_where_operation(
        "where([[0, 1], [1, 0]], [1], [2, 3])",
        "[[2, 1], [1, 3]]");
    test_where_operation(
        "where([[1, 0], [0, 1]], [1], [2, 3])",
        "[[1, 3], [2, 1]]");
    test_where_operation(
        "where([[1, 1], [1, 1]], [1], [2, 3])",
        "[[1, 1], [1, 1]]");

    test_where_operation(
        "where([[0, 0], [0, 0]], "
            "[[1]], [2, 3])",
        "[[2, 3], [2, 3]]");
    test_where_operation(
        "where([[0, 1], [1, 0]], "
            "[[1]], [2, 3])",
        "[[2, 1], [1, 3]]");
    test_where_operation(
        "where([[1, 0], [0, 1]], "
            "[[1]], [2, 3])",
        "[[1, 3], [2, 1]]");
    test_where_operation(
        "where([[1, 1], [1, 1]], "
            "[[1]], [2, 3])",
        "[[1, 1], [1, 1]]");

    test_where_operation(
        "where([[0, 0], [0, 0]], "
            "[1, 3], [[2]])",
        "[[2, 2], [2, 2]]");
    test_where_operation(
        "where([[0, 1], [1, 0]], "
            "[1, 3], [[2]])",
        "[[2, 3], [1, 2]]");
    test_where_operation(
        "where([[1, 0], [0, 1]], "
            "[1, 3], [[2]])",
        "[[1, 2], [2, 3]]");
    test_where_operation(
        "where([[1, 1], [1, 1]], "
            "[1, 3], [[2]])",
        "[[1, 3], [1, 3]]");
}

///////////////////////////////////////////////////////////////////////////////
int main(int argc, char* argv[])
{
    test_one_argument_where();
    test_three_argument_where_0d();
    test_three_argument_where_1d();
    test_three_argument_where_2d();

    return hpx::util::report_errors();
}
