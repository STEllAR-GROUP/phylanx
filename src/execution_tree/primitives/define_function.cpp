//  Copyright (c) 2017 Hartmut Kaiser
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>
#include <phylanx/execution_tree/primitives/define_function.hpp>
#include <phylanx/execution_tree/primitives/wrapped_function.hpp>

#include <hpx/include/components.hpp>
#include <hpx/include/util.hpp>

#include <vector>
#include <utility>

///////////////////////////////////////////////////////////////////////////////
using define_function_type = hpx::components::component<
        phylanx::execution_tree::primitives::define_function>;
HPX_REGISTER_DERIVED_COMPONENT_FACTORY(
    define_function_type, phylanx_define_function_component,
    "phylanx_primitive_component", hpx::components::factory_enabled)
HPX_DEFINE_GET_COMPONENT_TYPE(define_function_type::wrapped_type)

///////////////////////////////////////////////////////////////////////////////
namespace phylanx { namespace execution_tree { namespace primitives
{
    ///////////////////////////////////////////////////////////////////////////
    define_function::define_function(primitive_argument_type&& operand)
      : body_(std::move(operand))
    {}

    define_function::define_function(
            primitive_argument_type&& operand, std::string&& name)
      : body_(std::move(operand))
      , name_(std::move(name))
    {}

    primitive_result_type define_function::eval_direct(
        std::vector<primitive_argument_type> const& args) const
    {
        // this proxy was created where the function should be created on.
        if (!valid(target_))
        {
            primitive_argument_type operand = body_;
            target_ = primitive(hpx::new_<primitives::wrapped_function>(
                hpx::find_here(), std::move(operand), name_));

            // bind this name to the result of the expression right away
            primitive const* p = util::get_if<primitive>(&target_);
            if (p != nullptr)
            {
                p->eval_direct(args);
            }

            return target_;
        }

        // just evaluate the expression bound to this name
        primitive const* p = util::get_if<primitive>(&target_);
        if (p != nullptr)
        {
            return extract_value(p->eval_direct(args));
        }
        return extract_value(target_);
    }
}}}
