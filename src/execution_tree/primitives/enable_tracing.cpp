//  Copyright (c) 2017-2018 Hartmut Kaiser
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>
#include <phylanx/execution_tree/primitives/enable_tracing.hpp>

#include <hpx/include/lcos.hpp>
#include <hpx/include/naming.hpp>
#include <hpx/include/util.hpp>
#include <hpx/errors/throw_exception.hpp>

#include <memory>
#include <string>
#include <utility>
#include <vector>

///////////////////////////////////////////////////////////////////////////////
namespace phylanx { namespace execution_tree { namespace primitives
{
    ///////////////////////////////////////////////////////////////////////////
    primitive create_enable_tracing(hpx::id_type const& locality,
        primitive_arguments_type&& operands,
        std::string const& name, std::string const& codename)
    {
        static std::string type("enable_tracing");
        return create_primitive_component(
            locality, type, std::move(operands), name, codename);
    }

    match_pattern_type const enable_tracing::match_data =
    {
        hpx::util::make_tuple("enable_tracing",
            std::vector<std::string>{"enable_tracing(_1)"},
            &create_enable_tracing, &create_primitive<enable_tracing>,
            R"(eon
            Args:

                eon (boolean) : set to true/false to enable/disable tracing.

            Returns:)"
            )
    };

    ///////////////////////////////////////////////////////////////////////////
    enable_tracing::enable_tracing(
            primitive_arguments_type && operands,
            std::string const& name, std::string const& codename)
      : primitive_component_base(std::move(operands), name, codename)
    {}

    hpx::future<primitive_argument_type> enable_tracing::eval(
        primitive_arguments_type const& operands,
        primitive_arguments_type const& args, eval_context ctx) const
    {
        if (operands.size() != 1)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "enable_tracing::eval",
                generate_error_message(
                    "expected one (boolean) argument"));
        }

        if (!valid(operands[0]))
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "enable_tracing::eval",
                generate_error_message(
                    "the enable_tracing primitive requires that the "
                        "argument given by the operand is valid"));
        }

        primitive::enable_tracing =
            scalar_boolean_operand_sync(
                operands[0], args, name_, codename_, std::move(ctx)) != 0;

        return hpx::make_ready_future(primitive_argument_type{});
    }
}}}
