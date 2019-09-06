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
    std::string const& name, std::string const& codestr)
{
    phylanx::execution_tree::compiler::function_list snippets;
    phylanx::execution_tree::compiler::environment env =
        phylanx::execution_tree::compiler::default_environment();

    auto const& code =
        phylanx::execution_tree::compile(name, codestr, snippets, env);
    return code.run().arg_;
}

///////////////////////////////////////////////////////////////////////////////
void test_transpose_operation(std::string const& name, std::string const& code,
    std::string const& expected_str)
{
    HPX_TEST_EQ(
        compile_and_run(name, code), compile_and_run(name, expected_str));
}

////////////////////////////////////////////////////////////////////////////////
void test_transpose_1d()
{
    // transposing a vector results in the same vector with transposed annotations
    test_transpose_operation("test1d_1",
        R"(
            transpose_d(
                annotate_d(
                    [1, 2, 3],
                    "test1d_1",
                    list("tile", list("rows", 0, 3))
                )
            )
        )",
        R"(
            annotate_d(
                [1, 2, 3],
                "test1d_1_transposed/1",
                list("tile", list("columns", 0, 3))
            )
        )");
    test_transpose_operation("test1d_2",
        R"(
            transpose_d(
                annotate_d(
                    [1, 2, 3],
                    "test1d_2",
                    list("tile", list("columns", 0, 3))
                )
            )
        )",
        R"(
            annotate_d(
                [1, 2, 3],
                "test1d_2_transposed/1",
                list("tile", list("rows", 0, 3))
            )
        )");
}

////////////////////////////////////////////////////////////////////////////////
void test_transpose_2d()
{
    test_transpose_operation("test2d_1",
        R"(
            transpose_d(
                annotate_d(
                    [[1, 2, 3], [3, 4, 1]],
                    "test2d_1",
                    list("tile", list("columns", 0, 3), list("rows", 0, 2))
                )
            )
        )",
        R"(
            annotate_d(
                [[1, 3], [2, 4], [3, 1]],
                "test2d_1_transposed/1",
                list("tile", list("rows", 0, 3), list("columns", 0, 2))
            )
        )");

    // 2d transposition with axis [0, 1] results in a no-op
    test_transpose_operation("test2d_2",
        R"(
            transpose_d(
                annotate_d(
                    [[1, 2, 3], [3, 4, 1]],
                    "test2d_2",
                    list("tile", list("columns", 0, 3), list("rows", 0, 2))
                ),
                [0, 1]
            )
        )",
        R"(
            annotate_d(
                [[1, 2, 3], [3, 4, 1]],
                "test2d_2_transposed/1",
                list("tile", list("rows", 0, 2), list("columns", 0, 3))
            )
        )");

    // 2d transposition with axes [1, 0] is equivalent to default transpose
    test_transpose_operation("test2d_3",
        R"(
            transpose_d(
                annotate_d(
                    [[1, 2, 3], [3, 4, 1]],
                    "test2d_3",
                    list("tile", list("columns", 0, 3), list("rows", 0, 2))
                ),
                [1, 0]
            )
        )",
        R"(
            annotate_d(
                [[1, 3], [2, 4], [3, 1]],
                "test2d_3_transposed/1",
                list("tile", list("rows", 0, 3), list("columns", 0, 2))
            )
        )");
}

////////////////////////////////////////////////////////////////////////////////
int main(int argc, char* argv[])
{
    // transposing a scalar is a no-op
    test_transpose_operation("test0d", "transpose_d(5.0)", "5.0");

    test_transpose_1d();
    test_transpose_2d();

    return hpx::util::report_errors();
}
