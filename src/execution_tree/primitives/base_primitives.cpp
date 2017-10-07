//  Copyright (c) 2017 Hartmut Kaiser
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>
#include <phylanx/execution_tree/primitives/base_primitive.hpp>
#include <phylanx/ir/node_data.hpp>
#include <phylanx/util/optional.hpp>
#include <phylanx/util/serialization/optional.hpp>

#include <hpx/include/actions.hpp>
#include <hpx/include/components.hpp>

#include <utility>

///////////////////////////////////////////////////////////////////////////////
// Add factory registration functionality
HPX_REGISTER_COMPONENT_MODULE()

///////////////////////////////////////////////////////////////////////////////
// Serialization support for the base_file actions
typedef phylanx::execution_tree::primitives::base_primitive base_primitive_type;

HPX_REGISTER_ACTION(
    base_primitive_type::eval_action, phylanx_primitive_eval_action)
HPX_DEFINE_GET_COMPONENT_TYPE(base_primitive_type)

///////////////////////////////////////////////////////////////////////////////
namespace phylanx { namespace execution_tree
{
    hpx::future<util::optional<ir::node_data<double>>> primitive::eval() const
    {
        using action_type = primitives::base_primitive::eval_action;
        return hpx::async(action_type(), this->base_type::get_id());
    }

    ///////////////////////////////////////////////////////////////////////////
    ir::node_data<double> extract_literal_value(
        primitive_argument_type const& val)
    {
        switch (val.index())
        {
        case 1:     // bool
            return ir::node_data<double>{double(util::get<1>(val))};

        case 2:     // std::uint64_t
            return ir::node_data<double>{double(util::get<2>(val))};

        case 4:     // phylanx::ir::node_data<double>
            return util::get<4>(val);

        default:
            break;
        }

        HPX_THROW_EXCEPTION(hpx::bad_parameter,
            "phylanx::execution_tree::extract_literal_value",
            "primitive_value_type does not hold a literal value type");
    }

    ///////////////////////////////////////////////////////////////////////////
    primitive extract_primitive(primitive_argument_type const& val)
    {
        primitive const* p = util::get_if<primitive>(&val);
        if (p != nullptr)
            return *p;

        HPX_THROW_EXCEPTION(hpx::bad_parameter,
            "phylanx::execution_tree::extract_primitive",
            "primitive_value_type does not hold a primitive");
    }

    ///////////////////////////////////////////////////////////////////////////
    hpx::future<util::optional<ir::node_data<double>>> evaluate_operand(
        primitive_argument_type const& val)
    {
        primitive const* p = util::get_if<primitive>(&val);
        if (p != nullptr)
            return p->eval();

        HPX_ASSERT(valid(val));
        return hpx::make_ready_future(util::optional<ir::node_data<double>>(
            extract_literal_value(val)));
    }

    ///////////////////////////////////////////////////////////////////////////
    primitive_argument_type to_primitive_value_type(
        ast::literal_value_type && val)
    {
        switch (val.index())
        {
        case 0:
            return ast::nil{};

        case 1:     // bool
            return util::get<1>(std::move(val));

        case 2:     // std::uint64_t
            return util::get<2>(std::move(val));

        case 3:     // std::string
            return util::get<3>(std::move(val));

        case 4:     // phylanx::ir::node_data<double>
            return util::get<4>(std::move(val));

        default:
            break;
        }

        HPX_THROW_EXCEPTION(hpx::bad_parameter,
            "phylanx::execution_tree::primitives::to_primitive_value_type",
            "unsupported literal_value_type");
    }
}}

