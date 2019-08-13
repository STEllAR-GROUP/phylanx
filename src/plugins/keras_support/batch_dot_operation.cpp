// Copyright (c) 2019 Bita Hasheminezhad
// Copyright (c) 2019 Hartmut Kaiser
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>
#include <phylanx/execution_tree/primitives/node_data_helpers.hpp>
#include <phylanx/ir/node_data.hpp>
#include <phylanx/plugins/keras_support/batch_dot_operation.hpp>

#include <hpx/include/lcos.hpp>
#include <hpx/include/naming.hpp>
#include <hpx/include/util.hpp>
#include <hpx/errors/throw_exception.hpp>

#include <array>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include <blaze/Math.h>
#include <blaze_tensor/Math.h>

///////////////////////////////////////////////////////////////////////////////
namespace phylanx { namespace execution_tree { namespace primitives
{
    ///////////////////////////////////////////////////////////////////////////
    match_pattern_type const batch_dot_operation::match_data = {
        hpx::util::make_tuple("batch_dot",
            std::vector<std::string>{
                "batch_dot(_1, _2)", "batch_dot(_1, _2, _3)"},
            &create_batch_dot_operation, &create_primitive<batch_dot_operation>,
            R"(
            x, y, axes
            Args:
                x (array) : a matrix, a tensor or a quatern
                y (array) : a matrix, a tensor or a quatern
                axes (optional, integer or tuple of integers): None by default.
                    Target dimensions to be reduced.
            Returns:
            The batch dot along specified axes. For more detail refer to
            https://keras.io/backend/#batch_dot)")};

    ///////////////////////////////////////////////////////////////////////////
    batch_dot_operation::batch_dot_operation(
            primitive_arguments_type&& operands,
            std::string const& name, std::string const& codename)
      : primitive_component_base(std::move(operands), name, codename)
    {}

    ///////////////////////////////////////////////////////////////////////////
    std::size_t batch_dot_operation::positivize_axis(
        std::int64_t axis, std::size_t const& ndim) const
    {
        switch (ndim)
        {
        case 2:
            axis += 2; return axis;
        case 3:
            axis += 3; return axis;
        case 4:
            axis += 4; return axis;
        default:
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "batch_dot_operation::positivize_axis",
                generate_error_message(
                    "the operand has an unsupported number of dimensions"));
        }
    }

    bool batch_dot_operation::validate_axes(ir::range const& axes,
        std::size_t&& ndims_a, std::size_t&& ndims_b,
        std::array<std::size_t, PHYLANX_MAX_DIMENSIONS>&& dims_a,
        std::array<std::size_t, PHYLANX_MAX_DIMENSIONS>&& dims_b,
        std::size_t& axis_a, std::size_t& axis_b) const
    {
        if (ndims_a < 2 || ndims_b < 2)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "batch_dot_operation::validate_axes",
                generate_error_message(
                    "batch_dot_operation requires the inputs of rank 2 "
                    "or more"));
        }

        if (axes.size() > 2)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "batch_dot_operation::validate_axes",
                generate_error_message(
                    "axes can only be an integer or a tuple of two integers"));
        }

        if (axes.size() != 0)
        {
            auto it = axes.begin();

            if (is_list_operand_strict(*it))
            {
                HPX_THROW_EXCEPTION(hpx::bad_parameter,
                    "batch_dot_operation::validate_axes",
                    generate_error_message(
                        "multiple target dimensions are not supported. Axes "
                        "can only be an integer or a tuple of two integers"));
            }

            std::int64_t a = extract_scalar_integer_value_strict(*it);
            axis_a = a >= 0 ? a : positivize_axis(a, ndims_a);

            if (axes.size() == 1)
            {
                axis_b = a >= 0 ? a : positivize_axis(a, ndims_b);
            }
            else
            {
                auto it2 = ++it;
                if (is_list_operand_strict(*it2))
                    HPX_THROW_EXCEPTION(hpx::bad_parameter,
                        "batch_dot_operation::validate_axes",
                        generate_error_message(
                            "multiple target dimensions are not supported. Axes "
                            "can only be an integer or a tuple of two integers"));

                std::int64_t b = extract_scalar_integer_value_strict(*it2);
                axis_b = b >= 0 ? b : positivize_axis(b, ndims_b);
            }
        }
        else
        {
            axis_a = ndims_a - 1;
            switch (ndims_b)
            {
            case 2: HPX_FALLTHROUGH;
            case 3:
                axis_b = 1; break;
            case 4:
                axis_b = 2; break;
            default:
                HPX_THROW_EXCEPTION(hpx::bad_parameter,
                    "batch_dot_operation::validate_axes",
                    generate_error_message("right hand side operand has "
                                           "unsupported number of dimensions"));
            }
        }

        if (axis_a == 0 || axis_b == 0)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "batch_dot_operation::validate_axes",
                generate_error_message(
                    "cannot perform batch dot over axis 0"));
        }

        if (axis_a >= ndims_a)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "batch_dot_operation::validate_axes",
                generate_error_message(
                    "the given axis corresponding to "
                    "the left operand is greater than its dimension"));
        }

        if (axis_b >= ndims_b)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "batch_dot_operation::validate_axes",
                generate_error_message(
                    "the given axis corresponding to "
                    "the right operand is greater than its dimension"));
        }

        if (dims_a[0] != dims_b[0])
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "batch_dot_operation::validate_axes",
                generate_error_message("cannot perform batch dot on inputs "
                    "with different batch sizes."));
        }

        if (dims_a[axis_a] != dims_b[axis_b])
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "batch_dot_operation::validate_axes",
                generate_error_message(
                    "operands have incompatible number of dimensions"));
        }
        return true;
    }

    ///////////////////////////////////////////////////////////////////////////
    hpx::future<primitive_argument_type> batch_dot_operation::eval(
        primitive_arguments_type const& operands,
        primitive_arguments_type const& args, eval_context ctx) const
    {
        if (operands.size() != 2 && operands.size() != 3)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "batch_dot_operation::eval",
                generate_error_message(
                    "the batch_dot_operation primitive requires two or three "
                        "operands"));
        }

        for (auto const& i : operands)
        {
            if (!valid(i))
            {
                HPX_THROW_EXCEPTION(hpx::bad_parameter,
                    "batch_dot_operation::eval",
                    generate_error_message(
                        "the batch_dot_operation primitive requires that the "
                        "arguments given by the operands array are valid"));
            }
        }

        auto this_ = this->shared_from_this();
        if (operands.size() == 2)
        {
            return hpx::dataflow(hpx::launch::sync, hpx::util::unwrapping(
                [this_ = std::move(this_)](primitive_argument_type&& op1,
                    primitive_argument_type&& op2)
                ->primitive_argument_type
            {
                std::size_t axis_a;
                std::size_t axis_b;
                ir::range axes(0); // an empty range

                if (this_->validate_axes(axes,
                        extract_numeric_value_dimension(
                            op1, this_->name_, this_->codename_),
                        extract_numeric_value_dimension(
                            op2, this_->name_, this_->codename_),
                        extract_numeric_value_dimensions(
                            op1, this_->name_, this_->codename_),
                        extract_numeric_value_dimensions(
                            op2, this_->name_, this_->codename_),
                        axis_a, axis_b))
                {
                    switch (extract_common_type(op1, op2))
                    {
                    case node_data_type_bool:
                        return this_->batch_dot_nd(
                            extract_boolean_value(
                                std::move(op1), this_->name_, this_->codename_),
                            extract_boolean_value(
                                std::move(op2), this_->name_, this_->codename_));

                    case node_data_type_int64:
                        return this_->batch_dot_nd(
                            extract_integer_value(
                                std::move(op1), this_->name_, this_->codename_),
                            extract_integer_value(
                                std::move(op2), this_->name_, this_->codename_));

                    case node_data_type_unknown:
                        HPX_FALLTHROUGH;
                    case node_data_type_double:
                        return this_->batch_dot_nd(
                            extract_numeric_value(
                                std::move(op1), this_->name_, this_->codename_),
                            extract_numeric_value(
                                std::move(op2), this_->name_, this_->codename_));

                    default:
                        break;
                    }
                    HPX_THROW_EXCEPTION(hpx::bad_parameter,
                        "batch_dot_operation::eval",
                        this_->generate_error_message(
                            "the dot primitive requires for all arguments to "
                            "be numeric data types"));
                }
                HPX_THROW_EXCEPTION(hpx::bad_parameter,
                    "batch_dot_operation::eval",
                    this_->generate_error_message(
                        "operands have incompatible number of dimensions"));
            }),
                value_operand(operands[0], args, name_, codename_, ctx),
                value_operand(operands[1], args, name_, codename_, ctx));
        }

        return hpx::dataflow(hpx::launch::sync,
            hpx::util::unwrapping([this_ = std::move(this_)](
                                      primitive_argument_type&& op1,
                                      primitive_argument_type&& op2,
                                      ir::range&& axes)
                                      -> primitive_argument_type {
                std::size_t axis_a;
                std::size_t axis_b;
                if (this_->validate_axes(axes,
                        extract_numeric_value_dimension(
                            op1, this_->name_, this_->codename_),
                        extract_numeric_value_dimension(
                            op2, this_->name_, this_->codename_),
                        extract_numeric_value_dimensions(
                            op1, this_->name_, this_->codename_),
                        extract_numeric_value_dimensions(
                            op2, this_->name_, this_->codename_),
                        axis_a, axis_b))
                {
                    switch (extract_common_type(op1, op2))
                    {
                    case node_data_type_bool:
                        return this_->batch_dot_nd(
                            extract_boolean_value(
                                std::move(op1), this_->name_, this_->codename_),
                            extract_boolean_value(
                                std::move(op2), this_->name_, this_->codename_),
                            axis_a, axis_b);

                    case node_data_type_int64:
                        return this_->batch_dot_nd(
                            extract_integer_value(
                                std::move(op1), this_->name_, this_->codename_),
                            extract_integer_value(
                                std::move(op2), this_->name_, this_->codename_),
                            axis_a, axis_b);

                    case node_data_type_unknown:
                        HPX_FALLTHROUGH;
                    case node_data_type_double:
                        return this_->batch_dot_nd(
                            extract_numeric_value(
                                std::move(op1), this_->name_, this_->codename_),
                            extract_numeric_value(
                                std::move(op2), this_->name_, this_->codename_),
                            axis_a, axis_b);

                    default:
                        break;
                    }
                    HPX_THROW_EXCEPTION(hpx::bad_parameter,
                        "batch_dot_operation::eval",
                        this_->generate_error_message(
                            "the batch_dot requires for all arguments to "
                            "be numeric data types"));
                }
                HPX_THROW_EXCEPTION(hpx::bad_parameter,
                    "batch_dot_operation::eval",
                    this_->generate_error_message(
                        "the given axes is incompatible with the dimensions of "
                        "the operands"));
            }),
            value_operand(operands[0], args, name_, codename_, ctx),
            value_operand(operands[1], args, name_, codename_, ctx),
            list_operand(operands[2], args, name_, codename_, ctx));
    }
}}}
