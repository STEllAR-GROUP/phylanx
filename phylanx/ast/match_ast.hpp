//   Copyright (c) 2017 Hartmut Kaiser
//
//   Distributed under the Boost Software License, Version 1.0. (See accompanying
//   file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(PHYLANX_AST_MATCH_HPP)
#define PHYLANX_AST_MATCH_HPP

#include <phylanx/config.hpp>
#include <phylanx/ast/detail/is_identifier.hpp>
#include <phylanx/ast/detail/is_placeholder.hpp>
#include <phylanx/ast/detail/is_placeholder_ellipses.hpp>
#include <phylanx/ast/node.hpp>
#include <phylanx/util/variant.hpp>

#include <hpx/assertion.hpp>
#include <hpx/datastructures.hpp>
#include <hpx/functional.hpp>
#include <hpx/type_support.hpp>

#include <cstddef>
#include <cstdint>
#include <map>
#include <type_traits>
#include <string>
#include <utility>
#include <vector>

namespace phylanx { namespace ast
{
    namespace detail
    {
        ///////////////////////////////////////////////////////////////////////
        struct on_placeholder_match
        {
            using placeholder_map_type =
                std::multimap<std::string, ast::expression>;

            placeholder_map_type& placeholders;

            template <typename Ast1, typename Ast2, typename... Ts>
            bool operator()(
                Ast1 const& ast1, Ast2 const& ast2, Ts const&... ts) const
            {
                using value_type = placeholder_map_type::value_type;

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
                    if (ast::detail::is_placeholder_ellipses(ast2))
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
    }

    ///////////////////////////////////////////////////////////////////////////
    template <typename Ast1, typename Ast2, typename F, typename ... Ts>
    bool match_ast(Ast1 const& ast1, Ast2 const& ast2, F && f, Ts const&... ts)
    {
        return false;       // by default things don't match
    }

    ///////////////////////////////////////////////////////////////////////////
    template <typename Ast1, typename Ast2, typename F, typename... Ts>
    bool match_ast(phylanx::util::recursive_wrapper<Ast1> const& ast1,
        Ast2 const& ast2, F&& f, Ts const&... ts)
    {
        return match_ast (ast1.get(), ast2, std::forward<F>(f), ts...);
    }
    template <typename Ast1, typename Ast2, typename F, typename... Ts>
    bool match_ast(Ast1 const& ast1,
        phylanx::util::recursive_wrapper<Ast2> const& ast2, F&& f,
        Ts const&... ts)
    {
        return match_ast (ast1, ast2.get(), std::forward<F>(f), ts...);
    }
    template <typename Ast1, typename Ast2, typename F, typename... Ts>
    bool match_ast(phylanx::util::recursive_wrapper<Ast1> const& ast1,
        phylanx::util::recursive_wrapper<Ast2> const& ast2, F&& f,
        Ts const&... ts)
    {
        return match_ast (ast1.get(), ast2.get(), std::forward<F>(f), ts...);
    }

    ///////////////////////////////////////////////////////////////////////////
    template <typename F, typename... Ts>
    bool match_ast(optoken const&, optoken const&, F&&, Ts const&...);

    template <typename F, typename... Ts>
    bool match_ast(identifier const&, identifier const&, F&&, Ts const&...);

    template <typename F, typename... Ts>
    bool match_ast(bool, identifier const&, F&&, Ts const&...);
    template <typename F, typename... Ts>
    bool match_ast(std::string const&, identifier const&, F&&, Ts const&...);
    template <typename F, typename... Ts>
    bool match_ast(std::int64_t, identifier const&, F&&, Ts const&...);
    template <typename T, typename F, typename... Ts>
    bool match_ast(ir::node_data<T> const&, identifier const&, F&&,
        Ts const&...);
    template <typename F, typename... Ts>
    bool match_ast(primary_expr const&, identifier const&, F&&, Ts const&...);
    template <typename F, typename... Ts>
    bool match_ast(primary_expr const&, primary_expr const&, F&&, Ts const&...);

    template <typename F, typename... Ts>
    bool match_ast(operand const&, identifier const&, F&&, Ts const&...);
    template <typename F, typename... Ts>
    bool match_ast(operand const&, operand const&, F&&, Ts const&...);

    template <typename F, typename... Ts>
    bool match_ast(unary_expr const&, identifier const&, F&&, Ts const&...);
    template <typename F, typename... Ts>
    bool match_ast(unary_expr const&, unary_expr const&, F&&, Ts const&...);
    template <typename F, typename... Ts>
    bool match_ast(unary_expr const&, primary_expr const&, F&&, Ts const&...);

    template <typename F, typename... Ts>
    bool match_ast(operation const&, identifier const&, F&&, Ts const&...);
    template <typename F, typename... Ts>
    bool match_ast(operation const&, operation const&, F&&, Ts const&...);

    template <typename F, typename... Ts>
    bool match_ast(expression const&, identifier const&, F&&, Ts const&...);
    template <typename F, typename... Ts>
    bool match_ast(expression const&, expression const&, F&&, Ts const&...);

    template <typename F, typename... Ts>
    bool match_ast(function_call const&, identifier const&, F&&, Ts const&...);
    template <typename F, typename... Ts>
    bool match_ast(
        function_call const&, function_call const&, F&&, Ts const&...);

    template <typename F, typename... Ts>
    bool match_ast(std::vector<ast::expression> const&, identifier const&, F&&,
        Ts const&...);
    template <typename F, typename... Ts>
    bool match_ast(std::vector<ast::expression> const&,
        std::vector<ast::expression> const&, F&&, Ts const&...);

    ///////////////////////////////////////////////////////////////////////////
    namespace detail
    {
        template <typename F, typename... Ts>
        struct match_visitor
        {
            match_visitor(F && f, hpx::util::tuple<Ts const&...> const& t)
              : f_(static_cast<F&&>(f)), t_(t)
            {}

            F && f_;
            hpx::util::tuple<Ts const&...> const& t_;

            template <typename Ast1, typename Ast2, std::size_t... Is>
            inline bool call(Ast1 const& ast1, Ast2 const& ast2,
                hpx::util::pack_c<std::size_t, Is...>) const
            {
                return match_ast(ast1, ast2, f_, hpx::util::get<Is>(t_)...);
            }

            template <typename Ast1, typename Ast2>
            bool operator()(Ast1 const& ast1, Ast2 const& ast2) const
            {
                return call(ast1, ast2,
                    typename hpx::util::make_index_pack<
                            sizeof...(Ts)
                         >::type());
            }
        };

        template <typename F, typename... Ts>
        match_visitor<F, Ts...>
        make_match_visitor(F && f, Ts const&... ts)
        {
            return match_visitor<F, Ts...>(
                std::forward<F>(f), hpx::util::forward_as_tuple(ts...));
        }
    }

    ///////////////////////////////////////////////////////////////////////////
    template <typename F, typename... Ts>
    bool match_ast(optoken const& t1, optoken const& t2, F&& f, Ts const&... ts)
    {
        return t1 == t2;
    }

    ///////////////////////////////////////////////////////////////////////////
    template <typename F, typename... Ts>
    bool match_ast(
        identifier const& id1, identifier const& id2, F&& f, Ts const&... ts)
    {
        // handle placeholder
        if (detail::is_placeholder(id1) || detail::is_placeholder(id2) ||
            id1 == id2)
        {
            return hpx::util::invoke(std::forward<F>(f), id1, id2, ts...);
        }
        return false;
    }

    ///////////////////////////////////////////////////////////////////////////
    template <typename F, typename... Ts>
    bool match_ast(bool b, identifier const& id, F&& f, Ts const&... ts)
    {
        // handle placeholder
        if (detail::is_placeholder(id))
        {
            return hpx::util::invoke(std::forward<F>(f), b, id, ts...);
        }
        return false;
    }

    template <typename F, typename... Ts>
    bool match_ast(std::string const& s, identifier const& id, F&& f, Ts const&... ts)
    {
        // handle placeholder
        if (detail::is_placeholder(id))
        {
            return hpx::util::invoke(std::forward<F>(f), s, id, ts...);
        }
        return false;
    }

    template <typename F, typename... Ts>
    bool match_ast(std::int64_t i, identifier const& id, F&& f, Ts const&... ts)
    {
        // handle placeholder
        if (detail::is_placeholder(id))
        {
            return hpx::util::invoke(std::forward<F>(f), i, id, ts...);
        }
        return false;
    }

    template <typename T, typename F, typename... Ts>
    bool match_ast(ir::node_data<T> const& nd, identifier const& id, F&& f,
        Ts const&... ts)
    {
        // handle placeholder
        if (detail::is_placeholder(id))
        {
            return hpx::util::invoke(std::forward<F>(f), nd, id, ts...);
        }
        return false;
    }

    template <typename F, typename... Ts>
    bool match_ast(
        primary_expr const& pe, identifier const& id, F&& f, Ts const&... ts)
    {
        // handle placeholder
        if (detail::is_placeholder(id))
        {
            return hpx::util::invoke(std::forward<F>(f), pe, id, ts...);
        }
        return false;
    }

    template <typename F, typename... Ts>
    bool match_ast(primary_expr const& pe1, primary_expr const& pe2, F&& f,
        Ts const&... ts)
    {
        return visit(
            detail::make_match_visitor(std::forward<F>(f), ts...), pe1, pe2);
    }

    ///////////////////////////////////////////////////////////////////////////
    template <typename F, typename... Ts>
    bool match_ast(
        operand const& op, identifier const& id, F&& f, Ts const&... ts)
    {
        // handle placeholder
        if (detail::is_placeholder(id))
        {
            return hpx::util::invoke(std::forward<F>(f), op, id, ts...);
        }
        return false;
    }

    template <typename F, typename... Ts>
    bool match_ast(
        operand const& op1, operand const& op2, F&& f, Ts const&... ts)
    {
        return visit(
            detail::make_match_visitor(std::forward<F>(f), ts...), op1, op2);
    }

    ///////////////////////////////////////////////////////////////////////////
    template <typename F, typename... Ts>
    bool match_ast(
        unary_expr const& ue, identifier const& id, F&& f, Ts const&... ts)
    {
        // handle placeholder
        if (detail::is_placeholder(id))
        {
            return hpx::util::invoke(std::forward<F>(f), ue, id, ts...);
        }
        return false;
    }

    template <typename F, typename... Ts>
    bool match_ast(
        unary_expr const& pe1, unary_expr const& pe2, F&& f, Ts const&... ts)
    {
        if (!match_ast(pe1.operator_, pe2.operator_, std::forward<F>(f), ts...))
        {
            return false;       // operator does not match
        }
        return match_ast(pe1.operand_, pe2.operand_, std::forward<F>(f), ts...);
    }

    template <typename F, typename... Ts>
    bool match_ast(
        unary_expr const& ue, primary_expr const& pe, F&& f, Ts const&... ts)
    {
        if (detail::is_placeholder(pe))
        {
            return hpx::util::invoke(
                std::forward<F>(f), ue, detail::placeholder_id(pe), ts...);
        }
        return false;
    }

    ///////////////////////////////////////////////////////////////////////////
    template <typename F, typename... Ts>
    bool match_ast(
        operation const& op1, operation const& op2, F&& f, Ts const&... ts)
    {
        if (!match_ast(op1.operator_, op2.operator_, std::forward<F>(f), ts...))
        {
            return false;       // operator does not match
        }
        return match_ast(op1.operand_, op2.operand_, std::forward<F>(f), ts...);
    }

    ///////////////////////////////////////////////////////////////////////////
    template <typename F, typename... Ts>
    bool match_ast(
        expression const& expr, identifier const& id, F&& f, Ts const&... ts)
    {
        // handle placeholder
        if (detail::is_placeholder(id))
        {
            return hpx::util::invoke(std::forward<F>(f), expr, id, ts...);
        }
        return false;
    }

    namespace detail
    {
        // Find full subexpression with a given (or higher) precedence
        template <typename Ast>
        expression extract_subexpression(
            Ast const& ast, int prec,
            std::vector<operation>::const_iterator& it,
            std::vector<operation>::const_iterator end)
        {
            expression result(it->operand_);
            while (++it != end && precedence_of(it->operator_) > prec)
            {
                result.append(*it);
            }
            return result;
        }

        inline expression extract_subexpression(
            primary_expr const& pe, int prec,
            std::vector<operation>::const_iterator& it,
            std::vector<operation>::const_iterator end);
        inline expression extract_subexpression(
            operand const& op, int prec,
            std::vector<operation>::const_iterator& it,
            std::vector<operation>::const_iterator end);

        inline expression extract_subexpression(
            expression const& expr, int prec,
            std::vector<operation>::const_iterator& it,
            std::vector<operation>::const_iterator end)
        {
            if (expr.rest.empty())
            {
                return extract_subexpression(expr.first, prec, it, end);
            }

            while (++it != end && precedence_of(it->operator_) > prec)
            {
                /**/;
            }
            return expr;
        }

        inline expression extract_subexpression(
            primary_expr const& pe, int prec,
            std::vector<operation>::const_iterator& it,
            std::vector<operation>::const_iterator end)
        {
            // primary expression refers to an expression itself
            if (pe.index() == 6)
            {
                return extract_subexpression(util::get<6>(pe.get()).get(),
                    prec, it, end);
            }

            expression result(it->operand_);
            while (++it != end && precedence_of(it->operator_) > prec)
            {
                result.append(*it);
            }
            return result;
        }

        inline expression extract_subexpression(
            operand const& op, int prec,
            std::vector<operation>::const_iterator& it,
            std::vector<operation>::const_iterator end)
        {
            // operand may refer to primary expression
            if (op.index() == 1)
            {
                return extract_subexpression(util::get<1>(op.get()).get(),
                    prec, it, end);
            }

            expression result(it->operand_);
            while (++it != end && precedence_of(it->operator_) > prec)
            {
                result.append(*it);
            }
            return result;
        }

        ///////////////////////////////////////////////////////////////////////
        template <typename Ast>
        bool is_expression(Ast const&)
        {
            return false;
        }

        inline bool is_expression(operand const& op);
        inline bool is_expression(expression const& expr);

        inline bool is_expression(primary_expr const& pe)
        {
            return pe.index() == 6 &&
                is_expression(util::get<6>(pe.get()).get());
        }

        inline bool is_expression(operand const& op)
        {
            return op.index() == 1 &&
                is_expression(util::get<1>(op.get()).get());
        }

        inline bool is_expression(expression const& expr)
        {
            return true;
        }

        ///////////////////////////////////////////////////////////////////////
        inline expression const& extract_expression(operand const& op);
        inline expression const& extract_expression(expression const& expr);

        inline expression const& extract_expression(primary_expr const& pe)
        {
            HPX_ASSERT(pe.index() == 6);
            return extract_expression(util::get<6>(pe.get()).get());
        }

        inline expression const& extract_expression(operand const& op)
        {
            HPX_ASSERT(op.index() == 1);
            return extract_expression(util::get<1>(op.get()).get());
        }

        inline expression const& extract_expression(expression const& expr)
        {
            if (expr.rest.empty() && is_expression(expr.first))
            {
                return extract_expression(expr.first);
            }
            return expr;
        }

        ///////////////////////////////////////////////////////////////////////
        // The Shunting-yard algorithm
        template <typename F, typename ... Ts>
        bool match_expression(
            int min_precedence,
            std::vector<operation>::const_iterator& it1,
            std::vector<operation>::const_iterator end1,
            std::vector<operation>::const_iterator& it2,
            std::vector<operation>::const_iterator end2,
            F && f, Ts const&... ts)
        {
            while (it1 != end1 && it2 != end2 &&
                precedence_of(it1->operator_) >= min_precedence)
            {
                operation const& curr1 = *it1;
                operation const& curr2 = *it2;

                int prec = precedence_of(curr1.operator_);

                if (detail::is_placeholder(curr1))
                {
                    if (!hpx::util::invoke(std::forward<F>(f), curr1,
                            extract_subexpression(
                                it2->operand_, prec, it2, end2),
                            ts...))
                    {
                        return false;
                    }

                    if (!match_ast(curr1.operator_, curr2.operator_,
                            std::forward<F>(f), ts...))
                    {
                        return false;
                    }

                    if (!detail::is_placeholder_ellipses(curr1) || it2 == end2)
                        ++it1;
                    continue;
                }
                if (detail::is_placeholder(curr2))
                {
                    if (!hpx::util::invoke(std::forward<F>(f),
                            extract_subexpression(
                                it1->operand_, prec, it1, end1),
                            curr2, ts...))
                    {
                        return false;
                    }

                    if (!match_ast(curr1.operator_, curr2.operator_,
                            std::forward<F>(f), ts...))
                    {
                        return false;
                    }

                    if (!detail::is_placeholder_ellipses(curr2) || it1 == end1)
                        ++it2;
                    continue;
                }

                if (!match_ast(curr1.operand_, curr2.operand_,
                        std::forward<F>(f), ts...))
                {
                    return false;
                }

                ++it1;
                ++it2;

                while (it1 != end1 && it2 != end2 &&
                    precedence_of(it1->operator_) > prec)
                {
                    if (!match_expression(precedence_of(it1->operator_), it1,
                            end1, it2, end2, std::forward<F>(f), ts...))
                    {
                        return false;
                    }
                }

                if (!match_ast(curr1.operator_, curr2.operator_,
                        std::forward<F>(f), ts...))
                {
                    return false;
                }
            }

            // bail out if the list lengths don't match
            if (it1 == end1)
            {
                return it2 == end2 || detail::is_placeholder_ellipses(*it2);
            }
            if (it2 == end2)
            {
                return detail::is_placeholder_ellipses(*it1);
            }

            return true;
        }
    }

    template <typename F, typename... Ts>
    bool match_ast(expression const& expr1, expression const& expr2, F&& f,
        Ts const&... ts)
    {
        if (detail::is_placeholder(expr1))
        {
            return hpx::util::invoke(std::forward<F>(f),
                detail::placeholder_id(expr1),
                detail::extract_expression(expr2), ts...);
        }
        if (detail::is_placeholder(expr2))
        {
            return hpx::util::invoke(std::forward<F>(f),
                detail::extract_expression(expr1),
                detail::placeholder_id(expr2), ts...);
        }

        // check whether first operand matches
        expression const subexpr1 = detail::extract_expression(expr1);
        expression const subexpr2 = detail::extract_expression(expr2);

        if (!match_ast(
                subexpr1.first, subexpr2.first, std::forward<F>(f), ts...))
        {
            return false;
        }

        // if one is empty, the other one should be empty as well
        if (subexpr1.rest.empty() || subexpr2.rest.empty())
        {
            return subexpr1.rest.size() == subexpr2.rest.size();
        }

        auto begin1 = subexpr1.rest.begin();
        auto begin2 = subexpr2.rest.begin();

        return detail::match_expression(0, begin1, subexpr1.rest.end(), begin2,
            subexpr2.rest.end(), std::forward<F>(f), ts...);
    }

    ///////////////////////////////////////////////////////////////////////////
    template <typename F, typename... Ts>
    bool match_ast(function_call const& fc, identifier const& id, F&& f,
        Ts const&... ts)
    {
        // handle placeholder
        if (detail::is_placeholder(id))
        {
            return hpx::util::invoke(std::forward<F>(f), fc, id, ts...);
        }
        return false;
    }

    template <typename F, typename... Ts>
    bool match_ast(function_call const& fc1, function_call const& fc2, F&& f,
        Ts const&... ts)
    {
        if (!match_ast(fc1.function_name, fc2.function_name, std::forward<F>(f),
                ts...))
        {
            return false;       // function name does not match
        }

        auto it1 = fc1.args.begin(), it2 = fc2.args.begin();
        auto end1 = fc1.args.end(), end2 = fc2.args.end();
        while (it1 != end1 && it2 != end2)
        {
            if (detail::is_placeholder_ellipses(*it1))
            {
                if (!hpx::util::invoke(std::forward<F>(f), *it1, *it2, ts...))
                {
                    return false;
                }
                ++it2;
                continue;
            }
            if (detail::is_placeholder_ellipses(*it2))
            {
                if (!hpx::util::invoke(std::forward<F>(f), *it1, *it2, ts...))
                {
                    return false;
                }
                ++it1;
                continue;
            }

            if (!match_ast(*it1, *it2, std::forward<F>(f), ts...))
            {
                return false;   // one of the remaining operands does not match
            }

            ++it1;
            ++it2;
        }

        // bail out if the list lengths don't match
        if (it1 == end1)
        {
            return it2 == end2 || detail::is_placeholder_ellipses(*it2);
        }
        if (it2 == end2)
        {
            return detail::is_placeholder_ellipses(*it1);
        }

        return true;
    }

    ///////////////////////////////////////////////////////////////////////////
    template <typename F, typename... Ts>
    bool match_ast(std::vector<ast::expression> const& l, identifier const& id,
        F&& f, Ts const&... ts)
    {
        // handle placeholder
        if (detail::is_placeholder(id))
        {
            return hpx::util::invoke(std::forward<F>(f), l, id, ts...);
        }
        return false;
    }

    template <typename F, typename... Ts>
    bool match_ast(std::vector<ast::expression> const& l1,
        std::vector<ast::expression> const& l2, F&& f, Ts const&... ts)
    {
        if (l1.size() != l2.size())
        {
            return false;
        }

        auto end1 = l1.end();
        for (auto it1 = l1.begin(), it2 = l2.begin(); it1 != end1; ++it1, ++it2)
        {
            if (!match_ast(*it1, *it2, std::forward<F>(f), ts...))
            {
                return false;
            }
        }

        return true;
    }
}}

#endif
