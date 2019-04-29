// Copyright (c) 2018 R. Tohid
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>
#include <phylanx/ir/node_data.hpp>
#include <phylanx/plugins/matrixops/linspace.hpp>

#include <hpx/include/lcos.hpp>
#include <hpx/include/naming.hpp>
#include <hpx/include/util.hpp>
#include <hpx/throw_exception.hpp>

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
    match_pattern_type const linspace::match_data =
    {
        match_pattern_type{
            "linspace",
            std::vector<std::string>{"linspace(_1, _2, _3)"},
            &create_linspace, &create_primitive<linspace>, R"(
            start, end, nelements
            Args:

                start (number) : the start of the numeric range
                end (number) : the end of the numeric range
                nelements (int) : the number of elements

            Returns:

            An array with the requested number of elements, the first "
            of which is `start`, and the last is `end`.)",
            true
        }
    };

    ///////////////////////////////////////////////////////////////////////////
    linspace::linspace(primitive_arguments_type&& args,
            std::string const& name, std::string const& codename)
      : primitive_component_base(std::move(args), name, codename)
      , dtype_(extract_dtype(name_))
    {}

    ///////////////////////////////////////////////////////////////////////////
    template <typename T>
    primitive_argument_type linspace::linspace1d(
        T start, T end, std::int64_t num_samples) const
    {
        if (num_samples < 1)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "phylanx::execution_tree::primitives::"
                    "detail::linspace1d",
                generate_error_message(
                    "the linspace primitive requires at least one interval"));
        }

        if (1 == num_samples)
        {
            blaze::DynamicVector<T> result(1, start);
            return primitive_argument_type{std::move(result)};
        }
        else
        {
            blaze::DynamicVector<T> result(num_samples);
            double dx = (end - start) / (num_samples - 1);
            for (std::size_t i = 0; i != num_samples; ++i)
            {
                result[i] = start + dx * i;
            }
            return primitive_argument_type{std::move(result)};
        }
    }

    primitive_argument_type linspace::linspace1d(
        primitive_argument_type&& start, primitive_argument_type&& end,
        std::int64_t nelements) const
    {
        node_data_type t = dtype_;
        if (t == node_data_type_unknown)
        {
            t = extract_common_type(start, end);
        }

        switch (t)
        {
        case node_data_type_int64:
            return linspace1d(
                extract_scalar_integer_value(std::move(start), name_, codename_),
                extract_scalar_integer_value(std::move(end), name_, codename_),
                nelements);

        case node_data_type_bool:   HPX_FALLTHROUGH;
        case node_data_type_double: HPX_FALLTHROUGH;
        case node_data_type_unknown:
            return linspace1d(
                extract_scalar_numeric_value(std::move(start), name_, codename_),
                extract_scalar_numeric_value(std::move(end), name_, codename_),
                nelements);

        default:
            break;
        }

        HPX_THROW_EXCEPTION(hpx::bad_parameter,
            "linspace::linspace1d",
            generate_error_message(
                "the linspace primitive requires for all arguments to "
                    "be numeric data types"));
    }

    ///////////////////////////////////////////////////////////////////////////
    hpx::future<primitive_argument_type> linspace::eval(
        primitive_arguments_type const& operands,
        primitive_arguments_type const& args, eval_context ctx) const
    {
        if (operands.size() != 3)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "phylanx::execution_tree::primitives::linspace",
                generate_error_message(
                    "the linspace primitive requires exactly three "
                    "arguments."));
        }

        for (std::size_t i = 0; i != operands.size(); ++i)
        {
            if (!valid(operands[i]))
            {
                HPX_THROW_EXCEPTION(hpx::bad_parameter,
                    "linspace::eval",
                    generate_error_message(
                        "at least one of the arguments passed to "
                        "linspace is not valid."));
            }
        }

        auto&& op0 = value_operand(operands[0], args, name_, codename_, ctx);
        auto&& op1 = value_operand(operands[1], args, name_, codename_, ctx);

        auto this_ = this->shared_from_this();
        return hpx::dataflow(hpx::launch::sync,
            [this_ = std::move(this_)](
                    hpx::future<primitive_argument_type>&& start,
                    hpx::future<primitive_argument_type>&& end,
                    hpx::future<std::int64_t>&& nelements)
            -> primitive_argument_type
            {
                return this_->linspace1d(start.get(), end.get(), nelements.get());
            },
            std::move(op0), std::move(op1),
            scalar_integer_operand(operands[2], args, name_, codename_, std::move(ctx)));
    }
}}}
