// Copyright (c) 2018 Shahrzad Shirzad
// Copyright (c) 2018 Hartmut Kaiser
// Copyright (c) 2018 Parsa Amini
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>
#include <phylanx/execution_tree/primitives/node_data_helpers.hpp>
#include <phylanx/ir/node_data.hpp>
#include <phylanx/plugins/matrixops/unique.hpp>
#include <phylanx/util/matrix_iterators.hpp>

#include <hpx/include/lcos.hpp>
#include <hpx/include/naming.hpp>
#include <hpx/include/util.hpp>
#include <hpx/throw_exception.hpp>
#include <hpx/util/iterator_facade.hpp>

#include <algorithm>
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
    match_pattern_type const unique::match_data = {hpx::util::make_tuple(
        "unique", std::vector<std::string>{"unique(_1)", "unique(_1, _2)"},
        &create_unique, &create_primitive<unique>, R"(
            a, axis
            Args:

                a (array_like) : input array
                axis (optional, int): which axis of a to use

            Returns:

            The sorted unique elements of an array."
            )")};

    ///////////////////////////////////////////////////////////////////////////
    unique::unique(primitive_arguments_type && operands,
        std::string const& name, std::string const& codename)
      : primitive_component_base(std::move(operands), name, codename)
    {
    }

    ///////////////////////////////////////////////////////////////////////////
    template <typename T>
    primitive_argument_type unique::unique0d(ir::node_data<T> && arg) const
    {
        blaze::DynamicVector<T> result(1UL, arg.scalar());
        return primitive_argument_type{std::move(result)};
    }

    primitive_argument_type unique::unique0d(primitive_arguments_type && args)
        const
    {
        std::size_t numargs = args.size();
        if (numargs == 2)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "phylanx::execution_tree::primitives::unique::unique0d",
                generate_error_message("invalid axis specified for unique"));
        }

        switch (extract_common_type(args[0]))
        {
        case node_data_type_bool:
            return unique0d(extract_boolean_value_strict(
                std::move(args[0]), name_, codename_));

        case node_data_type_int64:
            return unique0d(extract_integer_value_strict(
                std::move(args[0]), name_, codename_));

        case node_data_type_double:
            return unique0d(extract_numeric_value_strict(
                std::move(args[0]), name_, codename_));

        case node_data_type_unknown:
            return unique0d(
                extract_numeric_value(std::move(args[0]), name_, codename_));

        default:
            break;
        }

        HPX_THROW_EXCEPTION(hpx::bad_parameter,
            "phylanx::execution_tree::primitives::unique::unique0d",
            generate_error_message(
                "the unique primitive requires for all arguments to "
                "be numeric data types"));
    }

    ///////////////////////////////////////////////////////////////////////////
    template <typename T>
    primitive_argument_type unique::unique1d(ir::node_data<T> && arg) const
    {
        blaze::DynamicVector<T> a = arg.vector();
        // Sorting the vector
        std::sort(a.begin(), a.end());

        // Use std::unique to remove duplicacy
        auto ip = std::unique(a.begin(), a.end());

        // Resizing the vector to remove undifined elements
        a.resize(std::distance(a.begin(), ip));

        return primitive_argument_type{std::move(a)};
    }

    primitive_argument_type unique::unique1d(primitive_arguments_type && args)
        const
    {
        std::int64_t axis = 0;
        std::size_t numargs = args.size();
        if (numargs == 2)
        {
            axis = extract_scalar_integer_value(args[1], name_, codename_);
            if (axis < -1 || axis > 0)
            {
                HPX_THROW_EXCEPTION(hpx::bad_parameter,
                    "phylanx::execution_tree::primitives::argmax::argmax1d",
                    generate_error_message(
                        "operand axis can only between -1 and 0 for "
                        "an a operand that is 1d"));
            }
        }

        switch (extract_common_type(args[0]))
        {
        case node_data_type_bool:
            return unique1d(extract_boolean_value_strict(
                std::move(args[0]), name_, codename_));

        case node_data_type_int64:
            return unique1d(extract_integer_value_strict(
                std::move(args[0]), name_, codename_));

        case node_data_type_double:
            return unique1d(extract_numeric_value_strict(
                std::move(args[0]), name_, codename_));

        case node_data_type_unknown:
            return unique1d(
                extract_numeric_value(std::move(args[0]), name_, codename_));

        default:
            break;
        }

        HPX_THROW_EXCEPTION(hpx::bad_parameter,
            "phylanx::execution_tree::primitives::unique::unique1d",
            generate_error_message(
                "the arange primitive requires for all arguments to "
                "be numeric data types"));
    }

    ///////////////////////////////////////////////////////////////////////////
    template <typename T>
    primitive_argument_type unique::unique2d_flatten(
        ir::node_data<T>&& arg) const
    {
        using phylanx::util::matrix_row_iterator;
        auto a = arg.matrix();

        const matrix_row_iterator<decltype(a)> a_begin(a);
        const matrix_row_iterator<decltype(a)> a_end(a, a.rows());

        blaze::DynamicVector<double> result(a.rows() * a.columns());
        auto d = result.data();

        for (auto it = a_begin; it != a_end; ++it)
        {
            d = std::copy(it->begin(), it->end(), d);
        }

        // Sorting the vector
        std::sort(result.begin(), result.end());

        // Use std::unique to remove duplicacy
        auto ip = std::unique(result.begin(), result.end());

        // Resizing the vector to remove undifined elements
        result.resize(std::distance(result.begin(), ip));

        return primitive_argument_type{std::move(result)};
    }

    template <typename T>
    primitive_argument_type unique::unique2d_x_axis(
        ir::node_data<T>&& arg) const
    {
        auto a = arg.matrix();

        using phylanx::util::matrix_row_iterator;
        matrix_row_iterator<decltype(a)> a_begin(a);

        blaze::DynamicMatrix<T> result(a.rows(), a.columns());

        std::vector<matrix_row_iterator<decltype(a)>> indices(
            a.rows(), a_begin);
        std::iota(indices.begin(), indices.end(), a_begin);

        std::sort(indices.begin(), indices.end(),
            [&](const auto& lhs, const auto& rhs) {
                return std::lexicographical_compare(
                    lhs->begin(), lhs->end(), rhs->begin(), rhs->end());
            });

        for (size_t i = 0; i < a.rows(); i++)
        {
            blaze::row(result, i) = *(indices[i]);
        }

        return primitive_argument_type{std::move(result)};
    }

    template <typename T>
    primitive_argument_type unique::unique2d_y_axis(
        ir::node_data<T>&& arg) const
    {
        auto a = arg.matrix();

        using phylanx::util::matrix_column_iterator;
        matrix_column_iterator<decltype(a)> a_begin(a);

        blaze::DynamicMatrix<T> result(a.rows(), a.columns());

        std::vector<matrix_column_iterator<decltype(a)>> indices(
            a.columns(), a_begin);
        std::iota(indices.begin(), indices.end(), a_begin);

        std::sort(indices.begin(), indices.end(),
            [&](const auto& lhs, const auto& rhs) {
                return std::lexicographical_compare(
                    lhs->begin(), lhs->end(), rhs->begin(), rhs->end());
            });

        for (size_t i = 0; i < a.columns(); i++)
        {
            blaze::column(result, i) = *(indices[i]);
        }

        return primitive_argument_type{std::move(result)};
    }

    template <typename T>
    primitive_argument_type unique::unique2d(
        std::size_t numargs, ir::node_data<T>&& arg, std::int64_t axis) const
    {
        // m should not be empty
        auto m = arg.matrix();

        // `axis` is optional
        if (numargs == 1)
        {
            // Option 1: Flatten and find max
            return unique2d_flatten(std::move(arg));
        }

        // `axis` can only be -2, -1, 0, or 1
        if (axis < -2 || axis > 1)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "unique::unique2d",
                generate_error_message(
                    "operand axis can only between -2 and 1 for an an "
                    "operand that is 2d"));
        }

        switch (axis)
        {
        case -2:
            HPX_FALLTHROUGH;
        case 0:
            return unique2d_x_axis(std::move(arg));
        case -1:
            HPX_FALLTHROUGH;
        case 1:
            return unique2d_y_axis(std::move(arg));

        default:
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "unique::unique2d",
                generate_error_message(
                    "operand has an invalid value for the axis parameter"));
        }
    }

    primitive_argument_type unique::unique2d(
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
            return unique2d(numargs,
                extract_boolean_value_strict(
                    std::move(args[0]), name_, codename_),
                axis);

        case node_data_type_int64:
            return unique2d(numargs,
                extract_integer_value_strict(
                    std::move(args[0]), name_, codename_),
                axis);

        case node_data_type_double:
            return unique2d(numargs,
                extract_numeric_value_strict(
                    std::move(args[0]), name_, codename_),
                axis);

        case node_data_type_unknown:
            return unique2d(numargs,
                extract_numeric_value(std::move(args[0]), name_, codename_),
                axis);

        default:
            break;
        }

        HPX_THROW_EXCEPTION(hpx::bad_parameter,
            "phylanx::execution_tree::primitives::unique::unique2d",
            generate_error_message(
                "the unique primitive requires for all arguments to "
                "be numeric data types"));
    }

    ///////////////////////////////////////////////////////////////////////////
    hpx::future<primitive_argument_type> unique::eval(
        primitive_arguments_type const& operands,
        primitive_arguments_type const& args, eval_context ctx) const
    {
        if (operands.empty() || operands.size() > 2)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "unique::eval",
                generate_error_message("the unique primitive requires exactly "
                                       "one or two operands"));
        }

        for (auto const& i : operands)
        {
            if (!valid(i))
            {
                HPX_THROW_EXCEPTION(hpx::bad_parameter,
                    "unique::eval",
                    generate_error_message(
                        "the unique primitive requires that the "
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
                    return this_->unique0d(std::move(args));

                case 1:
                    return this_->unique1d(std::move(args));

                case 2:
                    return this_->unique2d(std::move(args));

                default:
                    HPX_THROW_EXCEPTION(hpx::bad_parameter,
                        "unique::eval",
                        this_->generate_error_message(
                            "operand a has an invalid number of dimensions"));
                }
            }),
            detail::map_operands(operands, functional::value_operand{}, args,
                name_, codename_, std::move(ctx)));
    }
}}}
