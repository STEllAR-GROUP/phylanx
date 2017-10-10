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
    template <typename Ast>
    identifier placeholder_id(Ast const&)
    {
        return identifier{};
    }

    inline bool is_placeholder(identifier const& id);
    inline identifier placeholder_id(identifier const& id);
    PHYLANX_EXPORT bool is_placeholder(primary_expr const& pe);
    PHYLANX_EXPORT identifier placeholder_id(primary_expr const& pe);
    PHYLANX_EXPORT bool is_placeholder(operand const& op);
    PHYLANX_EXPORT identifier placeholder_id(operand const& op);
    inline bool is_placeholder(operation const& op);
    inline identifier placeholder_id(operation const& op);
    inline bool is_placeholder(expression const& expr);
    inline identifier placeholder_id(expression const& expr);

    template <typename Ast>
    bool is_placeholder(util::recursive_wrapper<Ast> const& ast)
    {
        return is_placeholder(ast.get());
    }
    template <typename Ast>
    identifier placeholder_id(util::recursive_wrapper<Ast> const& ast)
    {
        return placeholder_id(ast.get());
    }

    inline bool is_placeholder(identifier const& id)
    {
        return !id.name.empty() && id.name[0] == '_';
    }
    inline identifier placeholder_id(identifier const& id)
    {
        return id;
    }

    inline bool is_placeholder(operation const& op)
    {
        return is_placeholder(op.operand_);
    }
    inline identifier placeholder_id(operation const& op)
    {
        return placeholder_id(op.operand_);
    }

    inline bool is_placeholder(expression const& expr)
    {
        if (!expr.rest.empty())
        {
            return false;
        }
        return is_placeholder(expr.first);
    }
    inline identifier placeholder_id(expression const& expr)
    {
        if (!expr.rest.empty())
        {
            return identifier{};
        }
        return placeholder_id(expr.first);
    }
}}}

#endif

