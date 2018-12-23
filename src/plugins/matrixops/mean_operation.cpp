// Copyright (c) 2018 Monil, Mohammad Alaul Haque
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>
#include <phylanx/ir/node_data.hpp>
#include <phylanx/plugins/matrixops/mean_operation.hpp>
#include <phylanx/util/matrix_iterators.hpp>

#include <hpx/include/lcos.hpp>
#include <hpx/include/naming.hpp>
#include <hpx/include/util.hpp>
#include <hpx/throw_exception.hpp>
#include <hpx/util/iterator_facade.hpp>

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include <blaze/Math.h>

///////////////////////////////////////////////////////////////////////////////
namespace phylanx { namespace execution_tree { namespace primitives
{
    ///////////////////////////////////////////////////////////////////////////
    match_pattern_type const mean_operation::match_data =
    {
        match_pattern_type{
            "mean",
            std::vector<std::string>{"mean(_1, _2)", "mean(_1)"},
            &create_mean_operation,
            &create_primitive<mean_operation>, R"(
            ar, axis
            Args:

                ar (array) : an array of values
                axis (optional, int) : the axis along which to calculate the mean

            Returns:

            The mean of the array. If an axis is specified, the result is the vector
            created when the mean is taken along the specified axis.)",
            true
        }
    };

    ///////////////////////////////////////////////////////////////////////////
    mean_operation::mean_operation(
        primitive_arguments_type&& operands,
        std::string const& name, std::string const& codename)
      : primitive_component_base(std::move(operands), name, codename)
      , dtype_(extract_dtype(name_))
    {
    }

    ///////////////////////////////////////////////////////////////////////////
    primitive_argument_type mean_operation::mean0d(
        primitive_argument_type&& arg, bool has_axis, std::int64_t axis) const
    {
        // `axis` is optional
        if (has_axis)
        {
            if (axis < -1 || axis > 0)
            {
                HPX_THROW_EXCEPTION(hpx::bad_parameter,
                    "mean_operation::mean0d",
                    generate_error_message(
                        "operand axis can only between -1 and 0 for "
                        "an a scalar operand"));
            }
        }
        return primitive_argument_type{std::move(arg)};
    }

    ///////////////////////////////////////////////////////////////////////////
    template <typename T>
    primitive_argument_type mean_operation::mean1d(ir::node_data<T>&& arg) const
    {
        // the given vector should not be empty
        auto a = arg.vector();
        if (a.size() == 0)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "mean_operation::mean1d",
                generate_error_message(
                    "attempt to get mean of an empty sequence"));
        }

        // Find the sum of all the elements
        T sum = std::accumulate(a.begin(), a.end(), T(0));

        // Return the mean
        return primitive_argument_type(T(sum / a.size()));
    }

    primitive_argument_type mean_operation::mean1d(
        primitive_argument_type&& arg, bool has_axis, std::int64_t axis) const
    {
        // `axis` is optional
        if (has_axis)
        {
            // `axis` can only be -1 or 0
            if (axis < -1 || axis > 0)
            {
                HPX_THROW_EXCEPTION(hpx::bad_parameter,
                    "mean_operation::mean1d",
                    generate_error_message(
                        "operand axis can only between -1 and 0 for "
                        "an a vector operand"));
            }
        }

        node_data_type t = dtype_;
        if (t == node_data_type_unknown)
        {
            t = extract_common_type(arg);
        }

        switch (t)
        {
        case node_data_type_bool:
            return mean1d(extract_boolean_value_strict(std::move(arg)));

        case node_data_type_int64:
            return mean1d(extract_integer_value_strict(std::move(arg)));

        case node_data_type_unknown: HPX_FALLTHROUGH;
        case node_data_type_double:
            return mean1d(extract_numeric_value(std::move(arg)));

        default:
            break;
        }

        HPX_THROW_EXCEPTION(hpx::bad_parameter,
            "mean_operation::mean1d",
            generate_error_message(
                "the mean primitive requires for its argument to "
                    "be numeric data type"));
    }

    ///////////////////////////////////////////////////////////////////////////
    template <typename T>
    primitive_argument_type mean_operation::mean2d_flatten(
        ir::node_data<T>&& arg) const
    {
        using phylanx::util::matrix_row_iterator;

        auto matrix = arg.matrix();

        matrix_row_iterator<decltype(matrix)> const matrix_begin(matrix);
        matrix_row_iterator<decltype(matrix)> const matrix_end(
            matrix, matrix.rows());

        T global_sum = T(0);
        std::size_t global_size = 0;
        for (auto it = matrix_begin; it != matrix_end; ++it)
        {
            global_sum += std::accumulate(it->begin(), it->end(), T(0));
            global_size += (*it).size();
        }

        return primitive_argument_type(T(global_sum / global_size));
    }

    template <typename T>
    primitive_argument_type mean_operation::mean2d_x_axis(
        ir::node_data<T>&& arg) const
    {
        using phylanx::util::matrix_row_iterator;

        auto matrix = arg.matrix();

        matrix_row_iterator<decltype(matrix)> const matrix_begin(matrix);
        matrix_row_iterator<decltype(matrix)> const matrix_end(
            matrix, matrix.rows());

        blaze::DynamicVector<T> result(matrix.rows());
        auto result_it = result.begin();
        for (auto it = matrix_begin; it != matrix_end; ++it, ++result_it)
        {
            auto local_sum = std::accumulate(it->begin(), it->end(), T(0));
            *result_it = local_sum / (*it).size();
        }
        return primitive_argument_type{std::move(result)};
    }

    template <typename T>
    primitive_argument_type mean_operation::mean2d_y_axis(
        ir::node_data<T>&& arg) const
    {
        using phylanx::util::matrix_column_iterator;

        auto matrix = arg.matrix();

        matrix_column_iterator<decltype(matrix)> const matrix_begin(matrix);
        matrix_column_iterator<decltype(matrix)> const matrix_end(
            matrix, matrix.columns());

        blaze::DynamicVector<T> result(matrix.columns());
        auto result_it = result.begin();
        for (auto it = matrix_begin; it != matrix_end; ++it, ++result_it)
        {
            auto local_sum = std::accumulate(it->begin(), it->end(), T(0));
            *result_it = local_sum / (*it).size();
        }
        return primitive_argument_type{std::move(result)};
    }

    template <typename T>
    primitive_argument_type mean_operation::mean2d(
        ir::node_data<T>&& arg, bool has_axis, std::int64_t axis) const
    {
        // matrix should not be empty
        auto m = arg.matrix();
        if (m.rows() == 0 || m.columns() == 0)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "mean_operation::mean2d",
                generate_error_message(
                    "attempt to get mean of an empty sequence"));
        }

        // `axis` is optional
        if (!has_axis)    // it should not be two
        {
            // Option 1: Flatten and find mean
            return mean2d_flatten(std::move(arg));
        }

        // `axis` can only be -2, -1, 0, or 1
        if (axis < -2 || axis > 1)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "mean_operation::mean2d",
                generate_error_message(
                    "operand axis can only between -2 and 1 "
                    "for a matrix operand"));
        }

        switch (axis)
        {
        case -2: HPX_FALLTHROUGH;
        case 0:
            // Option 2: Find mean among rows
            return mean2d_x_axis(std::move(arg));

        case -1: HPX_FALLTHROUGH;
        case 1:
            // Option 3: Find mean among columns
            return mean2d_y_axis(std::move(arg));

        default:
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "mean_operation::mean2d",
                generate_error_message(
                    "operand a has an invalid number of dimensions"));
        }
    }

    primitive_argument_type mean_operation::mean2d(
        primitive_argument_type&& arg, bool has_axis, std::int64_t axis) const
    {
        node_data_type t = dtype_;
        if (t == node_data_type_unknown)
        {
            t = extract_common_type(arg);
        }

        switch (t)
        {
        case node_data_type_bool:
            return mean2d(
                extract_boolean_value_strict(std::move(arg)), has_axis, axis);

        case node_data_type_int64:
            return mean2d(
                extract_integer_value_strict(std::move(arg)), has_axis, axis);

        case node_data_type_unknown: HPX_FALLTHROUGH;
        case node_data_type_double:
            return mean2d(
                extract_numeric_value(std::move(arg)), has_axis, axis);

        default:
            break;
        }

        HPX_THROW_EXCEPTION(hpx::bad_parameter,
            "mean_operation::mean2d",
            generate_error_message(
                "the mean primitive requires for its argument to "
                    "be numeric data type"));
    }

    ///////////////////////////////////////////////////////////////////////////
    hpx::future<primitive_argument_type> mean_operation::eval(
        primitive_arguments_type const& operands,
        primitive_arguments_type const& args, eval_context ctx) const
    {
        if (operands.size() != 1 && operands.size() != 2)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "phylanx::execution_tree::primitives::"
                "mean_operation::mean_operation",
                generate_error_message(
                    "the mean_operation primitive requires "
                    "either one or two arguments"));
        }

        bool arguments_valid = true;
        for (std::size_t i = 0; i != operands.size(); ++i)
        {
            if (!valid(operands[i]))
            {
                arguments_valid = false;
            }
        }

        if (!arguments_valid)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "mean_operation::eval",
                generate_error_message(
                    "the mean_operation primitive requires "
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
                    case 0:
                        return this_->mean0d(std::move(arg));

                    case 1:
                        return this_->mean1d(std::move(arg));

                    case 2:
                        return this_->mean2d(std::move(arg));

                    default:
                        HPX_THROW_EXCEPTION(hpx::bad_parameter,
                            "mean_operation::eval",
                            this_->generate_error_message(
                                "left hand side operand has unsupported "
                                "number of dimensions"));
                    }
                }),
                value_operand(
                    operands[0], args, name_, codename_, std::move(ctx)));
        }

        return hpx::dataflow(hpx::launch::sync, hpx::util::unwrapping(
            [this_ = std::move(this_)](
                    primitive_argument_type&& arg, std::int64_t axis)
            -> primitive_argument_type
            {
                switch (extract_numeric_value_dimension(
                    arg, this_->name_, this_->codename_))
                {
                case 0:
                    return this_->mean0d(std::move(arg), true, axis);

                case 1:
                    return this_->mean1d(std::move(arg), true, axis);

                case 2:
                    return this_->mean2d(std::move(arg), true, axis);

                default:
                    HPX_THROW_EXCEPTION(hpx::bad_parameter,
                        "mean_operation::eval",
                        this_->generate_error_message(
                            "left hand side operand has unsupported "
                            "number of dimensions"));
                }
            }),
            value_operand(operands[0], args, name_, codename_, ctx),
            scalar_integer_operand(operands[1], args, name_, codename_, ctx));
    }
}}}
