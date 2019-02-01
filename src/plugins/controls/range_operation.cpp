// Copyright (c) 2018 Parsa Amini
// Copyright (c) 2018 Hartmut Kaiser
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>
#include <phylanx/plugins/controls/range_operation.hpp>
#include <phylanx/ir/ranges.hpp>
#include <phylanx/util/matrix_iterators.hpp>

#include <hpx/include/lcos.hpp>
#include <hpx/include/util.hpp>
#include <hpx/throw_exception.hpp>

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include <blaze/Math.h>

///////////////////////////////////////////////////////////////////////////////
namespace phylanx { namespace execution_tree { namespace primitives
{
    ///////////////////////////////////////////////////////////////////////////
    match_pattern_type const range_operation::match_data =
    {
        hpx::util::make_tuple("range",
            std::vector<std::string>{
                "range(_1)", "range(_1, _2)", "range(_1, _2, _3)"
            },
            &create_range_operation, &create_primitive<range_operation>,
            R"(start, end, step
            Args:

                start (number) : a starting value
                end (optional, number) : an ending value
                step (optional, number) : a step size

            Returns:

            An iterator of values less than `end` where the values
            are equal to `start+step*n` where n is 0, 1, 2, ...
            This function works like the Python range function. )"
            )
    };

    ///////////////////////////////////////////////////////////////////////////
    range_operation::range_operation(
        primitive_arguments_type&& operands,
        std::string const& name, std::string const& codename)
      : primitive_component_base(std::move(operands), name, codename)
    {}

    ///////////////////////////////////////////////////////////////////////////
    primitive_argument_type range_operation::generate_range(
        args_type&& args) const
    {
        switch (args.size())
        {
        case 1:
            return ir::range(args[0].scalar());

        case 2:
            return ir::range(args[0].scalar(), args[1].scalar());

        case 3:
            return ir::range(args[0].scalar(), args[1].scalar(), args[2].scalar());

        default:
            break;
        }

        HPX_THROW_EXCEPTION(hpx::bad_parameter,
            "range_operation::generate_range",
            generate_error_message(
                "range_operation needs at most three operands"));
    }

    //////////////////////////////////////////////////////////////////////////
    hpx::future<primitive_argument_type> range_operation::eval(
        primitive_arguments_type const& operands,
        primitive_arguments_type const& args, eval_context ctx) const
    {
        if (operands.empty() || operands.size() > 3)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "range_operation::eval",
                generate_error_message(
                    "the range_operation primitive requires exactly one, two, "
                        "or three operands"));
        }

        for (auto const& i : operands)
        {
            if (!valid(i))
            {
                HPX_THROW_EXCEPTION(hpx::bad_parameter,
                    "range_operation::eval",
                    generate_error_message(
                        "the range_operation primitive requires that the "
                            "arguments given by the operands array are valid"));
            }
        }

        auto this_ = this->shared_from_this();
        return hpx::dataflow(hpx::launch::sync, hpx::util::unwrapping(
            [this_ = std::move(this_)](args_type&& args)
            -> primitive_argument_type
            {
                return this_->generate_range(std::move(args));
            }),
            detail::map_operands(
                operands, functional::integer_operand{}, args,
                name_, codename_, std::move(ctx)));
    }
}}}
