//  Copyright (c) 2017 Hartmut Kaiser
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>
#include <phylanx/execution_tree/primitives/access_argument.hpp>

#include <hpx/include/components.hpp>
#include <hpx/include/lcos.hpp>
#include <hpx/throw_exception.hpp>

#include <string>
#include <vector>

///////////////////////////////////////////////////////////////////////////////
typedef hpx::components::component<
    phylanx::execution_tree::primitives::access_argument>
    access_argument_type;
HPX_REGISTER_DERIVED_COMPONENT_FACTORY(
    access_argument_type, phylanx_access_argument_component,
    "phylanx_primitive_component", hpx::components::factory_enabled)
HPX_DEFINE_GET_COMPONENT_TYPE(access_argument_type::wrapped_type)

///////////////////////////////////////////////////////////////////////////////
namespace phylanx { namespace execution_tree { namespace primitives
{
    primitive_result_type access_argument::eval_direct(
        std::vector<primitive_argument_type> const& params) const
    {
        if (argnum_ >= params.size())
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "phylanx::execution_tree::primitives::"
                    "access_argument::eval_direct",
                "argument count out of bounds: " + std::to_string(argnum_));
        }
        return value_operand_sync(params[argnum_], params);
    }

    // Return whether this object could be evaluated using the given arguments
    bool access_argument::bind(
        std::vector<primitive_argument_type> const& params)
    {
        return argnum_ < params.size();
    }
}}}
