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
            )",
            true)
    };

    ///////////////////////////////////////////////////////////////////////////
    expand_dims::expand_dims(primitive_arguments_type&& operands,
            std::string const& name, std::string const& codename)
      : primitive_component_base(std::move(operands), name, codename)
      , dtype_(extract_dtype(name_))
    {}

    ///////////////////////////////////////////////////////////////////////////
    template <typename T>
    primitive_argument_type expand_dims::add_dim_0d(
        ir::node_data<T>&& arg) const
    {
        return primitive_argument_type{blaze::DynamicVector<T>{arg.scalar()}};
    }

    primitive_argument_type expand_dims::add_dim_0d(
        primitive_argument_type&& arg) const
    {
        node_data_type t = dtype_;
        if (t == node_data_type_unknown)
        {
            t = extract_common_type(arg);
        }

        switch (t)
        {
        case node_data_type_bool:
            return add_dim_0d(extract_boolean_value(
                std::move(arg), name_, codename_));

        case node_data_type_int64:
            return add_dim_0d(extract_integer_value(
                std::move(arg), name_, codename_));

        case node_data_type_unknown: HPX_FALLTHROUGH;
        case node_data_type_double:
            return add_dim_0d(extract_numeric_value(
                std::move(arg), name_, codename_));

        default:
            break;
        }

        HPX_THROW_EXCEPTION(hpx::bad_parameter,
            "phylanx::execution_tree::primitives::expand_dims::add_dim_0d",
            generate_error_message(
                "the arange primitive requires for all arguments to "
                    "be numeric data types"));
    }

    ///////////////////////////////////////////////////////////////////////////
    template <typename T>
    primitive_argument_type expand_dims::add_dim_1d(
        ir::node_data<T>&& arg) const
    {
        auto data = arg.vector();
        return primitive_argument_type{
            blaze::DynamicMatrix<T>{data.size(), 1, data.data()}};
    }

    primitive_argument_type expand_dims::add_dim_1d(
        primitive_argument_type&& arg) const
    {
        node_data_type t = dtype_;
        if (t == node_data_type_unknown)
        {
            t = extract_common_type(arg);
        }

        switch (t)
        {
        case node_data_type_bool:
            return add_dim_1d(extract_boolean_value(
                std::move(arg), name_, codename_));

        case node_data_type_int64:
            return add_dim_1d(extract_integer_value(
                std::move(arg), name_, codename_));

        case node_data_type_unknown: HPX_FALLTHROUGH;
        case node_data_type_double:
            return add_dim_1d(extract_numeric_value(
                std::move(arg), name_, codename_));

        default:
            break;
        }

        HPX_THROW_EXCEPTION(hpx::bad_parameter,
            "phylanx::execution_tree::primitives::expand_dims::add_dim_1d",
            generate_error_message(
                "the arange primitive requires for all arguments to "
                    "be numeric data types"));
    }

    ///////////////////////////////////////////////////////////////////////////
    hpx::future<primitive_argument_type> expand_dims::eval(
        primitive_arguments_type const& operands,
        primitive_arguments_type const& args, eval_context ctx) const
    {
        if (operands.size() != 1)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "expand_dims::eval",
                generate_error_message(
                    "the expand_dims primitive requires exactly one operand"));
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
                    return this_->add_dim_0d(std::move(args[0]));

                case 1:
                    return this_->add_dim_1d(std::move(args[0]));

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
