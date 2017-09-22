//  Copyright (c) 2017 Hartmut Kaiser
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>
#include <phylanx/execution_tree/primitives/add_operation.hpp>
#include <phylanx/execution_tree/primitives/literal_value.hpp>

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
    hpx::future<ir::node_data<double>> primitive::eval() const
    {
        using action_type = primitives::base_primitive::eval_action;
        return hpx::async(action_type(), this->base_type::get_id());
    }
}}

