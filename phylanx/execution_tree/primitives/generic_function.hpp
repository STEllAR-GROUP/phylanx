//  Copyright (c) 2018 Hartmut Kaiser
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(PHYLANX_PRIMITIVES_GENERIC_FUNCTION_FEB_15_2018_0125PM)
#define PHYLANX_PRIMITIVES_GENERIC_FUNCTION_FEB_15_2018_0125PM

#include <phylanx/config.hpp>
#include <phylanx/execution_tree/primitives/base_primitive.hpp>
#include <phylanx/execution_tree/primitives/primitive_component_base.hpp>

#include <hpx/async.hpp>
#include <hpx/lcos/future.hpp>

#include <string>
#include <utility>
#include <vector>

namespace phylanx { namespace execution_tree { namespace primitives
{
    ///////////////////////////////////////////////////////////////////////////
    // wrapping generic functions as actions
    template <typename Action>
    class generic_function
      : public primitive_component_base
    {
    public:
        static match_pattern_type const match_data;

        generic_function() = default;

        generic_function(primitive_arguments_type&& operands,
                std::string const& name, std::string const& codename)
          : primitive_component_base(std::move(operands), name, codename)
        {}

        hpx::future<primitive_argument_type> eval(
            primitive_arguments_type const& args,
            eval_context ctx) const override
        {
            return hpx::async(Action(), hpx::find_here(), operands_, args,
                name_, codename_, std::move(ctx));
        }
    };

    ///////////////////////////////////////////////////////////////////////////
    template <typename Action>
    primitive create_generic_function(
        hpx::id_type const& locality, primitive_arguments_type&& operands,
        std::string const& name = "", std::string const& codename = "")
    {
        return create_primitive_component(locality,
            hpx::actions::detail::get_action_name<Action>(),
            std::move(operands), name, codename);
    }
}}}

#endif


