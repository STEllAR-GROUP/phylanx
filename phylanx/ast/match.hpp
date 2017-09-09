//   Copyright (c) 2017 Hartmut Kaiser
//
//   Distributed under the Boost Software License, Version 1.0. (See accompanying
//   file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(PHYLANX_AST_MATCH_HPP)
#define PHYLANX_AST_MATCH_HPP

#include <phylanx/config.hpp>
#include <phylanx/ast/node.hpp>

namespace phylanx { namespace ast
{
    ///////////////////////////////////////////////////////////////////////////
    template <typename Ast1, typename Ast2, typename F, typename ... Ts>
    bool match(Ast1 const&, Ast2 const&, F && f, Ts const&... ts)
    {
        return false;       // by default things don't match
    }

    template <typename Ast1, typename Ast2, typename F, typename... Ts>
    bool match(phylanx::util::recursive_wrapper<Ast1> const& ast1,
        Ast2 const& ast2, F&& f, Ts const&... ts)
    {
        return match (ast1.get(), ast2, std::forward<F>(f), ts...);
    }
    template <typename Ast1, typename Ast2, typename F, typename... Ts>
    bool match(Ast1 const& ast1,
        phylanx::util::recursive_wrapper<Ast2> const& ast2, F&& f,
        Ts const&... ts)
    {
        return match (ast1, ast2.get(), std::forward<F>(f), ts...);
    }
    template <typename Ast1, typename Ast2, typename F, typename... Ts>
    bool match(phylanx::util::recursive_wrapper<Ast1> const& ast1,
        phylanx::util::recursive_wrapper<Ast2> const& ast2, F&& f,
        Ts const&... ts)
    {
        return match (ast1.get(), ast2.get(), std::forward<F>(f), ts...);
    }

    ///////////////////////////////////////////////////////////////////////////
    template <typename F, typename... Ts>
    bool match(identifier const&, identifier const&, F&&, Ts const&...);

    template <typename F, typename... Ts>
    bool match(primary_expr const&, identifier const&, F&&, Ts const&...);
    template <typename F, typename... Ts>
    bool match(primary_expr const&, primary_expr const&, F&&, Ts const&...);

    template <typename F, typename... Ts>
    bool match(operand const&, identifier const&, F&&, Ts const&...);
    template <typename F, typename... Ts>
    bool match(operand const&, operand const&, F&&, Ts const&...);

    template <typename F, typename... Ts>
    bool match(unary_expr const&, identifier const&, F&&, Ts const&...);
    template <typename F, typename... Ts>
    bool match(unary_expr const&, unary_expr const&, F&&, Ts const&...);

    template <typename F, typename... Ts>
    bool match(operation const&, identifier const&, F&&, Ts const&...);
    template <typename F, typename... Ts>
    bool match(operation const&, operation const&, F&&, Ts const&...);

    template <typename F, typename... Ts>
    bool match(expression const&, identifier const&, F&&, Ts const&...);
    template <typename F, typename... Ts>
    bool match(expression const&, expression const&, F&&, Ts const&...);

    template <typename F, typename... Ts>
    bool match(function_call const&, identifier const&, F&&, Ts const&...);
    template <typename F, typename... Ts>
    bool match(function_call const&, function_call const&, F&&, Ts const&...);

    ///////////////////////////////////////////////////////////////////////////
    namespace detail
    {
        struct match_visitor
        {
            template <typename Ast1, typename Ast2, typename F, typename... Ts>
            bool operator()(Ast1 const& ast1, Ast2 const& ast2, F&& f,
                Ts const&... ts) const
            {
                return match(ast1, ast2, std::forward<F>(f), ts...);
            }
        };
    }

    ///////////////////////////////////////////////////////////////////////////
    template <typename F, typename... Ts>
    bool match(
        identifier const& id1, identifier const& id2, F&& f, Ts const&... ts)
    {
        // handle placeholder
        if (is_placeholder(id2))
        {
            return true;
        }
        return id1 == id2;
    }

    ///////////////////////////////////////////////////////////////////////////
    template <typename F, typename... Ts>
    bool match(
        primary_expr const& pe1, primary_expr const& pe2, F&& f, Ts const&... ts)
    {
        return true;
    }

    ///////////////////////////////////////////////////////////////////////////
    template <typename F, typename... Ts>
    bool match(operand const& op1, identifier const& id, F&& f, Ts const&... ts)
    {
        // handle placeholder
        if (is_placeholder(id))
        {
            return true;
        }
        return false;
    }

    template <typename F, typename... Ts>
    bool match(operand const& op1, operand const& op2, F&& f, Ts const&... ts)
    {
        return visit(
            detail::match_visitor{}, op1, op2, std::forward<F>(f), ts...);
    }

    ///////////////////////////////////////////////////////////////////////////
    template <typename F, typename... Ts>
    bool match(
        unary_expr const& pe1, unary_expr const& pe2, F&& f, Ts const&... ts)
    {
        return true;
    }

    ///////////////////////////////////////////////////////////////////////////
    template <typename F, typename... Ts>
    bool match(
        operation const& op1, operation const& op2, F&& f, Ts const&... ts)
    {
        return true;
    }

    ///////////////////////////////////////////////////////////////////////////
    template <typename F, typename... Ts>
    bool match(
        expression const& expr1, identifier const& id, F&& f, Ts const&... ts)
    {
        // handle placeholder
        if (is_placeholder(id))
        {
            return true;
        }
        return false;
    }

    template <typename F, typename... Ts>
    bool match(expression const& expr1, expression const& expr2, F&& f,
        Ts const&... ts)
    {
        if (expr1.rest.size() != expr2.rest.size())
            return false;       // different number of operands

        if (!match(expr.first, expr.second, std::forward<Ts>(ts)...))
            return false;       // first operand does not match

        auto end1 = expr1.rest.end();
        for (auto it1 = expr1.rest.begin(), it2 = expr2.rest.begin();
             it1 != end1; ++it1, ++it2)
        {
            if (!match(*it1, *it2, std::forward<Ts>(ts)...))
                return false;   // one of the remaining operands do not match
        }

        return true;
    }

    ///////////////////////////////////////////////////////////////////////////
    template <typename F, typename... Ts>
    bool match(function_call const& fc1, function_call const& fc2, F&& f,
        Ts const&... ts)
    {
        return true;
    }
}}

#endif
