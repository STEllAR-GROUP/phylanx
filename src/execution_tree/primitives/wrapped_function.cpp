//  Copyright (c) 2017 Hartmut Kaiser
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>
#include <phylanx/execution_tree/primitives/wrapped_function.hpp>

#include <hpx/include/components.hpp>
#include <hpx/include/lcos.hpp>

#include <string>
#include <utility>
#include <vector>

///////////////////////////////////////////////////////////////////////////////
typedef hpx::components::component<
    phylanx::execution_tree::primitives::wrapped_function>
    wrapped_primitive_type;

HPX_REGISTER_DERIVED_COMPONENT_FACTORY(
    wrapped_primitive_type, phylanx_wrapped_primitive_component,
    "phylanx_primitive_component", hpx::components::factory_enabled)
HPX_DEFINE_GET_COMPONENT_TYPE(wrapped_primitive_type::wrapped_type)

///////////////////////////////////////////////////////////////////////////////
namespace phylanx { namespace execution_tree { namespace primitives
{
    wrapped_function::wrapped_function(std::string name)
      : name_(std::move(name))
    {}

    wrapped_function::wrapped_function(primitive_argument_type target)
      : target_(std::move(target))
    {}

    wrapped_function::wrapped_function(primitive_argument_type target,
            std::string name)
      : target_(std::move(target))
      , name_(std::move(name))
    {}

    wrapped_function::wrapped_function(primitive_argument_type target,
            std::vector<primitive_argument_type>&& args)
      : target_(std::move(target))
      , args_(std::move(args))
    {}

    wrapped_function::wrapped_function(primitive_argument_type target,
            std::vector<primitive_argument_type>&& args, std::string name)
      : target_(std::move(target))
      , args_(std::move(args))
      , name_(std::move(name))
    {}

    ///////////////////////////////////////////////////////////////////////////
    hpx::future<primitive_result_type> wrapped_function::eval(
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
