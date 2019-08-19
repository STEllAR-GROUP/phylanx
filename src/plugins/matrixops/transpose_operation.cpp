// Copyright (c) 2017-2018 Hartmut Kaiser
// Copyright (c) 2019 Bita Hasheminezhad
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>
#include <phylanx/ir/node_data.hpp>
#include <phylanx/execution_tree/primitives/node_data_helpers.hpp>
#include <phylanx/plugins/common/transpose_operation_nd.hpp>
#include <phylanx/plugins/matrixops/transpose_operation.hpp>

#include <hpx/include/lcos.hpp>
#include <hpx/include/naming.hpp>
#include <hpx/include/util.hpp>
#include <hpx/errors/throw_exception.hpp>

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
            std::vector<std::string>{"transpose(_1)", "transpose(_1,_2)"},
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
                for (auto& it : v)
                {
                    if (it < 0)
                        it += 2;
                    if (it != 0 && it != 1)
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
                for (auto& it : v)
                {
                    if (it < 0)
                        it += 3;
                    if (it != 0 && it != 1 && it != 2)
                        return false;
                }
                if (blaze::sum(v) == 3)
                    return true;
            }
        }
        return false;    // axes are not allowed for 0-d arrays
    }

    ////////////////////////////////////////////////////////////////////////////
    primitive_argument_type transpose_operation::transpose_nd(std::size_t dims,
        primitive_argument_type&& arg0, primitive_argument_type&& arg1) const
    {
        switch (dims)
        {
        case 0: HPX_FALLTHROUGH;
        case 1:
            return common::transpose0d1d(std::move(arg0));

        case 2:
            return common::transpose2d(std::move(arg0),
                extract_integer_value_strict(
                    std::move(arg1), name_, codename_), name_, codename_);

        case 3:
            return common::transpose3d(std::move(arg0),
                extract_integer_value_strict(
                    std::move(arg1), name_, codename_), name_, codename_);

        default:
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "transpose_operation::transpose_nd",
                generate_error_message(
                    "left hand side operand has unsupported "
                    "number of dimensions"));
        }
    }

    primitive_argument_type transpose_operation::transpose_nd(std::size_t dims,
        primitive_argument_type&& arg0) const
    {
        switch (dims)
        {
        case 0: HPX_FALLTHROUGH;
        case 1:
            return common::transpose0d1d(std::move(arg0));

        case 2:
            return common::transpose2d(std::move(arg0), name_, codename_);

        case 3:
            return common::transpose3d(std::move(arg0), name_, codename_);

        default:
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "transpose_operation::transpose_nd",
                generate_error_message(
                    "left hand side operand has unsupported "
                        "number of dimensions"));
        }
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

        if (!valid(operands[0]))
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "transpose_operation::transpose_operation",
                generate_error_message(
                    "the transpose_operation primitive requires "
                        "that the arguments given by the operands "
                        "array are valid"));
        }

        auto this_ = this->shared_from_this();
        return hpx::dataflow(hpx::launch::sync, hpx::util::unwrapping(
            [this_ = std::move(this_)](primitive_arguments_type&& args)
            -> primitive_argument_type
            {
                auto a_dims = extract_numeric_value_dimension(
                    args[0], this_->name_, this_->codename_);

                if (args.size() == 2 && valid(args[1]))
                {
                    // converting a range axes to a vector
                    if (is_list_operand_strict(args[1]))
                    {
                        ir::range axes = extract_list_value_strict(
                            std::move(args[1]), this_->name_, this_->codename_);
                        blaze::DynamicVector<std::int64_t> ops(axes.size());

                        std::size_t i = 0;
                        for (auto&& elem : axes)
                        {
                            ops[i++] = extract_scalar_integer_value_strict(
                                std::move(elem), this_->name_, this_->codename_);
                        }
                        args[1] = primitive_argument_type{std::move(ops)};
                    }

                    if (this_->validate_axes(a_dims,
                            extract_integer_value_strict(args[1], this_->name_,
                            this_->codename_)))
                    {
                        return this_->transpose_nd(a_dims, std::move(args[0]),
                            std::move(args[1]));
                    }

                    HPX_THROW_EXCEPTION(hpx::bad_parameter,
                        "transpose_operation::eval",
                        util::generate_error_message(
                            "At least one of the given axes is out of "
                            "bounds for the given array. Axes size should"
                            "be the same as array's number of dimensions."
                            "Having an n-d array each axis should be in "
                            "[-n, n-1]",
                            this_->name_, this_->codename_));
                }

                // no axes is given or axes=None
                return this_->transpose_nd(a_dims, std::move(args[0]));
            }),
            detail::map_operands(operands, functional::value_operand{}, args,
                name_, codename_, std::move(ctx)));
    }
}}}
