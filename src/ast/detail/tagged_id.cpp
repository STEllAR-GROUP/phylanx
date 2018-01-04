//   Copyright (c) 2017 Hartmut Kaiser
//
//   Distributed under the Boost Software License, Version 1.0. (See accompanying
//   file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>
#include <phylanx/ast/detail/tagged_id.hpp>
#include <phylanx/util/variant.hpp>

#include <cstdint>

namespace phylanx { namespace ast { namespace detail
{
    ///////////////////////////////////////////////////////////////////////////
    struct tagged_id_helper
    {
        template <typename Ast>
        std::int64_t operator()(Ast const& ast) const
        {
            return tagged_id(ast);
        }
    };

    std::int64_t tagged_id(primary_expr const& pe)
    {
        if (pe.id >= 0)
        {
            return pe.id;
        }
        return visit(tagged_id_helper(), pe);
    }

    std::int64_t tagged_id(unary_expr const& ue)
    {
        if (ue.id >= 0)
        {
            return ue.id;
        }
        return tagged_id(ue.operand_);
    }

    std::int64_t tagged_id(operand const& op)
    {
        return visit(tagged_id_helper(), op);
    }
}}}

