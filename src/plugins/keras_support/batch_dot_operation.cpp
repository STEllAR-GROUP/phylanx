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
    template <typename T>
    primitive_argument_type batch_dot_operation::batch_dot2d2d(
        ir::node_data<T>&& lhs, ir::node_data<T>&& rhs) const
    {
        if (lhs.dimension(0) != rhs.dimension(0) ||
            lhs.dimension(1) != rhs.dimension(1))
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "batch_dot_operation::batch_dot2d2d",
                generate_error_message(
                    "operands have incompatible number of dimensions"));
        }

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
        if (lhs.dimension(0) != rhs.dimension(0) ||
            lhs.dimension(1) != rhs.dimension(1))
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "batch_dot_operation::batch_dot2d3d",
                generate_error_message(
                    "operands have incompatible number of dimensions"));
        }

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
        if (lhs.dimension(0) != rhs.dimension(0) ||
            lhs.dimension(1) != rhs.dimension(2))
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "batch_dot_operation::batch_dot2d3d_axes12",
                generate_error_message(
                    "operands have incompatible number of dimensions"));
        }

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
        if (lhs.dimension(0) != rhs.dimension(0) ||
            lhs.dimension(2) != rhs.dimension(1))
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "batch_dot_operation::batch_dot3d2d",
                generate_error_message(
                    "operands have incompatible number of dimensions"));
        }

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
        if (lhs.dimension(0) != rhs.dimension(0) ||
            lhs.dimension(1) != rhs.dimension(1))
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "batch_dot_operation::batch_dot3d2d_axes11",
                generate_error_message(
                    "operands have incompatible number of dimensions"));
        }

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
        if (lhs.dimension(0) != rhs.dimension(0) ||
            lhs.dimension(2) != rhs.dimension(1))
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "batch_dot_operation::batch_dot3d3d",
                generate_error_message(
                    "operands have incompatible number of dimensions"));
        }

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
        if (lhs.dimension(0) != rhs.dimension(0) ||
            lhs.dimension(1) != rhs.dimension(1))
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "batch_dot_operation::batch_dot3d3d_axis1",
                generate_error_message(
                    "operands have incompatible number of dimensions"));
        }

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
        if (lhs.dimension(0) != rhs.dimension(0) ||
            lhs.dimension(2) != rhs.dimension(2))
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "batch_dot_operation::batch_dot3d3d_axis2",
                generate_error_message(
                    "operands have incompatible number of dimensions"));
        }

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
        if (lhs.dimension(0) != rhs.dimension(0) ||
            lhs.dimension(1) != rhs.dimension(2))
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "batch_dot_operation::batch_dot3d3d_axes12",
                generate_error_message(
                    "operands have incompatible number of dimensions"));
        }

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

        if (axes.size() == 2)
        {
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
            if (axis_a == 2 && axis_b == 1)
                return batch_dot3d3d(std::move(lhs), std::move(rhs));
        }
        HPX_THROW_EXCEPTION(hpx::bad_parameter,
            "batch_dot_operation::batch_dot3d3d",
            generate_error_message(
                "axis can only be an integer or a tuple of two integers"));
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
            return hpx::dataflow(hpx::launch::sync, hpx::util::unwrapping(
                [this_ = std::move(this_)](primitive_argument_type&& op1,
                    primitive_argument_type&& op2)
                ->primitive_argument_type
            {
                if (extract_numeric_value_dimension(
                        op1, this_->name_, this_->codename_) < 2 ||
                    extract_numeric_value_dimension(
                        op2, this_->name_, this_->codename_) < 2)
                {
                    HPX_THROW_EXCEPTION(hpx::bad_parameter,
                        "batch_dot_operation::eval",
                        this_->generate_error_message(
                            "batch_dot_operation requires the inputs of rank 2 "
                            "or more"));
                }
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
            }),
                value_operand(operands[0], args, name_, codename_, ctx),
                value_operand(operands[1], args, name_, codename_, ctx));
        }

        return hpx::dataflow(hpx::launch::sync,
            hpx::util::unwrapping(
                [this_ = std::move(this_)](primitive_argument_type&& op1,
                    primitive_argument_type&& op2,
                    ir::range&& axes) -> primitive_argument_type {

                std::size_t ndims_1 = extract_numeric_value_dimension(
                        op1, this_->name_, this_->codename_);
                std::size_t ndims_2 = extract_numeric_value_dimension(
                        op2, this_->name_, this_->codename_);

                if (ndims_1 < 2 || ndims_2 < 2)
                {
                    HPX_THROW_EXCEPTION(hpx::bad_parameter,
                        "batch_dot_operation::eval",
                        this_->generate_error_message(
                            "batch_dot_operation requires the inputs of rank 2 "
                            "or more"));
                }

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

                case node_data_type_unknown:
                    HPX_FALLTHROUGH;
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
                        "the dot primitive requires for all arguments to "
                        "be numeric data types"));

        }),
            value_operand(operands[0], args, name_, codename_, ctx),
            value_operand(operands[1], args, name_, codename_, ctx),
            list_operand(operands[2], args, name_, codename_, ctx));
    }
}}}
