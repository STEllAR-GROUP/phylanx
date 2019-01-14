// Copyright (c) 2017-2018 Hartmut Kaiser
// Copyright (c) 2019 Bita Hasheminezhad
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>
#include <phylanx/ir/node_data.hpp>
#include <phylanx/execution_tree/primitives/node_data_helpers.hpp>
#include <phylanx/plugins/matrixops/transpose_operation.hpp>

#include <hpx/include/lcos.hpp>
#include <hpx/include/naming.hpp>
#include <hpx/include/util.hpp>
#include <hpx/throw_exception.hpp>

#include <cmath>
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
    match_pattern_type const transpose_operation::match_data =
    {
        hpx::util::make_tuple("transpose",
            std::vector<std::string>{"transpose(_1)","transpose(_1,_2)"},
            &create_transpose_operation,
            &create_primitive<transpose_operation>, R"(
            arg, axes
            Args:

                arg (arr) : an array
                axes (optional, integer or a vector of integers) : By default,
                   reverse the dimensions, otherwise permute the axes according
                   to the values given.

            Returns:

            The transpose of `arg`. If axes are provided, it returns `arg` with
            its axes permuted.)")
    };

    ///////////////////////////////////////////////////////////////////////////
    transpose_operation::transpose_operation(
            primitive_arguments_type&& operands,
            std::string const& name, std::string const& codename)
      : primitive_component_base(std::move(operands), name, codename)
    {}

    ///////////////////////////////////////////////////////////////////////////
    bool transpose_operation::validate_axes(std::size_t a_dims,
        ir::node_data<std::int64_t>&& axes) const
    {
        if (a_dims == 1)
        {
            if (axes.num_dimensions() == 0)
                if (axes.scalar() == 0 || axes.scalar() == -1)
                    return true;
            if (axes.num_dimensions() == 1)
            {
                auto v = axes.vector();
                if (v.size() == 1)
                    if (v[0] == 0 || v[0] == -1)
                        return true;
            }
        }
        else if (a_dims == 2)
        {
            if (axes.num_dimensions() == 1)
            {
                auto v = axes.vector();
                for (auto it = v.begin(); it != v.end(); ++it)
                {
                    if (*it < 0)
                        *it += 2;
                    if (*it != 0 && *it != 1)
                        return false;
                }
                if (blaze::sum(v) == 1)
                    return true;
            }
        }
        else if (a_dims == 3)
        {
            if (axes.num_dimensions() == 1)
            {
                auto v = axes.vector();
                for (auto it = v.begin(); it != v.end(); ++it)
                {
                    if (*it < 0)
                        *it += 3;
                    if (*it != 0 && *it != 1 && *it != 2)
                        return false;
                }
                if (blaze::sum(v) == 3)
                    return true;
            }
        }
        return false;    // axes are not allowed for 0-d arrays
    }
    ///////////////////////////////////////////////////////////////////////////
    primitive_argument_type transpose_operation::transpose0d1d(
        primitive_argument_type&& arg) const
    {
        return primitive_argument_type{std::move(arg)};       // no-op
    }

    template <typename T>
    primitive_argument_type transpose_operation::transpose2d(
        ir::node_data<T>&& arg) const
    {
        if (arg.is_ref())
        {
            arg = blaze::trans(arg.matrix());
        }
        else
        {
            blaze::transpose(arg.matrix_non_ref());
        }

        return primitive_argument_type{std::move(arg)};
    }

    primitive_argument_type transpose_operation::transpose2d(
        primitive_argument_type&& arg) const
    {
        switch (extract_common_type(arg))
        {
        case node_data_type_bool:
            return transpose2d(extract_boolean_value_strict(std::move(arg)));

        case node_data_type_int64:
            return transpose2d(extract_integer_value_strict(std::move(arg)));

        case node_data_type_unknown: HPX_FALLTHROUGH;
        case node_data_type_double:
            return transpose2d(extract_numeric_value(std::move(arg)));

        default:
            break;
        }

        HPX_THROW_EXCEPTION(hpx::bad_parameter,
            "transpose_operation::transpose2d",
            generate_error_message(
                "the transpose primitive requires for its argument to "
                    "be numeric data type"));
    }

    template <typename T>
    primitive_argument_type transpose_operation::transpose2d(
        ir::node_data<T>&& arg, ir::node_data<std::int64_t>&& axes) const
    {
        auto v = axes.vector();
        for (auto it = v.begin(); it != v.end(); ++it)
            if (*it < 0)
                *it += 2;
        if (v[0] == 0 && v[1] == 1)
            return primitive_argument_type{std::move(arg)};
        else
            return transpose2d(std::move(arg));
    }

    primitive_argument_type transpose_operation::transpose2d(
        primitive_argument_type&& arg, ir::node_data<std::int64_t>&& axes) const
    {
        switch (extract_common_type(arg))
        {
        case node_data_type_bool:
            return transpose2d(
                extract_boolean_value_strict(std::move(arg)), std::move(axes));

        case node_data_type_int64:
            return transpose2d(
                extract_integer_value_strict(std::move(arg)), std::move(axes));

        case node_data_type_unknown:
            HPX_FALLTHROUGH;
        case node_data_type_double:
            return transpose2d(
                extract_numeric_value(std::move(arg)), std::move(axes));

        default:
            break;
        }

        HPX_THROW_EXCEPTION(hpx::bad_parameter,
            "transpose_operation::transpose2d",
            generate_error_message(
                "the transpose primitive requires for its argument to "
                "be numeric data type"));
    }
    ///////////////////////////////////////////////////////////////////////////
    hpx::future<primitive_argument_type> transpose_operation::eval(
        primitive_arguments_type const& operands,
        primitive_arguments_type const& args, eval_context ctx) const
    {
        if (operands.size() != 1 && operands.size() != 2)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "transpose_operation::transpose_operation",
                generate_error_message(
                    "the transpose_operation primitive requires"
                        "exactly one or two operands"));
        }

        if (!valid(operands[0]) ||
            (operands.size() == 2 && !valid(operands[1])))
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "transpose_operation::transpose_operation",
                generate_error_message(
                    "the transpose_operation primitive requires "
                        "that the arguments given by the operands "
                        "array are valid"));
        }

        auto this_ = this->shared_from_this();
        if (operands.size() == 1)
        {
            return hpx::dataflow(hpx::launch::sync, hpx::util::unwrapping(
            [this_ = std::move(this_)](primitive_argument_type&& arg)
            -> primitive_argument_type
            {
                switch (extract_numeric_value_dimension(
                    arg, this_->name_, this_->codename_))
                {
                case 0: HPX_FALLTHROUGH;
                case 1:
                    return this_->transpose0d1d(std::move(arg));

                case 2:
                    return this_->transpose2d(std::move(arg));

                default:
                    HPX_THROW_EXCEPTION(hpx::bad_parameter,
                        "transpose_operation::eval",
                        this_->generate_error_message(
                            "left hand side operand has unsupported "
                                "number of dimensions"));
                }
            }),
            value_operand(operands[0], args, name_, codename_, std::move(ctx)));
        }
        return hpx::dataflow(hpx::launch::sync,
            hpx::util::unwrapping([this_ = std::move(this_)](
                                      primitive_arguments_type&& args)
                                      -> primitive_argument_type {
                auto a_dims = extract_numeric_value_dimension(
                    args[0], this_->name_, this_->codename_);

                if (this_->validate_axes(
                        a_dims, extract_integer_value_strict(args[1])))
                {
                    switch (a_dims)
                    {
                    case 0: HPX_FALLTHROUGH;
                    case 1:
                        return this_->transpose0d1d(std::move(args[0]));

                    case 2:
                        return this_->transpose2d(std::move(args[0]),
                            extract_integer_value_strict(std::move(args[1])));

                    default:
                        HPX_THROW_EXCEPTION(hpx::bad_parameter,
                            "transpose_operation::eval",
                            this_->generate_error_message(
                                "left hand side operand has unsupported "
                                "number of dimensions"));
                    }
                }
                else
                    HPX_THROW_EXCEPTION(hpx::bad_parameter,
                        "transpose_operation::eval",
                        util::generate_error_message(
                            "At least one of the given axes is out of bounds "
                            "for the given array. Axes size should be the same "
                            "as array's number of dimensions. Having an n-d "
                            "array each axis should be in [-n, n-1]",
                            this_->name_, this_->codename_));
            }),
            detail::map_operands(operands, functional::value_operand{}, args,
                name_, codename_, std::move(ctx)));
    }
}}}
