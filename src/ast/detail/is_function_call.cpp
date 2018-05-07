//   Copyright (c) 2017 Hartmut Kaiser
//
//   Distributed under the Boost Software License, Version 1.0. (See accompanying
//   file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>
#include <phylanx/ast/detail/is_function_call.hpp>
#include <phylanx/util/variant.hpp>

#include <string>
#include <vector>

namespace phylanx { namespace ast { namespace detail
{
    ///////////////////////////////////////////////////////////////////////////
    struct is_function_call_helper
    {
        template <typename Ast>
        bool operator()(Ast const& ast) const
        {
            return is_function_call(ast);
        }
    };

    bool is_function_call(primary_expr const& pe)
    {
        return visit(is_function_call_helper(), pe);
    }

    bool is_function_call(operand const& op)
    {
        return visit(is_function_call_helper(), op);
    }

    ///////////////////////////////////////////////////////////////////////////
    struct function_name_helper
    {
        template <typename Ast>
        std::string operator()(Ast const& ast) const
        {
            return function_name(ast);
        }
    };

    std::string function_name(primary_expr const& pe)
    {
        return visit(function_name_helper(), pe);
    }

    std::string function_name(operand const& op)
    {
        return visit(function_name_helper(), op);
    }

    ///////////////////////////////////////////////////////////////////////////
    struct function_attribute_helper
    {
        template <typename Ast>
        std::string operator()(Ast const& ast) const
        {
            return function_attribute(ast);
        }
    };

    std::string function_attribute(primary_expr const& pe)
    {
        return visit(function_attribute_helper(), pe);
    }

    std::string function_attribute(operand const& op)
    {
        return visit(function_attribute_helper(), op);
    }

    ///////////////////////////////////////////////////////////////////////////
    struct function_arguments_helper
    {
        template <typename Ast>
        std::vector<expression> operator()(Ast const& ast) const
        {
            return function_arguments(ast);
        }
    };

    std::vector<expression> function_arguments(primary_expr const& pe)
    {
        return visit(function_arguments_helper(), pe);
    }

    std::vector<expression> function_arguments(operand const& op)
    {
        return visit(function_arguments_helper(), op);
    }
}}}

