//  Copyright (c) 2017 Hartmut Kaiser
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>
#include <phylanx/ast/generate_ast.hpp>
#include <phylanx/ast/match_ast.hpp>
#include <phylanx/ast/detail/is_identifier.hpp>
#include <phylanx/ast/detail/is_literal_value.hpp>
#include <phylanx/ast/detail/is_placeholder.hpp>
#include <phylanx/execution_tree/generate_tree.hpp>
#include <phylanx/execution_tree/primitives/base_primitive.hpp>
#include <phylanx/execution_tree/primitives/literal_value.hpp>
#include <phylanx/ir/node_data.hpp>

#include <hpx/throw_exception.hpp>
#include <hpx/util/tuple.hpp>

#include <map>
#include <string>
#include <utility>
#include <vector>

namespace phylanx { namespace execution_tree
{
    namespace detail
    {
        ///////////////////////////////////////////////////////////////////////
        using expression_pattern_list =
            std::vector<
                hpx::util::tuple<std::string, ast::expression, factory_function_type
            >>;

        expression_pattern_list generate_patterns(pattern_list const& patterns)
        {
            expression_pattern_list result;
            result.reserve(patterns.size());

            for (auto const& pattern : patterns)
            {
                result.push_back(hpx::util::make_tuple(pattern.first,
                    ast::generate_ast(pattern.first), pattern.second));
            }

            return result;
        }

        ///////////////////////////////////////////////////////////////////////
        struct on_placeholder_match
        {
            std::multimap<std::string, ast::expression>& placeholders;

            template <typename Ast1, typename Ast2, typename... Ts>
            bool operator()(
                Ast1 const& ast1, Ast2 const& ast2, Ts const&... ts) const
            {
                using value_type = typename std::multimap<std::string,
                    ast::expression>::value_type;

                if (ast::detail::is_placeholder(ast1))
                {
                    if (ast::detail::is_placeholder_ellipses(ast1))
                    {
                        placeholders.insert(value_type(
                            ast::detail::identifier_name(ast1).substr(1),
                            ast::expression(ast2)));
                    }
                    else
                    {
                        placeholders.insert(
                            value_type(ast::detail::identifier_name(ast1),
                                ast::expression(ast2)));
                    }
                }
                else if (ast::detail::is_placeholder(ast2))
                {
                    if (ast::detail::is_placeholder_ellipses(ast1))
                    {
                        placeholders.insert(value_type(
                            ast::detail::identifier_name(ast2).substr(1),
                            ast::expression(ast1)));
                    }
                    else
                    {
                        placeholders.insert(
                            value_type(ast::detail::identifier_name(ast2),
                                ast::expression(ast1)));
                    }
                }
                return true;
            }
        };

        primitive generate_tree(
            ast::expression const& expr,
            expression_pattern_list const& patterns,
            phylanx::execution_tree::variables const& variables)
        {
            primitive result;
            for (auto const& pattern : patterns)
            {
                std::multimap<std::string, ast::expression> placeholders;
                if (!ast::match_ast(expr, hpx::util::get<1>(pattern),
                        on_placeholder_match{placeholders}))
                {
                    continue;
                }

                std::vector<primitive_argument_type> arguments;
                arguments.reserve(placeholders.size());

                for (auto const& placeholder : placeholders)
                {
                    if (ast::detail::is_literal_value(placeholder.second))
                    {
                        arguments.push_back(
                            to_primitive_value_type(
                                ast::detail::literal_value(placeholder.second)
                            ));
                    }
                    else
                    {
                        arguments.push_back(generate_tree(
                            placeholder.second, patterns, variables));
                    }
                }

                // create primitive with given arguments
                result = hpx::util::get<2>(pattern)(
                    hpx::find_here(), std::move(arguments));

                return result;
            }

            // remaining expression could refer to a variable
            if (ast::detail::is_identifier(expr))
            {
                auto it = variables.find(ast::detail::identifier_name(expr));
                if (it != variables.end())
                {
                    return it->second;
                }
            }

            // otherwise the match was not complete, bail out
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "phylanx::execution_tree::generate_tree",
                "couldn't fully pattern-match the given expression: " +
                    ast::to_string(expr));
        }
    }

    ///////////////////////////////////////////////////////////////////////////
    primitive generate_tree(ast::expression const& expr,
        pattern_list const& patterns,
        phylanx::execution_tree::variables const& variables)
    {
        return detail::generate_tree(
            expr, detail::generate_patterns(patterns), variables);
    }

    primitive generate_tree(std::string const& exprstr,
        pattern_list const& patterns,
        phylanx::execution_tree::variables const& variables)
    {
        return detail::generate_tree(ast::generate_ast(exprstr),
            detail::generate_patterns(patterns), variables);
    }
}}

