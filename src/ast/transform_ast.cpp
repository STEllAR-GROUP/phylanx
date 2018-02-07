//  Copyright (c) 2017-2018 Hartmut Kaiser
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>
#include <phylanx/ast/generate_ast.hpp>
#include <phylanx/ast/match_ast.hpp>
#include <phylanx/ast/node.hpp>
#include <phylanx/ast/transform_ast.hpp>
#include <phylanx/ast/traverse.hpp>
#include <phylanx/util/variant.hpp>

#include <map>
#include <string>
#include <utility>
#include <vector>

namespace phylanx { namespace ast
{
    ///////////////////////////////////////////////////////////////////////////
    namespace detail
    {
        ///////////////////////////////////////////////////////////////////////
        identifier transduce(identifier const& pe, identifier const& match,
            expression const& replace);

        primary_expr transduce(primary_expr const& pe, identifier const& match,
            expression const& replace);

        operand transduce(operand const& op, identifier const& match,
            expression const& replace);

        unary_expr transduce(unary_expr const& ue, identifier const& match,
            expression const& replace);

        operation transduce(operation const& op, identifier const& match,
            expression const& replace);

        expression transduce(expression const& expr, identifier const& match,
            expression const& replace);

        function_call transduce(function_call const& expr,
            identifier const& match, expression const& replace);

        template <typename Ast>
        std::vector<Ast> transduce(std::vector<Ast> const& v,
            identifier const& match, expression const& replace);

        template <typename Ast>
        util::recursive_wrapper<Ast> transduce(
            util::recursive_wrapper<Ast> const& rw, identifier const& match,
            expression const& replace);

        ///////////////////////////////////////////////////////////////////////
        template <typename Ast>
        Ast transduce(Ast const& ast, identifier const& match,
            expression const& replace)
        {
            return ast;
        }

        template <typename Ast>
        std::vector<Ast> transduce(std::vector<Ast> const& v,
            identifier const& match, expression const& replace)
        {
            std::vector<Ast> result;
            result.reserve(v.size());
            for (auto const& t : v)
            {
                result.emplace_back(transduce(t, match, replace));
            }
            return result;
        }

        template <typename Ast>
        util::recursive_wrapper<Ast> transduce(
            util::recursive_wrapper<Ast> const& rw, identifier const& match,
            expression const& replace)
        {
            return util::recursive_wrapper<Ast>{
                transduce(rw.get(), match, replace)};
        }

        identifier transduce(identifier const& id, identifier const& match,
            expression const& replace)
        {
            if (id == match && is_identifier(replace))
            {
                return identifier{identifier_name(replace)};
            }
            return id;
        }

        primary_expr transduce(primary_expr const& pe, identifier const& match,
            expression const& replace)
        {
            switch (pe.index())
            {
            case 6:     // phylanx::util::recursive_wrapper<expression>
                return transduce(util::get<6>(pe.var).get(), match, replace);

            case 7:     // phylanx::util::recursive_wrapper<function_call>
                return transduce(util::get<7>(pe.var).get(), match, replace);

            case 8:     // phylanx::util::recursive_wrapper<std::vector<ast::expression>>
                return transduce(util::get<8>(pe.var).get(), match, replace);

            case 3:     // identifier
                {
                    if (util::get<3>(pe.var) == match)
                    {
                        // if the given replacement is just a primary_expr
                        // wrapped into an expression, unwrap it
                        if (replace.rest.empty() && replace.first.index() == 1)
                        {
                            return util::get<1>(replace.first.var).get();
                        }

                        // otherwise return the full replacement expression
                        return primary_expr{replace};
                    }
                }
                HPX_FALLTHROUGH;

            case 0: HPX_FALLTHROUGH;    // nil
            case 1: HPX_FALLTHROUGH;    // bool
            case 2: HPX_FALLTHROUGH;    // phylanx::ir::node_data<double>
            case 4: HPX_FALLTHROUGH;    // std::string
            case 5: HPX_FALLTHROUGH;    // std::int64_t
            default:
                return pe;
            }
        }

        operand transduce(operand const& op, identifier const& match,
            expression const& replace)
        {
            switch (op.index())
            {
            case 1:     // phylanx::util::recursive_wrapper<primary_expr>
                return operand{
                    transduce(util::get<1>(op.var).get(), match, replace)};

            case 2:     // phylanx::util::recursive_wrapper<unary_expr>
                return operand{
                    transduce(util::get<2>(op.var).get(), match, replace)};

            case 0: HPX_FALLTHROUGH;    // nil
            default:
                return op;
            }
        }

        unary_expr transduce(unary_expr const& ue, identifier const& match,
            expression const& replace)
        {
            operand op = transduce(ue.operand_, match, replace);
            return unary_expr{ue.operator_, std::move(op)};
        }

        operation transduce(operation const& oper, identifier const& match,
            expression const& replace)
        {
            operand op = transduce(oper.operand_, match, replace);
            return operation{oper.operator_, std::move(op)};
        }

        expression transduce(expression const& expr,
            identifier const& match, expression const& replace)
        {
            if (is_identifier(expr) && identifier_name(expr) == match.name)
            {
                return replace;
            }

            operand first = transduce(expr.first, match, replace);
            std::vector<operation> rest = transduce(expr.rest, match, replace);

            return expression{std::move(first), std::move(rest)};
        }

        function_call transduce(function_call const& fc,
            identifier const& match, expression const& replace)
        {
            identifier id = transduce(fc.function_name, match, replace);
            std::vector<expression> args = transduce(fc.args, match, replace);
            return function_call{std::move(id), std::move(args)};
        }

        ///////////////////////////////////////////////////////////////////////
        // recurse into a given expression in order to attempt transforming
        // all expression nodes
        primary_expr transform(primary_expr const& pe,
            transform_rule const& rule, bool& found_expr);

        operand transform(
            operand const& op, transform_rule const& rule, bool& found_expr);

        unary_expr transform(
            unary_expr const& ue, transform_rule const& rule, bool& found_expr);

        operation transform(operation const& oper, transform_rule const& rule,
            bool& found_expr);

        expression transform(expression const& expr, transform_rule const& rule,
            bool& found_expr);

        function_call transform(function_call const& fc,
            transform_rule const& rule, bool& found_expr);

        template <typename Ast>
        std::vector<Ast> transform(std::vector<Ast> const& v,
            transform_rule const& rule, bool& found_expr);

        template <typename Ast>
        util::recursive_wrapper<Ast> transform(
            util::recursive_wrapper<Ast> const& rw, transform_rule const& rule,
            bool& found_expr);

        ///////////////////////////////////////////////////////////////////////
        template <typename Ast>
        Ast transform(
            Ast const& ast, transform_rule const& rule, bool& found_expr)
        {
            return ast;
        }

        template <typename Ast>
        std::vector<Ast> transform(std::vector<Ast> const& v,
            transform_rule const& rule, bool& found_expr)
        {
            std::vector<Ast> result;
            result.reserve(v.size());
            for (auto const& t : v)
            {
                result.emplace_back(transform(t, rule, found_expr));
            }
            return result;
        }

        template <typename Ast>
        util::recursive_wrapper<Ast> transform(
            util::recursive_wrapper<Ast> const& rw, transform_rule const& rule,
            bool& found_expr)
        {
            return util::recursive_wrapper<Ast>{
                transform(rw.get(), rule, found_expr)};
        }

        primary_expr transform(primary_expr const& pe,
            transform_rule const& rule, bool& found_expr)
        {
            switch (pe.index())
            {
            case 6:     // phylanx::util::recursive_wrapper<expression>
                return transform(util::get<6>(pe.var).get(), rule, found_expr);

            case 7:     // phylanx::util::recursive_wrapper<function_call>
                return transform(util::get<7>(pe.var).get(), rule, found_expr);

            case 8:
                // phylanx::util::recursive_wrapper<std::vector<ast::expression>>
                return transform(util::get<8>(pe.var).get(), rule, found_expr);

            case 0: HPX_FALLTHROUGH;    // nil
            case 1: HPX_FALLTHROUGH;    // bool
            case 2: HPX_FALLTHROUGH;    // phylanx::ir::node_data<double>
            case 3: HPX_FALLTHROUGH;    // identifier
            case 4: HPX_FALLTHROUGH;    // std::string
            case 5: HPX_FALLTHROUGH;    // std::int64_t
            default:
                return pe;
            }
        }

        operand transform(
            operand const& op, transform_rule const& rule, bool& found_expr)
        {
            switch (op.index())
            {
            case 1:     // phylanx::util::recursive_wrapper<primary_expr>
                return operand{
                    transform(util::get<1>(op.var).get(), rule, found_expr)};

            case 2:     // phylanx::util::recursive_wrapper<unary_expr>
                return operand{
                    transform(util::get<2>(op.var).get(), rule, found_expr)};

            case 0: HPX_FALLTHROUGH;    // nil
            default:
                return op;
            }
        }

        unary_expr transform(
            unary_expr const& ue, transform_rule const& rule, bool& found_expr)
        {
            operand op = transform(ue.operand_, rule, found_expr);
            return unary_expr{ue.operator_, std::move(op)};
        }

        operation transform(
            operation const& oper, transform_rule const& rule, bool& found_expr)
        {
            operand op = transform(oper.operand_, rule, found_expr);
            return operation{oper.operator_, std::move(op)};
        }

        expression transform(expression const& expr, transform_rule const& rule,
            bool& found_expr)
        {
            // recurse for this expression
            found_expr = true;
            return ast::transform_ast(expr, rule);
        }

        function_call transform(function_call const& fc,
            transform_rule const& rule, bool& found_expr)
        {
            std::vector<expression> args = transform(fc.args, rule, found_expr);
            return function_call{fc.function_name, std::move(args)};
        }

        ///////////////////////////////////////////////////////////////////////
        // simplify expression by collapsing expressions consisting of only
        // one operand
        primary_expr simplify(primary_expr const& pe);

        operand simplify(operand const& op);

        unary_expr simplify(unary_expr const& ue);

        operation simplify(operation const& oper);

        expression simplify(expression const& expr);

        function_call simplify(function_call const& fc);

        template <typename Ast>
        std::vector<Ast> simplify(std::vector<Ast> const& v);

        template <typename Ast>
        util::recursive_wrapper<Ast> simplify(
            util::recursive_wrapper<Ast> const& rw);

        ///////////////////////////////////////////////////////////////////////
        template <typename Ast>
        Ast simplify(Ast const& ast)
        {
            return ast;
        }

        template <typename Ast>
        std::vector<Ast> simplify(std::vector<Ast> const& v)
        {
            std::vector<Ast> result;
            result.reserve(v.size());
            for (auto const& t : v)
            {
                result.emplace_back(simplify(t));
            }
            return result;
        }

        template <typename Ast>
        util::recursive_wrapper<Ast> simplify(
            util::recursive_wrapper<Ast> const& rw)
        {
            return util::recursive_wrapper<Ast>{simplify(rw.get())};
        }

        primary_expr simplify(primary_expr const& pe)
        {
            switch (pe.index())
            {
            case 6:     // phylanx::util::recursive_wrapper<expression>
                {
                    expression const& expr = util::get<6>(pe.var).get();
                    if (expr.rest.empty())
                    {
                        switch (expr.first.index())
                        {
                        case 1:
                            // phylanx::util::recursive_wrapper<primary_expr>
                            return simplify(util::get<1>(expr.first.var).get());

                        case 0: HPX_FALLTHROUGH;
                        case 2: HPX_FALLTHROUGH;
                        default:
                            break;
                        }
                    }
                    return simplify(expr);
                }

            case 7:     // phylanx::util::recursive_wrapper<function_call>
                return simplify(util::get<7>(pe.var).get());

            case 8:
                // phylanx::util::recursive_wrapper<std::vector<ast::expression>>
                return simplify(util::get<8>(pe.var).get());

            case 0: HPX_FALLTHROUGH;    // nil
            case 1: HPX_FALLTHROUGH;    // bool
            case 2: HPX_FALLTHROUGH;    // phylanx::ir::node_data<double>
            case 3: HPX_FALLTHROUGH;    // identifier
            case 4: HPX_FALLTHROUGH;    // std::string
            case 5: HPX_FALLTHROUGH;    // std::int64_t
            default:
                return pe;
            }
        }

        operand simplify(operand const& op)
        {
            switch (op.index())
            {
            case 1:     // phylanx::util::recursive_wrapper<primary_expr>
                {
                    primary_expr const& pe = util::get<1>(op.var).get();
                    if (pe.var.index() == 6)
                    {
                        // phylanx::util::recursive_wrapper<expression>
                        expression const& expr = util::get<6>(pe.var).get();
                        if (expr.rest.empty())
                        {
                            switch (expr.first.index())
                            {
                            case 1:
                                // phylanx::util::recursive_wrapper<primary_expr>
                                return operand{simplify(
                                    util::get<1>(expr.first.var).get())};

                            case 2:
                                // phylanx::util::recursive_wrapper<unary_expr>
                                return operand{simplify(
                                    util::get<2>(expr.first.var).get())};

                            case 0: HPX_FALLTHROUGH;    // nil
                            default:
                                break;
                            }
                        }
                    }
                    return operand{simplify(pe)};
                }

            case 2:     // phylanx::util::recursive_wrapper<unary_expr>
                return operand{simplify(util::get<2>(op.var).get())};

            case 0: HPX_FALLTHROUGH;    // nil
            default:
                return op;
            }
        }

        unary_expr simplify(unary_expr const& ue)
        {
            operand op = simplify(ue.operand_);
            return unary_expr{ue.operator_, std::move(op)};
        }

        operation simplify(operation const& oper)
        {
            operand op = simplify(oper.operand_);
            return operation{oper.operator_, std::move(op)};
        }

        expression simplify(expression const& expr)
        {
            if (expr.rest.empty())
            {
                switch (expr.first.index())
                {
                case 1:     // phylanx::util::recursive_wrapper<primary_expr>
                    return expression{
                        simplify(util::get<1>(expr.first.var).get())};

                case 2:     // phylanx::util::recursive_wrapper<unary_expr>
                    return expression{
                        simplify(util::get<2>(expr.first.var).get())};

                case 0: HPX_FALLTHROUGH;    // nil
                default:
                    break;
                }
            }

            operand first = simplify(expr.first);
            std::vector<operation> rest = simplify(expr.rest);

            return expression{std::move(first), std::move(rest)};
        }

        function_call simplify(function_call const& fc)
        {
            std::vector<expression> args = simplify(fc.args);
            return function_call{fc.function_name, std::move(args)};
        }
    }

    expression transform_ast(expression const& in, transform_rule const& rule)
    {
        std::multimap<std::string, expression> placeholders;
        if (!match_ast(
                in, rule.first, detail::on_placeholder_match{placeholders}))
        {
            // no match found for the current rule, recurse to the next
            // expression inside the current one and retry
            bool found_expr = false;
            operand op = detail::transform(in.first, rule, found_expr);
            std::vector<operation> ops =
                detail::transform(in.rest, rule, found_expr);

//            if (!found_expr)
//            {
//                HPX_THROW_EXCEPTION(hpx::bad_parameter,
//                    "phylanx::ast::transform_ast",
//                    "couldn't fully pattern-match the given expression: " +
//                        ast::to_string(in));
//            }

            return detail::simplify(expression{std::move(op), std::move(ops)});
        }

        expression result{rule.second};
        for (auto const& placeholder : placeholders)
        {
            result = detail::transduce(result, identifier{placeholder.first},
                placeholder.second);
        }

        // simplify tree (eliminate expressions with only one operand)
        return detail::simplify(result);
    }


    std::vector<expression> transform_ast(std::vector<expression> const& in,
        transform_rule const& rule)
    {
        std::vector<expression> result;
        result.reserve(in.size());

        for (auto const& expr : in)
        {
            result.push_back(transform_ast(expr, rule));
        }

        return result;
    }

    ///////////////////////////////////////////////////////////////////////////
    expression transform_ast(
        expression const& in, std::vector<transform_rule> const& rules)
    {
        if (rules.empty())
        {
            return in;
        }

        expression out = in;
        for (transform_rule const& rule : rules)
        {
            out = transform_ast(out, rule);
        }
        return out;
    }

    std::vector<expression> transform_ast(std::vector<expression> const& in,
        std::vector<transform_rule> const& rules)
    {
        std::vector<expression> result;
        result.reserve(in.size());

        for (auto const& expr : in)
        {
            result.push_back(transform_ast(expr, rules));
        }

        return result;
    }
}}
