// Copyright (c) 2017-2018 Bibek Wagle
// Copyright (c) 2017-2018 Hartmut Kaiser
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>
#include <phylanx/ir/node_data.hpp>
#include <phylanx/plugins/matrixops/slicing_operation.hpp>

#include <hpx/include/lcos.hpp>
#include <hpx/include/naming.hpp>
#include <hpx/include/util.hpp>
#include <hpx/throw_exception.hpp>

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <numeric>
#include <string>
#include <utility>
#include <vector>

#include <blaze/Math.h>
#include <blaze/math/Elements.h>

///////////////////////////////////////////////////////////////////////////////
namespace phylanx {namespace execution_tree {    namespace primitives {

    ///////////////////////////////////////////////////////////////////////////
    match_pattern_type const slicing_operation::match_data =
    {
        hpx::util::make_tuple("slice",
            std::vector<std::string>{
                "slice(_1)", "slice(_1, _2)", "slice(_1,_2,_3)"},
            &create_slicing_operation,
            &create_primitive<slicing_operation>)
    };

    ///////////////////////////////////////////////////////////////////////////
    slicing_operation::slicing_operation(
            std::vector<primitive_argument_type>&& operands,
            std::string const& name, std::string const& codename)
      : primitive_component_base(std::move(operands), name, codename)
    {
    }

    ///////////////////////////////////////////////////////////////////////////
    std::vector<std::int64_t> slicing_operation::create_list_slice(
        std::int64_t& start, std::int64_t& stop, std::int64_t step,
        std::size_t array_length) const
    {
        if (start < 0)
        {
            start = array_length + start;
        }

        if (stop < 0)
        {
            stop = array_length + stop;
        }

        std::vector<std::int64_t> result;

        if (step > 0)
        {
            for (std::int64_t i = start; i < stop; i += step)
            {
                result.push_back(i);
            }
        }
        else if (step < 0)
        {
            for (std::int64_t i = start; i > stop; i += step)
            {
                result.push_back(i);
            }
        }

        return result;
    }

    primitive_argument_type slicing_operation::slicing0d(arg_type&& arg) const
    {
        double scalar_data = arg.scalar();
        return primitive_argument_type{ir::node_data<double>{scalar_data}};
    }

    primitive_argument_type slicing_operation::slicing1d(arg_type&& arg,
        std::vector<std::int64_t> const& extracted_row) const
    {
        if (extracted_row.empty())
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "phylanx::execution_tree::primitives::"
                    "slicing_operation::slicing1d",
                generate_error_message("rows can not be empty"));
        }

        auto input_vector = arg.vector();
        if (extracted_row.size() == 1)
        {
            std::int64_t index = extracted_row[0];
            if (index < 0)
            {
                index = input_vector.size() + index;
            }
            return primitive_argument_type{input_vector[index]};
        }

        std::int64_t row_start = extracted_row[0];
        std::int64_t row_stop = extracted_row[1];
        std::int64_t step = 1;

        if (extracted_row.size() == 3)
        {
            step = extracted_row[2];

            if (step == 0)
            {
                HPX_THROW_EXCEPTION(hpx::bad_parameter,
                    "phylanx::execution_tree::primitives::"
                        "slicing_operation::slicing1d",
                    generate_error_message("step can not be zero"));
            }
        }

        auto init_list =
            create_list_slice(row_start, row_stop, step, arg.size());

        auto sv = blaze::elements(input_vector, init_list);
        storage1d_type v{sv};
        return primitive_argument_type{ir::node_data<double>(std::move(v))};
    }

    primitive_argument_type slicing_operation::slicing2d(
        arg_type&& arg, std::vector<std::int64_t> const& extracted_row,
        std::vector<std::int64_t> const& extracted_column) const
    {
        if (extracted_column.empty() || extracted_row.empty())
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "phylanx::execution_tree::primitives::"
                "slicing_operation::slicing2d",
                generate_error_message("columns/rows can not be empty"));
        }

        blaze::DynamicMatrix<double> input_matrix = arg.matrix();
        std::size_t num_matrix_rows = input_matrix.rows();
        std::size_t num_matrix_cols = input_matrix.columns();

        // return a value and not a vector if you are not given a list
        if (extracted_row.size() == 1)
        {
            std::int64_t index = extracted_row[0];
            if (index < 0)
            {
                index = num_matrix_rows + index;
            }

            auto sv = blaze::trans(blaze::row(input_matrix, index));
            if (extracted_column.size() == 1)
            {
                std::int64_t index_col = extracted_column[0];
                if (index_col < 0)
                {
                    index_col = num_matrix_cols + index_col;
                }

                double value = sv[index_col];
                storage0d_type v{value};
                return primitive_argument_type{
                    ir::node_data<double>{std::move(v)}};
            }

            std::int64_t col_start = extracted_column[0];
            std::int64_t col_stop = extracted_column[1];
            std::int64_t step_col = 1;

            if (extracted_column.size() == 3)
            {
                step_col = extracted_column[2];
                if (step_col == 0)
                {
                    HPX_THROW_EXCEPTION(hpx::bad_parameter,
                        "phylanx::execution_tree::primitives::"
                            "slicing_operation::slicing2d",
                        generate_error_message("step can not be zero"));
                }
            }

            auto init_list_col = create_list_slice(
                col_start, col_stop, step_col, num_matrix_cols);

            auto final_vec = blaze::elements(sv, init_list_col);
            storage1d_type v{final_vec};
            return primitive_argument_type{ir::node_data<double>{std::move(v)}};
        }

        std::int64_t row_start = extracted_row[0];
        std::int64_t row_stop = extracted_row[1];
        std::int64_t step = 1;

        if (extracted_row.size() == 3)
        {
            step = extracted_row[2];
            if (step == 0)
            {
                HPX_THROW_EXCEPTION(hpx::bad_parameter,
                    "phylanx::execution_tree::primitives::"
                        "row_slicing_operation::row_slicing_operation",
                    generate_error_message("step can not be zero"));
            }
        }

        auto init_list =
            create_list_slice(row_start, row_stop, step, num_matrix_rows);

        auto sm = blaze::rows(input_matrix, init_list);
        if (extracted_column.size() == 1)
        {
            std::int64_t index_col = extracted_column[0];
            if (index_col < 0)
            {
                index_col = num_matrix_cols + index_col;
            }
            auto vec = blaze::column(sm, index_col);
            storage1d_type v{vec};
            return primitive_argument_type{ir::node_data<double>{std::move(v)}};
        }

        std::int64_t col_start = extracted_column[0];
        std::int64_t col_stop = extracted_column[1];
        std::int64_t step_col = 1;

        if (extracted_column.size() == 3)
        {
            step_col = extracted_column[2];
            if (step_col == 0)
            {
                HPX_THROW_EXCEPTION(hpx::bad_parameter,
                    "phylanx::execution_tree::primitives::"
                        "slicing_operation::slicing2d",
                    generate_error_message("step can not be zero"));
            }
        }

        auto init_list_col =
            create_list_slice(col_start, col_stop, step_col, num_matrix_cols);

        auto final_mat = blaze::columns(sm, init_list_col);
        storage2d_type m{final_mat};
        return primitive_argument_type{ir::node_data<double>{std::move(m)}};
    }

    void slicing_operation::extract_slicing_args(
        std::vector<primitive_argument_type>&& args,
        std::vector<std::int64_t>& extracted_row,
        std::vector<std::int64_t>& extracted_column) const
    {
        // Extract the list or the single integer index
        // from second argument (row-> start, stop, step)
        if (args.size() > 1)
        {
            if (is_list_operand_strict(args[1]))
            {
                auto result =
                    extract_list_value(std::move(args[1]), name_, codename_);
                for (auto && a : std::move(result))
                {
                    extracted_row.push_back(
                        extract_integer_value(std::move(a)));
                }
            }
            else
            {
                extracted_row.push_back(extract_integer_value(
                    std::move(args[1]), name_, codename_));
            }
        }

        // Extract the list or the single integer index
        // from third argument (column-> start, stop, step)
        if (args.size() == 3)
        {
            if (is_list_operand_strict(args[2]))
            {
                auto result =
                    extract_list_value(std::move(args[2]), name_, codename_);
                for (auto && a : std::move(result))
                {
                    extracted_column.push_back(
                        extract_integer_value(std::move(a)));
                }
            }
            else
            {
                extracted_column.push_back(
                    extract_integer_value(std::move(args[2]), name_, codename_));
            }
        }
    }

    primitive_argument_type slicing_operation::handle_numeric_operand(
        std::vector<primitive_argument_type>&& args) const
    {
        // Extract the matrix i.e the first argument
        arg_type matrix_input =
            extract_numeric_value(std::move(args[0]), name_, codename_);

        std::vector<std::int64_t> extracted_row;
        std::vector<std::int64_t> extracted_column;
        extract_slicing_args(std::move(args), extracted_row, extracted_column);

        std::size_t matrix_dims = matrix_input.num_dimensions();
        switch (matrix_dims)
        {
        case 0:
            return slicing0d(std::move(matrix_input));

        case 1:
            return slicing1d(std::move(matrix_input), extracted_row);

        case 2:
            return slicing2d(
                std::move(matrix_input), extracted_row, extracted_column);

        default:
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "row_slicing_operation::eval",
                generate_error_message(
                    "left hand side operand has unsupported "
                        "number of dimensions"));
        }
    }

    std::vector<std::int64_t> slicing_operation::extract_list_slicing_args(
        std::vector<primitive_argument_type>&& args) const
    {
        std::vector<std::int64_t> columns;

        // Extract the list or the single integer index
        // from second argument (row-> start, stop, step)
        if (args.size() > 1)
        {
            if (is_list_operand_strict(args[1]))
            {
                auto result =
                    extract_list_value(std::move(args[1]), name_, codename_);
                for (auto && a : std::move(result))
                {
                    columns.push_back(extract_integer_value(std::move(a)));
                }
            }
            else
            {
                columns.push_back(extract_integer_value(
                    std::move(args[1]), name_, codename_));
            }
        }

        return columns;
    }

    primitive_argument_type slicing_operation::list_slicing(ir::range&& list,
        std::vector<std::int64_t> const& columns) const
    {
        if (columns.empty())
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "phylanx::execution_tree::primitives::"
                    "slicing_operation::list_slicing",
                generate_error_message("column indicies can not be empty"));
        }
        if (columns.size() > 3)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "phylanx::execution_tree::primitives::"
                    "slicing_operation::list_slicing",
                generate_error_message("too many column indicies"));
        }

        std::size_t list_size = list.size();
        if (columns.size() == 1)
        {
            std::int64_t index = columns[0];
            if (index < 0)
            {
                index = list_size + index;
            }

            auto it = list.begin();
            std::advance(it, index);
            return primitive_argument_type{std::move(*it)};
        }

        std::int64_t row_start = columns[0];
        std::int64_t row_stop = columns[1];
        std::int64_t step = 1;

        if (columns.size() == 3)
        {
            step = columns[2];
            if (step == 0)
            {
                HPX_THROW_EXCEPTION(hpx::bad_parameter,
                    "phylanx::execution_tree::primitives::"
                        "slicing_operation::list_slicing",
                    generate_error_message("step can not be zero"));
            }
        }

        // list of indices to extract
        std::vector<std::int64_t> index_list =
            create_list_slice(row_start, row_stop, step, list_size);

        std::vector<primitive_argument_type> result;
        result.reserve(index_list.size());

        auto idx_it = index_list.begin();
        auto idx_end = index_list.end();

        // extract elements from list at given indices
        if (row_start <= row_stop)
        {
            auto list_it = list.begin();
            auto list_end = list.end();
            std::size_t idx = (idx_it != idx_end) ? *idx_it : 0;
            std::advance(list_it, (std::min)(list_size, idx));

            for (/**/; list_it != list_end && idx_it != idx_end;
                 ++idx, ++list_it)
            {
                if (idx == *idx_it)
                {
                    ++idx_it;
                    result.emplace_back(std::move(*list_it));
                }
            }
        }
        else
        {
            auto list_it = list.rbegin();
            auto list_end = list.rend();
            std::size_t idx = list_size - 1;
            for (/**/; list_it != list_end && idx_it != idx_end;
                 --idx, ++list_it)
            {
                if (idx == *idx_it)
                {
                    ++idx_it;
                    result.emplace_back(std::move(*list_it));
                }
            }
        }

        return primitive_argument_type{std::move(result)};
    }

    primitive_argument_type slicing_operation::handle_list_operand(
        std::vector<primitive_argument_type>&& args) const
    {
        // Extract the matrix i.e the first argument
        return list_slicing(
            extract_list_value_strict(std::move(args[0]), name_, codename_),
            extract_list_slicing_args(std::move(args)));
    }

    hpx::future<primitive_argument_type> slicing_operation::eval(
        std::vector<primitive_argument_type> const& operands,
        std::vector<primitive_argument_type> const& args) const
    {
        if (operands.size() > 3)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "phylanx::execution_tree::primitives::"
                    "slicing_operation::slicing_operation",
                generate_error_message(
                    "the slicing_operation primitive requires "
                        "either one(0d), two(1d) or three arguments(2d)"));
        }

        bool arguments_valid = true;
        for (std::size_t i = 0; i != operands.size(); ++i)
        {
            if (!valid(operands[i]))
            {
                HPX_THROW_EXCEPTION(hpx::bad_parameter,
                    "slicing_operation::eval",
                    generate_error_message(
                        "the slicing_operation primitive requires "
                            "that the arguments given by the operands "
                            "array are valid"));
            }
        }

        auto this_ = this->shared_from_this();
        return hpx::dataflow(hpx::launch::sync, hpx::util::unwrapping(
            [this_](std::vector<primitive_argument_type>&& args)
            ->  primitive_argument_type
            {
                if (execution_tree::is_list_operand_strict(args[0]))
                {
                    return this_->handle_list_operand(std::move(args));
                }
                return this_->handle_numeric_operand(std::move(args));
            }),
            detail::map_operands(
                operands, functional::value_operand{}, args, name_, codename_));
    }

    //////////////////////////////////////////////////////////////////////////
    hpx::future<primitive_argument_type> slicing_operation::eval(
        std::vector<primitive_argument_type> const& args) const
    {
        if (operands_.empty())
        {
            return eval(args, noargs);
        }
        return eval(operands_, args);
    }
}}}
