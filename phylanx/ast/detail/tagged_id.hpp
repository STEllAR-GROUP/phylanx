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
    inline std::int64_t tagged_id(identifier const& id);

    PHYLANX_EXPORT std::int64_t tagged_id(primary_expr const& pe);
    PHYLANX_EXPORT std::int64_t tagged_id(unary_expr const& ue);
    PHYLANX_EXPORT std::int64_t tagged_id(operand const& op);

    inline std::int64_t tagged_id(operation const& op);
    inline std::int64_t tagged_id(expression const& expr);
    inline std::int64_t tagged_id(function_call const& fc);

    template <typename Ast>
    std::int64_t tagged_id(Ast const&)
    {
        return 0;
    }

    template <typename Ast>
    std::int64_t tagged_id(util::recursive_wrapper<Ast> const& ast)
    {
        return tagged_id(ast.get());
    }

    inline std::int64_t tagged_id(identifier const& id)
    {
        return id.id;
    }

    inline std::int64_t tagged_id(operation const& op)
    {
        return tagged_id(op.operand_);
    }

    inline std::int64_t tagged_id(expression const& expr)
    {
        return tagged_id(expr.first);
    }

    inline std::int64_t tagged_id(function_call const& fc)
    {
        return tagged_id(fc.function_name);
    }
}}}

#endif

