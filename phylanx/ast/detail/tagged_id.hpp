//   Copyright (c) 2017 Hartmut Kaiser
//
//   Distributed under the Boost Software License, Version 1.0. (See accompanying
//   file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(PHYLANX_AST_DETAIL_TAGGED_ID_HPP)
#define PHYLANX_AST_DETAIL_TAGGED_ID_HPP

#include <phylanx/config.hpp>
#include <phylanx/ast/node.hpp>

#include <cstdint>

namespace phylanx { namespace ast { namespace detail
{
    ///////////////////////////////////////////////////////////////////////////
    inline tagged tagged_id(identifier const& id);

    PHYLANX_EXPORT tagged tagged_id(primary_expr const& pe);
    PHYLANX_EXPORT tagged tagged_id(unary_expr const& ue);
    PHYLANX_EXPORT tagged tagged_id(operand const& op);

    inline tagged tagged_id(operation const& op);
    inline tagged tagged_id(expression const& expr);
    inline tagged tagged_id(function_call const& fc);

    template <typename Ast>
    tagged tagged_id(Ast const&)
    {
        return {};
    }

    template <typename Ast>
    tagged tagged_id(util::recursive_wrapper<Ast> const& ast)
    {
        return tagged_id(ast.get());
    }

    inline tagged tagged_id(identifier const& id)
    {
        return id;
    }

    inline tagged tagged_id(operation const& op)
    {
        return tagged_id(op.operand_);
    }

    inline tagged tagged_id(expression const& expr)
    {
        return tagged_id(expr.first);
    }

    inline tagged tagged_id(function_call const& fc)
    {
        return tagged_id(fc.function_name);
    }
}}}

#endif

