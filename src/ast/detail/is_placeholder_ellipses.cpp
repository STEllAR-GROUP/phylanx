//   Copyright (c) 2017 Hartmut Kaiser
//
//   Distributed under the Boost Software License, Version 1.0. (See accompanying
//   file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>
#include <phylanx/ast/detail/is_placeholder_ellipses.hpp>
#include <phylanx/util/variant.hpp>

#include <string>

namespace phylanx { namespace ast { namespace detail
{
    struct is_placeholder_ellipses_helper
    {
        template <typename Ast>
        bool operator()(Ast const& ast) const
        {
            return is_placeholder_ellipses(ast);
        }
    };

    bool is_placeholder_ellipses(primary_expr const& pe)
    {
        return visit(is_placeholder_ellipses_helper(), pe);
    }

    bool is_placeholder_ellipses(operand const& op)
    {
        return visit(is_placeholder_ellipses_helper(), op);
    }
}}}

