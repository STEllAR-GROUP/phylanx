//   Copyright (c) 2001-2017 Hartmut Kaiser
//
//   Distributed under the Boost Software License, Version 1.0. (See accompanying
//   file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(PHYLANX_AST_TRAVERSE_HPP)
#define PHYLANX_AST_TRAVERSE_HPP

#include <phylanx/config.hpp>
#include <phylanx/ast/node.hpp>

#include <hpx/util/invoke.hpp>

namespace phylanx { namespace ast
{
    ///////////////////////////////////////////////////////////////////////////
    struct static_visitor {};

    ///////////////////////////////////////////////////////////////////////////
    template <typename F, typename Ast>
    bool traverse(phylanx::util::recursive_wrapper<Ast> const& rw, F && f)
    {
        return traverse(rw.get(), std::forward<F>(f));
    }

    template <typename F>
    bool traverse(bool b, F && f)
    {
        return hpx::util::invoke(f, b);
    }

    template <typename T, typename F>
    bool traverse(phylanx::ir::node_data<T> const& data, F && f)
    {
        return hpx::util::invoke(f, data);
    }

    template <typename F>
    bool traverse(optoken op, F && f)
    {
        return hpx::util::invoke(f, op);
    }

    template <typename F>
    bool traverse(nil, F && f)
    {
        return hpx::util::invoke(f, nil{});
    }

    template <typename F>
    bool traverse(identifier const& id, F && f)
    {
        return hpx::util::invoke(f, id);
    }

    template <typename T, typename F>
    bool traverse(primary_expr<T> const& pe, F && f);

    template <typename T, typename F>
    bool traverse(operand<T> const& op, F && f);

    template <typename T, typename F>
    bool traverse(unary_expr<T> const& ue, F && f);

    template <typename T, typename F>
    bool traverse(operation<T> const& op, F && f);

    template <typename T, typename F>
    bool traverse(expression<T> const& expr, F && f);

    namespace detail
    {
        template <typename F>
        struct unwrap_visitor
        {
            F f_;

            template <typename T>
            bool operator()(T const& t) const
            {
                return traverse(t, f_);
            }

            template <typename T>
            bool operator()(phylanx::util::recursive_wrapper<T> const& t) const
            {
                return traverse(t.get(), f_);
            }
        };

        template <typename F>
        unwrap_visitor<std::decay_t<F>> make_unwrap_visitor(F && f)
        {
            return unwrap_visitor<std::decay_t<F>>{std::forward<F>(f)};
        }
    }

    template <typename T, typename F>
    bool traverse(primary_expr<T> const& pe, F && f)
    {
        if (hpx::util::invoke(f, pe))
        {
            return phylanx::util::visit(
                detail::make_unwrap_visitor(std::forward<F>(f)), pe.value);
        }
        return false;
    }

    template <typename T, typename F>
    bool traverse(operand<T> const& op, F && f)
    {
        if (hpx::util::invoke(f, op))
        {
            return phylanx::util::visit(
                detail::make_unwrap_visitor(std::forward<F>(f)), op.value);
        }
        return false;
    }

    template <typename T, typename F>
    bool traverse(unary_expr<T> const& ue, F && f)
    {
        if (hpx::util::invoke(f, ue))
        {
            if (!traverse(ue.operand_, std::forward<F>(f)))
                return false;
            return traverse(ue.operator_, std::forward<F>(f));
        }
        return false;
    }

    template <typename T, typename F>
    bool traverse(operation<T> const& op, F && f)
    {
        if (hpx::util::invoke(f, op))
        {
            if (!traverse(op.operator_, std::forward<F>(f)))
                return false;
            return traverse(op.operand_, std::forward<F>(f));
        }
        return false;
    }

    template <typename T, typename F>
    bool traverse(expression<T> const& expr, F && f)
    {
        if (hpx::util::invoke(f, expr))
        {
            if (!traverse(expr.first, std::forward<F>(f)))
                return false;

            for (auto const& op : expr.rest)
            {
                if (!traverse(op, std::forward<F>(f)))
                    return false;
            }
        }
        return true;
    }
}}

#endif

