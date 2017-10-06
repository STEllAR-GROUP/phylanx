//  Copyright (c) 2017 Hartmut Kaiser
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>
#include <phylanx/execution_tree/primitives/literal_value.hpp>

#include <hpx/include/components.hpp>
#include <hpx/include/lcos.hpp>

///////////////////////////////////////////////////////////////////////////////
typedef hpx::components::component<
    phylanx::execution_tree::primitives::literal_value>
    literal_type;
HPX_REGISTER_DERIVED_COMPONENT_FACTORY(
    literal_type, phylanx_literal_component,
    "phylanx_primitive_component", hpx::components::factory_enabled)
HPX_DEFINE_GET_COMPONENT_TYPE(literal_type::wrapped_type)

