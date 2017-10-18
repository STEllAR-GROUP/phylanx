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

    auto const& patterns = phylanx::execution_tree::get_all_known_patterns();
    phylanx::execution_tree::functions functions(
        phylanx::execution_tree::detail::builtin_functions(patterns));

    std::size_t num_functions = functions.size();

    auto result = phylanx::execution_tree::detail::generate_tree(
        phylanx::ast::generate_ast(expr),
        phylanx::execution_tree::detail::generate_patterns(
            phylanx::execution_tree::get_all_known_patterns()),
        variables, functions, hpx::find_here());

    HPX_TEST_EQ(variables.size(), std::size_t(1));
    HPX_TEST_EQ(functions.size(), num_functions);

    // verify function name
    auto p = functions.find(name);
    HPX_TEST(p.first == p.second);
}

void test_define_operation(char const* expr, char const* name,
    std::vector<char const*> const& args, char const* body)
{
    phylanx::execution_tree::variables variables;

    auto const& patterns = phylanx::execution_tree::get_all_known_patterns();
    phylanx::execution_tree::functions functions(
        phylanx::execution_tree::detail::builtin_functions(patterns));

    std::size_t num_functions = functions.size();

    auto result = phylanx::execution_tree::detail::generate_tree(
        phylanx::ast::generate_ast(expr),
        phylanx::execution_tree::detail::generate_patterns(
            phylanx::execution_tree::get_all_known_patterns()),
        variables, functions, hpx::find_here());

    HPX_TEST_EQ(variables.size(), std::size_t(0));
    HPX_TEST_EQ(functions.size(), num_functions + 1);

    auto p = functions.find(name);
    HPX_TEST(p.first != p.second);

    // verify name, arguments, and body
    std::vector<phylanx::ast::expression> argexprs;
    argexprs.reserve(args.size() + 2);

    argexprs.push_back(
        phylanx::ast::expression(phylanx::ast::identifier(name)));
    for (auto const& s : args)
    {
        argexprs.push_back(phylanx::ast::generate_ast(s));
    }
    argexprs.push_back(phylanx::ast::generate_ast(body));

    HPX_TEST(p.first->second.ast() == argexprs);
}

int main(int argc, char* argv[])
{
    test_define_operation_var("define(x, 3.14)", "x", {}, "3.14");
    test_define_operation("define(y, x, x + 1)", "y", {"x"}, "x + 1");
    test_define_operation("define(add, x, y, x + y)", "add", {"x", "y"}, "x + y");

    return hpx::util::report_errors();
}

