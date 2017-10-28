//  Copyright (c) 2017 Hartmut Kaiser
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>
#include <phylanx/execution_tree/primitives/wrapped_primitive.hpp>

#include <hpx/include/components.hpp>
#include <hpx/include/lcos.hpp>

#include <string>
#include <utility>
#include <vector>

///////////////////////////////////////////////////////////////////////////////
typedef hpx::components::component<
    phylanx::execution_tree::primitives::wrapped_primitive>
    wrapped_primitive_type;

HPX_REGISTER_DERIVED_COMPONENT_FACTORY(
    wrapped_primitive_type, phylanx_wrapped_primitive_component,
    "phylanx_primitive_component", hpx::components::factory_enabled)
HPX_DEFINE_GET_COMPONENT_TYPE(wrapped_primitive_type::wrapped_type)

HPX_REGISTER_ACTION(wrapped_primitive_type::set_target_direct_action,
    phylanx_wrapped_primitive_set_target_action)

///////////////////////////////////////////////////////////////////////////////
namespace phylanx { namespace execution_tree { namespace primitives
{
    wrapped_primitive::wrapped_primitive(std::string name)
      : name_(std::move(name))
    {}

    wrapped_primitive::wrapped_primitive(primitive_argument_type target)
      : target_(std::move(target))
    {}

    wrapped_primitive::wrapped_primitive(primitive_argument_type target,
            std::string name)
      : target_(std::move(target))
      , name_(std::move(name))
    {}

    wrapped_primitive::wrapped_primitive(primitive_argument_type target,
            std::vector<primitive_argument_type>&& args)
      : target_(std::move(target))
      , args_(std::move(args))
    {}

    wrapped_primitive::wrapped_primitive(primitive_argument_type target,
            std::vector<primitive_argument_type>&& args, std::string name)
      : target_(std::move(target))
      , args_(std::move(args))
      , name_(std::move(name))
    {}

    ///////////////////////////////////////////////////////////////////////////
    void wrapped_primitive::set_target(primitive_argument_type target)
    {
        target_ = std::move(target);
    }

    ///////////////////////////////////////////////////////////////////////////
    hpx::future<primitive_result_type> wrapped_primitive::eval(
        std::vector<primitive_argument_type> const& params) const
    {
        if (!args_.empty())
        {
            std::vector<primitive_result_type> fargs;
            fargs.reserve(args_.size());
            for (auto const& arg : args_)
            {
                fargs.push_back(value_operand_sync(arg, params));
            }
            return value_operand(target_, std::move(fargs));
        }

        return value_operand(target_, params);
    }
}}}

///////////////////////////////////////////////////////////////////////////////
namespace phylanx { namespace execution_tree
{
    hpx::future<void> wrapped_primitive::set_target(
        primitive_argument_type&& target)
    {
        using action_type =
            primitives::wrapped_primitive::set_target_direct_action;
        return hpx::async(
            action_type(), this->primitive::get_id(), std::move(target));
    }

    void wrapped_primitive::set_target(hpx::launch::sync_policy,
        primitive_argument_type&& target)
    {
        using action_type =
            primitives::wrapped_primitive::set_target_direct_action;
        action_type()(this->primitive::get_id(), std::move(target));
    }
}}
