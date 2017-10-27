//   Copyright (c) 2017 Hartmut Kaiser
//
//   Distributed under the Boost Software License, Version 1.0. (See accompanying
//   file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/phylanx.hpp>
#include <hpx/hpx_main.hpp>

#include <iostream>

// This example transforms a (matrix) multiplication 'A * B' into a set of
// 4 (tiled) multiplications

int main(int argc, char* argv[])
{
    char const* expr_str = "A * B";

    char const* to_match_str = "_1 * _2";
    char const* to_replace_str = R"(
        block(
            define(columnsA, shape(_1, 1)),
            define(A1, slice_columns(_1, 0, columsA / 2)),
            define(A2, slice_columns(_1, columsA / 2)),
            define(rowsB, shape(_2, 0)),
            define(columnsB, shape(_2, 1)),
            define(B1, slice_rows(_2, 0, rowsB / 2)),
            define(B2, slice_rows(_2, rowsB / 2)),
            combine_columns(
                combine_rows(A1 * B1, A2 * B1)
                combine_rows(A1 * B2, A2 * B2)
            )
        )
    )";

    phylanx::ast::expression result =
        phylanx::ast::transform_ast(
            phylanx::ast::generate_ast(expr_str),
            phylanx::ast::transform_rule{
                phylanx::ast::generate_ast(to_match_str),
                phylanx::ast::generate_ast(to_replace_str)
            });

    std::cout << phylanx::ast::to_string(result) << '\n';

    return 0;
}
