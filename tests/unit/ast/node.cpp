//   Copyright (c) 2017 Hartmut Kaiser
//
//   Distributed under the Boost Software License, Version 1.0. (See accompanying
//   file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/phylanx.hpp>

#include <hpx/hpx_main.hpp>
#include <hpx/testing.hpp>

#include <cstdint>
#include <utility>
#include <vector>

#include <blaze/Math.h>

///////////////////////////////////////////////////////////////////////////////
template <typename Ast>
void test_serialization(Ast const& in)
{
    std::vector<char> buffer = phylanx::util::serialize(in);
    Ast out = phylanx::util::unserialize<Ast>(buffer);

    HPX_TEST_EQ(in, out);
}

///////////////////////////////////////////////////////////////////////////////
void test_identifier()
{
    phylanx::ast::identifier id("some_name");
    test_serialization(id);
}

///////////////////////////////////////////////////////////////////////////////
void test_primary_expr()
{
    phylanx::ast::primary_expr p1(true);
    test_serialization(p1);

    phylanx::ast::primary_expr p2(
        phylanx::ast::identifier("some_name"));
    test_serialization(p2);

    blaze::Rand<blaze::DynamicVector<double>> gen{};
    blaze::DynamicVector<double> v = gen.generate(1007UL);
    phylanx::ast::primary_expr p3{
        phylanx::ir::node_data<double>{v}};
    test_serialization(p3);

    phylanx::ast::primary_expr p4{"some string"};
    test_serialization(p4);

    phylanx::ast::primary_expr p5{std::uint64_t(42)};
    test_serialization(p5);
}

///////////////////////////////////////////////////////////////////////////////
void test_operand()
{
    phylanx::ast::primary_expr p1(true);
    phylanx::ast::operand op1(p1);
    test_serialization(op1);

    phylanx::ast::primary_expr p2(
        phylanx::ast::identifier{"some_name"});
    phylanx::ast::operand op2(std::move(p2));
    test_serialization(op2);

    phylanx::ast::primary_expr p3{"some string"};
    phylanx::ast::operand op3(std::move(p3));
    test_serialization(op3);

    phylanx::ast::primary_expr p4{std::uint64_t(42)};
    phylanx::ast::operand op4(std::move(p4));
    test_serialization(op4);
}

///////////////////////////////////////////////////////////////////////////////
void test_unary_expr()
{
    phylanx::ast::primary_expr p1(true);
    phylanx::ast::operand op1(p1);
    phylanx::ast::unary_expr u1(phylanx::ast::optoken::op_not, op1);
    test_serialization(u1);

    phylanx::ast::primary_expr p2(
        phylanx::ast::identifier{"some_name"});
    phylanx::ast::operand op2(p2);
    phylanx::ast::unary_expr u2(
        phylanx::ast::optoken::op_negative, std::move(op2));
    test_serialization(u2);
}

///////////////////////////////////////////////////////////////////////////////
void test_operation()
{
    phylanx::ast::primary_expr p1(true);
    phylanx::ast::operand op1(p1);
    phylanx::ast::operation u1(phylanx::ast::optoken::op_plus, op1);
    test_serialization(u1);

    phylanx::ast::primary_expr p2(
        phylanx::ast::identifier{"some_name"});
    phylanx::ast::operand op2(p2);
    phylanx::ast::operation u2(
        phylanx::ast::optoken::op_minus, std::move(op2));
    test_serialization(u2);
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

    test_serialization(e1);

    phylanx::ast::primary_expr p2(std::move(e1));
    test_serialization(p2);
}

void test_function_call()
{
    phylanx::ast::identifier id("function_name");
    phylanx::ast::function_call fc(std::move(id));

    phylanx::ast::primary_expr p1(true);
    phylanx::ast::operand op1(p1);
    phylanx::ast::expression e1(std::move(p1));

    fc.append(e1);

    std::vector<phylanx::ast::expression> exprs = {e1, e1};
    fc.append(exprs);

    test_serialization(fc);

    phylanx::ast::primary_expr p2(std::move(fc));
    test_serialization(p2);
}

void test_lists()
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
        phylanx::ast::expression{fc},
        phylanx::ast::expression{op1}
    };

    test_serialization(list);
}

///////////////////////////////////////////////////////////////////////////////
int main(int argc, char* argv[])
{
    test_identifier();
    test_primary_expr();
    test_operand();
    test_unary_expr();
    test_operation();
    test_expression();
    test_function_call();
    test_lists();

    return hpx::util::report_errors();
}

