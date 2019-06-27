//   Copyright (c) 2017-2019 Hartmut Kaiser
//   Copyright (c) 2019 Bita Hasheminezhad
//
//   Distributed under the Boost Software License, Version 1.0. (See accompanying
//   file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/phylanx.hpp>

#include <hpx/hpx_main.hpp>
#include <hpx/include/lcos.hpp>
#include <hpx/util/lightweight_test.hpp>

#include <cstdint>
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
    return code.run();
}

///////////////////////////////////////////////////////////////////////////////
void test_transpose_operation(std::string const& code,
    std::string const& expected_str)
{
    HPX_TEST_EQ(compile_and_run(code), compile_and_run(expected_str));
}

////////////////////////////////////////////////////////////////////////////////
void test_transpose_1d()
{
    // transposing a vector results in the same vector with transposed annotations
    test_transpose_operation(
        R"(
            transpose_d(
                annotate(
                    [1, 2, 3],
                    list(format("meta_{}", locality()),
                        list("locality", locality(), num_localities()),
                        list("tile", list("rows", 0, 3))
                    )
                )
            )
        )",
        R"(
            annotate(
                [1, 2, 3],
                list(format("meta_{}", locality()),
                    list("locality", locality(), num_localities()),
                    list("tile", list("columns", 0, 3))
                )
            )
        )");
    test_transpose_operation(
        R"(
            transpose_d(
                annotate(
                    [1, 2, 3],
                    list(format("meta_{}", locality()),
                        list("locality", locality(), num_localities()),
                        list("tile", list("columns", 0, 3))
                    )
                )
            )
        )",
        R"(
            annotate(
                [1, 2, 3],
                list(format("meta_{}", locality()),
                    list("locality", locality(), num_localities()),
                    list("tile", list("rows", 0, 3))
                )
            )
        )");
}

////////////////////////////////////////////////////////////////////////////////
void test_transpose_2d()
{
    test_transpose_operation(
        R"(
            transpose_d(
                annotate(
                    [[1, 2, 3], [3, 4, 1]],
                    list(format("meta_{}", locality()),
                        list("locality", locality(), num_localities()),
                        list("tile", list("columns", 0, 3), list("rows", 0, 2))
                    )
                )
            )
        )",
        R"(
            annotate(
                [[1, 3], [2, 4], [3, 1]],
                list(format("meta_{}", locality()),
                    list("locality", locality(), num_localities()),
                    list("tile", list("rows", 0, 3), list("columns", 0, 2))
                )
            )
        )");

    // 2d transposition with axis [0, 1] results in a no-op
    test_transpose_operation(
        R"(
            transpose_d(
                annotate(
                    [[1, 2, 3], [3, 4, 1]],
                    list(format("meta_{}", locality()),
                        list("locality", locality(), num_localities()),
                        list("tile", list("columns", 0, 3), list("rows", 0, 2))
                    )
                ),
                [0, 1]
            )
        )",
        R"(
            annotate(
                [[1, 2, 3], [3, 4, 1]],
                list(format("meta_{}", locality()),
                    list("locality", locality(), num_localities()),
                    list("tile", list("columns", 0, 3), list("rows", 0, 2))
                )
            )
        )");

    // 2d transposition with axes [1, 0] is equivalent to default transpose
    test_transpose_operation(
        R"(
            transpose_d(
                annotate(
                    [[1, 2, 3], [3, 4, 1]],
                    list(format("meta_{}", locality()),
                        list("locality", locality(), num_localities()),
                        list("tile", list("columns", 0, 3), list("rows", 0, 2))
                    )
                ),
                [1, 0]
            )
        )",
        R"(
            annotate(
                [[1, 3], [2, 4], [3, 1]],
                list(format("meta_{}", locality()),
                    list("locality", locality(), num_localities()),
                    list("tile", list("rows", 0, 3), list("columns", 0, 2))
                )
            )
        )");
}

////////////////////////////////////////////////////////////////////////////////
int main(int argc, char* argv[])
{
    // transposing a scalar is a no-op
    test_transpose_operation("transpose_d(5.0)", "5.0");

    test_transpose_1d();
    test_transpose_2d();

// #if defined(PHYLANX_HAVE_BLAZE_TENSOR)
//     test_transpose_operation(
//         "transpose([[[1,2,3],[4,5,6]]])", "[[[1], [4]],[[2], [5]],[[3], [6]]]");
//     test_transpose_operation(
//         "transpose([[[1,2,3],[4,5,6]], [[7,8,9],[10,11,12]],"
//         "[[13,14,15],[16,17,18]], [[19,20,21],[22,23,24]]], [2, 1, 0])",
//         "[[[ 1,  7, 13, 19],[ 4, 10, 16, 22]],[[ 2,  8, 14, 20],"
//         "[ 5, 11, 17, 23]],[[ 3,  9, 15, 21],[ 6, 12, 18, 24]]]");
//     test_transpose_operation(
//         "transpose([[[1,2,3],[4,5,6]], [[7,8,9],[10,11,12]],"
//         "[[13,14,15],[16,17,18]],[[19,20,21],[22,23,24]]], make_list(1, 0, 2))",
//         "[[[ 1,  2,  3],[ 7,  8,  9],[13, 14, 15],[19, 20, 21]],"
//         "[[ 4,  5,  6],[10, 11, 12],[16, 17, 18],[22, 23, 24]]]");
//     test_transpose_operation(
//         "transpose([[[1,2,3],[4,5,6]], [[7,8,9],[10,11,12]],"
//         "[[13,14,15],[16,17,18]], [[19,20,21],[22,23,24]]], [0, 2, 1])",
//         "[[[ 1,  4],[ 2,  5],[ 3,  6]], [[ 7, 10],[ 8, 11],[ 9, 12]],"
//         "[[13, 16],[14, 17],[15, 18]], [[19, 22],[20, 23],[21, 24]]]");
//     test_transpose_operation(
//         "transpose([[[1,2,3],[4,5,6]], [[7,8,9],[10,11,12]],"
//         "[[13,14,15],[16,17,18]],[[19,20,21],[22,23,24]]], make_list(1, 2, 0))",
//         "[[[ 1,  7, 13, 19],[ 2,  8, 14, 20],[ 3,  9, 15, 21]],"
//         "[[ 4, 10, 16, 22],[ 5, 11, 17, 23],[ 6, 12, 18, 24]]]");
//     test_transpose_operation(
//         "transpose([[[1,2,3],[4,5,6]], [[7,8,9],[10,11,12]],"
//         "[[13,14,15],[16,17,18]], [[19,20,21],[22,23,24]]], [1, -1, -3])",
//         "[[[ 1,  7, 13, 19],[ 2,  8, 14, 20],[ 3,  9, 15, 21]],"
//         "[[ 4, 10, 16, 22],[ 5, 11, 17, 23],[ 6, 12, 18, 24]]]");
//     test_transpose_operation(
//         "transpose([[[1,2,3],[4,5,6]], [[7,8,9],[10,11,12]],"
//         "[[13,14,15],[16,17,18]], [[19,20,21],[22,23,24]]], [2, 0, 1])",
//         "[[[ 1,  4],[ 7, 10],[13, 16],[19, 22]],"
//         "[[ 2,  5],[ 8, 11],[14, 17],[20, 23]],"
//         "[[ 3,  6],[ 9, 12],[15, 18],[21, 24]]]");
//     test_transpose_operation(
//         "transpose([[[1,2,3],[4,5,6]], [[7,8,9],[10,11,12]],"
//         "[[13,14,15],[16,17,18]], [[19,20,21],[22,23,24]]], [0, -2, -1])",
//         "[[[ 1,  2,  3],[ 4,  5,  6]],[[ 7,  8,  9],[10, 11, 12]],"
//         "[[13, 14, 15],[16, 17, 18]],[[19, 20, 21],[22, 23, 24]]]");
// #endif

    return hpx::util::report_errors();
}
