//  Copyright (c) 2018 Hartmut Kaiser
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(PHYLANX_PRIMITIVES_GENERIC_FUNCTION_IMPL_FEB_15_2018_0125PM)
#define PHYLANX_PRIMITIVES_GENERIC_FUNCTION_IMPL_FEB_15_2018_0125PM

#include <phylanx/config.hpp>
#include <phylanx/execution_tree/primitives/generic_function.hpp>

#include <hpx/include/actions.hpp>
#include <hpx/include/lcos.hpp>
#include <hpx/include/naming.hpp>
#include <hpx/include/util.hpp>
#include <hpx/throw_exception.hpp>

#include <string>
#include <utility>
#include <vector>

///////////////////////////////////////////////////////////////////////////////
namespace phylanx { namespace execution_tree { namespace primitives
{
    ///////////////////////////////////////////////////////////////////////////
    template <typename Action>
    primitive create_generic_function(hpx::id_type const& locality,
        std::vector<primitive_argument_type>&& operands,
        std::string const& name)
    {
        return create_primitive_component(locality,
            hpx::actions::detail::get_action_name<Action>(),
            std::move(operands), name);
    }

    ///////////////////////////////////////////////////////////////////////////
    template <typename Action>
    generic_function<Action>::generic_function(
            std::vector<primitive_argument_type>&& operands)
      : primitive_component_base(std::move(operands))
    {}

    template <typename Action>
    hpx::future<primitive_argument_type> generic_function<Action>::eval(
        std::vector<primitive_argument_type> const& params) const
    {
        std::vector<primitive_argument_type> fargs;
        fargs.reserve(operands_.size());

        for (auto const& operand : operands_)
        {
            fargs.push_back(value_operand_sync(operand, params));
        }

        return hpx::async(Action(), hpx::find_here(), std::move(fargs));
    }
}}}

#endif
