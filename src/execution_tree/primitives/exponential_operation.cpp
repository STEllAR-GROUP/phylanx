//   Copyright (c) 2017 Bibek Wagle
//
//   Distributed under the Boost Software License, Version 1.0. (See accompanying
//   file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/execution_tree/primitives/exponential_operation.hpp>

#include <hpx/include/components.hpp>

////////////////////////////////////////////////////////////////////////////////////////
typedef hpx::components::component<
    phylanx::execution_tree::primitives::exponential_operation>
    exponential_operation_type;
HPX_REGISTER_DERIVED_COMPONENT_FACTORY(exponential_operation_type
    , phylanx_exponential_operation_type
    , "phylanx_primitive_component"
    , hpx::components::factory_enabled)
HPX_DEFINE_GET_COMPONENT_TYPE(exponential_operation_type::wrapped_type)
///////////////////////////////////////////////////////////////////////////////////////

