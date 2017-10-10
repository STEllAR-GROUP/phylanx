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
    std::multimap<std::string, phylanx::ast::expression>& placeholders;

    template <typename Ast1, typename Ast2, typename ... Ts>
    bool operator()(Ast1 const& ast1, Ast2 const& ast2, Ts const&... ts) const
    {
        using value_type = typename std::multimap<std::string,
            phylanx::ast::expression>::value_type;

        if (phylanx::ast::detail::is_placeholder(ast1))
        {
            if (phylanx::ast::detail::is_placeholder_ellipses(ast1))
            {
                placeholders.insert(value_type(
                    phylanx::ast::detail::identifier_name(ast1).substr(1),
                    phylanx::ast::expression(ast2)));
            }
            else
            {
                placeholders.insert(
                    value_type(phylanx::ast::detail::identifier_name(ast1),
                        phylanx::ast::expression(ast2)));
            }
        }
        else if (phylanx::ast::detail::is_placeholder(ast2))
        {
            if (phylanx::ast::detail::is_placeholder_ellipses(ast2))
            {
                placeholders.insert(value_type(
                    phylanx::ast::detail::identifier_name(ast2).substr(1),
                    phylanx::ast::expression(ast1)));
            }
            else
            {
                placeholders.insert(
                    value_type(phylanx::ast::detail::identifier_name(ast2),
                        phylanx::ast::expression(ast1)));
            }
        }
        return true;
    }
};

void test_placeholder_matching(std::string const& to_match,
    std::string const& expr_to_match, std::string const& expected_match)
{
    phylanx::ast::expression match = phylanx::ast::generate_ast(to_match);
    phylanx::ast::expression expr = phylanx::ast::generate_ast(expr_to_match);

    std::multimap<std::string, phylanx::ast::expression> placeholders;
    HPX_TEST(phylanx::ast::match_ast(
        expr, match, on_placeholder_match{placeholders}));

    HPX_TEST(placeholders.size() == 1);
    auto r = placeholders.equal_range("_1");
    HPX_TEST(std::distance(r.first, r.second) > 0);
    HPX_TEST((*r.first).second == phylanx::ast::generate_ast(expected_match));
}

void test_placeholder_matching(std::string const& to_match,
    std::string const& expr_to_match, std::string const& expected_match1,
    std::string const& expected_match2)
{
    phylanx::ast::expression match = phylanx::ast::generate_ast(to_match);
    phylanx::ast::expression expr = phylanx::ast::generate_ast(expr_to_match);

    std::multimap<std::string, phylanx::ast::expression> placeholders;
    HPX_TEST(phylanx::ast::match_ast(
        expr, match, on_placeholder_match{placeholders}));

    HPX_TEST(placeholders.size() == 2);

    auto r1 = placeholders.equal_range("_1");
    HPX_TEST(std::distance(r1.first, r1.second) == 1);
    HPX_TEST((*r1.first).second == phylanx::ast::generate_ast(expected_match1));

    auto r2 = placeholders.equal_range("_2");
    HPX_TEST(std::distance(r2.first, r2.second) == 1);
    HPX_TEST((*r2.first).second == phylanx::ast::generate_ast(expected_match2));
}

void test_placeholder_matching_ellipses(std::string const& to_match,
    std::string const& expr_to_match, std::string const& expected_match,
    std::vector<std::string> const& expected_matches)
{
    phylanx::ast::expression match = phylanx::ast::generate_ast(to_match);
    phylanx::ast::expression expr = phylanx::ast::generate_ast(expr_to_match);

    std::multimap<std::string, phylanx::ast::expression> placeholders;
    HPX_TEST(phylanx::ast::match_ast(
        expr, match, on_placeholder_match{placeholders}));

    HPX_TEST(placeholders.size() == expected_matches.size() + 1);

    auto r1 = placeholders.equal_range("_1");
    HPX_TEST(std::distance(r1.first, r1.second) == 1);
    HPX_TEST((*r1.first).second == phylanx::ast::generate_ast(expected_match));

    auto r2 = placeholders.equal_range("_2");
    auto it = r2.first, end = r2.second;
    HPX_TEST(std::distance(it, end) == expected_matches.size());
    for (auto const& match : expected_matches)
    {
        HPX_TEST(it != end);
        HPX_TEST(it->second == phylanx::ast::generate_ast(match));
        ++it;
    }
}

int main(int argc, char* argv[])
{
    // one placeholder
    test_placeholder_matching("_1", "A + B", "A + B");
    test_placeholder_matching("_1", "(A + B)", "A + B");
    test_placeholder_matching("_1", "((A + B))", "A + B");

    test_placeholder_matching("A + _1", "A + B", "B");
    test_placeholder_matching("_1 + B", "A + B", "A");

    test_placeholder_matching("A + _1", "A + B * C", "B * C");
    test_placeholder_matching("A + _1", "A + (B * C)", "B * C");
    test_placeholder_matching("_1 + B * C", "A + B * C", "A");

    test_placeholder_matching("A + _1", "A + (B * C)", "B * C");
    test_placeholder_matching("_1 * C", "(A + B) * C", "A + B");

    test_placeholder_matching("func(_1)", "func(A)", "A");
    test_placeholder_matching("func(_1)", "func(-A)", "-A");
    test_placeholder_matching("func(_1)", "func(A + B)", "A + B");

    test_placeholder_matching("_1(A)", "func(A)", "func");

    test_placeholder_matching("!_1", "!A", "A");
    test_placeholder_matching("-_1", "-A", "A");

    // two placeholders
    test_placeholder_matching("_1 + _2", "A + B", "A", "B");
    test_placeholder_matching("_2 + _1", "A + B", "B", "A");

    test_placeholder_matching("_1 * _2", "(A + B) * (C + D)", "A + B", "C + D");
    test_placeholder_matching("_1 * _2", "((A + B) * (C + D))", "A + B", "C + D");

    test_placeholder_matching("A + _1 + _2", "A + B * C + D", "B * C", "D");
    test_placeholder_matching("A + _1 + _2", "(A + (B * C) + D)", "B * C", "D");

    test_placeholder_matching("func(_1, _2)", "func(A, B)", "A", "B");
    test_placeholder_matching("func(_1, _2)", "func(A, -B)", "A", "-B");
    test_placeholder_matching(
        "func(_1, _2)", "func(A + B, C * D)", "A + B", "C * D");
    test_placeholder_matching("_1(_2)", "func(A)", "func", "A");

    test_placeholder_matching("_1 + _2", "A + ((B - C) * D)", "A", "(B - C) * D");
    test_placeholder_matching("_1 * _2", "(A + (B - C)) * D", "A + (B - C)", "D");

    // using placeholder ellipses
    test_placeholder_matching_ellipses(
        "_1 + __2", "A + B + C", "A",
        std::vector<std::string>{"B", "C"});
    test_placeholder_matching_ellipses(
        "_1 + __2", "A + B + C + D", "A",
        std::vector<std::string>{"B", "C", "D"});

    test_placeholder_matching_ellipses(
        "func(_1, __2)", "func(A, B, C, D)", "A",
        std::vector<std::string>{"B", "C", "D"});

    return hpx::util::report_errors();
}


