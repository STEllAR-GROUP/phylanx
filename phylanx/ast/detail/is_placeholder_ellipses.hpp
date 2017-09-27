//   Copyright (c) 2017 Hartmut Kaiser
//
//   Distributed under the Boost Software License, Version 1.0. (See accompanying
//   file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(PHYLANX_AST_DETAIL_IS_PLACEHOLDER_ELLIPSES_HPP)
#define PHYLANX_AST_DETAIL_IS_PLACEHOLDER_ELLIPSES_HPP

#include <phylanx/config.hpp>
#include <phylanx/ast/node.hpp>
#include <phylanx/util/variant.hpp>

namespace phylanx { namespace ast { namespace detail
{
    ///////////////////////////////////////////////////////////////////////////
    template <typename Ast>
    bool is_placeholder_ellipses(Ast const&)
    {
        return false;
    }

    inline bool is_placeholder_ellipses(identifier const& id);
    PHYLANX_EXPORT bool is_placeholder_ellipses(primary_expr const& pe);
    PHYLANX_EXPORT bool is_placeholder_ellipses(operand const& op);
    inline bool is_placeholder_ellipses(unary_expr const& ue);
    inline bool is_placeholder_ellipses(operation const& op);
    inline bool is_placeholder_ellipses(expression const& expr);

    template <typename Ast>
    bool is_placeholder_ellipses(util::recursive_wrapper<Ast> const& ast)
    {
        return is_placeholder_ellipses(ast.get());
    }

    inline bool is_placeholder_ellipses(identifier const& id)
    {
        return id.name.size() >= 2 && id.name[0] == '_' && id.name[1] == '_';
    }

    inline bool is_placeholder_ellipses(unary_expr const& ue)
    {
        return is_placeholder_ellipses(ue.operand_);
    }

    inline bool is_placeholder_ellipses(operation const& op)
    {
        return is_placeholder_ellipses(op.operand_);
    }

    inline bool is_placeholder_ellipses(expression const& expr)
    {
        if (!expr.rest.empty())
        {
            return false;
        }
        return is_placeholder_ellipses(expr.first);
    }
}}}

#endif

