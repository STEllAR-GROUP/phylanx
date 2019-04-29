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
#include <hpx/throw_exception.hpp>

#include <array>
#include <cstddef>
#include <cstdint>
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
    match_pattern_type const batch_dot_operation::match_data = {
        hpx::util::make_tuple("batch_dot",
            std::vector<std::string>{
                "batch_dot(_1, _2)", "batch_dot(_1, _2, _3)"},
            &create_batch_dot_operation, &create_primitive<batch_dot_operation>,
            R"(
            x, y, axes
            Args:
                x (array) : a matrix or a tensor
                y (array) : a matrix or a tensor
                axes (optional, integer or tuple of integers): None by default.
                    Target dimensions to be reduced.
            Returns:
            The batch dot along specified axes. For more deyail refer to
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
        std::array<std::size_t, PHYLANX_MAX_DIMENSIONS>&& dims_b) const
    {
        if (ndims_a < 2 || ndims_b < 2)
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "batch_dot_operation::validate_axes",
                generate_error_message(
                    "batch_dot_operation requires the inputs of rank 2 "
                    "or more"));

        if (axes.size() > 2)
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "batch_dot_operation::validate_axes",
                generate_error_message(
                    "axes can only be an integer or a tuple of two integers"));

        std::size_t axis_a;
        std::size_t axis_b;
        if (axes.size() != 0)
        {
            auto it = axes.begin();

            if (is_list_operand_strict(*it))
                HPX_THROW_EXCEPTION(hpx::bad_parameter,
                    "batch_dot_operation::validate_axes",
                    generate_error_message(
                        "multiple target dimensions are not supported. Axes "
                        "can only be an integer or a tuple of two integers"));

            axis_a = extract_scalar_integer_value_strict(*it) >= 0 ?
                extract_scalar_integer_value_strict(*it) :
                positivize_axis(
                    extract_scalar_integer_value_strict(*it), ndims_a);

            if (axes.size() == 1)
                axis_b = axis_a;
            else
            {
                auto it2 = ++it;
                if (is_list_operand_strict(*it2))
                    HPX_THROW_EXCEPTION(hpx::bad_parameter,
                        "batch_dot_operation::validate_axes",
                        generate_error_message(
                            "multiple target dimensions are not supported. Axes "
                            "can only be an integer or a tuple of two integers"));

                auto a = extract_scalar_integer_value_strict(*it2);
                axis_b = extract_scalar_integer_value_strict(*it2) >= 0 ?
                    extract_scalar_integer_value_strict(*it2) :
                    positivize_axis(
                        extract_scalar_integer_value_strict(*it2), ndims_b);
            }
        }
        else
        {
            //default cases; 2d lhs:(1,1), 3d lhs:(2,1)
            axis_b = 1;
            if (ndims_a == 2)
                axis_a = 1;

            else
                axis_a = 2;
        }

        if (axis_a == 0 || axis_b == 0)
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "batch_dot_operation::validate_axes",
                generate_error_message(
                    "cannot perform batch dot over axis 0"));

        if (axis_a >= ndims_a)
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "batch_dot_operation::validate_axes",
                generate_error_message(
                    "the given axis corresponding to "
                    "the left operand is greater than its dimension"));

        if (axis_b >= ndims_b)
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "batch_dot_operation::validate_axes",
                generate_error_message(
                    "the given axis corresponding to "
                    "the right operand is greater than its dimension"));

        if(dims_a[0] != dims_b[0])
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "batch_dot_operation::validate_axes",
                generate_error_message("cannot perform batch dot on inputs "
                                       "with different batch sizes."));

        if (dims_a[axis_a] != dims_b[axis_b])
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "batch_dot_operation::validate_axes",
                generate_error_message(
                    "operands have incompatible number of dimensions"));
        return true;
    }

    ///////////////////////////////////////////////////////////////////////////
    template <typename T>
    primitive_argument_type batch_dot_operation::batch_dot2d2d(
        ir::node_data<T>&& lhs, ir::node_data<T>&& rhs) const
    {
        auto m1 = lhs.matrix();
        auto m2 = rhs.matrix();

        blaze::DynamicMatrix<T> result(m1.rows(), 1);

        for (std::size_t i = 0; i != m1.rows(); ++i)

            blaze::row(result, i) =
                blaze::dot(blaze::row(m1, i), blaze::row(m2, i));

        return primitive_argument_type{std::move(result)};
    }

#if defined(PHYLANX_HAVE_BLAZE_TENSOR)
    template <typename T>
    primitive_argument_type batch_dot_operation::batch_dot2d3d(
        ir::node_data<T>&& lhs, ir::node_data<T>&& rhs) const
    {
        auto m = lhs.matrix();
        auto t = rhs.tensor();

        blaze::DynamicMatrix<T> result(t.pages(), t.columns());

        for (std::size_t i = 0; i != m.rows(); ++i)

            blaze::row(result, i) = blaze::row(m, i) * blaze::pageslice(t, i);

        return primitive_argument_type{std::move(result)};
    }

    template <typename T>
    primitive_argument_type batch_dot_operation::batch_dot2d3d_axes12(
        ir::node_data<T>&& lhs, ir::node_data<T>&& rhs) const
    {
        auto m = lhs.matrix();
        auto t = rhs.tensor();

        blaze::DynamicMatrix<T> result(t.pages(), t.rows());

        for (std::size_t i = 0; i != t.pages(); ++i)
            blaze::row(result, i) =
                blaze::row(m, i) * blaze::trans(blaze::pageslice(t, i));

        return primitive_argument_type{std::move(result)};
    }

    template <typename T>
    primitive_argument_type batch_dot_operation::batch_dot2d3d(
        ir::node_data<T>&& lhs, ir::node_data<T>&& rhs, ir::range&& axes) const
    {
        if (axes.size() == 2)
        {
            std::int64_t axis_b =
                extract_scalar_integer_value_strict(*++axes.begin());
            if (axis_b == 2 || axis_b == -1)
                return batch_dot2d3d_axes12(std::move(lhs), std::move(rhs));
        }
        return batch_dot2d3d(std::move(lhs), std::move(rhs));
    }
#endif

    template <typename T>
    primitive_argument_type batch_dot_operation::batch_dot2d(
        ir::node_data<T>&& lhs, ir::node_data<T>&& rhs) const
    {
        switch (rhs.num_dimensions())
        {
        case 2:
            return batch_dot2d2d(std::move(lhs), std::move(rhs));

#if defined(PHYLANX_HAVE_BLAZE_TENSOR)
        case 3:
            return batch_dot2d3d(std::move(lhs), std::move(rhs));
#endif

        default:
            break;
        }
        HPX_THROW_EXCEPTION(hpx::bad_parameter,
            "batch_dot_operation::batch_dot2d",
            generate_error_message(
                "the right operand has unsupported number of dimensions"));
    }

    template <typename T>
    primitive_argument_type batch_dot_operation::batch_dot2d(
        ir::node_data<T>&& lhs, ir::node_data<T>&& rhs, ir::range&& axes) const
    {
        switch (rhs.num_dimensions())
        {
        case 2:
            return batch_dot2d2d(std::move(lhs), std::move(rhs));

#if defined(PHYLANX_HAVE_BLAZE_TENSOR)
        case 3:
            return batch_dot2d3d(
                std::move(lhs), std::move(rhs), std::move(axes));
#endif

        default:
            break;
        }
        HPX_THROW_EXCEPTION(hpx::bad_parameter,
            "batch_dot_operation::batch_dot2d",
            generate_error_message(
                "the right operand has unsupported number of dimensions"));
    }

    ///////////////////////////////////////////////////////////////////////////
#if defined(PHYLANX_HAVE_BLAZE_TENSOR)
    template <typename T>
    primitive_argument_type batch_dot_operation::batch_dot3d2d(
        ir::node_data<T>&& lhs, ir::node_data<T>&& rhs) const
    {
        auto m = rhs.matrix();
        auto t = lhs.tensor();

        blaze::DynamicMatrix<T> result(t.pages(), t.rows());

        for (std::size_t i = 0; i != t.pages(); ++i)

            blaze::row(result, i) =
                blaze::row(m, i) * blaze::trans(blaze::pageslice(t, i));

        return primitive_argument_type{std::move(result)};
    }

    template <typename T>
    primitive_argument_type batch_dot_operation::batch_dot3d2d_axes11(
        ir::node_data<T>&& lhs, ir::node_data<T>&& rhs) const
    {
        auto m = rhs.matrix();
        auto t = lhs.tensor();

        blaze::DynamicMatrix<T> result(t.pages(), t.columns());

        for (std::size_t i = 0; i != t.pages(); ++i)
            blaze::row(result, i) = blaze::row(m, i) * blaze::pageslice(t, i);

        return primitive_argument_type{std::move(result)};
    }

    template <typename T>
    primitive_argument_type batch_dot_operation::batch_dot3d2d(
        ir::node_data<T>&& lhs, ir::node_data<T>&& rhs, ir::range&& axes) const
    {
        if (axes.size() == 2)
        {
            std::int64_t axis_a =
                extract_scalar_integer_value_strict(*axes.begin());
            if (axis_a == 2 || axis_a == -1)
                return batch_dot3d2d(std::move(lhs), std::move(rhs));
        }
        return batch_dot3d2d_axes11(std::move(lhs), std::move(rhs));
    }

    template <typename T>
    primitive_argument_type batch_dot_operation::batch_dot3d3d(
        ir::node_data<T>&& lhs, ir::node_data<T>&& rhs) const
    {
        auto t1 = lhs.tensor();
        auto t2 = rhs.tensor();

        blaze::DynamicTensor<T> result(t1.pages(), t1.rows(), t2.columns());

        for (std::size_t i = 0; i != t1.pages(); ++i)

            blaze::pageslice(result, i) =
                blaze::pageslice(t1, i) * blaze::pageslice(t2, i);

        return primitive_argument_type{std::move(result)};
    }

    template <typename T>
    primitive_argument_type batch_dot_operation::batch_dot3d3d_axis1(
        ir::node_data<T>&& lhs, ir::node_data<T>&& rhs) const
    {
        auto t1 = lhs.tensor();
        auto t2 = rhs.tensor();

        blaze::DynamicTensor<T> result(t1.pages(), t1.columns(), t2.columns());

        for (std::size_t i = 0; i != t1.pages(); ++i)

            blaze::pageslice(result, i) =
                blaze::trans(blaze::pageslice(t1, i)) * blaze::pageslice(t2, i);

        return primitive_argument_type{std::move(result)};
    }

    template <typename T>
    primitive_argument_type batch_dot_operation::batch_dot3d3d_axis2(
        ir::node_data<T>&& lhs, ir::node_data<T>&& rhs) const
    {
        auto t1 = lhs.tensor();
        auto t2 = rhs.tensor();

        blaze::DynamicTensor<T> result(t1.pages(), t1.rows(), t2.rows());

        for (std::size_t i = 0; i != t1.pages(); ++i)

            blaze::pageslice(result, i) =
                blaze::pageslice(t1, i) * blaze::trans(blaze::pageslice(t2, i));

        return primitive_argument_type{std::move(result)};
    }

    template <typename T>
    primitive_argument_type batch_dot_operation::batch_dot3d3d_axes12(
        ir::node_data<T>&& lhs, ir::node_data<T>&& rhs) const
    {
        auto t1 = lhs.tensor();
        auto t2 = rhs.tensor();

        blaze::DynamicTensor<T> result(t1.pages(), t1.columns(), t2.rows());

        for (std::size_t i = 0; i != t1.pages(); ++i)

            blaze::pageslice(result, i) =
                blaze::trans(blaze::pageslice(t2, i) * blaze::pageslice(t1, i));

        return primitive_argument_type{std::move(result)};
    }

    template <typename T>
    primitive_argument_type batch_dot_operation::batch_dot3d3d(
        ir::node_data<T>&& lhs, ir::node_data<T>&& rhs, ir::range&& axes) const
    {
        if (axes.size() == 1)
        {
            std::int64_t axis =
                extract_scalar_integer_value_strict(*axes.begin());
            if (axis == 1 || axis == -2)
                return batch_dot3d3d_axis1(std::move(lhs), std::move(rhs));
            if (axis == 2 || axis == -1)
                return batch_dot3d3d_axis2(std::move(lhs), std::move(rhs));
        }

        auto it = axes.begin();
        std::int64_t axis_a = extract_scalar_integer_value_strict(*it);
        std::int64_t axis_b = extract_scalar_integer_value_strict(*++it);

        if (axis_a < 0)
            axis_a += 3;
        if (axis_b < 0)
            axis_b += 3;

        if (axis_a == 1 && axis_b == 1)
            return batch_dot3d3d_axis1(std::move(lhs), std::move(rhs));
        if (axis_a == 2 && axis_b == 2)
            return batch_dot3d3d_axis2(std::move(lhs), std::move(rhs));
        if (axis_a == 1 && axis_b == 2)
            return batch_dot3d3d_axes12(std::move(lhs), std::move(rhs));
        //if (axis_a == 2 && axis_b == 1)
        return batch_dot3d3d(std::move(lhs), std::move(rhs));

    }

    template <typename T>
    primitive_argument_type batch_dot_operation::batch_dot3d(
        ir::node_data<T>&& lhs, ir::node_data<T>&& rhs) const
    {
        switch (rhs.num_dimensions())
        {
        case 2:
            return batch_dot3d2d(std::move(lhs), std::move(rhs));

        case 3:
            return batch_dot3d3d(std::move(lhs), std::move(rhs));

        default:
            break;
        }
        HPX_THROW_EXCEPTION(hpx::bad_parameter,
            "batch_dot_operation::batch_dot3d",
            generate_error_message(
                "the right operand has unsupported number of dimensions"));
    }

    template <typename T>
    primitive_argument_type batch_dot_operation::batch_dot3d(
        ir::node_data<T>&& lhs, ir::node_data<T>&& rhs, ir::range&& axes) const
    {
        switch (rhs.num_dimensions())
        {
        case 2:
            return batch_dot3d2d(
                std::move(lhs), std::move(rhs), std::move(axes));

        case 3:
            return batch_dot3d3d(
                std::move(lhs), std::move(rhs), std::move(axes));

        default:
            break;
        }
        HPX_THROW_EXCEPTION(hpx::bad_parameter,
            "batch_dot_operation::batch_dot3d",
            generate_error_message(
                "the right operand has unsupported number of dimensions"));
    }
#endif

    ///////////////////////////////////////////////////////////////////////////
    template <typename T>
    primitive_argument_type batch_dot_operation::batch_dot_nd(
        ir::node_data<T>&& lhs, ir::node_data<T>&& rhs) const
    {
        switch (lhs.num_dimensions())
        {
        case 2:
            return batch_dot2d(std::move(lhs), std::move(rhs));

#if defined(PHYLANX_HAVE_BLAZE_TENSOR)
        case 3:
            return batch_dot3d(std::move(lhs), std::move(rhs));
#endif

        default:
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "batch_dot_operation::batch_dot_nd",
                generate_error_message(
                    "the left operand has unsupported number of dimensions"));
        }
    }

    template <typename T>
    primitive_argument_type batch_dot_operation::batch_dot_nd(
        ir::node_data<T>&& lhs, ir::node_data<T>&& rhs, ir::range&& axes) const
    {
        switch (lhs.num_dimensions())
        {
        case 2:
            return batch_dot2d(std::move(lhs), std::move(rhs), std::move(axes));

#if defined(PHYLANX_HAVE_BLAZE_TENSOR)
        case 3:
            return batch_dot3d(std::move(lhs), std::move(rhs), std::move(axes));
#endif

        default:
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "batch_dot_operation::batch_dot_nd",
                generate_error_message(
                    "the left operand has unsupported number of dimensions"));
        }
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
            auto&& op0 =
                value_operand(operands[0], args, name_, codename_, ctx);

            return hpx::dataflow(hpx::launch::sync,
                [this_ = std::move(this_)](
                        hpx::future<primitive_argument_type>&& f1,
                        hpx::future<primitive_argument_type>&& f2)
                ->primitive_argument_type
                {
                    auto&& op1 = f1.get();
                    auto&& op2 = f2.get();

                    ir::range axes(0); // an empty range

                    if (this_->validate_axes(axes,
                            extract_numeric_value_dimension(
                                op1, this_->name_, this_->codename_),
                            extract_numeric_value_dimension(
                                op2, this_->name_, this_->codename_),
                            extract_numeric_value_dimensions(
                                op1, this_->name_, this_->codename_),
                            extract_numeric_value_dimensions(
                                op2, this_->name_, this_->codename_)))
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

                        case node_data_type_unknown: HPX_FALLTHROUGH;
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
                },
                std::move(op0),
                value_operand(operands[1], args, name_, codename_, std::move(ctx)));
        }

        auto&& op0 = value_operand(operands[0], args, name_, codename_, ctx);
        auto&& op1 = value_operand(operands[1], args, name_, codename_, ctx);

        return hpx::dataflow(hpx::launch::sync,
            [this_ = std::move(this_)](
                    hpx::future<primitive_argument_type>&& f1,
                    hpx::future<primitive_argument_type>&& f2,
                    hpx::future<ir::range>&& faxes)
            -> primitive_argument_type
            {
                auto&& op1 = f1.get();
                auto&& op2 = f2.get();
                auto&& axes = faxes.get();

                if (this_->validate_axes(axes,
                        extract_numeric_value_dimension(
                            op1, this_->name_, this_->codename_),
                        extract_numeric_value_dimension(
                            op2, this_->name_, this_->codename_),
                        extract_numeric_value_dimensions(
                            op1, this_->name_, this_->codename_),
                        extract_numeric_value_dimensions(
                            op2, this_->name_, this_->codename_)))
                {
                    switch (extract_common_type(op1, op2))
                    {
                    case node_data_type_bool:
                        return this_->batch_dot_nd(
                            extract_boolean_value(
                                std::move(op1), this_->name_, this_->codename_),
                            extract_boolean_value(
                                std::move(op2), this_->name_, this_->codename_),
                            std::move(axes));

                    case node_data_type_int64:
                        return this_->batch_dot_nd(
                            extract_integer_value(
                                std::move(op1), this_->name_, this_->codename_),
                            extract_integer_value(
                                std::move(op2), this_->name_, this_->codename_),
                            std::move(axes));

                    case node_data_type_unknown: HPX_FALLTHROUGH;
                    case node_data_type_double:
                        return this_->batch_dot_nd(
                            extract_numeric_value(
                                std::move(op1), this_->name_, this_->codename_),
                            extract_numeric_value(
                                std::move(op2), this_->name_, this_->codename_),
                            std::move(axes));

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
            },
            std::move(op0), std::move(op1),
            list_operand(operands[2], args, name_, codename_, std::move(ctx)));
    }
}}}
