//  Copyright (c) 2017-2018 Hartmut Kaiser
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>
#include <phylanx/execution_tree/primitives/wrapped_variable.hpp>

#include <hpx/include/components.hpp>
#include <hpx/include/lcos.hpp>

#include <string>
#include <utility>
#include <vector>

///////////////////////////////////////////////////////////////////////////////
typedef hpx::components::component<
        phylanx::execution_tree::primitives::wrapped_variable
    > wrapped_variable_type;

HPX_REGISTER_DERIVED_COMPONENT_FACTORY(
    wrapped_variable_type, phylanx_wrapped_variable_component,
    "phylanx_primitive_component", hpx::components::factory_enabled)
HPX_DEFINE_GET_COMPONENT_TYPE(wrapped_variable_type::wrapped_type)

///////////////////////////////////////////////////////////////////////////////
namespace phylanx { namespace execution_tree { namespace primitives
{
    wrapped_variable::wrapped_variable(primitive_argument_type target,
            std::string name)
      : target_(std::move(target))
      , name_(std::move(name))
    {}

    ///////////////////////////////////////////////////////////////////////////
    hpx::future<primitive_result_type> wrapped_variable::eval(
        std::vector<primitive_argument_type> const& params) const
    {
        return value_operand(target_, params);
    }

    void wrapped_variable::store(primitive_result_type && val)
    {
        primitive* p = util::get_if<primitive>(&target_);
        if (p != nullptr)
        {
            p->store(hpx::launch::sync, std::move(val));
        }
    }

    bool wrapped_variable::bind(
        std::vector<primitive_argument_type> const& params)
    {
        bool result = true;

        primitive* p = util::get_if<primitive>(&target_);
        if (p != nullptr)
        {
            result = p->bind(hpx::launch::sync, params) && result;
        }

        return result;
    }
}}}

