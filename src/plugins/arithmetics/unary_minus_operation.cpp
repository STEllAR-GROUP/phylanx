// Copyright (c) 2017-2018 Hartmut Kaiser
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>
#include <phylanx/execution_tree/primitives/node_data_helpers.hpp>
#include <phylanx/ir/node_data.hpp>
#include <phylanx/plugins/arithmetics/unary_minus_operation.hpp>

#include <hpx/include/lcos.hpp>
#include <hpx/include/util.hpp>
#include <hpx/throw_exception.hpp>

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
    match_pattern_type const unary_minus_operation::match_data =
    {
        match_pattern_type{"__minus",
            std::vector<std::string>{"-_1", "__minus(_1)"},
            &create_unary_minus_operation,
            &create_primitive<unary_minus_operation>, R"(
            arg
            Args:

                 arg (number): a numeric value\n"

            Returns:

            The negated value arg.)",
            true
        }
    };

    ///////////////////////////////////////////////////////////////////////////
    unary_minus_operation::unary_minus_operation(
            primitive_arguments_type&& operands,
            std::string const& name, std::string const& codename)
      : primitive_component_base(std::move(operands), name, codename)
      , dtype_(extract_dtype(name_))
    {}

    ///////////////////////////////////////////////////////////////////////////
    template <typename T>
    primitive_argument_type unary_minus_operation::neg0d(
        ir::node_data<T>&& op) const
    {
        if (op.is_ref())
        {
            op = -op.scalar();
        }
        else
        {
            op.scalar() = -op.scalar();
        }
        return primitive_argument_type(std::move(op));
    }

    template <typename T>
    primitive_argument_type unary_minus_operation::neg1d(
        ir::node_data<T>&& op) const
    {
        if (op.is_ref())
        {
            op = -op.vector();
        }
        else
        {
            op.vector() = -op.vector();
        }
        return primitive_argument_type(std::move(op));
    }

    template <typename T>
    primitive_argument_type unary_minus_operation::neg2d(
        ir::node_data<T>&& op) const
    {
        if (op.is_ref())
        {
            op = -op.matrix();
        }
        else
        {
            op.matrix() = -op.matrix();
        }
        return primitive_argument_type(std::move(op));
    }

    ///////////////////////////////////////////////////////////////////////////
    primitive_argument_type unary_minus_operation::neg0d(
        primitive_argument_type&& op) const
    {
        node_data_type t = dtype_;
        if (t == node_data_type_unknown)
        {
            t = extract_common_type(op);
        }

        switch (t)
        {
        case node_data_type_bool:
            return neg0d(extract_value_scalar<std::uint8_t>(
                std::move(op), name_, codename_));

        case node_data_type_int64:
            return neg0d(extract_value_scalar<std::int64_t>(
                std::move(op), name_, codename_));

        case node_data_type_unknown: HPX_FALLTHROUGH;
        case node_data_type_double:
            return neg0d(
                extract_value_scalar<double>(std::move(op), name_, codename_));

        default:
            break;
        }

        HPX_THROW_EXCEPTION(hpx::bad_parameter,
            "unary_minus_operation::neg0d",
            generate_error_message("operand has unsupported type"));
    }

    primitive_argument_type unary_minus_operation::neg1d(
        primitive_argument_type&& op) const
    {
        node_data_type t = dtype_;
        if (t == node_data_type_unknown)
        {
            t = extract_common_type(op);
        }

        auto sizes = extract_numeric_value_dimensions(op, name_, codename_);
        switch (t)
        {
        case node_data_type_bool:
            return neg1d(extract_value_vector<std::uint8_t>(
                std::move(op), sizes[0], name_, codename_));

        case node_data_type_int64:
            return neg1d(extract_value_vector<std::int64_t>(
                std::move(op), sizes[0], name_, codename_));

        case node_data_type_unknown: HPX_FALLTHROUGH;
        case node_data_type_double:
            return neg1d(extract_value_vector<double>(
                std::move(op), sizes[0], name_, codename_));

        default:
            break;
        }

        HPX_THROW_EXCEPTION(hpx::bad_parameter,
            "unary_minus_operation::neg1d",
            generate_error_message("operand has unsupported type"));
    }

    primitive_argument_type unary_minus_operation::neg2d(
        primitive_argument_type&& op) const
    {
        node_data_type t = dtype_;
        if (t == node_data_type_unknown)
        {
            t = extract_common_type(op);
        }

        auto sizes = extract_numeric_value_dimensions(op, name_, codename_);
        switch (t)
        {
        case node_data_type_bool:
            return neg2d(extract_value_matrix<std::uint8_t>(
                std::move(op), sizes[0], sizes[1], name_, codename_));

        case node_data_type_int64:
            return neg2d(extract_value_matrix<std::int64_t>(
                std::move(op), sizes[0], sizes[1], name_, codename_));

        case node_data_type_unknown: HPX_FALLTHROUGH;
        case node_data_type_double:
            return neg2d(extract_value_matrix<double>(
                std::move(op), sizes[0], sizes[1], name_, codename_));

        default:
            break;
        }

        HPX_THROW_EXCEPTION(hpx::bad_parameter,
            "unary_minus_operation::neg2d",
            generate_error_message("operand has unsupported type"));
    }

    ///////////////////////////////////////////////////////////////////////////
    hpx::future<primitive_argument_type> unary_minus_operation::eval(
        primitive_arguments_type const& operands,
        primitive_arguments_type const& args, eval_context ctx) const
    {
        if (operands.size() != 1)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "unary_minus_operation::eval",
                generate_error_message(
                    "the unary_minus_operation primitive requires "
                        "exactly one operand"));
        }

        if (!valid(operands[0]))
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "unary_minus_operation::eval",
                generate_error_message(
                    "the unary_minus_operation primitive requires "
                        "that the argument given by the operands "
                        "array is valid"));
        }

        auto this_ = this->shared_from_this();
        return hpx::dataflow(hpx::launch::sync, hpx::util::unwrapping(
            [this_ = std::move(this_)](primitive_argument_type && op)
            -> primitive_argument_type
            {
                std::size_t lhs_dims = extract_numeric_value_dimension(
                    op, this_->name_, this_->codename_);

                switch (lhs_dims)
                {
                case 0:
                    return this_->neg0d(std::move(op));

                case 1:
                    return this_->neg1d(std::move(op));

                case 2:
                    return this_->neg2d(std::move(op));

                default:
                    HPX_THROW_EXCEPTION(hpx::bad_parameter,
                        "unary_minus_operation::eval",
                        this_->generate_error_message(
                            "operand has unsupported number of dimensions"));
                }
            }),
            value_operand(operands[0], args, name_, codename_, std::move(ctx)));
    }
}}}
