//   Copyright (c) 2017 Hartmut Kaiser
//
//   Distributed under the Boost Software License, Version 1.0. (See accompanying
//   file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(PHYLANX_AST_DETAIL_IS_FUNCTION_CALL_HPP)
#define PHYLANX_AST_DETAIL_IS_FUNCTION_CALL_HPP

#include <phylanx/config.hpp>
#include <phylanx/ast/node.hpp>

#include <string>
#include <vector>

namespace phylanx { namespace ast { namespace detail
{
    ///////////////////////////////////////////////////////////////////////////
    PHYLANX_EXPORT bool is_function_call(primary_expr const& pe);
    PHYLANX_EXPORT std::string function_name(primary_expr const& pe);
    PHYLANX_EXPORT std::string function_attribute(primary_expr const& pe);
    PHYLANX_EXPORT std::vector<expression> function_arguments(
        primary_expr const& pe);

    PHYLANX_EXPORT bool is_function_call(operand const& op);
    PHYLANX_EXPORT std::string function_name(operand const& op);
    PHYLANX_EXPORT std::string function_attribute(operand const& op);
    PHYLANX_EXPORT std::vector<expression> function_arguments(
        operand const& op);

    inline bool is_function_call(operation const& op);
    inline std::string function_name(operation const& op);
    inline std::string function_attribute(operation const& op);
    inline std::vector<expression> function_arguments(
        operation const& op);

    inline bool is_function_call(expression const& expr);
    inline std::string function_name(expression const& expr);
    inline std::string function_attribute(expression const& expr);
    inline std::vector<expression> function_arguments(
        expression const& expr);

    inline bool is_function_call(function_call const& fc);
    inline std::string function_name(function_call const& fc);
    inline std::string function_attribute(function_call const& fc);
    inline std::vector<expression> function_arguments(
            function_call const& id);

    template <typename Ast>
    bool is_function_call(Ast const&)
    {
        return false;
    }
    template <typename Ast>
    std::string function_name(Ast const&)
    {
        return "";
    }
    template <typename Ast>
    std::string function_attribute(Ast const&)
    {
        return "";
    }
    template <typename Ast>
    inline std::vector<expression> function_arguments(Ast const& ast)
    {
        static std::vector<expression> noargs;
        return noargs;
    }

    template <typename Ast>
    bool is_function_call(util::recursive_wrapper<Ast> const& ast)
    {
        return is_function_call(ast.get());
    }
    template <typename Ast>
    std::string function_name(util::recursive_wrapper<Ast> const& ast)
    {
        return function_name(ast.get());
    }
    template <typename Ast>
    std::string function_attribute(util::recursive_wrapper<Ast> const& ast)
    {
        return function_attribute(ast.get());
    }
    template <typename Ast>
    inline std::vector<expression> function_arguments(
        util::recursive_wrapper<Ast> const& ast)
    {
        return function_arguments(ast.get());
    }

    inline bool is_function_call(operation const& op)
    {
        return is_function_call(op.operand_);
    }
    inline std::string function_name(operation const& op)
    {
        return function_name(op.operand_);
    }
    inline std::string function_attribute(operation const& op)
    {
        return function_attribute(op.operand_);
    }
    inline std::vector<expression> function_arguments(
        operation const& op)
    {
        return function_arguments(op.operand_);
    }

    inline bool is_function_call(expression const& expr)
    {
        if (!expr.rest.empty())
        {
            return false;
        }
        return is_function_call(expr.first);
    }
    inline std::string function_name(expression const& expr)
    {
        if (!expr.rest.empty())
        {
            return "";
        }
        return function_name(expr.first);
    }
    inline std::string function_attribute(expression const& expr)
    {
        if (!expr.rest.empty())
        {
            return "";
        }
        return function_attribute(expr.first);
    }
    inline std::vector<expression> function_arguments(
        expression const& expr)
    {
        if (!expr.rest.empty())
        {
            static std::vector<expression> noargs;
            return noargs;
        }
        return function_arguments(expr.first);
    }

    inline bool is_function_call(function_call const& fc)
    {
        return !fc.function_name.name.empty();
    }
    inline std::string function_name(function_call const& fc)
    {
        return fc.function_name.name;
    }
    inline std::string function_attribute(function_call const& fc)
    {
        return fc.attribute;
    }
    inline std::vector<expression> function_arguments(
        function_call const& fc)
    {
        return fc.args;
    }
}}}

#endif

