//  Copyright (c) 2017-2018 Hartmut Kaiser
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>
#include <phylanx/execution_tree/primitives/enable_tracing.hpp>

#include <hpx/include/components.hpp>
#include <hpx/include/lcos.hpp>
#include <hpx/throw_exception.hpp>

#include <cstdint>
#include <memory>
#include <string>
#include <utility>
#include <vector>

///////////////////////////////////////////////////////////////////////////////
typedef hpx::components::component<
    phylanx::execution_tree::primitives::enable_tracing>
    enable_tracing_type;
HPX_REGISTER_DERIVED_COMPONENT_FACTORY(
    enable_tracing_type, phylanx_enable_tracing_component,
    "phylanx_primitive_component", hpx::components::factory_enabled)
HPX_DEFINE_GET_COMPONENT_TYPE(enable_tracing_type::wrapped_type)

///////////////////////////////////////////////////////////////////////////////
namespace phylanx { namespace execution_tree { namespace primitives
{
    ///////////////////////////////////////////////////////////////////////////
    match_pattern_type const enable_tracing::match_data =
    {
        hpx::util::make_tuple("enable_tracing",
            std::vector<std::string>{"enable_tracing(_1)"},
            &create<enable_tracing>)
    };

    ///////////////////////////////////////////////////////////////////////////
    enable_tracing::enable_tracing(
            std::vector<primitive_argument_type> && operands)
      : base_primitive(std::move(operands))
    {}

    namespace detail
    {
        primitive_argument_type eval_direct(
            std::vector<primitive_argument_type> const& operands,
            std::vector<primitive_argument_type> const& args)
        {
            if (operands.size() != 1)
            {
                HPX_THROW_EXCEPTION(hpx::bad_parameter,
                    "enable_tracing::eval_direct",
                    "expected one (boolean) argument");
            }

            if (!valid(operands[0]))
            {
                HPX_THROW_EXCEPTION(hpx::bad_parameter,
                    "enable_tracing::eval_direct",
                    "the enable_tracing primitive requires that the "
                        "argument given by the operand is valid");
            }

            primitive::enable_tracing =
                boolean_operand_sync(operands[0], args) != 0;
            return primitive_argument_type{};
        }
    }

    primitive_argument_type enable_tracing::eval_direct(
        std::vector<primitive_argument_type> const& args) const
    {
        if (operands_.empty())
        {
            return detail::eval_direct(args, noargs);
        }
        return detail::eval_direct(operands_, args);
    }
}}}
