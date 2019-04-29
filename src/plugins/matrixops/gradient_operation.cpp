// Copyright (c) 2018 Bibek Wagle
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>
#include <phylanx/ir/node_data.hpp>
#include <phylanx/execution_tree/primitives/node_data_helpers.hpp>
#include <phylanx/plugins/matrixops/gradient_operation.hpp>

#include <hpx/include/lcos.hpp>
#include <hpx/include/naming.hpp>
#include <hpx/include/util.hpp>
#include <hpx/throw_exception.hpp>

#include <cstddef>
#include <cstdint>
#include <cmath>
#include <memory>
#include <numeric>
#include <string>
#include <utility>
#include <vector>

#include <blaze/Math.h>

///////////////////////////////////////////////////////////////////////////////
namespace phylanx { namespace execution_tree { namespace primitives
{
    ///////////////////////////////////////////////////////////////////////////
    match_pattern_type const gradient_operation::match_data =
    {
        hpx::util::make_tuple("gradient",
            std::vector<std::string>{"gradient(_1, _2)", "gradient(_1)"},
            &create_gradient_operation,
            &create_primitive<gradient_operation>, R"(
            m, axis
            Args:

                m (vector or matrix) : values to take the gradient of
                axis (optional, integer) : the axis along which to take the gradient

            Returns:

            The numerical gradient, i.e., differences of adjacent values
            along the specified axis.)")
    };

    ///////////////////////////////////////////////////////////////////////////
    gradient_operation::gradient_operation(
            primitive_arguments_type&& operands,
            std::string const& name, std::string const& codename)
      : primitive_component_base(std::move(operands), name, codename)
    {}

    ///////////////////////////////////////////////////////////////////////////
    template <typename T>
    primitive_argument_type gradient_operation::gradient1d(
        ir::node_data<T>&& arg) const
    {
        auto input = arg.vector();
        std::size_t last = input.size() - 1;
        blaze::DynamicVector<T> gradient(last + 1);

        gradient[0] = input[1] - input[0];
        for (std::size_t i = 1; i != last; i++)
        {
            gradient[i] = (input[i + 1] - input[i - 1]) / 2;
        }
        gradient[last] = input[last] - input[last - 1];

        return primitive_argument_type{ir::node_data<T>{std::move(gradient)}};
    }

    template <typename T>
    primitive_argument_type gradient_operation::gradient2d(
        ir::node_data<T>&& arg, std::int64_t axis) const
    {
        auto input = arg.matrix();
        std::size_t num_rows = input.rows();
        std::size_t num_cols = input.columns();

        blaze::DynamicMatrix<T> gradient(num_rows, num_cols);

        if (axis >= 0)
        {
            if (axis == 0)
            {
                auto last = num_rows - 1;
                for (std::size_t i = 0; i != num_cols; i++)
                {
                    auto temp = blaze::column(input, i);
                    gradient(0, i) = temp[1] - temp[0];
                    for (std::size_t j = 1; j != last; j++)
                    {
                        gradient(j, i) = (temp[j + 1] - temp[j - 1]) / 2;
                    }
                    gradient(last, i) = temp[last] - temp[last - 1];
                }
            }
            else    // axis = 1
            {
                auto last = num_cols - 1;
                for (std::size_t i = 0; i != num_rows; i++)
                {
                    auto temp = blaze::row(input, i);
                    gradient(i, 0) = temp[1] - temp[0];
                    for (std::size_t j = 1; j != last; j++)
                    {
                        gradient(i, j) = (temp[j + 1] - temp[j - 1]) / 2;
                    }
                    gradient(i, last) = temp[last] - temp[last - 1];
                }
            }

            return primitive_argument_type{
                ir::node_data<T>{std::move(gradient)}};
        }

        // args_size = 1
        blaze::DynamicMatrix<T> gradient_1(num_rows, num_cols);
        auto last = num_rows - 1;
        for (std::size_t i = 0; i != num_cols; i++)
        {
            auto temp = blaze::column(input, i);
            auto temp_1 = blaze::row(input, i);

            gradient(0, i) = temp[1] - temp[0];
            gradient_1(i, 0) = temp_1[1] - temp_1[0];

            for (std::size_t j = 1; j != last; j++)
            {
                gradient(j, i) = (temp[j + 1] - temp[j - 1]) / 2;
                gradient_1(i, j) = (temp_1[j + 1] - temp_1[j - 1]) / 2;
            }

            gradient(last, i) = temp[last] - temp[last - 1];
            gradient_1(i, last) = temp_1[last] - temp_1[last - 1];
        }

        return primitive_argument_type{primitive_arguments_type{
            primitive_argument_type{ir::node_data<T>{std::move(gradient)}},
            primitive_argument_type{ir::node_data<T>{std::move(gradient_1)}}
        }};
    }

    primitive_argument_type gradient_operation::gradient0d(
        primitive_arguments_type&& args) const
    {
        HPX_THROW_EXCEPTION(hpx::bad_parameter,
            "gradient_operation::gradient0d",
            generate_error_message(
                "gradient operation is not supported on 0d input"));
    }

    primitive_argument_type gradient_operation::gradient1d(
        primitive_arguments_type&& args) const
    {
        switch (extract_common_type(args[0]))
        {
        case node_data_type_bool:
            return gradient1d(
                extract_boolean_value(std::move(args[0]), name_, codename_));

        case node_data_type_int64:
            return gradient1d(
                extract_integer_value(std::move(args[0]), name_, codename_));

        case node_data_type_double:
            return gradient1d(extract_numeric_value_strict(
                std::move(args[0]), name_, codename_));

        case node_data_type_unknown:
            return gradient1d(
                extract_numeric_value(std::move(args[0]), name_, codename_));

        default:
            break;
        }

        HPX_THROW_EXCEPTION(hpx::bad_parameter,
            "gradient_operation::gradient1d",
            generate_error_message(
                "the gradient primitive requires for all arguments to "
                    "be numeric data types"));
    }

    primitive_argument_type gradient_operation::gradient2d(
        primitive_arguments_type&& args) const
    {
        std::int64_t axis = -1;
        if (args.size() == 2)
        {
            axis = extract_scalar_integer_value(args[1], name_, codename_);
        }

        switch (extract_common_type(args[0]))
        {
        case node_data_type_bool:
            return gradient2d(
                extract_boolean_value(std::move(args[0]), name_, codename_),
                axis);

        case node_data_type_int64:
            return gradient2d(
                extract_integer_value(std::move(args[0]), name_, codename_),
                axis);

        case node_data_type_double:
            return gradient2d(extract_numeric_value_strict(
                std::move(args[0]), name_, codename_), axis);

        case node_data_type_unknown:
            return gradient2d(
                extract_numeric_value(std::move(args[0]), name_, codename_),
                axis);

        default:
            break;
        }

        HPX_THROW_EXCEPTION(hpx::bad_parameter,
            "gradient_operation::gradient2d",
            generate_error_message(
                "the gradient primitive requires for all arguments to "
                    "be numeric data types"));
    }

    ///////////////////////////////////////////////////////////////////////////
    hpx::future<primitive_argument_type> gradient_operation::eval(
        primitive_arguments_type const& operands,
        primitive_arguments_type const& args, eval_context ctx) const
    {
        if (operands.empty() || operands.size() > 2)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "phylanx::execution_tree::primitives::"
                "gradient_operation::gradient_operation",
                generate_error_message(
                    "the gradient_operation primitive requires "
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
                "gradient_operation::eval",
                generate_error_message(
                    "the gradient_operation primitive requires that the "
                    "arguments given by the operands array are valid"));
        }

        auto this_ = this->shared_from_this();
        return hpx::dataflow(hpx::launch::sync, hpx::util::unwrapping(
            [this_ = std::move(this_)](primitive_arguments_type&& args)
            -> primitive_argument_type
            {
                switch (extract_numeric_value_dimension(
                    args[0], this_->name_, this_->codename_))
                {
                case 0:
                    return this_->gradient0d(std::move(args));

                case 1:
                    return this_->gradient1d(std::move(args));

                case 2:
                    return this_->gradient2d(std::move(args));

                default:
                    HPX_THROW_EXCEPTION(hpx::bad_parameter,
                        "gradient_operation::eval",
                        this_->generate_error_message(
                            "left hand side operand has unsupported "
                                "number of dimensions"));
                }
            }),
            detail::map_operands(operands, functional::value_operand{},
                args, name_, codename_, std::move(ctx)));
    }
}}}
