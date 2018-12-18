// Copyright (c) 2018 Bita Hasheminezhad
// Copyright (c) 2018 Hartmut Kaiser
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>
#include <phylanx/ir/node_data.hpp>
#include <phylanx/plugins/matrixops/min_operation.hpp>
#include <phylanx/util/matrix_iterators.hpp>

#include <hpx/include/lcos.hpp>
#include <hpx/include/naming.hpp>
#include <hpx/include/util.hpp>
#include <hpx/throw_exception.hpp>
#include <hpx/util/iterator_facade.hpp>
#include <hpx/util/optional.hpp>

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
    match_pattern_type const min_operation::match_data =
    {
        hpx::util::make_tuple("min",
        std::vector<std::string>{"min(_1)", "min(_1,_2)", "min(_1,_2,_3)"},
        &create_min_operation, &create_primitive<min_operation>,
        "a, axis, keepdims\n"
        "Args:\n"
        "\n"
        "    a (vector or matrix) : a scalar, a vector or a matrix\n"
        "    axis (optional, integer): an axis to min along. By default, "
        "       flattened input is used.\n"
        "    keepdims (optional, bool): If this is set to True, the axes which "
        "       are reduced are left in the result as dimensions with size "
        "       one. False by default \n"
        "\n"
        "Returns:\n"
        "\n"
        "Returns the minimum of an array or minimum along an axis.")
    };

    ///////////////////////////////////////////////////////////////////////////
    min_operation::min_operation(primitive_arguments_type&& operands,
        std::string const& name, std::string const& codename)
        : primitive_component_base(std::move(operands), name, codename)
    {}

    template <typename T>
    primitive_argument_type min_operation::min0d(ir::node_data<T>&& arg,
        hpx::util::optional<std::int64_t> axis) const
    {
        if (axis && axis.value() != 0 && axis.value() != -1)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "min_operation::min0d",
                generate_error_message(
                    "the min_operation primitive requires operand axis to be "
                    "either 0 or -1 for scalar values."));
        }

        return primitive_argument_type{arg.scalar()};
    }

    template <typename T>
    primitive_argument_type min_operation::min1d(ir::node_data<T>&& arg,
        hpx::util::optional<std::int64_t> axis, bool keep_dims) const
    {
        if (axis && axis.value() != 0 && axis.value() != -1)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "min_operation::min1d",
                generate_error_message(
                    "the min_operation primitive requires operand axis to be "
                    "either 0 or -1 for vectors."));
        }
        auto v = arg.vector();
        T result = blaze::min(v);

        if (keep_dims)
        {
            return primitive_argument_type{blaze::DynamicVector<T>{result}};
        }
        return primitive_argument_type{ir::node_data<T>{result}};
    }

    template <typename T>
    primitive_argument_type min_operation::min2d(ir::node_data<T>&& arg,
        hpx::util::optional<std::int64_t> axis, bool keep_dims) const
    {
        if (axis)
        {
            switch (axis.value())
            {
            case -2:
                HPX_FALLTHROUGH;
            case 0:
                return min2d_axis0(std::move(arg), keep_dims);

            case -1:
                HPX_FALLTHROUGH;
            case 1:
                return min2d_axis1(std::move(arg), keep_dims);

            default:
                HPX_THROW_EXCEPTION(hpx::bad_parameter,
                    "min_operation::min2d",
                    generate_error_message(
                        "the min_operation primitive requires operand axis "
                        "to be between -2 and 1 for matrices."));
            }
        }
        return min2d_flat(std::move(arg), keep_dims);
    }

    template <typename T>
    primitive_argument_type min_operation::min2d_flat(
        ir::node_data<T>&& arg, bool keep_dims) const
    {
        auto m = arg.matrix();
        T result = blaze::min(m);

        if (keep_dims)
        {
            return primitive_argument_type{blaze::DynamicMatrix<T>{{result}}};
        }
        return primitive_argument_type{ir::node_data<T>{result}};
    }

    template <typename T>
    primitive_argument_type min_operation::min2d_axis0(
        ir::node_data<T>&& arg, bool keep_dims) const
    {
        auto m = arg.matrix();
        blaze::DynamicVector<T> result(m.columns());
        for (std::size_t i = 0; i < m.columns(); ++i)
        {
            result[i] = blaze::min(column(m, i));
        }

        if (keep_dims)
        {
            return primitive_argument_type{
                blaze::DynamicMatrix<T>{1, result.size(), result.data()}};
        }
        return primitive_argument_type{ir::node_data<T>{result}};
    }

    template <typename T>
    primitive_argument_type min_operation::min2d_axis1(
        ir::node_data<T>&& arg, bool keep_dims) const
    {
        auto m = arg.matrix();
        blaze::DynamicVector<T> result(m.rows());
        for (std::size_t i = 0; i < m.rows(); ++i)
        {
            result[i] = blaze::min(row(m, i));
        }

        if (keep_dims)
        {
            return primitive_argument_type{
                blaze::DynamicMatrix<T>{result.size(), 1, result.data()}};
        }
        return primitive_argument_type{ir::node_data<T>{result}};
    }

    template <typename T>
    primitive_argument_type min_operation::minnd(ir::node_data<T>&& arg,
        hpx::util::optional<std::int64_t> axis, bool keep_dims) const
    {
        std::size_t a_dims = arg.num_dimensions();
        switch (a_dims)
        {
        case 0:
            return min0d(std::move(arg), axis);

        case 1:
            return min1d(std::move(arg), axis, keep_dims);

        case 2:
            return min2d(std::move(arg), axis, keep_dims);

        default:
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "min_operation::eval",
                generate_error_message(
                    "operand a has an invalid number of dimensions"));
        }
    }

    ///////////////////////////////////////////////////////////////////////////
    hpx::future<primitive_argument_type> min_operation::eval(
        primitive_arguments_type const& operands,
        primitive_arguments_type const& args, eval_context ctx) const
    {
        if (operands.empty() || operands.size() > 3)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "min_operation::eval",
                generate_error_message("the min_operation primitive requires "
                                       "exactly one, two, or "
                                       "three operands"));
        }

        for (auto const& i : operands)
        {
            if (!valid(i))
            {
                HPX_THROW_EXCEPTION(hpx::bad_parameter,
                    "min_operation::eval",
                    generate_error_message(
                        "the min_operation primitive requires that the "
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
                return this_->minnd(extract_boolean_value(std::move(args[0]),
                                        this_->name_, this_->codename_),
                    axis, keep_dims);
            case node_data_type_int64:
                return this_->minnd(extract_integer_value(std::move(args[0]),
                                        this_->name_, this_->codename_),
                    axis, keep_dims);
            case node_data_type_double:
                return this_->minnd(extract_numeric_value(std::move(args[0]),
                                        this_->name_, this_->codename_),
                    axis, keep_dims);
            default:
                break;
            }

            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "min::eval",
                this_->generate_error_message(
                    "the min primitive requires for all arguments "
                    "to be numeric data types"));
        }),
            detail::map_operands(operands, functional::value_operand{}, args,
                name_, codename_, std::move(ctx)));
    }
}}}
