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
        using value_type = typename std::map<std::string,
            phylanx::ast::expression>::value_type;

        if (phylanx::ast::is_placeholder(ast1))
        {
            placeholders.insert(value_type(phylanx::ast::placeholder_name(ast1),
                phylanx::ast::expression(ast2)));
        }
        else if (phylanx::ast::is_placeholder(ast2))
        {
            placeholders.insert(value_type(phylanx::ast::placeholder_name(ast2),
                phylanx::ast::expression(ast1)));
        }
        return true;
    }
};

void test_placeholder_matching(std::string const& to_match,
    std::string const& expr_to_match, std::string const& expected_match)
{
    phylanx::ast::expression match = phylanx::ast::generate_ast(to_match);
    phylanx::ast::expression expr = phylanx::ast::generate_ast(expr_to_match);

    std::map<std::string, phylanx::ast::expression> placeholders;
    HPX_TEST(phylanx::ast::match(
        expr, match, on_placeholder_match{placeholders}));

    HPX_TEST(placeholders.size() == 1);
    HPX_TEST(placeholders.find("_1") != placeholders.end());
    HPX_TEST(placeholders["_1"] == phylanx::ast::generate_ast(expected_match));
}

void test_placeholder_matching(std::string const& to_match,
    std::string const& expr_to_match, std::string const& expected_match1,
    std::string const& expected_match2)
{
    phylanx::ast::expression match = phylanx::ast::generate_ast(to_match);
    phylanx::ast::expression expr = phylanx::ast::generate_ast(expr_to_match);

    std::map<std::string, phylanx::ast::expression> placeholders;
    HPX_TEST(phylanx::ast::match(
        expr, match, on_placeholder_match{placeholders}));

    HPX_TEST(placeholders.size() == 2);
    HPX_TEST(placeholders.find("_1") != placeholders.end());
    HPX_TEST(placeholders.find("_2") != placeholders.end());
    HPX_TEST(placeholders["_1"] == phylanx::ast::generate_ast(expected_match1));
    HPX_TEST(placeholders["_2"] == phylanx::ast::generate_ast(expected_match2));
}

int main(int argc, char* argv[])
{
    // one placeholder
    test_placeholder_matching("A + _1", "A + B", "B");
    test_placeholder_matching("_1 + B", "A + B", "A");

    test_placeholder_matching("A + _1", "A + B * C", "B * C");
    test_placeholder_matching("_1 + B * C", "A + B * C", "A");

    test_placeholder_matching("A + _1", "A + (B * C)", "B * C");
    test_placeholder_matching("_1 * C", "(A + B) * C", "A + B");

    // two placeholders
    test_placeholder_matching("_1 + _2", "A + B", "A", "B");
    test_placeholder_matching("_2 + _1", "A + B", "B", "A");

    test_placeholder_matching("_1 * _2", "(A + B) * (C + D)", "A + B", "C + D");

    test_placeholder_matching("A + _1 + _2", "A + B * C + D", "B * C", "D");

    return hpx::util::report_errors();
}


