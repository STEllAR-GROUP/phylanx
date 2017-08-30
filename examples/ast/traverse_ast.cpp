//   Copyright (c) 2001-2017 Hartmut Kaiser
//
//   Distributed under the Boost Software License, Version 1.0. (See accompanying
//   file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/phylanx.hpp>

#include <iostream>

struct traverse_ast : phylanx::ast::static_visitor
{
    template <typename T>
    bool operator()(T const& val) const
    {
        std::cout << val << std::endl;
        return true;
    }
};

int main(int argc, char* argv[])
{
    phylanx::ast::primary_expr<double> p1{
        phylanx::ir::node_data<double>{3.14}};

    phylanx::ast::operand<double> op1{p1};
    phylanx::ast::expression<double> e1{op1};

    phylanx::ast::operation<double> u1{
        phylanx::ast::optoken::op_plus, std::move(op1)};
    e1.append(u1);

    std::list<phylanx::ast::operation<double>> ops = {u1, u1};
    e1.append(ops);

    phylanx::ast::traverse(e1, traverse_ast{});

    return 0;
}
