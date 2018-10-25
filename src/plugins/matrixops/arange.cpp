// Copyright (c) 2018 Hartmut Kaiser
// Copyright (c) 2018 R. Tohid
// Copyright (c) 2018 Bita Hasheminezhad
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>
#include <phylanx/execution_tree/primitives/node_data_helpers.hpp>
#include <phylanx/ir/node_data.hpp>
#include <phylanx/plugins/matrixops/arange.hpp>

#include <hpx/include/lcos.hpp>
#include <hpx/include/naming.hpp>
#include <hpx/include/util.hpp>
#include <hpx/throw_exception.hpp>

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <string>
#include <utility>
#include <vector>

///////////////////////////////////////////////////////////////////////////////
namespace phylanx { namespace execution_tree { namespace primitives
{
    ///////////////////////////////////////////////////////////////////////////
    match_pattern_type const arange::match_data =
    {
        match_pattern_type{
            "arange",
            std::vector<std::string>{"arange(__1)"},
            &create_arange, &create_primitive<arange>, R"(
            start, stop, step
            Args:

                start (number) : Start of interval. The interval includes this
                    value. The default start value is 0.
                stop (number) : End of interval. The interval does not include
                    this value, except in some cases where step is
                    not an integer and floating point round-off
                    affects the length of out.
                step (number, optional) : Spacing between values. For any
                    output out, this is the distance between two adjacent
                    values, `out[i+1] - out[i]`. The default step size is `1`.
                    If step is specified as a position argument, start must
                    also be given.

            Returns:

            Array of evenly spaced values. For floating point arguments, the
            length of the result is `ceil((stop - start)/step)`. Because of
            floating point overflow, this rule may result in the last element
            of out being greater than stop.)",
            true
        }
    };

    ///////////////////////////////////////////////////////////////////////////
    arange::arange(primitive_arguments_type&& args,
            std::string const& name, std::string const& codename)
      : primitive_component_base(std::move(args), name, codename)
      , dtype_(extract_dtype(name_))
    {}

    ///////////////////////////////////////////////////////////////////////////
    template <typename T>
    primitive_argument_type arange::arange_helper(
        primitive_arguments_type&& args) const
    {

        T start = T(0);
        T step = T(1);
        T stop;

        if (args.size() > 1)
        {
          start = extract_scalar_data<T>(args[0], name_, codename_);
          stop = extract_scalar_data<T>(args[1], name_, codename_);

        }
        else
        {
          stop = extract_scalar_data<T>(args[0], name_, codename_);
        }

        if (args.size() == 3)
        {
          step = extract_scalar_data<T>(args[2], name_, codename_);
        }

        if (step == T(0))
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "phylanx::execution_tree::primitives::arange_helper",
                generate_error_message(
                    "the arange primitive requires a non-zero step"));
        }

        // because of round-off effects we might end up having one more
        // element than expected
        blaze::DynamicVector<T> result(
            (std::max)(T((stop - start) / step), T(0)) + T(1));

        std::size_t i = 0;

        if (step > T(0))
        {
            for (T val = start; val < stop; val += step, ++i)
            {
                result[i] = val;
            }
        }
        else
        {
            for (T val = start; val > stop; val += step, ++i)
            {
                result[i] = val;
            }
        }

        result.resize(i);
        return primitive_argument_type{std::move(result)};
    }

    hpx::future<primitive_argument_type> arange::eval(
        primitive_arguments_type const& operands,
        primitive_arguments_type const& args) const
    {
        if (operands.size() < 1 || operands.size() > 3)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "phylanx::execution_tree::primitives::arange",
                generate_error_message(
                    "the arange primitive requires one, two, or three arguments."));
        }

        for (std::size_t i = 0; i != operands.size(); ++i)
        {
            if (!valid(operands[i]))
            {
                HPX_THROW_EXCEPTION(hpx::bad_parameter,
                    "arange::eval",
                    generate_error_message(
                        "at least one of the arguments passed to arange is "
                        "not valid"));
            }
        }

        auto this_ = this->shared_from_this();
        return hpx::dataflow(hpx::launch::sync, hpx::util::unwrapping(
            [this_ = std::move(this_)](primitive_arguments_type&& args)
            -> primitive_argument_type
            {
                node_data_type t = this_->dtype_;
                if (t == node_data_type_unknown)
                {
                    t = extract_common_type(args);
                }

                switch (t)
                {
                case node_data_type_bool:
                    return this_->arange_helper<std::uint8_t>(std::move(args));

                case node_data_type_int64:
                    return this_->arange_helper<std::int64_t>(std::move(args));

                case node_data_type_double:
                    return this_->arange_helper<double>(std::move(args));

                default:
                    break;
                }

                HPX_THROW_EXCEPTION(hpx::bad_parameter,
                    "phylanx::execution_tree::primitives::arange::eval",
                    this_->generate_error_message(
                        "the arange primitive requires for all arguments to "
                            "be numeric data types"));
            }),
            detail::map_operands(
                operands, functional::value_operand{}, args,
                name_, codename_));
    }

    //////////////////////////////////////////////////////////////////////////
    hpx::future<primitive_argument_type> arange::eval(
        primitive_arguments_type const& args) const
    {
        if (this->no_operands())
        {
            return eval(args, noargs);
        }
        return eval(this->operands(), args);
    }
}}}
