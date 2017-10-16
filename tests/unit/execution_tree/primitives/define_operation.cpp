//   Copyright (c) 2017 Hartmut Kaiser
//
//   Distributed under the Boost Software License, Version 1.0. (See accompanying
//   file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/phylanx.hpp>

#include <hpx/hpx_main.hpp>
#include <hpx/util/lightweight_test.hpp>

void test_define_operation_var(char const* expr, char const* name,
    std::vector<char const*> const& args, char const* body)
{
    phylanx::execution_tree::variables variables;
    phylanx::execution_tree::functions functions;

    auto result = phylanx::execution_tree::detail::generate_tree(
        phylanx::ast::generate_ast(expr),
        phylanx::execution_tree::detail::generate_patterns(
            phylanx::execution_tree::get_all_known_patterns()),
        variables, functions);

    HPX_TEST_EQ(variables.size(), std::size_t(1));
    HPX_TEST_EQ(functions.size(), std::size_t(0));

    // verify function name
    auto p = functions.find(name);
    HPX_TEST(p.first == p.second);
}

void test_define_operation(char const* expr, char const* name,
    std::vector<char const*> const& args, char const* body)
{
    phylanx::execution_tree::variables variables;
    phylanx::execution_tree::functions functions;

    auto result = phylanx::execution_tree::detail::generate_tree(
        phylanx::ast::generate_ast(expr),
        phylanx::execution_tree::detail::generate_patterns(
            phylanx::execution_tree::get_all_known_patterns()),
        variables, functions);

    HPX_TEST_EQ(variables.size(), std::size_t(0));
    HPX_TEST_EQ(functions.size(), std::size_t(1));

    // verify function name
    auto p = functions.find(name);
    HPX_TEST(p.first != p.second);

    // verify arguments
    std::vector<phylanx::ast::expression> argexprs;
    argexprs.reserve(args.size());
    for (auto const& s : args)
    {
        argexprs.push_back(phylanx::ast::generate_ast(s));
    }

    HPX_TEST(p.first->second.first == argexprs);

    // verify body
    phylanx::ast::expression bodyexpr = phylanx::ast::generate_ast(body);
    HPX_TEST(p.first->second.second == bodyexpr);
}

int main(int argc, char* argv[])
{
    test_define_operation_var("define(x, 3.14)", "x", {}, "3.14");
    test_define_operation("define(y, x, x + 1)", "y", {"x"}, "x + 1");
    test_define_operation("define(add, x, y, x + y)", "add", {"x", "y"}, "x + y");

    return hpx::util::report_errors();
}

