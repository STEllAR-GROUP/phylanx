// Copyright (c) 2017-2018 Bibek Wagle
// Copyright (c) 2017-2018 Hartmut Kaiser
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>
#include <phylanx/execution_tree/primitives/row_slicing.hpp>
#include <phylanx/ir/node_data.hpp>

#include <hpx/include/lcos.hpp>
#include <hpx/include/naming.hpp>
#include <hpx/include/util.hpp>
#include <hpx/throw_exception.hpp>

#include <cstddef>
#include <memory>
#include <numeric>
#include <string>
#include <utility>
#include <vector>

#include <blaze/Math.h>
#include <blaze/math/Elements.h>

///////////////////////////////////////////////////////////////////////////////
namespace phylanx { namespace execution_tree { namespace primitives
{
    ///////////////////////////////////////////////////////////////////////////
    primitive create_row_slicing_operation(hpx::id_type const& locality,
        std::vector<primitive_argument_type>&& operands,
            std::string const& name, std::string const& codename)
    {
        static std::string type("slice_row");
        return create_primitive_component(
            locality, type, std::move(operands), name, codename);
    }

    match_pattern_type const row_slicing_operation::match_data =
    {
        hpx::util::make_tuple("slice_row",
            std::vector<std::string>{"slice_row(_1, _2)"},
            &create_row_slicing_operation,
            &create_primitive<row_slicing_operation>)
    };

    ///////////////////////////////////////////////////////////////////////////
    row_slicing_operation::row_slicing_operation(
            std::vector<primitive_argument_type>&& operands,
            std::string const& name, std::string const& codename)
      : primitive_component_base(std::move(operands), name, codename)
    {}

    ///////////////////////////////////////////////////////////////////////////
    std::vector<int> row_slicing_operation::create_list_row(
        int start, int stop, int step, int array_length) const
    {
        auto actual_start = 0;
        auto actual_stop = 0;

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

        std::vector<int> result;

        if (step > 0)
        {
            for (int i = actual_start; i < actual_stop; i += step)
            {
                result.push_back(i);
            }
        }

        if (step < 0)
        {
            for (int i = actual_start; i > actual_stop; i += step)
            {
                result.push_back(i);
            }
        }

        if (result.empty())
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "phylanx::execution_tree::primitives::"
                    "row_slicing_operation::create_list_row",
                execution_tree::generate_error_message(
                    "slicing will produce empty result, please "
                        "check your parameters",
                    name_, codename_));
        }
        return result;
    }

    primitive_argument_type row_slicing_operation::row_slicing0d(
        arg_type&& arg, std::vector<double> extracted) const
    {
        auto scalar_data = arg.scalar();
        return primitive_argument_type{ir::node_data<double>{scalar_data}};
    }

    primitive_argument_type row_slicing_operation::row_slicing1d(
        arg_type&& arg, std::vector<double> extracted) const
    {
        auto input_vector = arg.vector();

        //return a value and not a vector if you are not given a list
        if (extracted.size() == 1)
        {
            double index = extracted[0];
            if (index < 0)
            {
                index = input_vector.size() + index;
            }
            return primitive_argument_type{input_vector[index]};
        }

        auto row_start = extracted[0];
        auto row_stop = extracted[1];
        int step = 1;

        if (extracted.size() == 3)
        {
            step = extracted[2];

            if (step == 0)
            {
                HPX_THROW_EXCEPTION(hpx::bad_parameter,
                    "phylanx::execution_tree::primitives::"
                    "row_slicing_operation::row_slicing_operation",
                    execution_tree::generate_error_message(
                        "step can not be zero", name_, codename_));
            }
        }

        auto init_list = create_list_row(row_start, row_stop, step, arg.size());

        auto sv = blaze::elements(input_vector, init_list);

        storage1d_type v{sv};
        return primitive_argument_type{ir::node_data<double>(std::move(v))};
    }

    primitive_argument_type row_slicing_operation::row_slicing2d(
        arg_type&& arg, std::vector<double> extracted) const
    {
        auto input_matrix = arg.matrix();
        auto num_matrix_rows = input_matrix.rows();
        auto num_matrix_cols = input_matrix.columns();

        //return a value and not a vector if you are not given a list
        if (extracted.size() == 1)
        {
            double index = extracted[0];
            if (index < 0)
            {
                index = num_matrix_rows + index;
            }
            auto sv = blaze::trans(blaze::row(input_matrix, index));
            storage1d_type v{sv};
            return primitive_argument_type{ir::node_data<double>{std::move(v)}};
        }

         auto row_start = extracted[0];
         auto row_stop = extracted[1];

         int step = 1;

         if (extracted.size() == 3)
         {
             step = extracted[2];
             if (step == 0)
             {
                 HPX_THROW_EXCEPTION(hpx::bad_parameter,
                     "phylanx::execution_tree::primitives::"
                         "row_slicing_operation::row_slicing_operation",
                     execution_tree::generate_error_message(
                         "step can not be zero",
                         name_, codename_));
             }
         }

         auto init_list =
             create_list_row(row_start, row_stop, step, num_matrix_rows);

         auto sm = blaze::rows(input_matrix, init_list);

         storage2d_type m{sm};

         return primitive_argument_type{
             ir::node_data<double>(std::move(m))};
    }

    hpx::future<primitive_argument_type> row_slicing_operation::eval(
        std::vector<primitive_argument_type> const& operands,
        std::vector<primitive_argument_type> const& args) const
    {
        if (operands.size() != 2)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "phylanx::execution_tree::primitives::"
                    "row_slicing_operation::row_slicing_operation",
                execution_tree::generate_error_message(
                    "the row_slicing_operation primitive requires "
                        "exactly two arguments",
                    name_, codename_));
        }

        for (std::size_t i = 0; i != operands.size(); ++i)
        {
            if (!valid(operands[i]))
            {
                HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "row_slicing_operation::eval",
                execution_tree::generate_error_message(
                    "the row_slicing_operation primitive requires "
                        "that the arguments given by the operands "
                        "array are valid",
                    name_, codename_));
            }
        }

        auto this_ = this->shared_from_this();
        return hpx::dataflow(hpx::util::unwrapping(
            [this_](std::vector<primitive_argument_type>&& args) -> primitive_argument_type
            {
                //Extract the matrix i.e the first argument
                arg_type matrix_input = execution_tree::extract_numeric_value(
                    args[0], this_->name_, this_->codename_
                );

                std::vector<double > extracted;

                //Extract the list or the single double
                if (execution_tree::is_list_operand_strict(args[1]))
                {
                    auto result = execution_tree::extract_list_value(
                        args[1], this_->name_, this_->codename_);
                    for(auto a:result) {
                      extracted.push_back(execution_tree::extract_numeric_value(a)[0]);
                    }
                }
                else
                {
                    double result = execution_tree::extract_numeric_value(
                        args[1], this_->name_, this_->codename_)[0];
                    extracted.push_back(result);
                }

                std::size_t matrix_dims = matrix_input.num_dimensions();

                switch (matrix_dims)
                {
                case 0:
                    return this_->row_slicing0d(std::move(matrix_input), extracted);

                case 1:
                    return this_->row_slicing1d(std::move(matrix_input), extracted);

                case 2:
                    return this_->row_slicing2d(std::move(matrix_input), extracted);

                default:
                    HPX_THROW_EXCEPTION(hpx::bad_parameter,
                        "row_slicing_operation::eval",
                        execution_tree::generate_error_message(
                            "left hand side operand has unsupported "
                                "number of dimensions",
                            this_->name_, this_->codename_));
                }
            }),
            detail::map_operands(
                operands, functional::value_operand{}, args,
                name_, codename_));
    }

    //////////////////////////////////////////////////////////////////////////
    hpx::future<primitive_argument_type> row_slicing_operation::eval(
        std::vector<primitive_argument_type> const& args) const
    {
        if (operands_.empty())
        {
            return eval(args, noargs);
        }
        return eval(operands_, args);
    }
}}}
