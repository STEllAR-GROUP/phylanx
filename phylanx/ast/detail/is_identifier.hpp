//   Copyright (c) 2017 Hartmut Kaiser
//
//   Distributed under the Boost Software License, Version 1.0. (See accompanying
//   file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(PHYLANX_AST_DETAIL_IS_IDENTIFIER_HPP)
#define PHYLANX_AST_DETAIL_IS_IDENTIFIER_HPP

#include <phylanx/config.hpp>
#include <phylanx/ast/node.hpp>
#include <phylanx/util/variant.hpp>

#include <string>

namespace phylanx { namespace ast { namespace detail
{
    ///////////////////////////////////////////////////////////////////////////
    template <typename Ast>
    bool is_identifier(Ast const&)
    {
        return false;
    }

    template <typename Ast>
    std::string identifier_name(Ast const&)
    {
        return "";
    }

    inline bool is_identifier(identifier const& id);
    inline std::string identifier_name(identifier const& id);

    PHYLANX_EXPORT bool is_identifier(primary_expr const& pe);
    PHYLANX_EXPORT std::string identifier_name(primary_expr const& pe);

    PHYLANX_EXPORT bool is_identifier(operand const& op);
    PHYLANX_EXPORT std::string identifier_name(operand const& op);

    inline bool is_identifier(unary_expr const& ue);
    inline std::string identifier_name(unary_expr const& ue);

    inline bool is_identifier(operation const& op);
    inline std::string identifier_name(operation const& op);

    inline bool is_identifier(expression const& expr);
    inline std::string identifier_name(expression const& expr);

    template <typename Ast>
    bool is_identifier(util::recursive_wrapper<Ast> const& ast)
    {
        return is_identifier(ast.get());
    }

    template <typename Ast>
    std::string identifier_name(util::recursive_wrapper<Ast> const& ast)
    {
        return identifier_name(ast.get());
    }

    inline bool is_identifier(identifier const& id)
    {
        return !id.name.empty();
    }

    inline std::string identifier_name(identifier const& id)
    {
        return id.name;
    }

    inline bool is_identifier(unary_expr const& ue)
    {
        return is_identifier(ue.operand_);
    }

    inline std::string identifier_name(unary_expr const& ue)
    {
        return identifier_name(ue.operand_);
    }

    inline bool is_identifier(operation const& op)
    {
        return is_identifier(op.operand_);
    }

    inline std::string identifier_name(operation const& op)
    {
        return identifier_name(op.operand_);
    }

    inline bool is_identifier(expression const& expr)
    {
        if (!expr.rest.empty())
        {
            return false;
        }
        return is_identifier(expr.first);
    }

    inline std::string identifier_name(expression const& expr)
    {
        if (!expr.rest.empty())
        {
            return "";
        }
        return identifier_name(expr.first);
    }
}}}

#endif

