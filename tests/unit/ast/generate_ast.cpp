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

    std::stringstream& strm_;
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
//     test_expression(
//         "A + B",
//         "expression\n"
//             "identifier: A\n"
//             "identifier: B\n"
//             "op_plus\n"
//     );

    test_expression(
        "A + B + -C",
        "expression\n"
            "identifier: A\n"
            "identifier: B\n"
            "op_plus\n"
            "identifier: C\n"
            "op_negative\n"
            "op_plus\n"
    );

    test_expression(
        "A + B * C",
        "expression\n"
            "identifier: A\n"
            "identifier: B\n"
            "identifier: C\n"
            "op_times\n"
            "op_plus\n"
    );

    test_expression(
        "A * B + C",
        "expression\n"
            "identifier: A\n"
            "identifier: B\n"
            "op_times\n"
            "identifier: C\n"
            "op_plus\n"
    );

    test_expression(
        "func(A, B)",
        "expression\n"
            "function_call\n"
                "identifier: func\n"
                "expression\n"
                    "identifier: A\n"
                "expression\n"
                    "identifier: B\n"
    );

    return hpx::util::report_errors();
}
