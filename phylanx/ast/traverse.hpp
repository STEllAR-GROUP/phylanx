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
    template <typename F, typename Ast>
    bool traverse(phylanx::util::recursive_wrapper<Ast> const& rw, F && f)
    {
        return traverse(rw.get(), std::forward<F>(f));
    }

    template <typename F, typename T>
    bool traverse(std::list<T> const& l, F && f)
    {
        for (auto const& val : l)
        {
            if (!traverse(val, std::forward<F>(f)))
                return false;
        }
        return true;
    }

    template <typename F>
    bool traverse(bool b, F && f)
    {
        return hpx::util::invoke(f, b);
    }

    template <typename F>
    bool traverse(phylanx::ir::node_data<double> const& data, F && f)
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

    template <typename F>
    bool traverse(primary_expr const& pe, F && f);

    template <typename F>
    bool traverse(operand const& op, F && f);

    template <typename F>
    bool traverse(unary_expr const& ue, F && f);

    template <typename F>
    bool traverse(operation const& op, F && f);

    template <typename F>
    bool traverse(expression const& expr, F && f);

//     template <typename F>
//     bool traverse(function_call const& op, F && f);
//
//     template <typename F>
//     bool traverse(assignment const& op, F && f);
//
//     template <typename F>
//     bool traverse(variable_declaration const& op, F && f);
//
//     template <typename F>
//     bool traverse(statement const& op, F && f);
//
//     template <typename F>
//     bool traverse(if_statement const& op, F && f);
//
//     template <typename F>
//     bool traverse(while_statement const& op, F && f);
//
//     template <typename F>
//     bool traverse(return_statement const& op, F && f);
//
//     template <typename F>
//     bool traverse(function const& op, F && f);

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

    template <typename F>
    bool traverse(primary_expr const& pe, F && f)
    {
        if (hpx::util::invoke(f, pe))
        {
            return visit(detail::make_unwrap_visitor(std::forward<F>(f)), pe);
        }
        return false;
    }

    template <typename F>
    bool traverse(operand const& op, F && f)
    {
        if (hpx::util::invoke(f, op))
        {
            return visit(detail::make_unwrap_visitor(std::forward<F>(f)), op);
        }
        return false;
    }

    template <typename F>
    bool traverse(unary_expr const& ue, F && f)
    {
        if (hpx::util::invoke(f, ue))
        {
            if (!traverse(ue.operator_, std::forward<F>(f)))
                return false;
            return traverse(ue.operand_, std::forward<F>(f));
        }
        return false;
    }

    template <typename F>
    bool traverse(operation const& op, F && f)
    {
        if (hpx::util::invoke(f, op))
        {
            if (!traverse(op.operator_, std::forward<F>(f)))
                return false;
            return traverse(op.operand_, std::forward<F>(f));
        }
        return false;
    }

    template <typename F>
    bool traverse(expression const& expr, F && f)
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

//     template <typename F>
//     bool traverse(function_call const& fc, F && f)
//     {
//         if (hpx::util::invoke(f, fc))
//         {
//             if (!traverse(fc.function_name, std::forward<F>(f)))
//                 return false;
//             return traverse(fc.args, std::forward<F>(f));
//         }
//         return false;
//     }
//
//     template <typename F>
//     bool traverse(assignment const& assign, F && f)
//     {
//         if (hpx::util::invoke(f, assign))
//         {
//             if (!traverse(assign.lhs, std::forward<F>(f)))
//                 return false;
//             if (!traverse(assign.operator_, std::forward<F>(f)))
//                 return false;
//             return traverse(assign.rhs, std::forward<F>(f));
//         }
//         return false;
//     }
//
//     template <typename F>
//     bool traverse(variable_declaration const& vd, F && f)
//     {
//         if (hpx::util::invoke(f, vd))
//         {
//             if (!traverse(vd.lhs, std::forward<F>(f)))
//                 return false;
//             if (vd.rhs.has_value())
//                 return traverse(vd.rhs, std::forward<F>(f));
//             return true;
//         }
//         return false;
//     }
//
//     template <typename F>
//     bool traverse(statement const& stmt, F && f)
//     {
//         if (hpx::util::invoke(f, stmt))
//         {
//             return visit(detail::make_unwrap_visitor(std::forward<F>(f)), stmt);
//         }
//         return false;
//     }
//
//     template <typename F>
//     bool traverse(if_statement const& if_, F && f)
//     {
//         if (hpx::util::invoke(f, if_))
//         {
//             if (!traverse(if_.condition, std::forward<F>(f)))
//                 return false;
//             if (!traverse(if_.then, std::forward<F>(f)))
//                 return false;
//             if (if_.else_.has_value())
//                 return traverse(if_.else_, std::forward<F>(f));
//             return true;
//         }
//         return false;
//     }
//
//     template <typename F>
//     bool traverse(while_statement const& while_, F && f)
//     {
//         if (hpx::util::invoke(f, while_))
//         {
//             if (!traverse(while_.condition, std::forward<F>(f)))
//                 return false;
//             return traverse(while_.body, std::forward<F>(f));
//         }
//         return false;
//     }
//
//     template <typename F>
//     bool traverse(return_statement const& ret, F && f)
//     {
//         if (hpx::util::invoke(f, ret))
//         {
//             if (ret.expr.has_value())
//                 return traverse(ret.expr, std::forward<F>(f));
//             return true;
//         }
//         return false;
//     }
//
//     template <typename F>
//     bool traverse(function const& func, F && f)
//     {
//         if (hpx::util::invoke(f, func))
//         {
//             if (!traverse(func.function_name, std::forward<F>(f)))
//                 return false;
//             if (!traverse(func.args, std::forward<F>(f)))
//                 return false;
//             if (func.body.has_value())
//                 return traverse(func.body, std::forward<F>(f));
//             return true;
//         }
//         return false;
//     }
}}

#endif

