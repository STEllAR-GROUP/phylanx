//   Copyright (c) 2017 Hartmut Kaiser
//
//   Distributed under the Boost Software License, Version 1.0. (See accompanying
//   file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(PHYLANX_AST_DETAIL_IS_PLACEHOLDER_HPP)
#define PHYLANX_AST_DETAIL_IS_PLACEHOLDER_HPP

#include <phylanx/config.hpp>
#include <phylanx/ast/node.hpp>
#include <phylanx/util/variant.hpp>

namespace phylanx { namespace ast { namespace detail
{
    ///////////////////////////////////////////////////////////////////////////
    template <typename Ast>
    bool is_placeholder(Ast const&)
    {
        return false;
    }

    inline bool is_placeholder(identifier const& id);
    PHYLANX_EXPORT bool is_placeholder(primary_expr const& pe);
    PHYLANX_EXPORT bool is_placeholder(operand const& op);
    inline bool is_placeholder(unary_expr const& ue);
    inline bool is_placeholder(operation const& op);
    inline bool is_placeholder(expression const& expr);

    template <typename Ast>
    bool is_placeholder(util::recursive_wrapper<Ast> const& ast)
    {
        return is_placeholder(ast.get());
    }

    inline bool is_placeholder(identifier const& id)
    {
        return !id.name.empty() && id.name[0] == '_';
    }

    inline bool is_placeholder(unary_expr const& ue)
    {
        return is_placeholder(ue.operand_);
    }

    inline bool is_placeholder(operation const& op)
    {
        return is_placeholder(op.operand_);
    }

    inline bool is_placeholder(expression const& expr)
    {
        if (!expr.rest.empty())
        {
            return false;
        }
        return is_placeholder(expr.first);
    }
}}}

#endif

