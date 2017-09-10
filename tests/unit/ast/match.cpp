//   Copyright (c) 2017 Hartmut Kaiser
//
//   Distributed under the Boost Software License, Version 1.0. (See accompanying
//   file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/phylanx.hpp>

#include <hpx/hpx_main.hpp>
#include <hpx/util/lightweight_test.hpp>

#include <map>
#include <string>

struct on_placeholder_match
{
    std::map<std::string, phylanx::ast::expression>& placeholders;

    template <typename Ast1, typename Ast2, typename ... Ts>
    bool operator()(Ast1 const& ast1, Ast2 const& ast2, Ts const&... ts) const
    {
        return true;
    }

    template <typename... Ts>
    bool operator()(phylanx::ast::identifier const& id1,
        phylanx::ast::identifier const& id2, Ts const&... ts) const
    {
        using value_type = typename std::map<std::string,
            phylanx::ast::expression>::value_type;

        if (phylanx::ast::is_placeholder(id1))
        {
            placeholders.insert(
                value_type(id1.name, phylanx::ast::expression(id2)));
        }
        else if (phylanx::ast::is_placeholder(id2))
        {
            placeholders.insert(
                value_type(id2.name, phylanx::ast::expression(id1)));
        }
        return true;
    }
};

int main(int argc, char* argv[])
{
    phylanx::ast::expression to_match = phylanx::ast::generate_ast("A + _1");

    {
        phylanx::ast::expression expr = phylanx::ast::generate_ast("A + B");

        std::map<std::string, phylanx::ast::expression> placeholders;
        HPX_TEST(phylanx::ast::match(
            expr, to_match, on_placeholder_match{placeholders}));

        HPX_TEST(placeholders.size() == 1);
        HPX_TEST(placeholders.find("_1") != placeholders.end());
        HPX_TEST(placeholders["_1"] == phylanx::ast::generate_ast("B"));
    }

    {
        phylanx::ast::expression expr = phylanx::ast::generate_ast("A + B * C");

        std::map<std::string, phylanx::ast::expression> placeholders;
        HPX_TEST(phylanx::ast::match(
            expr, to_match, on_placeholder_match{placeholders}));

        HPX_TEST(placeholders.size() == 1);
        HPX_TEST(placeholders.find("_1") != placeholders.end());
        HPX_TEST(placeholders["_1"] == phylanx::ast::generate_ast("B * C"));
    }

    return hpx::util::report_errors();
}


