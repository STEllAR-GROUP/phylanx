// Copyright (c) 2020 Hartmut Kaiser
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>
#include <phylanx/execution_tree/annotation.hpp>
#include <phylanx/plugins/controls/sleep_operation.hpp>

#include <hpx/errors/throw_exception.hpp>
#include <hpx/include/lcos.hpp>
#include <hpx/include/naming.hpp>
#include <hpx/include/util.hpp>

#include <chrono>
#include <cstddef>
#include <string>
#include <utility>
#include <vector>

///////////////////////////////////////////////////////////////////////////////
namespace phylanx { namespace execution_tree { namespace primitives {

    ///////////////////////////////////////////////////////////////////////////
    match_pattern_type const sleep_operation::match_data =
    {
        hpx::util::make_tuple("sleep",
            std::vector<std::string>{"sleep(_1)"},
            &create_sleep_operation, &create_primitive<sleep_operation>, R"(
            time

            Args:

                time : an arbitrary expression that will be interpreted as an
                       integer denoting the time to sleep (in milliseconds)

            Returns:

            None)")
    };

    ///////////////////////////////////////////////////////////////////////////
    sleep_operation::sleep_operation(primitive_arguments_type&& operands,
        std::string const& name, std::string const& codename)
      : primitive_component_base(std::move(operands), name, codename)
    {
    }

    hpx::future<primitive_argument_type> sleep_operation::eval(
        primitive_arguments_type const& operands,
        primitive_arguments_type const& args, eval_context ctx) const
    {
        if (operands.size() != 1)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter, "sleep_operation::eval",
                generate_error_message(
                    "the sleep_operation primitive requires exactly "
                    "one operand"));
        }

        hpx::this_thread::sleep_for(
            std::chrono::milliseconds(extract_scalar_integer_value_strict(
                operands[0], name_, codename_)));

        return hpx::make_ready_future(primitive_argument_type{});
    }
}}}    // namespace phylanx::execution_tree::primitives
