//   Copyright (c) 2001-2017 Hartmut Kaiser
//
//   Distributed under the Boost Software License, Version 1.0. (See accompanying
//   file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(PHYLANX_AST_TRAVERSE_HPP)
#define PHYLANX_AST_TRAVERSE_HPP

#include <phylanx/config.hpp>
#include <phylanx/ast/node.hpp>

#include <hpx/traits/has_member_xxx.hpp>
#include <hpx/util/invoke.hpp>

#include <list>
#include <type_traits>
#include <utility>

namespace phylanx { namespace ast
{
    ///////////////////////////////////////////////////////////////////////////
    struct static_visitor
    {
        template <typename T>
        bool on_enter(T && val) const
        {
            return true;
        }

        template <typename T>
        bool on_exit(T && val) const
        {
            return true;
        }
    };

    ///////////////////////////////////////////////////////////////////////////
    namespace detail
    {
        HPX_HAS_MEMBER_XXX_TRAIT_DEF(on_enter);

        struct on_enter
        {
            template <typename F, typename Ast>
            static std::enable_if_t<has_on_enter<std::decay_t<F>>::value, bool>
            call(F && f, Ast const& ast)
            {
                return f.on_enter(ast);
            }

            template <typename F, typename Ast>
            static std::enable_if_t<!has_on_enter<std::decay_t<F>>::value, bool>
            call(F && f, Ast const& ast)
            {
                return hpx::util::invoke(f, ast);
            }
        };

        HPX_HAS_MEMBER_XXX_TRAIT_DEF(on_exit);

        struct on_exit
        {
            template <typename F, typename Ast>
            static std::enable_if_t<has_on_exit<std::decay_t<F>>::value, bool>
            call(F && f, Ast const& ast)
            {
                return f.on_exit(ast);
            }

            template <typename F, typename Ast>
            static std::enable_if_t<!has_on_exit<std::decay_t<F>>::value, bool>
            call(F && f, Ast const& ast)
            {
                return true;
            }
        };

        ///////////////////////////////////////////////////////////////////////
        template <typename F, typename Ast, typename Visitor>
        bool on_visit(F && f, Ast const& ast, Visitor && visit)
        {
            try {
                f(ast, std::forward<Visitor>(visit));
            }
            catch (...) {
                detail::on_exit::call(visit, ast);
                throw;
            }
            return detail::on_exit::call(visit, ast);
        }
    }

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
        return detail::on_visit(
            [](bool b, F && f)
            {
                detail::on_enter::call(f, b);
            },
            b, std::forward<F>(f));
    }

    template <typename F>
    bool traverse(phylanx::ir::node_data<double> const& data, F && f)
    {
        return detail::on_visit(
            [](phylanx::ir::node_data<double> const& data, F && f)
            {
                detail::on_enter::call(f, data);
            },
            data, std::forward<F>(f));
    }

    template <typename F>
    bool traverse(optoken op, F && f)
    {
        return detail::on_visit(
            [](optoken op, F && f)
            {
                detail::on_enter::call(f, op);
            },
            op, std::forward<F>(f));
    }

    template <typename F>
    bool traverse(nil, F && f)
    {
        return detail::on_visit(
            [](nil, F && f)
            {
                detail::on_enter::call(f, nil{});
            },
            nil{}, std::forward<F>(f));
    }

    template <typename F>
    bool traverse(identifier const& id, F && f)
    {
        return detail::on_visit(
            [](identifier const& id, F && f)
            {
                detail::on_enter::call(f, id);
            },
            id, std::forward<F>(f));
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
        return detail::on_visit(
            [](primary_expr const& pe, F && f)
            {
                if (detail::on_enter::call(f, pe))
                {
                    visit(detail::make_unwrap_visitor(std::forward<F>(f)), pe);
                }
            },
            pe, std::forward<F>(f));
    }

    template <typename F>
    bool traverse(operand const& op, F && f)
    {
        return detail::on_visit(
            [](operand const& op, F && f)
            {
                if (detail::on_enter::call(f, op))
                {
                    visit(detail::make_unwrap_visitor(std::forward<F>(f)), op);
                }
            },
            op, std::forward<F>(f));
    }

    template <typename F>
    bool traverse(unary_expr const& ue, F && f)
    {
        return detail::on_visit(
            [](unary_expr const& ue, F && f)
            {
                if (detail::on_enter::call(f, ue))
                {
                    if (traverse(ue.operator_, std::forward<F>(f)))
                        traverse(ue.operand_, std::forward<F>(f));
                }
            },
            ue, std::forward<F>(f));
    }

    template <typename F>
    bool traverse(operation const& op, F && f)
    {
        return detail::on_visit(
            [](operation const& op, F && f)
            {
                if (detail::on_enter::call(f, op))
                {
                    if (traverse(op.operator_, std::forward<F>(f)))
                        traverse(op.operand_, std::forward<F>(f));
                }
            },
            op, std::forward<F>(f));
    }

    template <typename F>
    bool traverse(expression const& expr, F && f)
    {
        return detail::on_visit(
            [](expression const& expr, F && f)
            {
                if (detail::on_enter::call(f, expr))
                {
                    if (traverse(expr.first, std::forward<F>(f)))
                    {
                        for (auto const& op : expr.rest)
                        {
                            if (!traverse(op, std::forward<F>(f)))
                                break;
                        }
                    }
                }
            },
            expr, std::forward<F>(f));
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

