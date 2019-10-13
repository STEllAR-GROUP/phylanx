// Copyright (c) 2018 Hartmut Kaiser
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/phylanx.hpp>

#include <hpx/hpx_main.hpp>
#include <hpx/include/lcos.hpp>
#include <hpx/testing.hpp>

#include <cstddef>
#include <string>
#include <utility>
#include <vector>

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

void test_integer_slicing(char const* code, char const* expected)
{
    auto result =
        phylanx::execution_tree::extract_integer_value(compile_and_run(code));
    auto exprected_result = phylanx::execution_tree::extract_integer_value(
        compile_and_run(expected));

    HPX_TEST_EQ(result, exprected_result);
}

///////////////////////////////////////////////////////////////////////////////
void test_integer_slicing_1d_0d()
{
    // direct array based indexing
    test_integer_slicing("slice([42], 0, nil)", "42");
    test_integer_slicing("slice([42], -1, nil)", "42");

    test_integer_slicing("slice([42, 43], 0, nil)", "42");
    test_integer_slicing("slice([42, 43], -1, nil)", "43");

    // indexing using a list of arrays
    test_integer_slicing("slice([42], list(0), nil)", "42");
    test_integer_slicing("slice([42], list(-1), nil)", "42");

    test_integer_slicing("slice([42, 43], list(0), nil)", "42");
    test_integer_slicing("slice([42, 43], list(-1), nil)", "43");
}

///////////////////////////////////////////////////////////////////////////////
void test_integer_slicing_1d_1d()
{
    // direct array based indexing
    test_integer_slicing(
        "slice([42], [0], nil)", "[42]");
    test_integer_slicing(
        "slice([42], [-1], nil)", "[42]");

    test_integer_slicing(
        "slice([42], [0, 0], nil)", "[42, 42]");
    test_integer_slicing(
        "slice([42], [-1, 0], nil)", "[42, 42]");

    test_integer_slicing(
        "slice([42, 43], [0], nil)", "[42]");
    test_integer_slicing(
        "slice([42, 43], [-1], nil)", "[43]");

    test_integer_slicing(
        "slice([42, 43], [0, 1, 0], nil)", "[42, 43, 42]");
    test_integer_slicing(
        "slice([42, 43], [-2, -1, 0], nil)", "[42, 43, 42]");

    // indexing using a list of arrays
    test_integer_slicing(
        "slice([42], list([0]), nil)", "[42]");
    test_integer_slicing(
        "slice([42], list([-1]), nil)", "[42]");

    test_integer_slicing(
        "slice([42], list([0, 0]), nil)", "[42, 42]");
    test_integer_slicing(
        "slice([42], list([-1, 0]), nil)", "[42, 42]");

    test_integer_slicing(
        "slice([42, 43], list([0]), nil)", "[42]");
    test_integer_slicing(
        "slice([42, 43], list([-1]), nil)", "[43]");

    test_integer_slicing(
        "slice([42, 43], list([0, 1, 0]), nil)",
        "[42, 43, 42]");
    test_integer_slicing(
        "slice([42, 43], list([-2, -1, 0]), nil)",
        "[42, 43, 42]");
}

///////////////////////////////////////////////////////////////////////////////
void test_integer_slicing_1d_2d()
{
    // direct array based indexing
    test_integer_slicing(
        "slice([42], [[0]], nil)",
        "[[42]]");
    test_integer_slicing(
        "slice([42], [[-1]], nil)",
        "[[42]]");

    test_integer_slicing(
        "slice([42], [[0, 0]], nil)",
        "[[42, 42]]");
    test_integer_slicing(
        "slice([42], [[-1, 0]], nil)",
        "[[42, 42]]");

    test_integer_slicing(
        "slice([42], [[0], [0]], nil)",
        "[[42], [42]]");
    test_integer_slicing(
        "slice([42], [[-1], [0]], nil)",
        "[[42], [42]]");

    test_integer_slicing(
        "slice([42], [[0, 0], [0, 0]], nil)",
        "[[42, 42], [42, 42]]");
    test_integer_slicing(
        "slice([42], [[-1, 0], [0, -1]], nil)",
        "[[42, 42], [42, 42]]");

    test_integer_slicing(
        "slice([42, 43], [[0]], nil)",
        "[[42]]");
    test_integer_slicing(
        "slice([42, 43], [[-1]], nil)",
        "[[43]]");

    test_integer_slicing(
        "slice([42, 43], [[0, 1]], nil)",
        "[[42, 43]]");
    test_integer_slicing(
        "slice([42, 43], [[-1, 0]], nil)",
        "[[43, 42]]");

    test_integer_slicing(
        "slice([42, 43], [[0], [1]], nil)",
        "[[42], [43]]");
    test_integer_slicing(
        "slice([42, 43], [[-1], [0]], nil)",
        "[[43], [42]]");

    test_integer_slicing(
        "slice([42, 43], [[0, 1, 0], [0, 0, 1]], nil)",
        "[[42, 43, 42], [42, 42, 43]]");
    test_integer_slicing(
        "slice([42, 43], [[-2, -1, 0], [0, -2, -1]], nil)",
        "[[42, 43, 42], [42, 42, 43]]");

    // indexing using a list of arrays
    test_integer_slicing(
        "slice([42], list([[0]]), nil)",
        "[[42]]");
    test_integer_slicing(
        "slice([42], list([[-1]]), nil)",
        "[[42]]");

    test_integer_slicing(
        "slice([42], list([[0, 0]]), nil)",
        "[[42, 42]]");
    test_integer_slicing(
        "slice([42], list([[-1, 0]]), nil)",
        "[[42, 42]]");

    test_integer_slicing(
        "slice([42], list([[0], [0]]), nil)",
        "[[42], [42]]");
    test_integer_slicing(
        "slice([42], list([[-1], [0]]), nil)",
        "[[42], [42]]");

    test_integer_slicing(
        "slice([42], list([[0, 0], [0, 0]]), nil)",
        "[[42, 42], [42, 42]]");
    test_integer_slicing(
        "slice([42], list([[-1, 0], [0, -1]]), nil)",
        "[[42, 42], [42, 42]]");

    test_integer_slicing(
        "slice([42, 43], list([[0]]), nil)",
        "[[42]]");
    test_integer_slicing(
        "slice([42, 43], list([[-1]]), nil)",
        "[[43]]");

    test_integer_slicing(
        "slice([42, 43], list([[0, 1]]), nil)",
        "[[42, 43]]");
    test_integer_slicing(
        "slice([42, 43], list([[-1, 0]]), nil)",
        "[[43, 42]]");

    test_integer_slicing(
        "slice([42, 43], list([[0], [1]]), nil)",
        "[[42], [43]]");
    test_integer_slicing(
        "slice([42, 43], list([[-1], [0]]), nil)",
        "[[43], [42]]");

    test_integer_slicing(
        "slice([42, 43], list([[0, 1, 0], [0, 0, 1]]), nil)",
        "[[42, 43, 42], [42, 42, 43]]");
    test_integer_slicing(
        "slice([42, 43], list([[-2, -1, 0], [0, -2, -1]]), nil)",
        "[[42, 43, 42], [42, 42, 43]]");
}

///////////////////////////////////////////////////////////////////////////////
void test_integer_slicing_2d_0d()
{
    // direct array based indexing
    test_integer_slicing("slice([[42], [43]], 0)", "[42]");
    test_integer_slicing("slice([[42, 43]], 0)", "[42, 43]");

    test_integer_slicing("slice([[42], [43]], 0, nil)", "[[42]]");
    test_integer_slicing("slice([[42, 43]], 0, nil)", "[[42, 43]]");

    test_integer_slicing("slice([[42], [43]], nil, 0)", "[[42]]");
    test_integer_slicing("slice([[42, 43]], nil, 0)", "[[42, 43]]");

    test_integer_slicing("slice([[42], [43]], 0, 0)", "42");
    test_integer_slicing("slice([[42, 43]], 0, 1)", "43");
}

void test_integer_slicing_2d_1d()
{
    // direct array based single-value indexing
    test_integer_slicing("slice([[42], [43]], [0])", "[[42]]");
    test_integer_slicing("slice([[42, 43]], [0])", "[[42, 43]]");

    test_integer_slicing("slice([[42], [43]], [0], nil)", "[[[42]]]");
    test_integer_slicing("slice([[42, 43]], [0], nil)", "[[[42, 43]]]");

    test_integer_slicing("slice([[42], [43]], nil, [0])", "[[[42]]]");
    test_integer_slicing("slice([[42, 43]], nil, [0])", "[[[42, 43]]]");

    // direct array based array-value indexing
    test_integer_slicing(
        "slice([[42], [43]], [0, 1, 0])", "[[42], [43], [42]]");
    test_integer_slicing("slice([[42, 43]], [0, 0])", "[[42, 43], [42, 43]]");

    test_integer_slicing(
        "slice([[42], [43]], [0, 1, 0], nil)", "[[[42], [43], [42]]]");
    test_integer_slicing(
        "slice([[42, 43]], [0, 0], nil)", "[[[42, 43], [42, 43]]]");

    test_integer_slicing(
        "slice([[42], [43]], nil, [0, 0])", "[[[42], [42]]]");
    test_integer_slicing("slice([[42, 43]], nil, [0, 0, 0])",
        "[[[42, 43], [42, 43], [42, 43]]]");

    // 'real' advanced integer indexing
    test_integer_slicing("slice([[42], [43]], [0], 0)", "[42]");
    test_integer_slicing("slice([[42], [43]], 0, [0])", "[42]");
    test_integer_slicing("slice([[42], [43]], [0], [0])", "[42]");

    test_integer_slicing("slice([[42], [43]], [0, 1], 0)", "[42, 43]");
    test_integer_slicing("slice([[42], [43]], 0, [0, 0])", "[42, 42]");

    test_integer_slicing("slice([[42], [43]], [0, 1], [0])", "[42, 43]");
    test_integer_slicing("slice([[42], [43]], [0], [0, 0])", "[42, 42]");

    test_integer_slicing("slice([[42], [43]], [0, 1], [0, 0])", "[42, 43]");

    test_integer_slicing("slice([[42, 43]], [0], 0)", "[42]");
    test_integer_slicing("slice([[42, 43]], 0, [0])", "[42]");
    test_integer_slicing("slice([[42, 43]], [0], [0])", "[42]");

    test_integer_slicing("slice([[42, 43]], [0, 0], 0)", "[42, 42]");
    test_integer_slicing("slice([[42, 43]], 0, [0, 1])", "[42, 43]");

    test_integer_slicing("slice([[42, 43]], [0, 0], [0])", "[42, 42]");
    test_integer_slicing("slice([[42, 43]], [0], [0, 1])", "[42, 43]");

    test_integer_slicing("slice([[42, 43]], [0, 0], [0, 1])", "[42, 43]");
}

void test_integer_slicing_2d_2d()
{
    // direct array based single-value indexing
    test_integer_slicing("slice([[42], [43]], [[0]])", "[[[42]]]");
    test_integer_slicing("slice([[42, 43]], [[0]])", "[[[42, 43]]]");

//     test_integer_slicing("slice([[42], [43]], [[0, 1]])", "[[[42], [43]]]");
//     test_integer_slicing(
//         "slice([[42, 43]], [[0], [0]])", "[[[42, 43], [42, 43]]]");
//         "slice([[42, 43]], 0, nil)", "[42]");
//     test_integer_slicing(
//         "slice([[42], [43]], 0, nil)", "[42, 43]");
//
//     test_integer_slicing(
//         "slice([[42, 43]], nil, 0)", "[[42]]");
// }

    // requires 4D support
//     test_integer_slicing("slice([[42], [43]], [[0]], nil)", "[[[[42]]]]");
//     test_integer_slicing("slice([[42, 43]], [[0]], nil)", "[[[[42, 43]]]]");

//     test_integer_slicing("slice([[42], [43]], nil, [[0]])", "[[[[42], [43]]]]");
//     test_integer_slicing("slice([[42, 43]], nil, [[0]])", "[[[[42]]]]");
}

///////////////////////////////////////////////////////////////////////////////
void test_integer_slicing_3d_0d()
{
    // direct array based indexing
    test_integer_slicing("slice([[[42], [43]]], 0)", "[[42], [43]]");
    test_integer_slicing("slice([[[42, 43]]], 0)", "[[42, 43]]");

    test_integer_slicing("slice([[[42], [43]]], 0, 0)", "[42]");
    test_integer_slicing("slice([[[42, 43]]], 0, 0)", "[42, 43]");

    test_integer_slicing("slice([[[42], [43]]], 0, 1, 0)", "43");
    test_integer_slicing("slice([[[42, 43]]], 0, 0, 1)", "43");
}

///////////////////////////////////////////////////////////////////////////////
void test_integer_slicing_3d_1d()
{
    // direct array based indexing
    test_integer_slicing("slice([[[42], [43]]], [0])", "[[[42], [43]]]");
    test_integer_slicing("slice([[[42, 43]]], [0])", "[[[42, 43]]]");

    test_integer_slicing("slice([[[42], [43]]], [0], 0)", "[[42]]");
    test_integer_slicing("slice([[[42, 43]]], [0], 0)", "[[42, 43]]");
    test_integer_slicing("slice([[[42], [43]]], 0, [0])", "[[42]]");
    test_integer_slicing("slice([[[42, 43]]], 0, [0])", "[[42, 43]]");
    test_integer_slicing("slice([[[42], [43]]], [0], [0])", "[[42]]");
    test_integer_slicing("slice([[[42, 43]]], [0], [0])", "[[42, 43]]");

    test_integer_slicing("slice([[[42], [43]]], [0, 0], [0])", "[[42], [42]]");
    test_integer_slicing(
        "slice([[[42, 43]]], [0, 0], [0])", "[[42, 43], [42, 43]]");

    test_integer_slicing("slice([[[42], [43]]], [0], [1], [0])", "[43]");
    test_integer_slicing("slice([[[42, 43]]], [0], [0], [1])", "[43]");
    test_integer_slicing("slice([[[42], [43]]], [0, 0], [1], [0])", "[43, 43]");
    test_integer_slicing("slice([[[42, 43]]], [0, 0], [0], [1])", "[43, 43]");
}

///////////////////////////////////////////////////////////////////////////////
int main(int argc, char* argv[])
{
    test_integer_slicing_1d_0d();   // use 0d value as index for 1d array
    test_integer_slicing_1d_1d();   // use 1d value as index for 1d array
    test_integer_slicing_1d_2d();   // use 2d value as index for 1d array

    test_integer_slicing_2d_0d();   // use 0d value as index for 2d array
    test_integer_slicing_2d_1d();   // use 1d value as index for 2d array
    test_integer_slicing_2d_2d();   // use 2d value as index for 2d array

    test_integer_slicing_3d_0d();   // use 0d value as index for 3d array
    test_integer_slicing_3d_1d();   // use 1d value as index for 3d array

    return hpx::util::report_errors();
}
