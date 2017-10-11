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
#include <phylanx/ast/detail/is_placeholder_ellipses.hpp>
#include <phylanx/execution_tree/generate_tree.hpp>
#include <phylanx/execution_tree/primitives.hpp>
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

        primitive_argument_type generate_tree(
            ast::expression const& expr,
            expression_pattern_list const& patterns,
            phylanx::execution_tree::variables& variables)
        {
            primitive_argument_type result;
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
                std::string name = ast::detail::identifier_name(expr);
                auto it = variables.find(name);
                if (it != variables.end())
                {
                    if (!is_primitive_operand(it->second))
                    {
                        // create a new variable from the given value, replace
                        // entry in symbol table
                        it->second =
                            hpx::new_<primitives::variable>(hpx::find_here(),
                                std::move(it->second), std::move(name));
                    }
                    return it->second;
                }
                else
                {
                    // create an empty variable
                    primitive p = hpx::new_<primitives::variable>(
                        hpx::find_here(), name);

                    // attempt to insert the new variable into the symbol
                    // table
                    auto r = variables.insert(
                        execution_tree::variables::value_type(
                            std::move(name), p));
                    if (!r.second)
                    {
                        HPX_THROW_EXCEPTION(hpx::bad_parameter,
                            "phylanx::execution_tree::generate_tree",
                            "couldn't insert variable into symbol "
                                "table: " + name);
                    }
                    return p;
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
    pattern_list const& get_all_known_patterns()
    {
        static pattern_list patterns = {
            // variadic functions
            primitives::block_operation::match_data,
            primitives::parallel_block_operation::match_data,
            // binary functions
            primitives::dot_operation::match_data,
            primitives::file_read::match_data,
            primitives::file_write::match_data,
            primitives::while_operation::match_data,
            // unary functions
            primitives::constant::match_data,
            primitives::determinant::match_data,
            primitives::exponential_operation::match_data,
            primitives::inverse_operation::match_data,
            primitives::transpose_operation::match_data,
            primitives::random::match_data,
            // variadic operations
            primitives::add_operation::match_data,
            primitives::and_operation::match_data,
            primitives::div_operation::match_data,
            primitives::mul_operation::match_data,
            primitives::or_operation::match_data,
            primitives::sub_operation::match_data,
            // binary operations
            primitives::equal::match_data,
            primitives::greater::match_data,
            primitives::greater_equal::match_data,
            primitives::less::match_data,
            primitives::less_equal::match_data,
            primitives::not_equal::match_data,
            primitives::store_operation::match_data,
            // unary operations
            primitives::unary_minus_operation::match_data,
            primitives::unary_not_operation::match_data
        };

        return patterns;
    }

    ///////////////////////////////////////////////////////////////////////////
    primitive_argument_type generate_tree(std::string const& exprstr)
    {
        phylanx::execution_tree::variables vars;
        return detail::generate_tree(ast::generate_ast(exprstr),
            detail::generate_patterns(get_all_known_patterns()), vars);
    }

    primitive_argument_type generate_tree(std::string const& exprstr,
        phylanx::execution_tree::variables const& variables)
    {
        phylanx::execution_tree::variables vars(variables);
        return detail::generate_tree(ast::generate_ast(exprstr),
            detail::generate_patterns(get_all_known_patterns()), vars);
    }

    primitive_argument_type generate_tree(ast::expression const& expr,
        pattern_list const& patterns,
        phylanx::execution_tree::variables const& variables)
    {
        phylanx::execution_tree::variables vars(variables);
        return detail::generate_tree(
            expr, detail::generate_patterns(patterns), vars);
    }

    primitive_argument_type generate_tree(std::string const& exprstr,
        pattern_list const& patterns,
        phylanx::execution_tree::variables const& variables)
    {
        phylanx::execution_tree::variables vars(variables);
        return detail::generate_tree(ast::generate_ast(exprstr),
            detail::generate_patterns(patterns), vars);
    }
}}

