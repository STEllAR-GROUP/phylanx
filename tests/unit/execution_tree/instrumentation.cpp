//  Copyright (c) 2017 Hartmut Kaiser
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// phylanx-no-inspect

#include <phylanx/phylanx.hpp>

#include <hpx/hpx_main.hpp>
#include <hpx/util/lightweight_test.hpp>

#include <vector>

void test_unary_expr()
{
    std::string code =
        R"(block(
            define(func, A, B, C, store(A, A - B))
        ))";

    std::vector<std::string::const_iterator> iterators;
    phylanx::execution_tree::compiler::function_list snippets;

    auto const& code = phylanx::execution_tree::compile(
        phylanx::ast::generate_ast(code, iterators), snippets);
    auto f = code.run();

    auto entries = hpx::agas::find_symbols(hpx::launch::sync, "/phylanx/*$*");
}

int main(int argc, char* argv[])
{
    test_unary_expr();

    return hpx::util::report_errors();
}

