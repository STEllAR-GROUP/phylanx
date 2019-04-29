//  Copyright (c) 2017-2018 Hartmut Kaiser
//  Copyright (c) 2017 Parsa Amini
//  Copyright (c) 2019 Bita Hasheminezhad
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>
#include <phylanx/execution_tree/primitives/node_data_helpers.hpp>
#include <phylanx/ir/node_data.hpp>
#include <phylanx/plugins/matrixops/dot_operation.hpp>

#include <hpx/include/lcos.hpp>
#include <hpx/include/naming.hpp>
#include <hpx/include/util.hpp>
#include <hpx/throw_exception.hpp>

#include <cstddef>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#if defined(PHYLANX_HAVE_BLAZE_TENSOR)
#include <blaze_tensor/Math.h>
#endif
///////////////////////////////////////////////////////////////////////////////
namespace phylanx { namespace execution_tree { namespace primitives
{
    ///////////////////////////////////////////////////////////////////////////
    std::vector<match_pattern_type> const dot_operation::match_data = {
        match_pattern_type{"outer", std::vector<std::string>{"outer(_1, _2)"},
            &create_dot_operation, &create_primitive<dot_operation>, R"(
            a, b
            Args:

                a (array) : a scalar, vector, matrix or a tensor.Input is
                    flattened if not already 1-dimensional.
                b (array) : a scalar, vector, matrix or a tensor.Input is
                    flattened if not already 1-dimensional.

            Returns:

            Computes the outer product of two arrays. Always returns a matrix)"},

        match_pattern_type{"dot", std::vector<std::string>{"dot(_1, _2)"},
            &create_dot_operation, &create_primitive<dot_operation>, R"(
            a, b
            Args:

                a (array) : a scalar, vector, matrix or a tensor
                b (array) : a scalar, vector, matrix or a tensor

            Returns:

            The dot product of two arrays: `a` and `b`. The dot product of an
            N-D array and an M-D array is of dimension N+M-2)"},

        match_pattern_type{"tensordot",
            std::vector<std::string>{
                "tensordot(_1, _2)", "tensordot(_1, _2, _3)"},
            &create_dot_operation, &create_primitive<dot_operation>, R"(
            a, b, axes
            Args:

                a (array) : a vector, matrix or a tensor
                b (array) : a vector, matrix or a tensor
                axes(optional, integer or tuple of integers): if a scalar N, sum
                    over the last N axes of a and the first N axes of b in
                    order. The sizes of the corresponding axes must match.
                    If given `(axes_a,axes_b)`, sum the products of two arrays
                    elements (components) over the axes specified by `a_axes`
                    and `b_axes`.
                    The default is 2 (scalar axis).

            Returns:

            The tensor dot product along specified axes for arrays>=1-D.)"}};

    ///////////////////////////////////////////////////////////////////////////
    dot_operation::dot_mode extract_dot_mode(std::string const& name)
    {
        dot_operation::dot_mode result = dot_operation::dot_product;

        if (name.find("outer") != std::string::npos)
        {
            result = dot_operation::outer_product;
        }
        else if (name.find("tensordot") != std::string::npos)
        {
            result = dot_operation::doubledot_product;
        }
        return result;
    }

    dot_operation::dot_operation(
            primitive_arguments_type&& operands,
            std::string const& name, std::string const& codename)
      : primitive_component_base(std::move(operands), name, codename)
        , mode_(extract_dot_mode(name_))
    {}

    ///////////////////////////////////////////////////////////////////////////
    void dot_operation::positivize_axis(
        val_type& axis, std::size_t const& dim) const
    {
        switch (dim)
        {
        case 0:
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "dot_operation::positivize_axis",
                generate_error_message("tuple index out of range"));
        case 1:
            axis += 1; break;
        case 2:
            axis += 2; break;
        case 3:
            axis += 3; break;
        default:
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "dot_operation::positivize_axis",
                generate_error_message(
                    "the operand has >3 dimensions "
                    "which is not supported"));
        }
    }

    ///////////////////////////////////////////////////////////////////////////
    primitive_argument_type dot_operation::dot0d(
        primitive_argument_type&& lhs, primitive_argument_type&& rhs) const
    {
        switch (extract_common_type(lhs, rhs))
        {
        case node_data_type_bool:
            return dot0d(
                extract_boolean_value(std::move(lhs), name_, codename_),
                extract_boolean_value(std::move(rhs), name_, codename_));

        case node_data_type_int64:
            return dot0d(
                extract_integer_value(std::move(lhs), name_, codename_),
                extract_integer_value(std::move(rhs), name_, codename_));

        case node_data_type_unknown: HPX_FALLTHROUGH;
        case node_data_type_double:
            return dot0d(
                extract_numeric_value(std::move(lhs), name_, codename_),
                extract_numeric_value(std::move(rhs), name_, codename_));

        default:
            break;
        }

        HPX_THROW_EXCEPTION(hpx::bad_parameter,
            "dot_operation::dot0d",
            generate_error_message(
                "the dot primitive requires for all arguments to "
                    "be numeric data types"));
    }

    primitive_argument_type dot_operation::dot1d(
        primitive_argument_type&& lhs, primitive_argument_type&& rhs) const
    {
        switch (extract_common_type(lhs, rhs))
        {
        case node_data_type_bool:
            return dot1d(
                extract_boolean_value(std::move(lhs), name_, codename_),
                extract_boolean_value(std::move(rhs), name_, codename_));

        case node_data_type_int64:
            return dot1d(
                extract_integer_value(std::move(lhs), name_, codename_),
                extract_integer_value(std::move(rhs), name_, codename_));

        case node_data_type_unknown: HPX_FALLTHROUGH;
        case node_data_type_double:
            return dot1d(
                extract_numeric_value(std::move(lhs), name_, codename_),
                extract_numeric_value(std::move(rhs), name_, codename_));

        default:
            break;
        }

        HPX_THROW_EXCEPTION(hpx::bad_parameter,
            "dot_operation::dot1d",
            generate_error_message(
                "the dot primitive requires for all arguments to "
                    "be numeric data types"));
    }

    primitive_argument_type dot_operation::dot2d(
        primitive_argument_type&& lhs, primitive_argument_type&& rhs) const
    {
        switch (extract_common_type(lhs, rhs))
        {
        case node_data_type_bool:
            return dot2d(
                extract_boolean_value(std::move(lhs), name_, codename_),
                extract_boolean_value(std::move(rhs), name_, codename_));

        case node_data_type_int64:
            return dot2d(
                extract_integer_value(std::move(lhs), name_, codename_),
                extract_integer_value(std::move(rhs), name_, codename_));

        case node_data_type_unknown: HPX_FALLTHROUGH;
        case node_data_type_double:
            return dot2d(
                extract_numeric_value(std::move(lhs), name_, codename_),
                extract_numeric_value(std::move(rhs), name_, codename_));

        default:
            break;
        }

        HPX_THROW_EXCEPTION(hpx::bad_parameter,
            "dot_operation::dot2d",
            generate_error_message(
                "the dot primitive requires for all arguments to "
                    "be numeric data types"));
    }

#if defined(PHYLANX_HAVE_BLAZE_TENSOR)
    primitive_argument_type dot_operation::dot3d(
        primitive_argument_type&& lhs, primitive_argument_type&& rhs) const
    {
        switch (extract_common_type(lhs, rhs))
        {
        case node_data_type_bool:
            return dot3d(
                extract_boolean_value(std::move(lhs), name_, codename_),
                extract_boolean_value(std::move(rhs), name_, codename_));

        case node_data_type_int64:
            return dot3d(
                extract_integer_value(std::move(lhs), name_, codename_),
                extract_integer_value(std::move(rhs), name_, codename_));

        case node_data_type_unknown: HPX_FALLTHROUGH;
        case node_data_type_double:
            return dot3d(
                extract_numeric_value(std::move(lhs), name_, codename_),
                extract_numeric_value(std::move(rhs), name_, codename_));

        default:
            break;
        }

        HPX_THROW_EXCEPTION(hpx::bad_parameter,
            "dot_operation::dot3d",
            generate_error_message(
                "the dot primitive requires for all arguments to "
                "be numeric data types"));
    }
#endif

    primitive_argument_type dot_operation::outer1d(
        primitive_argument_type&& lhs, primitive_argument_type&& rhs) const
    {
        switch (extract_common_type(lhs, rhs))
        {
        case node_data_type_bool:
            return outer1d(
                extract_boolean_value(std::move(lhs), name_, codename_),
                extract_boolean_value(std::move(rhs), name_, codename_));

        case node_data_type_int64:
            return outer1d(
                extract_integer_value(std::move(lhs), name_, codename_),
                extract_integer_value(std::move(rhs), name_, codename_));

        case node_data_type_unknown:
            HPX_FALLTHROUGH;
        case node_data_type_double:
            return outer1d(
                extract_numeric_value(std::move(lhs), name_, codename_),
                extract_numeric_value(std::move(rhs), name_, codename_));

        default:
            break;
        }

        HPX_THROW_EXCEPTION(hpx::bad_parameter,
            "dot_operation::outer1d",
            generate_error_message(
                "the dot_operation primitive requires for all arguments to "
                "be numeric data types"));
    }

    primitive_argument_type dot_operation::outer2d(
        primitive_argument_type&& lhs, primitive_argument_type&& rhs) const
    {
        switch (extract_common_type(lhs, rhs))
        {
        case node_data_type_bool:
            return outer2d(
                extract_boolean_value(std::move(lhs), name_, codename_),
                extract_boolean_value(std::move(rhs), name_, codename_));

        case node_data_type_int64:
            return outer2d(
                extract_integer_value(std::move(lhs), name_, codename_),
                extract_integer_value(std::move(rhs), name_, codename_));

        case node_data_type_unknown:
            HPX_FALLTHROUGH;
        case node_data_type_double:
            return outer2d(
                extract_numeric_value(std::move(lhs), name_, codename_),
                extract_numeric_value(std::move(rhs), name_, codename_));

        default:
            break;
        }

        HPX_THROW_EXCEPTION(hpx::bad_parameter,
            "dot_operation::outer2d",
            generate_error_message(
                "the dot_operation primitive requires for all arguments to "
                "be numeric data types"));
    }

#if defined(PHYLANX_HAVE_BLAZE_TENSOR)
    primitive_argument_type dot_operation::outer3d(
        primitive_argument_type&& lhs, primitive_argument_type&& rhs) const
    {
        switch (extract_common_type(lhs, rhs))
        {
        case node_data_type_bool:
            return outer3d(
                extract_boolean_value(std::move(lhs), name_, codename_),
                extract_boolean_value(std::move(rhs), name_, codename_));

        case node_data_type_int64:
            return outer3d(
                extract_integer_value(std::move(lhs), name_, codename_),
                extract_integer_value(std::move(rhs), name_, codename_));

        case node_data_type_unknown:
            HPX_FALLTHROUGH;
        case node_data_type_double:
            return outer3d(
                extract_numeric_value(std::move(lhs), name_, codename_),
                extract_numeric_value(std::move(rhs), name_, codename_));

        default:
            break;
        }

        HPX_THROW_EXCEPTION(hpx::bad_parameter,
            "dot_operation::outer3d",
            generate_error_message(
                "the dot_operation primitive requires for all arguments to "
                "be numeric data types"));
    }
#endif

    primitive_argument_type dot_operation::contraction2d(
        primitive_argument_type&& lhs, primitive_argument_type&& rhs) const
    {
        switch (extract_common_type(lhs, rhs))
        {
        case node_data_type_bool:
            return contraction2d(
                extract_boolean_value(std::move(lhs), name_, codename_),
                extract_boolean_value(std::move(rhs), name_, codename_));

        case node_data_type_int64:
            return contraction2d(
                extract_integer_value(std::move(lhs), name_, codename_),
                extract_integer_value(std::move(rhs), name_, codename_));

        case node_data_type_unknown:
            HPX_FALLTHROUGH;
        case node_data_type_double:
            return contraction2d(
                extract_numeric_value(std::move(lhs), name_, codename_),
                extract_numeric_value(std::move(rhs), name_, codename_));

        default:
            break;
        }

        HPX_THROW_EXCEPTION(hpx::bad_parameter,
            "dot_operation::contraction2d",
            generate_error_message(
                "the dot_operation primitive requires for all arguments to "
                "be numeric data types"));
    }

#if defined(PHYLANX_HAVE_BLAZE_TENSOR)
    primitive_argument_type dot_operation::contraction3d(
        primitive_argument_type&& lhs, primitive_argument_type&& rhs) const
    {
        switch (extract_common_type(lhs, rhs))
        {
        case node_data_type_bool:
            return contraction3d(
                extract_boolean_value(std::move(lhs), name_, codename_),
                extract_boolean_value(std::move(rhs), name_, codename_));

        case node_data_type_int64:
            return contraction3d(
                extract_integer_value(std::move(lhs), name_, codename_),
                extract_integer_value(std::move(rhs), name_, codename_));

        case node_data_type_unknown:
            HPX_FALLTHROUGH;
        case node_data_type_double:
            return contraction3d(
                extract_numeric_value(std::move(lhs), name_, codename_),
                extract_numeric_value(std::move(rhs), name_, codename_));

        default:
            break;
        }

        HPX_THROW_EXCEPTION(hpx::bad_parameter,
            "dot_operation::contraction3d",
            generate_error_message(
                "the dot_operation primitive requires for all arguments to "
                "be numeric data types"));
    }
#endif

    primitive_argument_type dot_operation::tensordot_range_of_scalars(
        primitive_argument_type&& lhs, primitive_argument_type&& rhs,
        val_type axis_a, val_type axis_b) const
    {
        switch (extract_common_type(lhs, rhs))
        {
        case node_data_type_bool:
            return tensordot_range_of_scalars(
                extract_boolean_value(std::move(lhs), name_, codename_),
                extract_boolean_value(std::move(rhs), name_, codename_), axis_a,
                axis_b);

        case node_data_type_int64:
            return tensordot_range_of_scalars(
                extract_integer_value(std::move(lhs), name_, codename_),
                extract_integer_value(std::move(rhs), name_, codename_), axis_a,
                axis_b);

        case node_data_type_unknown:
            HPX_FALLTHROUGH;
        case node_data_type_double:
            return tensordot_range_of_scalars(
                extract_numeric_value(std::move(lhs), name_, codename_),
                extract_numeric_value(std::move(rhs), name_, codename_), axis_a,
                axis_b);

        default:
            break;
        }

        HPX_THROW_EXCEPTION(hpx::bad_parameter,
            "dot_operation::tensordot_range_of_scalars",
            generate_error_message(
                "the dot_operation primitive requires for all arguments to "
                "be numeric data types"));
    }

    primitive_argument_type dot_operation::outer_nd_helper(
        primitive_argument_type&& lhs, primitive_argument_type&& rhs) const
    {
        switch (extract_common_type(lhs, rhs))
        {
        case node_data_type_bool:
            return outer_nd_helper(
                extract_boolean_value(std::move(lhs), name_, codename_),
                extract_boolean_value(std::move(rhs), name_, codename_));

        case node_data_type_int64:
            return outer_nd_helper(
                extract_integer_value(std::move(lhs), name_, codename_),
                extract_integer_value(std::move(rhs), name_, codename_));

        case node_data_type_unknown:
            HPX_FALLTHROUGH;
        case node_data_type_double:
            return outer_nd_helper(
                extract_numeric_value(std::move(lhs), name_, codename_),
                extract_numeric_value(std::move(rhs), name_, codename_));

        default:
            break;
        }

        HPX_THROW_EXCEPTION(hpx::bad_parameter,
            "dot_operation::outer_nd_helper",
            generate_error_message(
                "the dot_operation primitive requires for all arguments to "
                "be numeric data types"));
    }

    ///////////////////////////////////////////////////////////////////////////
    primitive_argument_type dot_operation::dot_nd(
        primitive_argument_type&& lhs, primitive_argument_type&& rhs) const
    {
        switch (extract_numeric_value_dimension(lhs, name_, codename_))
        {
        case 0:
            return dot0d(std::move(lhs), std::move(rhs));

        case 1:
            return dot1d(std::move(lhs), std::move(rhs));

        case 2:
            return dot2d(std::move(lhs), std::move(rhs));

#if defined(PHYLANX_HAVE_BLAZE_TENSOR)
        case 3:
            return dot3d(std::move(lhs), std::move(rhs));
#endif

        default:
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "dot_operation::dot_nd",
                generate_error_message("left hand side operand has unsupported "
                                       "number of dimensions"));
        }
    }

    primitive_argument_type dot_operation::outer_nd(
        primitive_argument_type&& lhs, primitive_argument_type&& rhs) const
    {
        switch (extract_numeric_value_dimension(lhs, name_, codename_))
        {
        case 0:
            //outer0d has the same functionality as dot0d
            return dot0d(std::move(lhs), std::move(rhs));

        case 1:
            return outer1d(std::move(lhs), std::move(rhs));

        case 2:
            return outer2d(std::move(lhs), std::move(rhs));

#if defined(PHYLANX_HAVE_BLAZE_TENSOR)
        case 3:
            return outer3d(std::move(lhs), std::move(rhs));
#endif

        default:
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "dot_operation::outer_nd",
                generate_error_message("left hand side operand has unsupported "
                                       "number of dimensions"));
        }
    }

    primitive_argument_type dot_operation::contraction_nd(
        primitive_argument_type&& lhs, primitive_argument_type&& rhs) const
    {
        switch (extract_numeric_value_dimension(lhs, name_, codename_))
        {

        case 2:
            return contraction2d(std::move(lhs), std::move(rhs));

#if defined(PHYLANX_HAVE_BLAZE_TENSOR)
        case 3:
            return contraction3d(std::move(lhs), std::move(rhs));
#endif

        default:
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "dot_operation::contraction_nd",
                generate_error_message("left hand side operand has unsupported "
                                       "number of dimensions"));
        }
    }

    ///////////////////////////////////////////////////////////////////////////
    primitive_argument_type dot_operation::tensordot_scalar_axis(
        primitive_argument_type&& lhs, primitive_argument_type&& rhs,
        ir::range&& axes) const
    {
        std::size_t axis =
            extract_scalar_integer_value_strict(*axes.begin()) > 0 ?
            extract_scalar_integer_value_strict(*axes.begin()) :
            0;

        if (extract_numeric_value_dimension(lhs) < axis ||
            extract_numeric_value_dimension(rhs) < axis)
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "dot_operation::tensordot_scalar_axis",
                generate_error_message(
                    "the given axes should not be "
                    "greater than any of operands dimensions"));
        else
        {
            switch (axis)
            {
            case 0:
                return outer_nd(std::move(lhs), std::move(rhs));

            case 1:
                return dot_nd(std::move(lhs), std::move(rhs));

            case 2:
                return contraction_nd(std::move(lhs), std::move(rhs));

            default:
                HPX_THROW_EXCEPTION(hpx::bad_parameter,
                    "dot_operation::tensordot_scalar_axis",
                    generate_error_message("the given axes is out of range. A "
                                           "scalar axis should be <3"));
            }
        }
    }

    primitive_argument_type dot_operation::tensordot_range_axes(
        primitive_argument_type&& lhs, primitive_argument_type&& rhs,
        ir::range&& axes) const
    {
        auto it = axes.begin();
        if (!is_list_operand_strict(*it) && !is_list_operand_strict(*++it))
        {
            val_type axis_a = extract_scalar_integer_value_strict(*axes.begin());
            val_type axis_b = extract_scalar_integer_value_strict(*it);

            if (axis_a < 0)
                positivize_axis(axis_a,
                    extract_numeric_value_dimension(lhs, name_, codename_));

            if (axis_b < 0)
                positivize_axis(axis_b,
                    extract_numeric_value_dimension(rhs, name_, codename_));

            return tensordot_range_of_scalars(
                std::move(lhs), std::move(rhs), axis_a, axis_b);
        }
        HPX_THROW_EXCEPTION(hpx::bad_parameter,
            "dot_operation::tensordot_range_axes",
            generate_error_message(
                "range of ranges is not supportedby this version"));
    }

    ///////////////////////////////////////////////////////////////////////////
    hpx::future<primitive_argument_type> dot_operation::eval(
        primitive_arguments_type const& operands,
        primitive_arguments_type const& args, eval_context ctx) const
    {
        if (operands.size() != 2 && operands.size() != 3)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "dot_operation::eval",
                generate_error_message(
                    "the dot_operation primitive requires exactly "
                        "two or three operands"));
        }

        if (!valid(operands[0]) || !valid(operands[1]))
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "dot_operation::eval",
                generate_error_message(
                    "the dot_operation primitive requires that the "
                        "arguments given by the operands array are valid"));
        }

        auto this_ = this->shared_from_this();
        if (operands.size() == 2)
        {
            auto&& op0 =
                value_operand(operands[0], args, name_, codename_, ctx);

            return hpx::dataflow(hpx::launch::sync,
                [this_ = std::move(this_)](
                        hpx::future<primitive_argument_type>&& op1,
                        hpx::future<primitive_argument_type>&& op2)
                -> primitive_argument_type
                {
                    if (this_->mode_ == outer_product)
                    {
                        return this_->outer_nd_helper(op1.get(), op2.get());
                    }
                    else if (this_->mode_ == dot_product)
                    {
                        return this_->dot_nd(op1.get(), op2.get());
                    }
                    else if (this_->mode_ == doubledot_product)
                    {
                        return this_->contraction_nd(op1.get(), op2.get());
                    }

                    HPX_THROW_EXCEPTION(hpx::bad_parameter,
                        "dot_operation::eval",
                        this_->generate_error_message(
                            "unsupported dot mode requested"));
                },
                std::move(op0),
                value_operand(operands[1], args, name_, codename_, std::move(ctx)));
        }
        else if (operands.size() == 3 && valid(operands[2]))
        {
            if (mode_ == dot_product || mode_ == outer_product)
            {
                HPX_THROW_EXCEPTION(hpx::bad_parameter,
                    "dot_operation::eval",
                    this_->generate_error_message(
                        "the dot/outer product requires exactly two operands"));
            }

            if (mode_ == doubledot_product)
            {
                auto&& op0 =
                    value_operand(operands[0], args, name_, codename_, ctx);
                auto&& op1 =
                    value_operand(operands[1], args, name_, codename_, ctx);

                return hpx::dataflow(hpx::launch::sync,
                    [this_ = std::move(this_)](
                            hpx::future<primitive_argument_type>&& op1,
                            hpx::future<primitive_argument_type>&& op2,
                            hpx::future<ir::range>&& f_axes)
                    -> primitive_argument_type
                    {
                        auto&& axes = f_axes.get();

                        switch (axes.size())
                        {
                        case 1:
                            return this_->tensordot_scalar_axis(
                                op1.get(), op2.get(), std::move(axes));

                        case 2:
                            return this_->tensordot_range_axes(
                                op1.get(), op2.get(), std::move(axes));

                        default:
                            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                                "dot_operation::eval",
                                this_->generate_error_message(
                                    "the axes can only be an integer, or a tuple "
                                    "indicating a_axes and b_axes where a_axes and "
                                    "b_axes can be integers or tuples of "
                                    "integers"));
                        }
                    },
                    std::move(op0), std::move(op1),
                    list_operand(operands[2], args, name_, codename_, std::move(ctx)));
            }

            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "dot_operation::eval",
                this_->generate_error_message(
                    "unsupported dot mode requested"));
        }

        HPX_THROW_EXCEPTION(hpx::bad_parameter,
            "dot_operation::eval",
            generate_error_message(
                "the dot_operation primitive requires that the "
                "arguments given by the operands array are valid"));
    }
}}}
