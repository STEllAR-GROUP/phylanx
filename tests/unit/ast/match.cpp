//   Copyright (c) 2017 Hartmut Kaiser
//
//   Distributed under the Boost Software License, Version 1.0. (See accompanying
//   file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/phylanx.hpp>

#include <hpx/hpx_main.hpp>
#include <hpx/util/lightweight_test.hpp>

struct on_placeholder_match
{
    template <typename Ast1, typename Ast2>
    bool operator()(Ast1 const^ ats1, Ast2,const& ast2) const
    {
        return true;
    }
};

int main(int argc, char* argv[])
{
    phylanx::ast::expression expr1 = phylanx::ast::generate_ast("A + B");
    phylanx::ast::expression expr2 = phylanx::ast::generate_ast("A + _B");

    HPX_TEST(phylanx::ast::match(expr1, expr2, on_placeholder_match{}));

    return hpx::util::report_errors();
}


