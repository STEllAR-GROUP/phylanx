//  Copyright (c) 2018 Parsa Amini
//  Copyright (c) 2018 Hartmut Kaiser
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>
#include <phylanx/execution_tree/primitives/node_data_helpers.hpp>
#include <phylanx/ir/node_data.hpp>
#include <phylanx/plugins/matrixops/argmin.hpp>
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
    match_pattern_type const argmin::match_data =
    {
        hpx::util::make_tuple("argmin",
            std::vector<std::string>{"argmin(_1, _2)", "argmin(_1)"},
            &create_argmin, &create_primitive<argmin>, R"(
            ar, axis
            Args:

                ar (array) : a vector or matrix
                axis (optional, int) : the axis along which to find the min

            Returns:

            The index of the minimum value in the array. If an axis is "
            specified, a vector of minima along the axis is returned."
            )")
    };

    ///////////////////////////////////////////////////////////////////////////
    argmin::argmin(primitive_arguments_type&& operands,
            std::string const& name, std::string const& codename)
      : primitive_component_base(std::move(operands), name, codename)
    {}

    ///////////////////////////////////////////////////////////////////////////
    template <typename T>
    primitive_argument_type argmin::argmin0d(
        std::size_t numargs, ir::node_data<T>&& arg, std::int64_t axis) const
    {
        // `axis` is optional
        if (numargs == 2)
        {
            // `axis` can only be -1 or 0
            if (axis < -1 || axis > 0)
            {
                HPX_THROW_EXCEPTION(hpx::bad_parameter,
                    "phylanx::execution_tree::primitives::argmin::argmin0d",
                    generate_error_message(
                        "operand axis can only between -1 and 0 for "
                        "an a operand that is 0d"));
            }
        }
        return primitive_argument_type(std::int64_t(0));
    }


    primitive_argument_type argmin::argmin0d(
        primitive_arguments_type&& args) const
    {
        std::int64_t axis = -1;
        std::size_t numargs = args.size();
        if (numargs == 2)
        {
            axis = extract_scalar_integer_value(args[1], name_, codename_);
        }

        switch (extract_common_type(args[0]))
        {
        case node_data_type_bool:
            return argmin0d(numargs, extract_boolean_value_strict(
                std::move(args[0]), name_, codename_), axis);

        case node_data_type_int64:
            return argmin0d(numargs, extract_integer_value_strict(
                std::move(args[0]), name_, codename_), axis);

        case node_data_type_double:
            return argmin0d(numargs, extract_numeric_value_strict(
                std::move(args[0]), name_, codename_), axis);

        case node_data_type_unknown:
            return argmin0d(numargs, extract_numeric_value(
                std::move(args[0]), name_, codename_), axis);

        default:
            break;
        }

        HPX_THROW_EXCEPTION(hpx::bad_parameter,
            "phylanx::execution_tree::primitives::argmin::argmin0d",
            generate_error_message(
                "the arange primitive requires for all arguments to "
                    "be numeric data types"));
    }

    ///////////////////////////////////////////////////////////////////////////
    template <typename T>
    primitive_argument_type argmin::argmin1d(
        std::size_t numargs, ir::node_data<T>&& arg, std::int64_t axis) const
    {
        // `axis` is optional
        if (numargs == 2)
        {
            // `axis` can only be -1 or 0
            if (axis < -1 || axis > 0)
            {
                HPX_THROW_EXCEPTION(hpx::bad_parameter,
                    "phylanx::execution_tree::primitives::argmin::argmin1d",
                    generate_error_message(
                        "operand axis can only between -1 and 0 for "
                        "an a operand that is 1d"));
            }
        }

        // a should not be empty
        auto a = arg.vector();
        if (a.size() == 0)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "argmax::argmax1d",
                generate_error_message(
                    "attempt to get argmax of an empty sequence"));
        }

        // Find the maximum value among the elements
        auto min_it = std::min_element(a.begin(), a.end());

        // Return min's index
        return primitive_argument_type(
            (std::int64_t)(std::distance(a.begin(), min_it)));
    }

    primitive_argument_type argmin::argmin1d(
        primitive_arguments_type&& args) const
    {
        std::int64_t axis = -1;
        std::size_t numargs = args.size();
        if (numargs == 2)
        {
            axis = extract_scalar_integer_value(args[1], name_, codename_);
        }

        switch (extract_common_type(args[0]))
        {
        case node_data_type_bool:
            return argmin1d(numargs, extract_boolean_value_strict(
                std::move(args[0]), name_, codename_), axis);

        case node_data_type_int64:
            return argmin1d(numargs, extract_integer_value_strict(
                std::move(args[0]), name_, codename_), axis);

        case node_data_type_double:
            return argmin1d(numargs, extract_numeric_value_strict(
                std::move(args[0]), name_, codename_), axis);

        case node_data_type_unknown:
            return argmin1d(numargs, extract_numeric_value(
                std::move(args[0]), name_, codename_), axis);

        default:
            break;
        }

        HPX_THROW_EXCEPTION(hpx::bad_parameter,
            "phylanx::execution_tree::primitives::argmin::argmin1d",
            generate_error_message(
                "the arange primitive requires for all arguments to "
                    "be numeric data types"));
    }

    ///////////////////////////////////////////////////////////////////////////
    template <typename T>
    primitive_argument_type argmin::argmin2d_flatten(
        ir::node_data<T>&& arg) const
    {
        auto a = arg.matrix();

        using phylanx::util::matrix_row_iterator;
        matrix_row_iterator<decltype(a)> a_begin(a);
        matrix_row_iterator<decltype(a)> a_end(a, a.rows());

        T global_min = (std::numeric_limits<T>::max)();
        std::size_t global_index = 0ul;
        std::size_t passed_rows = 0ul;
        for (auto it = a_begin; it != a_end; ++it, ++passed_rows)
        {
            auto local_min = std::min_element(it->begin(), it->end());
            auto local_min_val = *local_min;

            if (local_min_val < global_min)
            {
                global_min = local_min_val;
                global_index = std::distance(it->begin(), local_min) +
                    passed_rows * it->size();
            }
        }
        return primitive_argument_type(std::int64_t(global_index));
    }

    template <typename T>
    primitive_argument_type argmin::argmin2d_x_axis(
        ir::node_data<T>&& arg) const
    {
        auto a = arg.matrix();

        using phylanx::util::matrix_row_iterator;
        matrix_row_iterator<decltype(a)> a_begin(a);
        matrix_row_iterator<decltype(a)> a_end(a, a.rows());

        blaze::DynamicVector<std::int64_t> result(a.rows());
        auto result_it = result.begin();
        for (auto it = a_begin; it != a_end; ++it, ++result_it)
        {
            auto local_min = std::min_element(it->begin(), it->end());
            *result_it = std::distance(it->begin(), local_min);
        }
        return primitive_argument_type{std::move(result)};
    }

    template <typename T>
    primitive_argument_type argmin::argmin2d_y_axis(ir::node_data<T>&& arg) const
    {
        auto a = arg.matrix();

        using phylanx::util::matrix_column_iterator;
        matrix_column_iterator<decltype(a)> a_begin(a);
        matrix_column_iterator<decltype(a)> a_end(a, a.columns());

        blaze::DynamicVector<std::int64_t> result(a.columns());
        auto result_it = result.begin();
        for (auto it = a_begin; it != a_end; ++it, ++result_it)
        {
            auto local_min = std::min_element(it->begin(), it->end());
            *result_it = std::distance(it->begin(), local_min);
        }
        return primitive_argument_type{std::move(result)};
    }

    template <typename T>
    primitive_argument_type argmin::argmin2d(
        std::size_t numargs, ir::node_data<T>&& arg, std::int64_t axis) const
    {
        // a should not be empty
        auto m = arg.matrix();

        if (m.rows() == 0 || m.columns() == 0)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "argmin::argmin2d",
                generate_error_message(
                    "attempt to get argmin of an empty sequence"));
        }

        // `axis` is optional
        if (numargs == 1)
        {
            // Option 1: Flatten and find min
            return argmin2d_flatten(std::move(arg));
        }

        // `axis` can only be -2, -1, 0, or 1
        if (axis < -2 || axis > 1)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "argmin::argmin2d",
                generate_error_message(
                    "operand axis can only between -2 and 1 for an a "
                    "operand that is 2d"));
        }

        switch (axis)
        {
        // Option 2: Find min among rows
        case -2: HPX_FALLTHROUGH;
        case 0:
            return argmin2d_x_axis(std::move(arg));

        // Option 3: Find min among columns
        case -1: HPX_FALLTHROUGH;
        case 1:
            return argmin2d_y_axis(std::move(arg));

        default:
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "argmin::argmin2d",
                generate_error_message(
                    "operand has an invalid value for the axis parameter"));
        }
    }


    primitive_argument_type argmin::argmin2d(
        primitive_arguments_type&& args) const
    {
        std::int64_t axis = -1;
        std::size_t numargs = args.size();
        if (numargs == 2)
        {
            axis = extract_scalar_integer_value(args[1], name_, codename_);
        }

        switch (extract_common_type(args[0]))
        {
        case node_data_type_bool:
            return argmin2d(numargs, extract_boolean_value_strict(
                std::move(args[0]), name_, codename_), axis);

        case node_data_type_int64:
            return argmin2d(numargs, extract_integer_value_strict(
                std::move(args[0]), name_, codename_), axis);

        case node_data_type_double:
            return argmin2d(numargs, extract_numeric_value_strict(
                std::move(args[0]), name_, codename_), axis);

        case node_data_type_unknown:
            return argmin2d(numargs, extract_numeric_value(
                std::move(args[0]), name_, codename_), axis);

        default:
            break;
        }

        HPX_THROW_EXCEPTION(hpx::bad_parameter,
            "phylanx::execution_tree::primitives::argmin::argmin2d",
            generate_error_message(
                "the arange primitive requires for all arguments to "
                    "be numeric data types"));
    }

    ///////////////////////////////////////////////////////////////////////////
    hpx::future<primitive_argument_type> argmin::eval(
        primitive_arguments_type const& operands,
        primitive_arguments_type const& args, eval_context ctx) const
    {
        if (operands.empty() || operands.size() > 2)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "argmin::eval",
                generate_error_message(
                    "the argmin primitive requires exactly one or two operands"));
        }

        for (auto const& i : operands)
        {
            if (!valid(i))
            {
                HPX_THROW_EXCEPTION(hpx::bad_parameter,
                    "argmin::eval",
                    generate_error_message(
                        "the argmin primitive requires that the "
                        "arguments given by the operands array are valid"));
            }
        }


        auto this_ = this->shared_from_this();
        return hpx::dataflow(hpx::launch::sync, hpx::util::unwrapping(
            [this_ = std::move(this_)](primitive_arguments_type&& args)
            -> primitive_argument_type
            {
                std::size_t a_dims = extract_numeric_value_dimension(
                    args[0], this_->name_, this_->codename_);
                switch (a_dims)
                {
                case 0:
                    return this_->argmin0d(std::move(args));

                case 1:
                    return this_->argmin1d(std::move(args));

                case 2:
                    return this_->argmin2d(std::move(args));

                default:
                    HPX_THROW_EXCEPTION(hpx::bad_parameter,
                        "argmin::eval",
                        this_->generate_error_message(
                            "operand a has an invalid number of dimensions"));
                }
            }),
            detail::map_operands(
                operands, functional::value_operand{}, args,
                name_, codename_, std::move(ctx)));
    }
}}}
