//  Copyright (c) 2017-2018 Bibek Wagle
//  Copyright (c) 2017-2018 Hartmut Kaiser
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>
#include <phylanx/ir/node_data.hpp>
#include <phylanx/plugins/matrixops/column_slicing.hpp>

#include <hpx/include/lcos.hpp>
#include <hpx/include/naming.hpp>
#include <hpx/include/util.hpp>
#include <hpx/throw_exception.hpp>

#include <cstddef>
#include <cstdint>
#include <memory>
#include <numeric>
#include <string>
#include <utility>
#include <vector>

#include <blaze/Math.h>
#include <blaze/math/views/Elements.h>

///////////////////////////////////////////////////////////////////////////////
namespace phylanx { namespace execution_tree { namespace primitives
{
    ///////////////////////////////////////////////////////////////////////////
    match_pattern_type const column_slicing_operation::match_data =
    {
        hpx::util::make_tuple("slice_column",
            std::vector<std::string>{"slice_column(_1, _2)"},
            &create_column_slicing_operation,
            &create_primitive<column_slicing_operation>)
    };

    ///////////////////////////////////////////////////////////////////////////
    column_slicing_operation::column_slicing_operation(
            std::vector<primitive_argument_type>&& operands,
            std::string const& name, std::string const& codename)
      : primitive_component_base(std::move(operands), name, codename)
    {}

    ///////////////////////////////////////////////////////////////////////////

    std::vector<std::int64_t> column_slicing_operation::create_list(
        std::int64_t start, std::int64_t stop, std::int64_t step,
        std::size_t array_length) const
    {
        std::int64_t actual_start = 0;
        std::int64_t actual_stop = 0;

        if (start >= 0)
        {
            actual_start = start;
        }

        if (start < 0)
        {
            actual_start = array_length + start;
        }

        if (stop >= 0)
        {
            actual_stop = stop;
        }

        if (stop < 0)
        {
            actual_stop = array_length + stop;
        }

        std::vector<std::int64_t> result;

        if (step > 0)
        {
            for (std::int64_t i = actual_start; i < actual_stop; i += step)
            {
                result.push_back(i);
            }
        }

        if (step < 0)
        {
            for (std::int64_t i = actual_start; i > actual_stop; i += step)
            {
                result.push_back(i);
            }
        }

        if (result.empty())
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "phylanx::execution_tree::primitives::"
                "column_slicing_operation::create_list",
                execution_tree::generate_error_message(
                    "column slicing will produce empty result, "
                    "please check your parameters",
                    name_, codename_));
        }

        return result;
    }

    primitive_argument_type column_slicing_operation::column_slicing0d(
        arg_type&& arg) const
    {
        double scalar_data = arg.scalar();
        return primitive_argument_type{ir::node_data<double>{scalar_data}};
    }

    primitive_argument_type column_slicing_operation::column_slicing1d(
        arg_type&& arg, std::vector<double> extracted) const
    {
        if (extracted.empty())
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "phylanx::execution_tree::primitives::"
                "column_slicing_operation::column_slicing1d",
                execution_tree::generate_error_message(
                    "column can not be empty", name_, codename_));
        }

        auto input_vector = arg.vector();

        //return a value and not a vector if you are not given a list
        if (extracted.size() == 1)
        {
            std::int64_t index = extracted[0];
            if (index < 0)
            {
                index = input_vector.size() + index;
            }
            return primitive_argument_type{input_vector[index]};
        }

        std::int64_t col_start = extracted[0];
        std::int64_t col_stop = extracted[1];
        std::int64_t step = 1;

        if (extracted.size() == 3)
        {
            step = extracted[2];

            if (step == 0)
            {
                HPX_THROW_EXCEPTION(hpx::bad_parameter,
                    "phylanx::execution_tree::primitives::"
                    "column_slicing_operation::column_slicing1d",
                    execution_tree::generate_error_message(
                        "step can not be zero", name_, codename_));
            }
        }

        auto init_list = create_list(col_start, col_stop, step, arg.size());

        auto sv = blaze::elements(input_vector, init_list);

        storage1d_type v{sv};
        return primitive_argument_type{ir::node_data<double>(std::move(v))};
        }

    primitive_argument_type column_slicing_operation::column_slicing2d(
        arg_type&& arg , std::vector<double> extracted) const
    {
        if (extracted.empty())
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "phylanx::execution_tree::primitives::"
                "column_slicing_operation::column_slicing2d",
                execution_tree::generate_error_message(
                    "column can not be empty", name_, codename_));
    }

        auto input_matrix = arg.matrix();
        std::size_t num_matrix_rows = input_matrix.rows();
        std::size_t num_matrix_cols = input_matrix.columns();

        //return a value and not a vector if you are not given a list
        if (extracted.size() == 1)
    {
            std::int64_t index = extracted[0];
            if (index < 0)
            {
                index = num_matrix_rows + index;
            }
            auto sv = blaze::column(input_matrix, index);
            storage1d_type v{sv};
            return primitive_argument_type{ir::node_data<double>{std::move(v)}};
        }

        std::int64_t column_start = extracted[0];
        std::int64_t column_stop = extracted[1];

        std::int64_t step = 1;

        if (extracted.size() == 3)
        {
            step = extracted[2];
            if (step == 0)
            {
                HPX_THROW_EXCEPTION(hpx::bad_parameter,
                    "phylanx::execution_tree::primitives::"
                    "column_slicing_operation::column_slicing2d",
                    execution_tree::generate_error_message(
                        "step can not be zero", name_, codename_));
            }
        }

        auto init_list =
            create_list(column_start, column_stop, step, num_matrix_cols);

        auto sm = blaze::columns(input_matrix, init_list);

        storage2d_type m{sm};

        return primitive_argument_type{ir::node_data<double>(std::move(m))};
        }

    hpx::future<primitive_argument_type> column_slicing_operation::eval(
        std::vector<primitive_argument_type> const& operands,
        std::vector<primitive_argument_type> const& args) const
    {
        if (operands.size() > 2)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "phylanx::execution_tree::primitives::"
                "column_slicing_operation::column_slicing_operation",
                execution_tree::generate_error_message(
                    "the column_slicing_operation primitive requires "
                    "either one or two arguments",
                    name_, codename_));
        }

        for (std::size_t i = 0; i != operands.size(); ++i)
        {
            if (!valid(operands[i]))
            {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "column_slicing_operation::eval",
                execution_tree::generate_error_message(
                    "the column_slicing_operation primitive requires "
                    "that the arguments given by the operands "
                    "array are valid",
                    name_, codename_));
        }
        }

        auto this_ = this->shared_from_this();
        return hpx::dataflow(hpx::launch::sync,
            hpx::util::unwrapping([this_](std::vector<primitive_argument_type>&&
                                          args) -> primitive_argument_type {
                //Extract the matrix i.e the first argument
                arg_type matrix_input = execution_tree::extract_numeric_value(
                    args[0], this_->name_, this_->codename_);

                std::vector<double> extracted;

                //Extract the list or the single double
                if (args.size() == 2)
                {
                    if (execution_tree::is_list_operand_strict(args[1]))
                    {
                        auto result = execution_tree::extract_list_value(
                            args[1], this_->name_, this_->codename_);
                        for (auto a : result)
                        {
                            extracted.push_back(
                                execution_tree::extract_numeric_value(a)[0]);
                        }
                    }
                    else
                    {
                        double result = execution_tree::extract_numeric_value(
                            args[1], this_->name_, this_->codename_)[0];
                        extracted.push_back(result);
                    }
                }
                std::size_t matrix_dims = matrix_input.num_dimensions();

                switch (matrix_dims)
                {
                case 0:
                    return this_->column_slicing0d(std::move(matrix_input));

                case 1:
                    return this_->column_slicing1d(
                        std::move(matrix_input), extracted);

                case 2:
                    return this_->column_slicing2d(
                        std::move(matrix_input), extracted);

                default:
                    HPX_THROW_EXCEPTION(hpx::bad_parameter,
                        "column_slicing_operation::eval",
                        execution_tree::generate_error_message(
                            "left hand side operand has unsupported "
                            "number of dimensions",
                            this_->name_, this_->codename_));
                }
            }),
            detail::map_operands(
                operands, functional::value_operand{}, args, name_, codename_));
    }

    hpx::future<primitive_argument_type> column_slicing_operation::eval(
        std::vector<primitive_argument_type> const& args) const
    {
        if (operands_.empty())
        {
            return eval(args, noargs);
        }
        return eval(operands_, args);
    }
}}}
