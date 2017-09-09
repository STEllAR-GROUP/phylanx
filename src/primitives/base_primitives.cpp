//  Copyright (c) 2017 Hartmut Kaiser
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>
#include <phylanx/primitives/add_operation.hpp>
#include <phylanx/primitives/literal_value.hpp>

#include <hpx/hpx.hpp>

#include <utility>

///////////////////////////////////////////////////////////////////////////////
// Add factory registration functionality
HPX_REGISTER_COMPONENT_MODULE()

///////////////////////////////////////////////////////////////////////////////
typedef hpx::components::component<phylanx::primitives::literal_value>
    literal_type;
HPX_REGISTER_DERIVED_COMPONENT_FACTORY(
    literal_type, phylanx_literal_component,
    "phylanx_primitive_component", hpx::components::factory_enabled)
HPX_DEFINE_GET_COMPONENT_TYPE(literal_type::wrapped_type)

///////////////////////////////////////////////////////////////////////////////
typedef hpx::components::component<phylanx::primitives::add_operation>
    add_operation_type;
HPX_REGISTER_DERIVED_COMPONENT_FACTORY(
    add_operation_type, phylanx_add_operation_component,
    "phylanx_primitive_component", hpx::components::factory_enabled)
HPX_DEFINE_GET_COMPONENT_TYPE(add_operation_type::wrapped_type)

///////////////////////////////////////////////////////////////////////////////
// Serialization support for the base_file actions
typedef phylanx::primitives::base_primitive base_primitive_type;

HPX_REGISTER_ACTION(
    base_primitive_type::eval_action, phylanx_primitive_eval_action)
HPX_DEFINE_GET_COMPONENT_TYPE(base_primitive_type)

///////////////////////////////////////////////////////////////////////////////
namespace phylanx { namespace primitives
{
    hpx::future<ir::node_data<double>> primitive::eval() const
    {
        using action_type = base_primitive::eval_action;
        return hpx::async(action_type(), this->base_type::get_id());
    }
}}

