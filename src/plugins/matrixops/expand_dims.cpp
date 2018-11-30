// Copyright (c) 2018 Parsa Amini
// Copyright (c) 2018 Hartmut Kaiser
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>
#include <phylanx/ir/node_data.hpp>
#include <phylanx/plugins/matrixops/expand_dims.hpp>
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
#if defined(PHYLANX_HAVE_BLAZE_TENSOR)
#include <blaze_tensor/Math.h>
#endif

///////////////////////////////////////////////////////////////////////////////
namespace phylanx { namespace execution_tree { namespace primitives
{
    ///////////////////////////////////////////////////////////////////////////
    match_pattern_type const expand_dims::match_data =
    {
        match_pattern_type("expand_dims",
            std::vector<std::string>{"expand_dims(_1)"},
            &create_expand_dims, &create_primitive<expand_dims>, R"(
            arg
            Args:

                arg (number or list of numbers): number or list of numbers

            Returns:

            Adds a dimension, making a scalar a vector or a vector a matrix.
            )")
    };

    ///////////////////////////////////////////////////////////////////////////
    expand_dims::expand_dims(primitive_arguments_type&& operands,
            std::string const& name, std::string const& codename)
      : primitive_component_base(std::move(operands), name, codename)
    {}

    ///////////////////////////////////////////////////////////////////////////
    template <typename T>
    primitive_argument_type expand_dims::add_dim_0d(
        ir::node_data<T>&& arg) const
    {
        return primitive_argument_type{
            blaze::DynamicVector<T>(1, arg.scalar())};
    }

    primitive_argument_type expand_dims::add_dim_0d(
        primitive_arguments_type&& args) const
    {
        std::int64_t axis = 0;
        if (args.size() == 2)
        {
            axis =
                extract_scalar_integer_value_strict(args[1], name_, codename_);
        }

        if (axis != 0 || axis < -1)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "phylanx::execution_tree::primitives::expand_dims::add_dim_0d",
                generate_error_message(
                    "the axis parameter to expand_dims is out of range"));
        }

        if (axis < 0)
        {
            axis += 1;
        }

        switch (extract_common_type(args[0]))
        {
        case node_data_type_bool:
            return add_dim_0d(extract_boolean_value_strict(
                std::move(args[0]), name_, codename_));

        case node_data_type_int64:
            return add_dim_0d(extract_integer_value_strict(
                std::move(args[0]), name_, codename_));

        case node_data_type_double:
            return add_dim_0d(extract_numeric_value_strict(
                std::move(args[0]), name_, codename_));

        case node_data_type_unknown:
            return add_dim_0d(extract_numeric_value(
                std::move(args[0]), name_, codename_));

        default:
            break;
        }

        HPX_THROW_EXCEPTION(hpx::bad_parameter,
            "phylanx::execution_tree::primitives::expand_dims::add_dim_0d",
            generate_error_message(
                "the expand_dims primitive requires for all arguments to "
                    "be numeric data types"));
    }

    ///////////////////////////////////////////////////////////////////////////
    template <typename T>
    primitive_argument_type expand_dims::add_dim_1d(
        ir::node_data<T>&& arg, std::int64_t axis) const
    {
        auto data = arg.vector();
        if (axis == 0)
        {
            blaze::DynamicMatrix<T> result(1, data.size());
            for (std::size_t i = 0; i != data.size(); ++i)
            {
                result(0, i) = data[i];
            }
            return primitive_argument_type{std::move(result)};
        }

        blaze::DynamicMatrix<T> result(data.size(), 1);
        for (std::size_t i = 0; i != data.size(); ++i)
        {
            result(i, 0) = data[i];
        }
        return primitive_argument_type{std::move(result)};
    }

    primitive_argument_type expand_dims::add_dim_1d(
        primitive_arguments_type&& args) const
    {
        std::int64_t axis = 0;
        if (args.size() == 2)
        {
            axis =
                extract_scalar_integer_value_strict(args[1], name_, codename_);
        }

        if (axis > 1 || axis < -2)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "phylanx::execution_tree::primitives::expand_dims::add_dim_1d",
                generate_error_message(
                    "the axis parameter to expand_dims is out of range"));
        }

        if (axis < 0)
        {
            axis += 2;
        }

        switch (extract_common_type(args[0]))
        {
        case node_data_type_bool:
            return add_dim_1d(extract_boolean_value_strict(
                std::move(args[0]), name_, codename_), axis);

        case node_data_type_int64:
            return add_dim_1d(extract_integer_value_strict(
                std::move(args[0]), name_, codename_), axis);

        case node_data_type_double:
            return add_dim_1d(extract_numeric_value_strict(
                std::move(args[0]), name_, codename_), axis);

        case node_data_type_unknown:
            return add_dim_1d(extract_numeric_value(
                std::move(args[0]), name_, codename_), axis);

        default:
            break;
        }

        HPX_THROW_EXCEPTION(hpx::bad_parameter,
            "phylanx::execution_tree::primitives::expand_dims::add_dim_1d",
            generate_error_message(
                "the expand_dims primitive requires for all arguments to "
                    "be numeric data types"));
    }

#if defined(PHYLANX_HAVE_BLAZE_TENSOR)
    ///////////////////////////////////////////////////////////////////////////
    template <typename T>
    primitive_argument_type expand_dims::add_dim_2d(
        ir::node_data<T>&& arg, std::int64_t axis) const
    {
        auto data = arg.matrix();
        if (axis == 0)
        {
            blaze::DynamicTensor<T> result(1, data.rows(), data.columns());
            blaze::pageslice(result, 0) = data;
            return primitive_argument_type{std::move(result)};
        }
        else if (axis == 1)
        {
            blaze::DynamicTensor<T> result(data.rows(), 1, data.columns());
            blaze::rowslice(result, 0) = data;
            return primitive_argument_type{std::move(result)};
        }

        blaze::DynamicTensor<T> result(data.rows(), data.columns(), 1);
        blaze::columnslice(result, 0) = data;
        return primitive_argument_type{std::move(result)};
    }

    primitive_argument_type expand_dims::add_dim_2d(
        primitive_arguments_type&& args) const
    {
        std::int64_t axis = 0;
        if (args.size() == 2)
        {
            axis =
                extract_scalar_integer_value_strict(args[1], name_, codename_);
        }

        if (axis > 2 || axis < -3)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "phylanx::execution_tree::primitives::expand_dims::add_dim_2d",
                generate_error_message(
                    "the axis parameter to expand_dims is out of range"));
        }

        if (axis < 0)
        {
            axis += 3;
        }

        switch (extract_common_type(args[0]))
        {
        case node_data_type_bool:
            return add_dim_2d(extract_boolean_value_strict(
                std::move(args[0]), name_, codename_), axis);

        case node_data_type_int64:
            return add_dim_2d(extract_integer_value_strict(
                std::move(args[0]), name_, codename_), axis);

        case node_data_type_double:
            return add_dim_2d(extract_numeric_value_strict(
                std::move(args[0]), name_, codename_), axis);

        case node_data_type_unknown:
            return add_dim_2d(extract_numeric_value(
                std::move(args[0]), name_, codename_), axis);

        default:
            break;
        }

        HPX_THROW_EXCEPTION(hpx::bad_parameter,
            "phylanx::execution_tree::primitives::expand_dims::add_dim_2d",
            generate_error_message(
                "the arange primitive requires for all arguments to "
                    "be numeric data types"));
    }
#endif

    ///////////////////////////////////////////////////////////////////////////
    hpx::future<primitive_argument_type> expand_dims::eval(
        primitive_arguments_type const& operands,
        primitive_arguments_type const& args, eval_context ctx) const
    {
        if (operands.size() != 1 && operands.size() != 2)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "expand_dims::eval",
                generate_error_message(
                    "the expand_dims primitive requires exactly one or two "
                    "operands"));
        }

        if (!valid(operands[0]))
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "expand_dims::eval",
                generate_error_message(
                    "the expand_dims primitive requires that the "
                        "arguments given by the operands array are valid"));
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
                    return this_->add_dim_0d(std::move(args));

                case 1:
                    return this_->add_dim_1d(std::move(args));

#if defined(PHYLANX_HAVE_BLAZE_TENSOR)
                case 2:
                    return this_->add_dim_2d(std::move(args));
#endif
                default:
                    HPX_THROW_EXCEPTION(hpx::bad_parameter,
                        "expand_dims::eval",
                        this_->generate_error_message(
                            "operand a has an invalid number of dimensions"));
                }
            }),
            detail::map_operands(
                operands, functional::value_operand{}, args,
                name_, codename_, std::move(ctx)));
    }
}}}
