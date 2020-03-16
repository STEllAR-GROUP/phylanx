// Copyright (c) 2020 Hartmut Kaiser
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>
#include <phylanx/execution_tree/annotation.hpp>
#include <phylanx/plugins/controls/async_operation.hpp>

#include <hpx/errors/throw_exception.hpp>
#include <hpx/include/lcos.hpp>
#include <hpx/include/naming.hpp>
#include <hpx/include/util.hpp>

#include <algorithm>
#include <cstddef>
#include <string>
#include <utility>
#include <vector>

///////////////////////////////////////////////////////////////////////////////
namespace phylanx { namespace execution_tree { namespace primitives {
    ///////////////////////////////////////////////////////////////////////////
    match_pattern_type const async_operation::match_data =
    {
        hpx::util::make_tuple("async",
            std::vector<std::string>{"async(_1)"},
            &create_async_operation, &create_primitive<async_operation>, R"(
            expr

            Args:

                expr : an arbitrary expression that will be evaluated
                       asynchronously

            Returns:\n"

            Returns a future representing the result of the evaluation of the
            given expression)")
    };

    ///////////////////////////////////////////////////////////////////////////
    async_operation::async_operation(primitive_arguments_type&& operands,
        std::string const& name, std::string const& codename)
      : primitive_component_base(std::move(operands), name, codename)
    {
    }

    hpx::future<primitive_argument_type> async_operation::eval(
        primitive_arguments_type const& operands,
        primitive_arguments_type const& args, eval_context ctx) const
    {
        if (operands.size() != 1)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter, "async_operation::eval",
                generate_error_message(
                    "the async_operation primitive requires exactly "
                    "one operand"));
        }

        if (!valid(operands[0]))
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter, "async_operation::eval",
                generate_error_message(
                    "the async_operation primitive requires that the "
                    "argument given by the operand is valid"));
        }

        annotation_wrapper wrap(operands[0]);
        return hpx::make_ready_future(wrap.propagate(primitive_argument_type{
            value_operand(operands[0], args, name_, codename_, std::move(ctx))
                .share()}));
    }
}}}    // namespace phylanx::execution_tree::primitives
