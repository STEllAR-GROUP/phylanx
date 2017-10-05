//   Copyright (c) 2017 Hartmut Kaiser
//
//   Distributed under the Boost Software License, Version 1.0. (See accompanying
//   file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/phylanx.hpp>

#include <hpx/hpx_main.hpp>
#include <hpx/util/lightweight_test.hpp>

#include <cstddef>
#include <string>
#include <utility>
#include <vector>

#include <Eigen/Dense>

///////////////////////////////////////////////////////////////////////////////
template <typename Ast>
void test_to_string(Ast const& ast, std::string const& expected)
{
    phylanx::ast::expression expr(ast);
    HPX_TEST_EQ(phylanx::ast::to_string(expr), expected);
}

///////////////////////////////////////////////////////////////////////////////
void test_identifier()
{
    phylanx::ast::identifier id("some_name");
    test_to_string(id, "some_name");
}

///////////////////////////////////////////////////////////////////////////////
void test_primary_expr()
{
    phylanx::ast::primary_expr p1(true);
    test_to_string(p1, "true");

    phylanx::ast::primary_expr p2(
        phylanx::ast::identifier("some_name"));
    test_to_string(p2, "some_name");

    Eigen::VectorXd v = Eigen::VectorXd::Random(1007);
    phylanx::ast::primary_expr p3{
        phylanx::ir::node_data<double>{v}};
    std::string expected("[");
    for (std::size_t i = 0; i != v.size(); ++i)
    {
        if (i != 0)
            expected += ", ";
        expected += std::to_string(v[i]);
    }
    expected += "]";
    test_to_string(p3, expected);

    phylanx::ast::primary_expr p4{"some string"};
    test_to_string(p4, "\"some string\"");

    phylanx::ast::primary_expr p5{std::uint64_t(42)};
    test_to_string(p5, "42");
}

///////////////////////////////////////////////////////////////////////////////
void test_operand()
{
    phylanx::ast::primary_expr p1(false);
    phylanx::ast::operand op1(p1);
    test_to_string(op1, "false");

    phylanx::ast::primary_expr p2(
        phylanx::ast::identifier{"some_name"});
    phylanx::ast::operand op2(std::move(p2));
    test_to_string(op2, "some_name");

    phylanx::ast::primary_expr p3{"some string"};
    phylanx::ast::operand op3(std::move(p3));
    test_to_string(op3, "\"some string\"");

    phylanx::ast::primary_expr p4{std::uint64_t(42)};
    phylanx::ast::operand op4(std::move(p4));
    test_to_string(op4, "42");
}

///////////////////////////////////////////////////////////////////////////////
void test_unary_expr()
{
    phylanx::ast::primary_expr p1(true);
    phylanx::ast::operand op1(p1);
    phylanx::ast::unary_expr u1(phylanx::ast::optoken::op_not, op1);
    test_to_string(u1, "!true");

    phylanx::ast::primary_expr p2(
        phylanx::ast::identifier{"some_name"});
    phylanx::ast::operand op2(p2);
    phylanx::ast::unary_expr u2(
        phylanx::ast::optoken::op_negative, std::move(op2));
    test_to_string(u2, "-some_name");
}

///////////////////////////////////////////////////////////////////////////////
void test_expression()
{
    phylanx::ast::primary_expr p1(true);
    phylanx::ast::operand op1(p1);
    phylanx::ast::expression e1(op1);

    phylanx::ast::operation u1(phylanx::ast::optoken::op_plus, std::move(op1));
    e1.append(u1);

    std::list<phylanx::ast::operation> ops = {u1, u1};
    e1.append(ops);

    test_to_string(e1, "(true + true + true + true)");

    phylanx::ast::primary_expr p2(std::move(e1));
    test_to_string(p2, "(true + true + true + true)");
}

void test_function_call()
{
    phylanx::ast::identifier id("function_name");
    phylanx::ast::function_call fc(std::move(id));

    phylanx::ast::primary_expr p1(true);
    phylanx::ast::operand op1(p1);
    phylanx::ast::expression e1(std::move(p1));

    fc.append(e1);

    std::list<phylanx::ast::expression> exprs = {e1, e1};
    fc.append(exprs);

    test_to_string(fc, "function_name(true, true, true)");

    phylanx::ast::primary_expr p2(std::move(fc));
    test_to_string(p2, "function_name(true, true, true)");
}

///////////////////////////////////////////////////////////////////////////////
int main(int argc, char* argv[])
{
    test_identifier();
    test_primary_expr();
    test_operand();
    test_unary_expr();
    test_expression();
    test_function_call();

    return hpx::util::report_errors();
}

