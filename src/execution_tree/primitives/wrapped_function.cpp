//  Copyright (c) 2017-2018 Hartmut Kaiser
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>
#include <phylanx/execution_tree/primitives/function_reference.hpp>
#include <phylanx/execution_tree/primitives/wrapped_function.hpp>

#include <hpx/include/components.hpp>
#include <hpx/include/lcos.hpp>

#include <string>
#include <utility>
#include <vector>

///////////////////////////////////////////////////////////////////////////////
typedef hpx::components::component<
        phylanx::execution_tree::primitives::wrapped_function
    > wrapped_function_type;

HPX_REGISTER_DERIVED_COMPONENT_FACTORY(
    wrapped_function_type, phylanx_wrapped_function_component,
    "phylanx_primitive_component", hpx::components::factory_enabled)
HPX_DEFINE_GET_COMPONENT_TYPE(wrapped_function_type::wrapped_type)

///////////////////////////////////////////////////////////////////////////////
namespace phylanx { namespace execution_tree { namespace primitives
{
    std::string extract_function_name(std::string const& name)
    {
        if (name.find("define-") == 0)
        {
            return name.substr(7);
        }
        return name;
    }

    wrapped_function::wrapped_function(primitive_argument_type target,
            std::string name)
      : target_(std::move(target))
      , name_(extract_function_name(name))
    {
        if (!valid(target_))
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "wrapped_function::wrapped_function",
                "no target given");
        }
    }

    wrapped_function::wrapped_function(primitive_argument_type target,
            std::vector<primitive_argument_type>&& args, std::string name)
      : target_(std::move(target))
      , args_(std::move(args))
      , name_(extract_function_name(name))
    {
        if (!valid(target_))
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "wrapped_function::wrapped_function",
                "no target given");
        }
    }

    ///////////////////////////////////////////////////////////////////////////
    hpx::future<primitive_argument_type> wrapped_function::eval(
        std::vector<primitive_argument_type> const& params) const
    {
        primitive const* p = util::get_if<primitive>(&target_);
        if (p == nullptr)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                " wrapped_function::eval", "no target given");
        }

        // evaluation of the define-function yields the function body
        auto body = p->eval_direct(params);

        if (args_.empty())
        {
            std::vector<primitive_argument_type> fargs(params);
            return hpx::make_ready_future(
                primitive_argument_type{primitive{
                    hpx::new_<primitives::function_reference>(
                        hpx::find_here(), std::move(body), std::move(fargs),
                        name_),
                    name_
                }});
        }

        std::vector<primitive_argument_type> fargs;
        fargs.reserve(args_.size());
        for (auto const& arg : args_)
        {
            fargs.push_back(value_operand_sync(arg, params));
        }

        return value_operand(body, fargs);
    }

    topology wrapped_function::expression_topology() const
    {
        if (!valid(target_))
        {
            HPX_THROW_EXCEPTION(hpx::invalid_status,
                "wrapped_function::expression_topology",
                "expression representing the function body was not "
                    "initialized yet");
        }

        primitive const* p = util::get_if<primitive>(&target_);
        if (p != nullptr)
        {
            return p->expression_topology(hpx::launch::sync);
        }

        return {};
    }
}}}

