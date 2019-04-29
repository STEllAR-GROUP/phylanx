// Copyright (c) 2018 Bita Hasheminezhad
// Copyright (c) 2018 Parsa Amini
// Copyright (c) 2018 Hartmut Kaiser
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>
#include <phylanx/execution_tree/primitives/node_data_helpers.hpp>
#include <phylanx/ir/node_data.hpp>
#include <phylanx/plugins/matrixops/squeeze_operation.hpp>

#include <hpx/include/lcos.hpp>
#include <hpx/include/naming.hpp>
#include <hpx/include/util.hpp>
#include <hpx/throw_exception.hpp>
#include <hpx/util/optional.hpp>

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include <blaze/Math.h>
#if defined(PHYLANX_HAVE_BLAZE_TENSOR)
#include <blaze_tensor/Math.h>
#endif

///////////////////////////////////////////////////////////////////////////////
namespace phylanx { namespace execution_tree { namespace primitives
{
    ///////////////////////////////////////////////////////////////////////////
    match_pattern_type const squeeze_operation::match_data =
    {
        hpx::util::make_tuple("squeeze",
        std::vector<std::string>{"squeeze(_1)","squeeze(_1,_2)"},
        &create_squeeze_operation, &create_primitive<squeeze_operation>,
        R"(a, axis
        Args:

            a (array) : a scalar, vector, matrix or a tensor
            axis (optional, integer): an axis to squeeze along. If an axis is
                selected with shape entry greater than one, an error is raised.

        Returns:

        Remove single-dimensional entries from the shape of an array)")
    };

    ///////////////////////////////////////////////////////////////////////////
    squeeze_operation::squeeze_operation(primitive_arguments_type&& operands,
        std::string const& name, std::string const& codename)
      : primitive_component_base(std::move(operands), name, codename)
    {}

    ///////////////////////////////////////////////////////////////////////////
    primitive_argument_type squeeze_operation::squeeze0d(
        primitive_argument_type&& arg,
        hpx::util::optional<std::int64_t> axis) const
    {
        if(axis && axis.value() != 0 && axis.value() != -1)
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "squeeze_operation::squeeze0d",
                generate_error_message(
                    "the axis can be only 0 or -1 for 0-d arrays"));

        return primitive_argument_type{std::move(arg)};
    }

    ///////////////////////////////////////////////////////////////////////////
    template <typename T>
    primitive_argument_type squeeze_operation::squeeze1d(
        ir::node_data<T>&& arg) const
    {
        auto v = arg.vector();
        if (v.size() == 1)
            return primitive_argument_type{v[0]};

        return primitive_argument_type{std::move(arg)};
    }

    primitive_argument_type squeeze_operation::squeeze1d(
        primitive_argument_type&& arg,
        hpx::util::optional<std::int64_t> axis) const
    {
        if (axis && axis.value() != 0 && axis.value() != -1)
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "squeeze_operation::squeeze1d",
                generate_error_message(
                    "the axis can be only 0 or -1 for vectors"));

        switch (extract_common_type(arg))
        {
        case node_data_type_bool:
            return squeeze1d(
                extract_boolean_value_strict(std::move(arg), name_, codename_));

        case node_data_type_int64:
            return squeeze1d(
                extract_integer_value_strict(std::move(arg), name_, codename_));

        case node_data_type_double:
            return squeeze1d(
                extract_numeric_value_strict(std::move(arg), name_, codename_));

        case node_data_type_unknown:
            return squeeze1d(
                extract_numeric_value(std::move(arg), name_, codename_));

        default:
            break;
        }

        HPX_THROW_EXCEPTION(hpx::bad_parameter,
            "phylanx::execution_tree::primitives::squeeze_operation::squeeze1d",
            generate_error_message(
                "the squeeze primitive requires for all arguments to "
                "be numeric data types"));
    }

    ///////////////////////////////////////////////////////////////////////////
    template <typename T>
    primitive_argument_type squeeze_operation::squeeze2d_axis0(
        ir::node_data<T>&& arg) const
    {
        auto m = arg.matrix();
        if (m.rows() == 1)
            return primitive_argument_type{
                blaze::DynamicVector<T>{blaze::trans(blaze::row(m, 0ul))}};

        HPX_THROW_EXCEPTION(hpx::bad_parameter,
            "squeeze_operation::squeeze2d_axis0",
            generate_error_message("cannot select an axis to squeeze out which "
                                   "has size not equal to one"));
    }

    template <typename T>
    primitive_argument_type squeeze_operation::squeeze2d_axis1(
        ir::node_data<T>&& arg) const
    {
        auto m = arg.matrix();
        if (m.columns() == 1)
            return primitive_argument_type{
                blaze::DynamicVector<T>{blaze::column(m, 0ul)}};

        HPX_THROW_EXCEPTION(hpx::bad_parameter,
            "squeeze_operation::squeeze2d_axis1",
            generate_error_message("cannot select an axis to squeeze out which "
                                   "has size not equal to one"));
    }

    template <typename T>
    primitive_argument_type squeeze_operation::squeeze2d_all_axes(
        ir::node_data<T>&& arg) const
    {

        auto m = arg.matrix();
        if (m.columns() == 1 || m.rows() == 1)
        {
            if (m.columns() == 1 && m.rows() == 1)
                return primitive_argument_type{m(0, 0)};
            else if (m.columns() == 1)
                return primitive_argument_type{
                    blaze::DynamicVector<T>{blaze::column(m, 0ul)}};
            else if (m.rows() == 1)
                return primitive_argument_type{
                    blaze::DynamicVector<T>{blaze::trans(blaze::row(m, 0ul))}};
        }
        return primitive_argument_type{std::move(arg)};
    }

    template <typename T>
    primitive_argument_type squeeze_operation::squeeze2d(
        ir::node_data<T>&& arg, hpx::util::optional<std::int64_t> axis) const
    {
        if (axis)
        {
            if (axis.value() < 0)
                axis.value() += 2;

            if (axis.value() == 0)
                return squeeze2d_axis0(std::move(arg));

            return squeeze2d_axis1(std::move(arg));
        }
        return squeeze2d_all_axes(std::move(arg));
    }

    primitive_argument_type squeeze_operation::squeeze2d(
        primitive_argument_type&& arg,
        hpx::util::optional<std::int64_t> axis) const
    {
        if (axis && axis.value() != 0 && axis.value() != -2 &&
            axis.value() != 1 && axis.value() != -1)
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "squeeze_operation::squeeze2d",
                generate_error_message(
                    "the axis can be between -2 and 1 for matrices"));

        switch (extract_common_type(arg))
        {
        case node_data_type_bool:
            return squeeze2d(
                extract_boolean_value_strict(std::move(arg), name_, codename_),
                axis);

        case node_data_type_int64:
            return squeeze2d(
                extract_integer_value_strict(std::move(arg), name_, codename_),
                axis);

        case node_data_type_double:
            return squeeze2d(
                extract_numeric_value_strict(std::move(arg), name_, codename_),
                axis);

        case node_data_type_unknown:
            return squeeze2d(
                extract_numeric_value(std::move(arg), name_, codename_), axis);

        default:
            break;
        }

        HPX_THROW_EXCEPTION(hpx::bad_parameter,
            "phylanx::execution_tree::primitives::squeeze_operation::squeeze2d",
            generate_error_message(
                "the squeeze primitive requires for all arguments to "
                "be numeric data types"));
    }

    ///////////////////////////////////////////////////////////////////////////
#if defined(PHYLANX_HAVE_BLAZE_TENSOR)
    template <typename T>
    primitive_argument_type squeeze_operation::squeeze3d_axis0(
        ir::node_data<T>&& arg) const
    {
        auto t = arg.tensor();
        if (t.pages() == 1)
            return primitive_argument_type{
                blaze::DynamicMatrix<T>{blaze::pageslice(t, 0ul)}};

        HPX_THROW_EXCEPTION(hpx::bad_parameter,
            "squeeze_operation::squeeze3d_axis0",
            generate_error_message("cannot select an axis to squeeze out which "
                                   "has size not equal to one"));
    }

    template <typename T>
    primitive_argument_type squeeze_operation::squeeze3d_axis1(
        ir::node_data<T>&& arg) const
    {
        auto t = arg.tensor();
        if (t.rows() == 1)
            return primitive_argument_type{
                blaze::DynamicMatrix<T>{blaze::trans(blaze::rowslice(t, 0ul))}};

        HPX_THROW_EXCEPTION(hpx::bad_parameter,
            "squeeze_operation::squeeze3d_axis1",
            generate_error_message("cannot select an axis to squeeze out which "
                                   "has size not equal to one"));
    }

    template <typename T>
    primitive_argument_type squeeze_operation::squeeze3d_axis2(
        ir::node_data<T>&& arg) const
    {
        auto t = arg.tensor();
        if (t.columns() == 1)
            return primitive_argument_type{
                blaze::DynamicMatrix<T>{blaze::columnslice(t, 0ul)}};

        HPX_THROW_EXCEPTION(hpx::bad_parameter,
            "squeeze_operation::squeeze3d_axis2",
            generate_error_message("cannot select an axis to squeeze out which "
                                   "has size not equal to one"));
    }

    template <typename T>
    primitive_argument_type squeeze_operation::squeeze3d_all_axes(
        ir::node_data<T>&& arg) const
    {
        auto t = arg.tensor();
        if (t.pages() == 1 || t.rows() == 1 || t.columns() == 1)
        {
            if (t.pages() == 1 && t.rows() == 1 && t.columns() == 1)
                return primitive_argument_type{t(0, 0, 0)};

            else
            {
                if (t.pages() == 1)
                {
                    if (t.rows() != 1 && t.columns() != 1)
                        return primitive_argument_type{
                            blaze::DynamicMatrix<T>{blaze::pageslice(t, 0ul)}};

                    else if (t.rows() == 1 && t.columns() != 1)
                        return primitive_argument_type{
                            blaze::DynamicVector<T>{blaze::trans(
                                blaze::row(blaze::pageslice(t, 0ul), 0ul))}};

                    else if (t.rows() != 1 && t.columns() == 1)
                        return primitive_argument_type{blaze::DynamicVector<T>{
                            blaze::column(blaze::pageslice(t, 0ul), 0ul)}};
                }
                else
                {
                    if (t.rows() == 1 && t.columns() == 1)
                        return primitive_argument_type{blaze::DynamicVector<T>{
                            blaze::column(blaze::columnslice(t, 0ul), 0ul)}};

                    else if (t.rows() == 1 && t.columns() != 1)
                        return primitive_argument_type{blaze::DynamicMatrix<T>{
                            blaze::trans(blaze::rowslice(t, 0ul))}};

                    else if (t.rows() != 1 && t.columns() == 1)
                        return primitive_argument_type{blaze::DynamicMatrix<T>{
                            blaze::columnslice(t, 0ul)}};
                }
            }
        }
        return primitive_argument_type{std::move(arg)};
    }

    template <typename T>
    primitive_argument_type squeeze_operation::squeeze3d(
        ir::node_data<T>&& arg, hpx::util::optional<std::int64_t> axis) const
    {
        if (axis)
        {
            if (axis.value() < 0)
                axis.value() += 3;

            if (axis.value() == 0)
                return squeeze3d_axis0(std::move(arg));

            else if (axis.value() == 1)
                return squeeze3d_axis1(std::move(arg));

            return squeeze3d_axis2(std::move(arg));
        }
        return squeeze3d_all_axes(std::move(arg));
    }

   primitive_argument_type squeeze_operation::squeeze3d(
        primitive_argument_type&& arg,
        hpx::util::optional<std::int64_t> axis) const
    {
        if (axis &&
            axis.value() != 0 && axis.value() != -3 &&
            axis.value() != 1 && axis.value() != -2 &&
            axis.value() != 2 && axis.value() != -1)
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "squeeze_operation::squeeze3d",
                generate_error_message(
                    "the axis can be between -3 and 2 for tensors"));

        switch (extract_common_type(arg))
        {
        case node_data_type_bool:
            return squeeze3d(
                extract_boolean_value_strict(std::move(arg), name_, codename_),
                axis);

        case node_data_type_int64:
            return squeeze3d(
                extract_integer_value_strict(std::move(arg), name_, codename_),
                axis);

        case node_data_type_double:
            return squeeze3d(
                extract_numeric_value_strict(std::move(arg), name_, codename_),
                axis);

        case node_data_type_unknown:
            return squeeze3d(
                extract_numeric_value(std::move(arg), name_, codename_), axis);

        default:
            break;
        }

        HPX_THROW_EXCEPTION(hpx::bad_parameter,
            "squeeze_operation::squeeze3d",
            generate_error_message(
                "the squeeze primitive requires for all arguments to "
                "be numeric data types"));
    }
#endif

    ///////////////////////////////////////////////////////////////////////////
    hpx::future<primitive_argument_type> squeeze_operation::eval(
        primitive_arguments_type const& operands,
        primitive_arguments_type const& args, eval_context ctx) const
    {
        if (operands.empty() || operands.size() > 2)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "squeeze_operation::eval",
                util::generate_error_message(
                    "the squeeze_operation primitive requires exactly one, or "
                    "two operands",
                    name_, codename_));
        }

        if (!valid(operands[0]))
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "squeeze_operation::eval",
                util::generate_error_message(
                    "the squeeze_operation primitive requires that the "
                    "arguments given by the operands array are valid",
                    name_, codename_));
        }

        auto this_ = this->shared_from_this();
        return hpx::dataflow(hpx::launch::sync, hpx::util::unwrapping(
            [this_ = std::move(this_)](primitive_arguments_type&& args)
            -> primitive_argument_type
            {
                // Presence of axis changes behavior for >2d cases
                hpx::util::optional<std::int64_t> axis;
                if (args.size() > 1)
                {
                    if (valid(args[1]))
                    {
                        axis =
                            execution_tree::extract_scalar_integer_value_strict(
                                args[1], this_->name_, this_->codename_);
                    }
                    else
                    {
                        HPX_THROW_EXCEPTION(hpx::bad_parameter,
                            "squeeze_operation::eval",
                            this_->generate_error_message(
                                "the squeeze_operation primitive requires that the "
                                "arguments given by the operands array are valid"));
                    }
                }

                switch (extract_numeric_value_dimension(
                    args[0], this_->name_, this_->codename_))
                {
                case 0:
                    return this_->squeeze0d(std::move(args[0]), axis);

                case 1:
                    return this_->squeeze1d(std::move(args[0]), axis);

                case 2:
                    return this_->squeeze2d(std::move(args[0]), axis);

    #if defined(PHYLANX_HAVE_BLAZE_TENSOR)
                case 3:
                    return this_->squeeze3d(std::move(args[0]), axis);
    #endif
                default:
                    HPX_THROW_EXCEPTION(hpx::bad_parameter,
                        "squeeze_operation::eval",
                        util::generate_error_message(
                            "operand a has an invalid "
                            "number of dimensions",
                            this_->name_, this_->codename_));
                }
            }),
            detail::map_operands(operands, functional::value_operand{},
                args, name_, codename_, std::move(ctx)));
    }
}}}
