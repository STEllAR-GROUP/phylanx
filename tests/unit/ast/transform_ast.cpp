//   Copyright (c) 2017 Hartmut Kaiser
//
//   Distributed under the Boost Software License, Version 1.0. (See accompanying
//   file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/phylanx.hpp>

#include <hpx/hpx_main.hpp>
#include <hpx/util/lightweight_test.hpp>

#include <iostream>
#include <string>
#include <utility>

void test_transform_ast(char const* matchstr, char const* rulestr,
    char const* replacestr, char const* expectedstr)
{
    phylanx::ast::expression match = phylanx::ast::generate_ast(matchstr);

    phylanx::ast::transform_rule r{
        phylanx::ast::generate_ast(rulestr),
        phylanx::ast::generate_ast(replacestr)
    };
    phylanx::ast::expression result = phylanx::ast::transform_ast(match, r);

    std::cout << phylanx::ast::to_string(result) << '\n';

    phylanx::ast::expression expected = phylanx::ast::generate_ast(expectedstr);

    HPX_TEST(result == expected);
}

int main(int argc, char* argv[])
{
    // simple non-recursive transforms
    test_transform_ast("A + B", "_1 + _2", "_2 + _1", "B + A");
    test_transform_ast("A + B", "_1 + _2", "_2 - _1", "B - A");

    test_transform_ast("-A", "-_1", "_1", "A");
    test_transform_ast("A", "_1", "-_1", "-A");

    test_transform_ast("(A + B) * C", "_1 * _2", "_2 * _1", "C * (A + B)");
    test_transform_ast("C * (A + B)", "_1 * _2", "_2 * _1", "(A + B) * C");

    test_transform_ast("A + B", "_1", "-_1", "-(A + B)");

    test_transform_ast("A + B", "_1 + _2", "add(_1, _2)", "add(A, B)");
    test_transform_ast("add(A, B)", "add(_1, _2)", "_1 + _2", "A + B");

    test_transform_ast("add(A, B)", "_1(_2, _3)", "wrap(_1(_2, _3))",
        "wrap(add(A, B))");

    // simple transforms requiring simplification
    test_transform_ast("-A + B", "_1 + _2", "_2 + _1", "B + -A");
    test_transform_ast("A + -B", "_1 + _2", "_2 - _1", "-B - A");

    // more complex transforms requiring recursion and simplification
    test_transform_ast("A + (B * C)", "_1 * _2", "_2 * _1", "A + (C * B)");
    test_transform_ast("A + (B * C)", "_1 * _2", "mul(_2, _1)", "A + mul(C, B)");

    return hpx::util::report_errors();
}


