//   Copyright (c) 2017 Hartmut Kaiser
//
//   Distributed under the Boost Software License, Version 1.0. (See accompanying
//   file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/phylanx.hpp>

#include <hpx/hpx_main.hpp>
#include <hpx/include/serialization.hpp>
#include <hpx/util/lightweight_test.hpp>

#include <iostream>
#include <strstream>

struct traverse_ast
{
    template <typename T>
    bool operator()(T const& val) const
    {
        strm << val << '\n';
        return true;
    }

    std::stringstream& strm;
};

void test_expression(std::string const& expr, std::string const& expected)
{
    phylanx::ast::expression ast = phylanx::ast::generate_ast(expr);
    std::stringstream strm;
    phylanx::ast::traverse(ast, traverse_ast{strm});
    HPX_TEST_EQ(strm.str(), expected);
}

int main(int argc, char* argv[])
{
    test_expression(
        "A + B",
        "expression\n"
            "operand\n"
                "primary_expr\n"
                    "identifier: A\n"
            "operation\n"
                "op_plus\n"
            "operand\n"
                "primary_expr\n"
                    "identifier: B\n"
    );

    test_expression(
        "A + B + -C",
        "expression\n"
            "operand\n"
                "primary_expr\n"
                    "identifier: A\n"
            "operation\n"
                "op_plus\n"
            "operand\n"
                "primary_expr\n"
                    "identifier: B\n"
            "operation\n"
                "op_plus\n"
            "operand\n"
                "unary_expr\n"
                    "op_negative\n"
                    "operand\n"
                        "primary_expr\n"
                            "identifier: C\n"
    );

    return hpx::util::report_errors();
}
