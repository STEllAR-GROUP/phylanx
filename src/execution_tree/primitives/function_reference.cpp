//  Copyright (c) 2017-2018 Hartmut Kaiser
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>
#include <phylanx/execution_tree/primitives/function_reference.hpp>

#include <hpx/include/components.hpp>
#include <hpx/include/lcos.hpp>

#include <algorithm>
#include <iterator>
#include <string>
#include <utility>
#include <vector>

///////////////////////////////////////////////////////////////////////////////
typedef hpx::components::component<
        phylanx::execution_tree::primitives::function_reference
    > function_reference_type;

HPX_REGISTER_DERIVED_COMPONENT_FACTORY(
    function_reference_type, phylanx_function_reference_component,
    "phylanx_primitive_component", hpx::components::factory_enabled)
HPX_DEFINE_GET_COMPONENT_TYPE(function_reference_type::wrapped_type)

///////////////////////////////////////////////////////////////////////////////
namespace phylanx { namespace execution_tree { namespace primitives
{
    function_reference::function_reference(primitive_argument_type target,
            std::vector<primitive_argument_type>&& args, std::string name)
      : target_(std::move(target))
      , args_(std::move(args))
      , name_(std::move(name))
    {
        if (!valid(target_))
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "function_reference::function_reference",
                "no target given");
        }
    }

    ///////////////////////////////////////////////////////////////////////////
    hpx::future<primitive_result_type> function_reference::eval(
        std::vector<primitive_argument_type> const& params) const
    {
        if (!args_.empty())
        {
            if (!params.empty())
            {
                std::vector<primitive_result_type> fargs(args_);
                fargs.reserve(args_.size() + params.size());
                std::copy(params.begin(), params.end(), std::back_inserter(fargs));
                return value_operand(target_, std::move(fargs));
            }

            return value_operand(target_, args_);
        }

        return value_operand(target_, params);
    }

    topology function_reference::expression_topology() const
    {
        primitive const* p = util::get_if<primitive>(&target_);
        if (p != nullptr)
        {
            return p->expression_topology(hpx::launch::sync);
        }
        return {};
    }
}}}

