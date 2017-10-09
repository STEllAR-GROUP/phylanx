//   Copyright (c) 2017 Hartmut Kaiser
//
//   Distributed under the Boost Software License, Version 1.0. (See accompanying
//   file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(PHYLANX_AST_DETAIL_IS_LITERAL_VALUE_HPP)
#define PHYLANX_AST_DETAIL_IS_LITERAL_VALUE_HPP

#include <phylanx/config.hpp>
#include <phylanx/ast/node.hpp>
#include <phylanx/util/variant.hpp>

#include <string>

namespace phylanx { namespace ast { namespace detail
{
    ///////////////////////////////////////////////////////////////////////////
    template <typename Ast>
    bool is_literal_value(Ast const&)
    {
        return false;
    }

    template <typename Ast>
    literal_value_type literal_value(Ast const&)
    {
        return nil{};
    }

    inline bool is_literal_value(identifier const& id);
    inline literal_value_type literal_value(identifier const& id);

    PHYLANX_EXPORT bool is_literal_value(primary_expr const& pe);
    PHYLANX_EXPORT literal_value_type literal_value(primary_expr const& pe);

    PHYLANX_EXPORT bool is_literal_value(operand const& op);
    PHYLANX_EXPORT literal_value_type literal_value(operand const& op);

    inline bool is_literal_value(operation const& op);
    inline literal_value_type literal_value(operation const& op);

    inline bool is_literal_value(expression const& expr);
    inline literal_value_type literal_value(expression const& expr);

    template <typename Ast>
    bool is_literal_value(util::recursive_wrapper<Ast> const& ast)
    {
        return is_literal_value(ast.get());
    }

    template <typename Ast>
    literal_value_type literal_value(util::recursive_wrapper<Ast> const& ast)
    {
        return literal_value(ast.get());
    }

    inline bool is_literal_value(operation const& op)
    {
        return is_literal_value(op.operand_);
    }

    inline literal_value_type literal_value(operation const& op)
    {
        return literal_value(op.operand_);
    }

    inline bool is_literal_value(expression const& expr)
    {
        if (!expr.rest.empty())
        {
            return false;
        }
        return is_literal_value(expr.first);
    }

    inline literal_value_type literal_value(expression const& expr)
    {
        if (!expr.rest.empty())
        {
            return "";
        }
        return literal_value(expr.first);
    }

    ///////////////////////////////////////////////////////////////////////////
    PHYLANX_EXPORT ir::node_data<double> literal_value(
        literal_value_type const&);
}}}

#endif

