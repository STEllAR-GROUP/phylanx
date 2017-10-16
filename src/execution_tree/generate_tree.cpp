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

#include <algorithm>
#include <cstddef>
#include <map>
#include <string>
#include <utility>
#include <vector>

namespace phylanx { namespace execution_tree
{
    namespace detail
    {
        ///////////////////////////////////////////////////////////////////////
        expression_pattern_list generate_patterns(
            pattern_list const& patterns_list)
        {
            expression_pattern_list result;
            result.reserve(patterns_list.size());

            for (auto const& patterns : patterns_list)
            {
                for (auto const& pattern : patterns)
                {
                    result.push_back(
                        hpx::util::make_tuple(
                            hpx::util::get<0>(pattern),
                            hpx::util::get<1>(pattern),
                            ast::generate_ast(hpx::util::get<1>(pattern)),
                            hpx::util::get<2>(pattern)));
                }
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

        ///////////////////////////////////////////////////////////////////////
        primitive_argument_type handle_placeholders(
            std::multimap<std::string, ast::expression>& placeholders,
            phylanx::execution_tree::variables& variables,
            phylanx::execution_tree::functions& functions,
            expression_pattern_list const& patterns,
            expression_pattern const& pattern)
        {
            std::vector<primitive_argument_type> arguments;
            arguments.reserve(placeholders.size());

            for (auto const& placeholder : placeholders)
            {
                if (ast::detail::is_literal_value(placeholder.second))
                {
                    arguments.push_back(to_primitive_value_type(
                        ast::detail::literal_value(placeholder.second)));
                }
                else
                {
                    arguments.push_back(generate_tree(
                        placeholder.second, patterns, variables, functions));
                }
            }

            // create primitive with given arguments
            return hpx::util::get<3>(pattern)(
                hpx::find_here(), std::move(arguments), variables, functions);
        }

        ///////////////////////////////////////////////////////////////////////
        primitive_argument_type handle_variable(ast::expression const& expr,
            phylanx::execution_tree::variables& variables,
            phylanx::execution_tree::functions& functions)
        {
            std::string name = ast::detail::identifier_name(expr);
            auto p = variables.find(name);
            if (is_empty_range(p))
            {
                if (!is_primitive_operand(p.first->second))
                {
                    // create a new variable from the given value, replace
                    // entry in symbol table
                    p.first->second =
                        hpx::new_<primitives::variable>(hpx::find_here(),
                            std::move(p.first->second), std::move(name));
                }
                return p.first->second;
            }
            else if (is_empty_range(functions.find(name)))
            {
                return primitive_argument_type{std::move(name)};
            }
            else
            {
                // create an empty variable
                primitive p =
                    hpx::new_<primitives::variable>(hpx::find_here(), name);

                // attempt to insert the new variable into the symbol table
                auto r = variables.insert(
                    execution_tree::variables::value_type(std::move(name), p));
                if (!r.second)
                {
                    HPX_THROW_EXCEPTION(hpx::bad_parameter,
                        "phylanx::execution_tree::extract_arguments",
                        "couldn't insert variable into symbol table: " + name);
                }
                return p;
            }
        }

        ///////////////////////////////////////////////////////////////////////
        template <typename Iterator>
        ast::expression extract_name(std::pair<Iterator, Iterator> const& p)
        {
            if (std::distance(p.first, p.second) < 2)
            {
                HPX_THROW_EXCEPTION(hpx::bad_parameter,
                    "phylanx::execution_tree::detail::extract_name",
                    "the define() operation requires at least 2 arguments");
            }

            if (!ast::detail::is_identifier(p.first->second))
            {
                HPX_THROW_EXCEPTION(hpx::bad_parameter,
                    "phylanx::execution_tree::detail::extract_name",
                    "the define() operation requires that the name of the "
                        "function to define is represented as a variable name "
                        "(not an expression)");
            }

            return p.first->second;
        }

        template <typename Iterator>
        std::vector<ast::expression> extract_arguments(
            std::pair<Iterator, Iterator> const& p)
        {
            std::ptrdiff_t size = std::distance(p.first, p.second);
            if (size < 2)
            {
                HPX_THROW_EXCEPTION(hpx::bad_parameter,
                    "phylanx::execution_tree::detail::extract_arguments",
                    "the define() operation requires at least 2 arguments");
            }

            std::vector<ast::expression> args;
            args.reserve(size - 2);

            auto first = p.first; ++first;
            auto last = p.second; --last;
            for (auto it = first; it != last; ++it)
            {
                if (!ast::detail::is_identifier(it->second))
                {
                    HPX_THROW_EXCEPTION(hpx::bad_parameter,
                        "phylanx::execution_tree::detail::extract_arguments",
                        "the define() operation requires that all arguments "
                            "are represented as variable names (not "
                            "expressions)");
                }
                args.push_back(it->second);
            }

            return args;
        }

        template <typename Iterator>
        ast::expression extract_body(std::pair<Iterator, Iterator> const& p)
        {
            if (std::distance(p.first, p.second) < 2)
            {
                HPX_THROW_EXCEPTION(hpx::bad_parameter,
                    "phylanx::execution_tree::detail::extract_body",
                    "the define() operation requires at least 2 arguments");
            }

            auto last = p.second; --last;
            return last->second;
        }

        primitive_argument_type handle_define_variable(
            ast::expression && nameexpr, ast::expression && bodyexpr,
            phylanx::execution_tree::variables& variables,
            phylanx::execution_tree::functions& functions,
            expression_pattern_list const& patterns)
        {
            std::string name = ast::detail::identifier_name(nameexpr);
            auto pv = variables.find(name);
            if (!is_empty_range(pv))
            {
                HPX_THROW_EXCEPTION(hpx::bad_parameter,
                    "phylanx::execution_tree::detail::handle_define_variable",
                    "the define() operation attempts to redefine a "
                        "variable: " + name);
            }

            // create a new variable from the given expression (body)
            primitive_argument_type p =
                generate_tree(bodyexpr, patterns, variables, functions);

            if (!is_primitive_operand(p))
            {
                HPX_THROW_EXCEPTION(hpx::invalid_status,
                    "phylanx::execution_tree::handle_define_variable",
                    "couldn't create variable: " + name);
            }

            // attempt to insert the new variable into the symbol table
            auto r = variables.insert(execution_tree::variables::value_type(
                std::move(name), primitive_operand(p)));
            if (!r.second)
            {
                HPX_THROW_EXCEPTION(hpx::bad_parameter,
                    "phylanx::execution_tree::handle_define_variable",
                    "couldn't insert variable into symbol table: " + name);
            }
            return p;
        }

        primitive_argument_type handle_define(
            std::multimap<std::string, ast::expression>& placeholders,
            phylanx::execution_tree::variables& variables,
            phylanx::execution_tree::functions& functions,
            expression_pattern_list const& patterns,
            expression_pattern const& pattern)
        {
            // we know that 'define()' uses '__1' to match arguments
            using iterator =
                typename std::multimap<std::string, ast::expression>::iterator;
            std::pair<iterator, iterator> p = placeholders.equal_range("__1");

            // extract expressions representing the newly defined function
            ast::expression name = extract_name(p);
            std::vector<ast::expression> args = extract_arguments(p);
            ast::expression body = extract_body(p);

            // a define() without arguments defines a variable
            if (args.empty())
            {
                return handle_define_variable(std::move(name), std::move(body),
                    variables, functions, patterns);
            }

            // store new function description for later use
            using value_type =
                typename phylanx::execution_tree::functions::value_type;
            auto result = functions.insert(value_type(
                    ast::detail::identifier_name(name),
                    std::make_pair(std::move(args), std::move(body))
                ));
            if (!result.second)
            {
                HPX_THROW_EXCEPTION(hpx::bad_parameter,
                    "phylanx::execution_tree::detail::extract_name",
                    "the define() operation attempted to redefine the "
                        "function: " + ast::detail::identifier_name(name));
            }

            return primitive_argument_type{};
        }

        primitive_argument_type generate_tree(
            ast::expression const& expr,
            expression_pattern_list const& patterns,
            phylanx::execution_tree::variables& variables,
            phylanx::execution_tree::functions& functions)
        {
            primitive_argument_type result;
            for (auto const& pattern : patterns)
            {
                std::multimap<std::string, ast::expression> placeholders;
                if (!ast::match_ast(expr, hpx::util::get<2>(pattern),
                        on_placeholder_match{placeholders}))
                {
                    continue;   // no match found for the current pattern
                }

                // Handle define(__1)
                if (hpx::util::get<0>(pattern) == "define")
                {
                    return handle_define(
                        placeholders, variables, functions, patterns, pattern);
                }

                return handle_placeholders(
                    placeholders, variables, functions, patterns, pattern);
            }

            // remaining expression could refer to a variable
            if (ast::detail::is_identifier(expr))
            {
                return handle_variable(expr, variables, functions);
            }

            // alternatively it could refer to a literal value
            if (ast::detail::is_literal_value(expr))
            {
                return hpx::new_<primitives::variable>(hpx::find_here(),
                    to_primitive_value_type(ast::detail::literal_value(expr)));
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
            primitives::define_::match_data,
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
            primitives::unary_not_operation::match_data,
            // must be last, otherwise it might intercept some predefined
            // function
            primitives::define_::invocation_match_data
        };

        return patterns;
    }

    ///////////////////////////////////////////////////////////////////////////
    primitive_argument_type generate_tree(std::string const& exprstr)
    {
        phylanx::execution_tree::variables vars;
        phylanx::execution_tree::functions funcs;
        return detail::generate_tree(ast::generate_ast(exprstr),
            detail::generate_patterns(get_all_known_patterns()), vars, funcs);
    }

    ///////////////////////////////////////////////////////////////////////////
    primitive_argument_type generate_tree(std::string const& exprstr,
        phylanx::execution_tree::variables const& variables)
    {
        phylanx::execution_tree::variables vars(variables);
        phylanx::execution_tree::functions funcs;
        return detail::generate_tree(ast::generate_ast(exprstr),
            detail::generate_patterns(get_all_known_patterns()), vars, funcs);
    }

    primitive_argument_type generate_tree(ast::expression const& expr,
        pattern_list const& patterns,
        phylanx::execution_tree::variables const& variables)
    {
        phylanx::execution_tree::variables vars(variables);
        phylanx::execution_tree::functions funcs;
        return detail::generate_tree(
            expr, detail::generate_patterns(patterns), vars, funcs);
    }

    primitive_argument_type generate_tree(std::string const& exprstr,
        pattern_list const& patterns,
        phylanx::execution_tree::variables const& variables)
    {
        phylanx::execution_tree::variables vars(variables);
        phylanx::execution_tree::functions funcs;
        return detail::generate_tree(ast::generate_ast(exprstr),
            detail::generate_patterns(patterns), vars, funcs);
    }

    ///////////////////////////////////////////////////////////////////////////
    primitive_argument_type generate_tree(std::string const& exprstr,
        phylanx::execution_tree::variables const& variables,
        phylanx::execution_tree::functions const& functions)
    {
        phylanx::execution_tree::variables vars(variables);
        phylanx::execution_tree::functions funcs(functions);
        return detail::generate_tree(ast::generate_ast(exprstr),
            detail::generate_patterns(get_all_known_patterns()), vars, funcs);
    }

    primitive_argument_type generate_tree(ast::expression const& expr,
        pattern_list const& patterns,
        phylanx::execution_tree::variables const& variables,
        phylanx::execution_tree::functions const& functions)
    {
        phylanx::execution_tree::variables vars(variables);
        phylanx::execution_tree::functions funcs(functions);
        return detail::generate_tree(
            expr, detail::generate_patterns(patterns), vars, funcs);
    }

    primitive_argument_type generate_tree(std::string const& exprstr,
        pattern_list const& patterns,
        phylanx::execution_tree::variables const& variables,
        phylanx::execution_tree::functions const& functions)
    {
        phylanx::execution_tree::variables vars(variables);
        phylanx::execution_tree::functions funcs(functions);
        return detail::generate_tree(ast::generate_ast(exprstr),
            detail::generate_patterns(patterns), vars, funcs);
    }
}}

