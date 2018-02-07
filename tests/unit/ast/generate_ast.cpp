//   Copyright (c) 2017-2018 Hartmut Kaiser
//
//   Distributed under the Boost Software License, Version 1.0. (See accompanying
//   file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/phylanx.hpp>

#include <hpx/hpx_main.hpp>
#include <hpx/include/serialization.hpp>
#include <hpx/util/lightweight_test.hpp>

#include <cstddef>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

///////////////////////////////////////////////////////////////////////////////
template <typename Ast>
void test_ast(std::string const& exprstr, Ast const& expected)
{
    auto exprs = phylanx::ast::generate_ast(exprstr);
    HPX_TEST_EQ(exprs.size(), std::size_t(1));
    HPX_TEST_EQ(exprs[0], phylanx::ast::expression(expected));
}

///////////////////////////////////////////////////////////////////////////////
struct traverse_ast
{
    traverse_ast(std::stringstream& strm)
      : strm_(strm)
    {
    }

    template <typename T>
    bool operator()(T const& val) const
    {
        strm_ << val << '\n';
        return true;
    }

    // skip expressions
    bool operator()(phylanx::ast::expression const& val) const
    {
        return true;
    }

    // skip function calls
    bool operator()(phylanx::ast::function_call const& val) const
    {
        return true;
    }

    // skip lists
    bool operator()(std::vector<phylanx::ast::expression> const& val) const
    {
        return true;
    }

    std::stringstream& strm_;
};

struct traverse_ast_enter_exit
{
    traverse_ast_enter_exit(std::stringstream& strm)
      : strm_(strm)
    {
    }

    template <typename T, typename ... Ts>
    bool on_enter(T && val, Ts const&... ts) const
    {
        strm_ << val << '\n';
        return true;
    }

    // skip expressions
    template <typename ... Ts>
    bool on_enter(phylanx::ast::expression const& val, Ts const&... ts) const
    {
        return true;
    }

    // skip function calls
    template <typename ... Ts>
    bool on_enter(phylanx::ast::function_call const& val, Ts const&... ts) const
    {
        return true;
    }

    // skip lists
    template <typename... Ts>
    bool on_enter(std::vector<phylanx::ast::expression> const& val,
        Ts const&... ts) const
    {
        return true;
    }

    template <typename T, typename ... Ts>
    bool on_exit(T && val, Ts const&... ts) const
    {
        return true;
    }

    std::stringstream& strm_;
};

void test_expression(std::string const& expr, std::string const& expected)
{
    {
        auto ast = phylanx::ast::generate_ast(expr);
        HPX_TEST_EQ(ast.size(), std::size_t(1));
        std::stringstream strm;
        strm << std::boolalpha;
        phylanx::ast::traverse(ast[0], traverse_ast{strm});
        HPX_TEST_EQ(strm.str(), expected);
    }
    {
        auto ast = phylanx::ast::generate_ast(expr);
        HPX_TEST_EQ(ast.size(), std::size_t(1));
        std::stringstream strm;
        strm << std::boolalpha;
        phylanx::ast::traverse(ast[0], traverse_ast_enter_exit{strm});
        HPX_TEST_EQ(strm.str(), expected);
    }
}

int main(int argc, char* argv[])
{
    test_ast("A", phylanx::ast::identifier("A"));
    test_ast("A#1#2", phylanx::ast::identifier("A", 1, 2));

    test_ast("A()",
        phylanx::ast::function_call(phylanx::ast::identifier("A")));
    test_ast("A#1#2()",
        phylanx::ast::function_call(phylanx::ast::identifier("A", 1, 2)));

    test_expression(
        "A + B",
            "A\n"
            "B\n"
            "+\n"
    );

    test_expression(
        "A#1#0 + B#1#5",
            "A#1#0\n"
            "B#1#5\n"
            "+\n"
    );

    test_expression(
        "A + B + -C",
            "A\n"
            "B\n"
            "+\n"
            "C\n"
            "-\n"
            "+\n"
    );

    test_expression(
        "A + B * C",
            "A\n"
            "B\n"
            "C\n"
            "*\n"
            "+\n"
    );

    test_expression(
        "A * B + C",
            "A\n"
            "B\n"
            "*\n"
            "C\n"
            "+\n"
    );

    test_expression(
        "func(A, B)",
            "func\n"
            "A\n"
            "B\n"
    );

    test_expression(
        "func#1#0(A#1#6, B#1#9)",
            "func#1#0\n"
            "A#1#6\n"
            "B#1#9\n"
    );

    test_expression(
        "\"test\"",
            "test\n"
    );

    test_expression(
        "1.0 / (1.0 + exp(-dot(A, B)))",
            "1\n"
            "1\n"
            "exp\n"
            "dot\n"
            "A\n"
            "B\n"
            "-\n"
            "+\n"
            "/\n"
    );

    test_expression(
        "'()",
            ""
    );

    test_expression(
        "'(true, 1, 1.0, A, A + B)",
            "true\n"
            "1\n"
            "1\n"
            "A\n"
            "A\n"
            "B\n"
            "+\n"
    );

    test_expression(
        "'(true, 1, '(1.0, A, A + B))",
            "true\n"
            "1\n"
            "1\n"
            "A\n"
            "A\n"
            "B\n"
            "+\n"
    );

    test_expression(
        "[1.0, 2.0, 3.0]",
        "[1, 2, 3]\n"
    );

    test_expression(
        "[[1.0, 2.0, 3.0], [2.0, 3.0, 1.0], [3.0, 1.0, 2.0]]",
        "[[1, 2, 3], [2, 3, 1], [3, 1, 2]]\n"
    );

    return hpx::util::report_errors();
}
