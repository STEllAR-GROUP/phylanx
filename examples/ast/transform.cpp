//   Copyright (c) 2017-2018 Hartmut Kaiser
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
    char const* expr_str = R"(block(
        define(multiply, A, B, A * B),
        multiply
    ))";

    char const* to_match_str = "_1 * _2";

    char const* to_replace_str = R"(block(
        define(rowsA, shape(_1, 0)),
        define(columnsA, shape(_1, 1)),
        define(A11, slice(_1, 0, rowsA / 2, 0, columnsA / 2)),
        define(A12, slice(_1, 0, rowsA / 2, columnsA / 2, columnsA)),
        define(A21, slice(_1, rowsA / 2, rowsA, 0, columnsA / 2)),
        define(A22, slice(_1, rowsA / 2, rowsA, columnsA / 2, columnsA)),
        define(rowsB, shape(_2, 0)),
        define(columnsB, shape(_2, 1)),
        define(B11, slice(_2, 0, rowsB / 2, 0, columnsB / 2)),
        define(B12, slice(_2, 0, rowsB / 2, columnsB / 2, columnsB)),
        define(B21, slice(_2, rowsB / 2, rowsB, 0, columnsB / 2)),
        define(B22, slice(_2, rowsB / 2, rowsB, columnsB / 2, columnsB)),
        vstack(
            hstack((A11 * B11) + (A12 * B21), (A11 * B12) + (A12 * B22)),
            hstack((A21 * B11) + (A22 * B21), (A21 * B12) + (A22 * B22))
        )
    ))";

    auto result =
        phylanx::ast::transform_ast(
            phylanx::ast::generate_ast(expr_str),
            phylanx::ast::transform_rule{
                phylanx::ast::generate_ast(to_match_str)[0],
                phylanx::ast::generate_ast(to_replace_str)[0]
            });

    std::cout << phylanx::ast::to_string(result[0]) << '\n';

    phylanx::execution_tree::compiler::function_list snippets;

    auto code_direct =
        phylanx::execution_tree::compile("direct", expr_str, snippets);
    auto direct = code_direct.run();

    auto code_transformed =
        phylanx::execution_tree::compile("transformed", result, snippets);
    auto transformed = code_transformed.run();

    blaze::Rand<blaze::DynamicMatrix<double>> gen{};

    blaze::DynamicMatrix<double> A = gen.generate(16, 16);
    blaze::DynamicMatrix<double> B = gen.generate(16, 16);

    auto direct_result = direct(A, B);
    auto transformed_result = transformed(A, B);

    if (direct_result != transformed_result)
    {
        std::cout << "The transformed result differs from the direct one!\n";

        std::cout << direct_result << "\n";
        std::cout << transformed_result << "\n";
    }

    return 0;
}
