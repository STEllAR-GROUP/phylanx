//   Copyright (c) 2017 Hartmut Kaiser
//
//   Distributed under the Boost Software License, Version 1.0. (See accompanying
//   file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>
#include <phylanx/ast/detail/is_literal_value.hpp>
#include <phylanx/util/variant.hpp>

#include <hpx/throw_exception.hpp>

#include <string>

namespace phylanx { namespace ast { namespace detail
{
    ///////////////////////////////////////////////////////////////////////////
    struct is_literal_value_helper
    {
        template <typename Ast>
        bool operator()(Ast const& ast) const
        {
            return is_literal_value(ast);
        }
    };

    bool is_literal_value(primary_expr const& pe)
    {
        switch (pe.index())
        {
        case 1: HPX_FALLTHROUGH;    // bool
        case 2: HPX_FALLTHROUGH;    // phylanx::ir::node_data<double>
        case 4: HPX_FALLTHROUGH;    // std::string
        case 5:                     // std::uint64_t
            return true;

        default:
            break;
        }
        return false;
    }

    bool is_literal_value(operand const& op)
    {
        return visit(is_literal_value_helper(), op);
    }

    ///////////////////////////////////////////////////////////////////////////
    struct literal_value_helper
    {
        template <typename Ast>
        literal_value_type operator()(Ast const& ast) const
        {
            return literal_value(ast);
        }
    };

    literal_value_type literal_value(primary_expr const& pe)
    {
        switch (pe.index())
        {
        case 1:     // bool
            return util::get<1>(pe.get());

        case 2:     // phylanx::ir::node_data<double>
            return util::get<2>(pe.get());

        case 4:     // std::string
            return util::get<4>(pe.get());

        case 5:     // std::uint64_t
            return util::get<5>(pe.get());

        default:
            break;
        }
        return nil{};
    }

    literal_value_type literal_value(operand const& op)
    {
        return visit(literal_value_helper(), op);
    }

    ///////////////////////////////////////////////////////////////////////////
    ir::node_data<double> literal_value(literal_value_type const& val)
    {
        switch (val.index())
        {
        case 4:     // phylanx::ir::node_data<double>
            return util::get<4>(val);

        case 1:     // bool
            return ir::node_data<double>{double(util::get<1>(val))};

        case 2:     // std::uint64_t
            return ir::node_data<double>{double(util::get<2>(val))};

        default:
            break;
        }

        HPX_THROW_EXCEPTION(hpx::bad_parameter,
            "phylanx::ast::detail::literal_value",
            "unsupported literal_value_type");
    }
}}}

