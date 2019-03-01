//   Copyright (c) 2017 Hartmut Kaiser
//
//   Distributed under the Boost Software License, Version 1.0. (See accompanying
//   file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/phylanx.hpp>

#include <hpx/hpx_main.hpp>
#include <hpx/util/lightweight_test.hpp>

#include <cstddef>
#include <cstdint>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

#include <blaze/Math.h>

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

    blaze::Rand<blaze::DynamicVector<double>> gen{};
    blaze::DynamicVector<double> v = gen.generate(1007UL);
    phylanx::ast::primary_expr p3{
        phylanx::ir::node_data<double>{v}};
    std::stringstream strm;
    strm << p3;
    test_to_string(p3, strm.str());

    phylanx::ast::primary_expr p4{"some string"};
    test_to_string(p4, "some string");

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
    test_to_string(op3, "some string");

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

    std::vector<phylanx::ast::operation> ops = {u1, u1};
    e1.append(ops);

    test_to_string(e1, "(true + true + true + true)");

    phylanx::ast::primary_expr p2(std::move(e1));
    test_to_string(p2, "(true + true + true + true)");
}

void test_function_call()
{
    phylanx::ast::identifier id("function_name");
    phylanx::ast::function_call fc(id);

    phylanx::ast::primary_expr p1(true);
    phylanx::ast::operand op1(p1);
    phylanx::ast::expression e1(std::move(p1));

    fc.append(e1);

    std::vector<phylanx::ast::expression> exprs = {e1, e1};
    fc.append(exprs);

    test_to_string(fc, "function_name(true, true, true)");

    phylanx::ast::function_call fc1(id, "attribute", std::move(exprs));
    test_to_string(fc1, "function_name{attribute}(true, true)");

    phylanx::ast::primary_expr p2(std::move(fc));
    test_to_string(p2, "function_name(true, true, true)");
}

void test_list()
{
    phylanx::ast::identifier id("function_name");
    phylanx::ast::function_call fc(std::move(id));

    phylanx::ast::primary_expr p1(true);
    phylanx::ast::operand op1(p1);
    phylanx::ast::expression e1(std::move(p1));

    fc.append(e1);

    std::vector<phylanx::ast::expression> exprs = {e1, e1};
    fc.append(exprs);

    std::vector<phylanx::ast::expression> list = {
        phylanx::ast::expression{op1},
        phylanx::ast::expression{fc}
    };

    test_to_string(list, "list(true, function_name(true, true, true))");
}

void test_vector()
{
    std::vector<double> v = {1.0, 2.0, 3.0};
    phylanx::ast::primary_expr p1(std::move(v));
    phylanx::ast::operand op1(p1);
    phylanx::ast::expression e1(std::move(p1));

    test_to_string(e1, "[1, 2, 3]");
}

void test_matrix()
{
    std::vector<double> v1 = {1.0, 2.0, 3.0};
    std::vector<double> v2 = {2.0, 3.0, 1.0};
    std::vector<double> v3 = {3.0, 1.0, 2.0};

    std::vector<std::vector<double>> v = {v1, v2, v3};

    phylanx::ast::primary_expr p1(std::move(v));
    phylanx::ast::operand op1(p1);
    phylanx::ast::expression e1(std::move(p1));

    test_to_string(e1, "[[1, 2, 3], [2, 3, 1], [3, 1, 2]]");
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
    test_list();
    test_vector();
    test_matrix();

    return hpx::util::report_errors();
}

