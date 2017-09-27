//   Copyright (c) 2017 Hartmut Kaiser
//
//   Distributed under the Boost Software License, Version 1.0. (See accompanying
//   file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>
#include <phylanx/ast/detail/is_identifier.hpp>
#include <phylanx/util/variant.hpp>

#include <string>

namespace phylanx { namespace ast { namespace detail
{
    ///////////////////////////////////////////////////////////////////////////
    struct is_identifier_helper
    {
        template <typename Ast>
        bool operator()(Ast const& ast) const
        {
            return is_identifier(ast);
        }
    };

    bool is_identifier(primary_expr const& pe)
    {
        return visit(is_identifier_helper(), pe);
    }

    bool is_identifier(operand const& op)
    {
        return visit(is_identifier_helper(), op);
    }

    ///////////////////////////////////////////////////////////////////////////
    struct identifier_name_helper
    {
        template <typename Ast>
        std::string operator()(Ast const& ast) const
        {
            return identifier_name(ast);
        }
    };

    std::string identifier_name(primary_expr const& pe)
    {
        return visit(identifier_name_helper(), pe);
    }

    std::string identifier_name(operand const& op)
    {
        return visit(identifier_name_helper(), op);
    }
}}}

