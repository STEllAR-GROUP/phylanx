//  Copyright (c) 2017 Hartmut Kaiser
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>
#include <phylanx/execution_tree/primitives/define_variable.hpp>
#include <phylanx/execution_tree/primitives/variable.hpp>

#include <hpx/include/components.hpp>
#include <hpx/include/util.hpp>

#include <vector>
#include <utility>

///////////////////////////////////////////////////////////////////////////////
using define_type = hpx::components::component<
        phylanx::execution_tree::primitives::define_variable>;
HPX_REGISTER_DERIVED_COMPONENT_FACTORY(
    define_type, phylanx_define_component,
    "phylanx_primitive_component", hpx::components::factory_enabled)
HPX_DEFINE_GET_COMPONENT_TYPE(define_type::wrapped_type)

///////////////////////////////////////////////////////////////////////////////
namespace phylanx { namespace execution_tree { namespace primitives
{
    ///////////////////////////////////////////////////////////////////////////
    std::vector<match_pattern_type> const define_variable::match_data =
    {
        // We don't need a creation function as 'define()' is explicitly
        // handled by generate_tree.
        hpx::util::make_tuple("define", "define(__1)", nullptr)
    };

    ///////////////////////////////////////////////////////////////////////////
    define_variable::define_variable(primitive_argument_type&& operand)
      : body_(std::move(operand))
    {}

    define_variable::define_variable(
            primitive_argument_type&& operand, std::string&& name)
      : body_(std::move(operand))
      , name_(std::move(name))
    {}

    primitive_result_type define_variable::eval_direct(
        std::vector<primitive_argument_type> const& args) const
    {
        // this proxy was created where the variable should be created on.
        if (!valid(target_))
        {
            primitive_argument_type operand = body_;
            target_ = primitive(hpx::new_<primitives::variable>(
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

    void define_variable::store(primitive_result_type const& val)
    {
        if (!valid(target_))
        {
            HPX_THROW_EXCEPTION(hpx::invalid_status,
                "define_variable::store",
                "the variable associated with this define has not been "
                    "initialized yet");
        }

        primitive* p = util::get_if<primitive>(&target_);
        if (p == nullptr)
        {
            HPX_THROW_EXCEPTION(hpx::invalid_status,
                "define_variable::store",
                "the variable associated with this define has not been "
                    "properly initialized");
        }
        p->store(hpx::launch::sync, val);
    }
}}}
