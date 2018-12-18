// Copyright (c) 2018 Shahrzad Shirzad
// Copyright (c) 2018 Parsa Amini
// Copyright (c) 2018 Hartmut Kaiser
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>
#include <phylanx/execution_tree/primitives/node_data_helpers.hpp>
#include <phylanx/ir/node_data.hpp>
#include <phylanx/plugins/arithmetics/prod_operation.hpp>

#include <hpx/include/lcos.hpp>
#include <hpx/include/naming.hpp>
#include <hpx/include/util.hpp>
#include <hpx/throw_exception.hpp>
#include <hpx/util/optional.hpp>

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include <blaze/Math.h>

///////////////////////////////////////////////////////////////////////////////
namespace phylanx { namespace execution_tree { namespace primitives
{
    ///////////////////////////////////////////////////////////////////////////
    match_pattern_type const prod_operation::match_data = {
        match_pattern_type{"prod",
            std::vector<std::string>{
                "prod(_1)", "prod(_1, _2)", "prod(_1, _2, _3)"},
            &create_prod_operation, &create_primitive<prod_operation>, R"(
            v, a1, a2
            Args:

                v (vector or matrix) : a vector or matrix
                a1 (optional, integer): a axis to prod along
                a2 (optional, integer): a second axis to prod along

            Returns:

            The product of all values along the specified axes.
            )",
            true}};

    ///////////////////////////////////////////////////////////////////////////
    prod_operation::prod_operation(primitive_arguments_type&& operands,
        std::string const& name, std::string const& codename)
      : primitive_component_base(std::move(operands), name, codename)
      , dtype_(extract_dtype(name_))
    {
    }

    template <typename T>
    primitive_argument_type prod_operation::prod0d(ir::node_data<T>&& arg,
        hpx::util::optional<std::int64_t> axis, bool keep_dims) const
    {
        if (axis && axis.value() != 0 && axis.value() != -1)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "prod_operation::prod0d",
                generate_error_message(
                    "the prod_operation primitive requires operand axis to be "
                    "either 0 or -1 for scalar values."));
        }

        return primitive_argument_type{arg.scalar()};
    }

    template <typename T>
    primitive_argument_type prod_operation::prod1d(ir::node_data<T>&& arg,
        hpx::util::optional<std::int64_t> axis, bool keep_dims) const
    {
        if (axis && axis.value() != 0 && axis.value() != -1)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "prod_operation::prod1d",
                generate_error_message(
                    "the prod_operation primitive requires operand axis to be "
                    "either 0 or -1 for vectors."));
        }

        auto v = arg.vector();
        T result =
            std::accumulate(v.begin(), v.end(), (T) 1, std::multiplies<T>());

        if (keep_dims)
        {
            return primitive_argument_type{blaze::DynamicVector<T>{result}};
        }
        return primitive_argument_type{result};
    }

    template <typename T>
    primitive_argument_type prod_operation::prod2d(ir::node_data<T>&& arg,
        hpx::util::optional<std::int64_t> axis, bool keep_dims) const
    {
        if (axis)
        {
            switch (axis.value())
            {
            case -2:
                HPX_FALLTHROUGH;
            case 0:
                return prod2d_axis0(std::move(arg), keep_dims);

            case -1:
                HPX_FALLTHROUGH;
            case 1:
                return prod2d_axis1(std::move(arg), keep_dims);

            default:
                HPX_THROW_EXCEPTION(hpx::bad_parameter,
                    "prod_operation::prod1d",
                    generate_error_message(
                        "the prod_operation primitive requires operand axis "
                        "to be between -2 and 1 for matrices."));
            }
        }
        return prod2d_flat(std::move(arg), keep_dims);
    }

    template <typename T>
    primitive_argument_type prod_operation::prod2d_flat(
        ir::node_data<T>&& arg, bool keep_dims) const
    {
        auto m = arg.matrix();
        T result = 1;
        for (std::size_t i = 0; i < m.columns(); ++i)
        {
            auto col = blaze::column(m, i);
            result *= std::accumulate(
                col.begin(), col.end(), T(1), std::multiplies<T>());
        }

        if (keep_dims)
        {
            return primitive_argument_type{blaze::DynamicMatrix<T>{{result}}};
        }
        return primitive_argument_type{result};
    }

    template <typename T>
    primitive_argument_type prod_operation::prod2d_axis0(
        ir::node_data<T>&& arg, bool keep_dims) const
    {
        auto m = arg.matrix();
        blaze::DynamicVector<T> result(m.columns());
        for (std::size_t i = 0; i < m.columns(); ++i)
        {
            auto col = blaze::column(m, i);
            result[i] = std::accumulate(
                col.begin(), col.end(), T(1), std::multiplies<T>());
        }

        if (keep_dims)
        {
            return primitive_argument_type{
                blaze::DynamicMatrix<T>{1, result.size(), result.data()}};
        }
        return primitive_argument_type{result};
    }

    template <typename T>
    primitive_argument_type prod_operation::prod2d_axis1(
        ir::node_data<T>&& arg, bool keep_dims) const
    {
        auto m = arg.matrix();
        blaze::DynamicVector<T> result(m.rows());
        for (std::size_t i = 0; i < m.rows(); ++i)
        {
            auto col = blaze::row(m, i);
            result[i] = std::accumulate(
                col.begin(), col.end(), T(1), std::multiplies<T>());
        }

        if (keep_dims)
        {
            return primitive_argument_type{
                blaze::DynamicMatrix<T>{1, result.size(), result.data()}};
        }
        return primitive_argument_type{result};
    }

    template <typename T>
    primitive_argument_type prod_operation::prodnd(ir::node_data<T>&& arg,
        hpx::util::optional<std::int64_t> axis, bool keep_dims) const
    {
        std::size_t a_dims = arg.num_dimensions();
        switch (a_dims)
        {
        case 0:
            return prod0d(std::move(arg), axis, keep_dims);

        case 1:
            return prod1d(std::move(arg), axis, keep_dims);

        case 2:
            return prod2d(std::move(arg), axis, keep_dims);

        default:
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "prod_operation::eval",
                generate_error_message(
                    "operand a has an invalid number of dimensions"));
        }
    }

    ///////////////////////////////////////////////////////////////////////////
    hpx::future<primitive_argument_type> prod_operation::eval(
        primitive_arguments_type const& operands,
        primitive_arguments_type const& args, eval_context ctx) const
    {
        if (operands.empty() || operands.size() > 3)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "prod_operation::eval",
                generate_error_message("the prod_operation primitive requires "
                                       "exactly one, two, or "
                                       "three operands"));
        }

        for (auto const& i : operands)
        {
            if (!valid(i))
            {
                HPX_THROW_EXCEPTION(hpx::bad_parameter,
                    "prod_operation::eval",
                    generate_error_message(
                        "the prod_operation primitive requires that the "
                        "arguments given by the operands array are valid"));
            }
        }

        auto this_ = this->shared_from_this();
        return hpx::dataflow(hpx::launch::sync,
            hpx::util::unwrapping(
                [this_ = std::move(this_)](primitive_arguments_type&& args)
                    -> primitive_argument_type {
                    // Extract axis and keep_dims
                    // Presence of axis changes behavior for >1d cases
                    hpx::util::optional<std::int64_t> axis;
                    bool keep_dims = false;

                    // axis is argument #2
                    if (args.size() > 1)
                    {
                        if (valid(args[1]))
                        {
                            axis = extract_scalar_integer_value(
                                args[1], this_->name_, this_->codename_);
                        }

                        // keep_dims is argument #3
                        if (args.size() == 3)
                        {
                            keep_dims = extract_scalar_boolean_value(
                                args[2], this_->name_, this_->codename_);
                        }
                    }

                    node_data_type t = this_->dtype_;
                    if (t == node_data_type_unknown)
                    {
                        t = extract_common_type(args[0]);
                    }

                    switch (t)
                    {
                    case node_data_type_bool:
                        return this_->prodnd(
                            extract_boolean_value(std::move(args[0]),
                                this_->name_, this_->codename_),
                            axis, keep_dims);
                    case node_data_type_int64:
                        return this_->prodnd(
                            extract_integer_value(std::move(args[0]),
                                this_->name_, this_->codename_),
                            axis, keep_dims);
                    case node_data_type_double:
                        return this_->prodnd(
                            extract_numeric_value(std::move(args[0]),
                                this_->name_, this_->codename_),
                            axis, keep_dims);
                    default:
                        break;
                    }

                    HPX_THROW_EXCEPTION(hpx::bad_parameter,
                        "prod::eval",
                        this_->generate_error_message(
                            "the prod primitive requires for all arguments "
                            "to be numeric data types"));
                }),
            detail::map_operands(operands, functional::value_operand{}, args,
                name_, codename_, std::move(ctx)));
    }
}}
}
