//   Copyright (c) 2017 Hartmut Kaiser
//
//   Distributed under the Boost Software License, Version 1.0. (See accompanying
//   file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/phylanx.hpp>

#include <hpx/hpx_main.hpp>
#include <hpx/include/serialization.hpp>
#include <hpx/util/lightweight_test.hpp>

#include <iostream>
#include <sstream>

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
        phylanx::ast::expression ast = phylanx::ast::generate_ast(expr);
        std::stringstream strm;
        phylanx::ast::traverse(ast, traverse_ast{strm});
        HPX_TEST_EQ(strm.str(), expected);
    }
    {
        phylanx::ast::expression ast = phylanx::ast::generate_ast(expr);
        std::stringstream strm;
        phylanx::ast::traverse(ast, traverse_ast_enter_exit{strm});
        HPX_TEST_EQ(strm.str(), expected);
    }
}

int main(int argc, char* argv[])
{
    test_expression(
        "A + B",
            "A\n"
            "B\n"
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
        "\"test\"",
            "test\n"
    );

    test_expression(
        "1.0 / (1.0 + exp(-dot(A, B)))",
            "1.000000\n"
            "1.000000\n"
            "exp\n"
            "dot\n"
            "A\n"
            "B\n"
            "-\n"
            "+\n"
            "/\n"
    );

    return hpx::util::report_errors();
}
