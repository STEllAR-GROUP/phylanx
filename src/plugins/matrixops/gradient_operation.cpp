// Copyright (c) 2018 Bibek Wagle
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>
#include <phylanx/ir/node_data.hpp>
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
            &create_primitive<gradient_operation>,
            "m,axis\n"
            "Args:\n"
            "\n"
            "    m (vector or matrix) : values to take the gradient of\n"
            "    axis (optional, integer) : the axis along whic to take\n"
            "                    the gradient.\n"
            "\n"
            "Returns:\n"
            "\n"
            "The numerical gradient, i.e., differences of adjacent values "
            "along the specified axis."
            )
    };

    ///////////////////////////////////////////////////////////////////////////
    gradient_operation::gradient_operation(
            primitive_arguments_type&& operands,
            std::string const& name, std::string const& codename)
      : primitive_component_base(std::move(operands), name, codename)
    {}

    ///////////////////////////////////////////////////////////////////////////
    using arg_type = ir::node_data<double>;
    using args_type = std::vector<arg_type, arguments_allocator<arg_type>>;
    using storage1d_type = typename arg_type::storage1d_type;
    using storage2d_type = typename arg_type::storage2d_type;

    primitive_argument_type gradient_operation::gradient0d(
        args_type&& args) const
    {
        HPX_THROW_EXCEPTION(hpx::bad_parameter,
            "gradient_operation::gradient0d",
            util::generate_error_message(
                "gradient operation is not supported on 0d input", this->name_,
                this->codename_));
    }

    primitive_argument_type gradient_operation::gradient1d(
        args_type&& args) const
    {
        blaze::DynamicVector<double> input = args[0].vector();
        std::size_t length = input.size();
        blaze::DynamicVector<double> gradient(length);

        for (std::size_t i = 0; i < length; i++)
        {
            if (i == 0)
            {
                gradient[i] = input[i + 1] - input[i];
            }
            else if (i == length - 1)
            {
                gradient[i] = input[i] - input[i - 1];
            }
            else
            {
                gradient[i] = (input[i + 1] - input[i - 1]) / 2;
            }
        }

        return primitive_argument_type{
            ir::node_data<double>{storage1d_type{std::move(gradient)}}};
    }

    primitive_argument_type gradient_operation::gradient2d(
        args_type&& args) const
    {
        blaze::DynamicMatrix<double> input = args[0].matrix();
        std::size_t num_rows = input.rows();
        std::size_t num_cols = input.columns();
        std::size_t args_size = args.size();

        blaze::DynamicMatrix<double> gradient(num_rows, num_cols);

        if (args_size == 2)
        {
            std::int64_t axis = args[1].scalar();

            if (axis == 0)
            {
                for (std::size_t i = 0; i < num_cols; i++)
                {
                    auto temp = blaze::column(input, i);
                    for (std::size_t j = 0; j < num_rows; j++)
                    {
                        if (j == 0)
                        {
                            gradient(j, i) = temp[j + 1] - temp[j];
                        }
                        else if (j == num_rows - 1)
                        {
                            gradient(j, i) = temp[j] - temp[j - 1];
                        }
                        else
                        {
                            gradient(j, i) = (temp[j + 1] - temp[j - 1]) / 2;
                        }
                    }
                }
            }
            else    //axis = 1
            {
                for (std::size_t i = 0; i < num_rows; i++)
                {
                    auto temp = blaze::row(input, i);
                    for (std::size_t j = 0; j < num_cols; j++)
                    {
                        if (j == 0)
                        {
                            gradient(i, j) = temp[j + 1] - temp[j];
                        }
                        else if (j == num_rows - 1)
                        {
                            gradient(i, j) = temp[j] - temp[j - 1];
                        }
                        else
                        {
                            gradient(i, j) = (temp[j + 1] - temp[j - 1]) / 2;
                        }
                    }
                }
            }
            return primitive_argument_type{
                ir::node_data<double>{storage2d_type{std::move(gradient)}}};
        }
        else    // args_size = 1
        {
            blaze::DynamicMatrix<double> gradient_1(num_rows, num_cols);
            for (std::size_t i = 0; i < num_cols; i++)
            {
                auto temp = blaze::column(input, i);
                auto temp_1 = blaze::row(input, i);
                for (std::size_t j = 0; j < num_rows; j++)
                {
                    if (j == 0)
                    {
                        gradient(j, i) = temp[j + 1] - temp[j];
                        gradient_1(i, j) = temp_1[j + 1] - temp_1[j];
                    }
                    else if (j == num_rows - 1)
                    {
                        gradient(j, i) = temp[j] - temp[j - 1];
                        gradient_1(i, j) = temp_1[j] - temp_1[j - 1];
                    }
                    else
                    {
                        gradient(j, i) = (temp[j + 1] - temp[j - 1]) / 2;
                        gradient_1(i, j) = (temp_1[j + 1] - temp_1[j - 1]) / 2;
                    }
                }
            }
            return primitive_argument_type{primitive_arguments_type{
                primitive_argument_type{
                    ir::node_data<double>{storage2d_type{std::move(gradient)}}},
                primitive_argument_type{ir::node_data<double>{
                    storage2d_type{std::move(gradient_1)}}}}};
        }
    }

    hpx::future<primitive_argument_type> gradient_operation::eval(
        primitive_arguments_type const& operands,
        primitive_arguments_type const& args) const
    {
        if (operands.empty() || operands.size() > 2)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "phylanx::execution_tree::primitives::"
                "gradient_operation::gradient_operation",
                util::generate_error_message(
                    "the gradient_operation primitive requires "
                    "either one or two arguments",
                    name_, codename_));
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
                util::generate_error_message(
                    "the gradient_operation primitive requires "
                    "that the arguments given by the operands "
                    "array are valid",
                    name_, codename_));
        }

        auto this_ = this->shared_from_this();
        return hpx::dataflow(hpx::launch::sync,
            hpx::util::unwrapping(
                [this_](args_type&& args) -> primitive_argument_type {
                    std::size_t matrix_dims = args[0].num_dimensions();
                    switch (matrix_dims)
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
                            util::generate_error_message(
                                "left hand side operand has unsupported "
                                    "number of dimensions",
                                this_->name_, this_->codename_));
                    }
                }),
            detail::map_operands(
                operands, functional::numeric_operand{}, args,
                name_, codename_));
    }

    hpx::future<primitive_argument_type> gradient_operation::eval(
        primitive_arguments_type const& args) const
    {
        if (this->no_operands())
        {
            return eval(args, noargs);
        }
        return eval(this->operands(), args);
    }
}}}
