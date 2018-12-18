// Copyright (c) 2018 Bita Hasheminezhad
// Copyright (c) 2018 Hartmut Kaiser
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>
#include <phylanx/ir/node_data.hpp>
#include <phylanx/plugins/matrixops/max_operation.hpp>
#include <phylanx/util/matrix_iterators.hpp>

#include <hpx/include/lcos.hpp>
#include <hpx/include/naming.hpp>
#include <hpx/include/util.hpp>
#include <hpx/throw_exception.hpp>
#include <hpx/util/iterator_facade.hpp>
#include <hpx/util/optional.hpp>

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
    match_pattern_type const max_operation::match_data =
    {
        hpx::util::make_tuple("max",
        std::vector<std::string>{"max(_1)", "max(_1,_2)", "max(_1,_2,_3)"},
        &create_max_operation, &create_primitive<max_operation>,
        "a, axis, keepdims\n"
        "Args:\n"
        "\n"
        "    a (vector or matrix) : a scalar, a vector or a matrix\n"
        "    axis (optional, integer): an axis to max along. By default, "
        "       flattened input is used.\n"
        "    keepdims (optional, bool): If this is set to True, the axes which "
        "       are reduced are left in the result as dimensions with size "
        "       one. False by default \n"
        "\n"
        "Returns:\n"
        "\n"
        "Returns the maximum of an array or maximum along an axis.")
    };

    ///////////////////////////////////////////////////////////////////////////
    max_operation::max_operation(primitive_arguments_type&& operands,
        std::string const& name, std::string const& codename)
        : primitive_component_base(std::move(operands), name, codename)
    {}

    template <typename T>
    primitive_argument_type max_operation::max0d(ir::node_data<T>&& arg,
        hpx::util::optional<std::int64_t> axis) const
    {
        if (axis && axis.value() != 0 && axis.value() != -1)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "max_operation::max0d",
                generate_error_message(
                    "the max_operation primitive requires operand axis to be "
                    "either 0 or -1 for scalar values."));
        }

        return primitive_argument_type{arg.scalar()};
    }

    template <typename T>
    primitive_argument_type max_operation::max1d(ir::node_data<T>&& arg,
        hpx::util::optional<std::int64_t> axis, bool keep_dims) const
    {
        if (axis && axis.value() != 0 && axis.value() != -1)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "max_operation::max1d",
                generate_error_message(
                    "the max_operation primitive requires operand axis to be "
                    "either 0 or -1 for vectors."));
        }
        auto v = arg.vector();
        T result = (blaze::max)(v);

        if (keep_dims)
        {
            return primitive_argument_type{blaze::DynamicVector<T>{std::move(result)}};
        }
        return primitive_argument_type{ir::node_data<T>{std::move(result)}};
    }

    template <typename T>
    primitive_argument_type max_operation::max2d(ir::node_data<T>&& arg,
        hpx::util::optional<std::int64_t> axis, bool keep_dims) const
    {
        if (axis)
        {
            switch (axis.value())
            {
            case -2:
                HPX_FALLTHROUGH;
            case 0:
                return max2d_axis0(std::move(arg), keep_dims);

            case -1:
                HPX_FALLTHROUGH;
            case 1:
                return max2d_axis1(std::move(arg), keep_dims);

            default:
                HPX_THROW_EXCEPTION(hpx::bad_parameter,
                    "max_operation::max2d",
                    generate_error_message(
                        "the max_operation primitive requires operand axis "
                        "to be between -2 and 1 for matrices."));
            }
        }
        return max2d_flat(std::move(arg), keep_dims);
    }

    template <typename T>
    primitive_argument_type max_operation::max2d_flat(
        ir::node_data<T>&& arg, bool keep_dims) const
    {
        auto m = arg.matrix();
        T result = (blaze::max)(m);

        if (keep_dims)
        {
            return primitive_argument_type{
                blaze::DynamicMatrix<T>{{std::move(result)}}};
        }
        return primitive_argument_type{ir::node_data<T>{std::move(result)}};
    }

    template <typename T>
    primitive_argument_type max_operation::max2d_axis0(
        ir::node_data<T>&& arg, bool keep_dims) const
    {
        auto m = arg.matrix();
        blaze::DynamicVector<T> result(m.columns());
        for (std::size_t i = 0; i < m.columns(); ++i)
        {
            result[i] = (blaze::max)(column(m, i));
        }

        if (keep_dims)
        {
            return primitive_argument_type{
                blaze::DynamicMatrix<T>{1, result.size(), result.data()}};
        }
        return primitive_argument_type{ir::node_data<T>{std::move(result)}};
    }

    template <typename T>
    primitive_argument_type max_operation::max2d_axis1(
        ir::node_data<T>&& arg, bool keep_dims) const
    {
        auto m = arg.matrix();
        blaze::DynamicVector<T> result(m.rows());
        for (std::size_t i = 0; i < m.rows(); ++i)
        {
            result[i] = (blaze::max)(row(m, i));
        }

        if (keep_dims)
        {
            return primitive_argument_type{
                blaze::DynamicMatrix<T>{result.size(), 1, result.data()}};
        }
        return primitive_argument_type{ir::node_data<T>{std::move(result)}};
    }

    template <typename T>
    primitive_argument_type max_operation::maxnd(ir::node_data<T>&& arg,
        hpx::util::optional<std::int64_t> axis, bool keep_dims) const
    {
        std::size_t a_dims = arg.num_dimensions();
        switch (a_dims)
        {
        case 0:
            return max0d(std::move(arg), axis);

        case 1:
            return max1d(std::move(arg), axis, keep_dims);

        case 2:
            return max2d(std::move(arg), axis, keep_dims);

        default:
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "max_operation::eval",
                generate_error_message(
                    "operand a has an invalid number of dimensions"));
        }
    }

    ///////////////////////////////////////////////////////////////////////////
    hpx::future<primitive_argument_type> max_operation::eval(
        primitive_arguments_type const& operands,
        primitive_arguments_type const& args, eval_context ctx) const
    {
        if (operands.empty() || operands.size() > 3)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "max_operation::eval",
                generate_error_message("the max_operation primitive requires "
                                       "exactly one, two, or "
                                       "three operands"));
        }

        for (auto const& i : operands)
        {
            if (!valid(i))
            {
                HPX_THROW_EXCEPTION(hpx::bad_parameter,
                    "max_operation::eval",
                    generate_error_message(
                        "the max_operation primitive requires that the "
                        "arguments given by the operands array are valid"));
            }
        }

        auto this_ = this->shared_from_this();
        return hpx::dataflow(hpx::launch::sync,
            hpx::util::unwrapping(
                [this_ = std::move(this_)](primitive_arguments_type&& args)
                ->primitive_argument_type {
            // Extract axis and keep_dims
            // Presence of axis changes behavior for >0d cases
            hpx::util::optional<std::int64_t> axis;
            bool keep_dims = false;

            // axis is argument #2
            if (args.size() > 1)
            {
                if (valid(args[1]))
                {
                    axis = extract_scalar_integer_value_strict(
                        args[1], this_->name_, this_->codename_);
                }

                // keep_dims is argument #3
                if (args.size() == 3)
                {
                    keep_dims = extract_scalar_boolean_value(
                        args[2], this_->name_, this_->codename_);
                }
            }

            switch (extract_common_type(args[0]))
            {
            case node_data_type_bool:
                return this_->maxnd(extract_boolean_value(std::move(args[0]),
                                        this_->name_, this_->codename_),
                    axis, keep_dims);
            case node_data_type_int64:
                return this_->maxnd(extract_integer_value(std::move(args[0]),
                                        this_->name_, this_->codename_),
                    axis, keep_dims);
            case node_data_type_double:
                return this_->maxnd(extract_numeric_value(std::move(args[0]),
                                        this_->name_, this_->codename_),
                    axis, keep_dims);
            default:
                break;
            }

            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "max::eval",
                this_->generate_error_message(
                    "the max primitive requires for all arguments "
                    "to be numeric data types"));
        }),
            detail::map_operands(operands, functional::value_operand{}, args,
                name_, codename_, std::move(ctx)));
    }
}}}
