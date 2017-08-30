//   Copyright (c) 2001-2017 Hartmut Kaiser
//
//   Distributed under the Boost Software License, Version 1.0. (See accompanying
//   file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/phylanx.hpp>

#include <hpx/include/serialization.hpp>
#include <hpx/util/lightweight_test.hpp>

#include <vector>

#include <Eigen/Dense>

///////////////////////////////////////////////////////////////////////////////
template <typename Ast>
void test_serialization(Ast const& in)
{
    std::vector<char> out_buffer;
    std::size_t archive_size = 0;

    {
        hpx::serialization::output_archive archive(out_buffer);
        archive << in;
        archive_size = archive.bytes_written();
    }

    Ast out;

    {
        hpx::serialization::input_archive archive(
            out_buffer, archive_size);

        archive >> out;
    }

    HPX_TEST(in == out);
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
    phylanx::ast::primary_expr<double> p1(true);
    test_serialization(p1);

    phylanx::ast::primary_expr<double> p2(
        phylanx::ast::identifier("some_name"));
    test_serialization(p2);

    Eigen::VectorXd v = Eigen::VectorXd::Random(1007);
    phylanx::ast::primary_expr<double> p3{
        phylanx::ir::node_data<double>{v}};
    test_serialization(p3);
}

///////////////////////////////////////////////////////////////////////////////
void test_operand()
{
    phylanx::ast::primary_expr<double> p1(true);
    phylanx::ast::operand<double> op1(p1);
    test_serialization(op1);

    phylanx::ast::primary_expr<double> p2(
        phylanx::ast::identifier{"some_name"});
    phylanx::ast::operand<double> op2(std::move(p2));
    test_serialization(op2);
}

///////////////////////////////////////////////////////////////////////////////
void test_unary_expr()
{
    phylanx::ast::primary_expr<double> p1(true);
    phylanx::ast::operand<double> op1(p1);
    phylanx::ast::unary_expr<double> u1(phylanx::ast::optoken::op_not, op1);
    test_serialization(u1);

    phylanx::ast::primary_expr<double> p2(
        phylanx::ast::identifier{"some_name"});
    phylanx::ast::operand<double> op2(p2);
    phylanx::ast::unary_expr<double> u2(
        phylanx::ast::optoken::op_negative, std::move(op2));
    test_serialization(u2);
}

///////////////////////////////////////////////////////////////////////////////
void test_operation()
{
    phylanx::ast::primary_expr<double> p1(true);
    phylanx::ast::operand<double> op1(p1);
    phylanx::ast::operation<double> u1(phylanx::ast::optoken::op_plus, op1);
    test_serialization(u1);

    phylanx::ast::primary_expr<double> p2(
        phylanx::ast::identifier{"some_name"});
    phylanx::ast::operand<double> op2(p2);
    phylanx::ast::operation<double> u2(
        phylanx::ast::optoken::op_minus, std::move(op2));
    test_serialization(u2);
}

///////////////////////////////////////////////////////////////////////////////
void test_expression()
{
    phylanx::ast::primary_expr<double> p1(true);
    phylanx::ast::operand<double> op1(p1);
    phylanx::ast::expression<double> e1(std::move(op1));

    phylanx::ast::operation<double> u1(phylanx::ast::optoken::op_plus, op1);
    e1.append(u1);

    std::list<phylanx::ast::operation<double>> ops = {u1, u1};
    e1.append(ops);

    test_serialization(e1);

    phylanx::ast::primary_expr<double> p2(std::move(e1));
    test_serialization(p2);
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

    return hpx::util::report_errors();
}

