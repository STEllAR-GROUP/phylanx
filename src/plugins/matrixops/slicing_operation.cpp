// Copyright (c) 2017-2018 Bibek Wagle
// Copyright (c) 2017-2018 Hartmut Kaiser
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>
#include <phylanx/execution_tree/compiler/primitive_name.hpp>
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
    std::vector<match_pattern_type> const slicing_operation::match_data = {
        hpx::util::make_tuple("slice",
            std::vector<std::string>{
                "slice(_1)", "slice(_1, _2)", "slice(_1,_2,_3)"},
            &create_slicing_operation,
            &create_primitive<slicing_operation>),

        hpx::util::make_tuple("slice_row",
            std::vector<std::string>{"slice_row(_1, _2)"},
            &create_slicing_operation,
            &create_primitive<slicing_operation>),

        hpx::util::make_tuple("slice_column",
            std::vector<std::string>{"slice_column(_1, _2)"},
            &create_slicing_operation,
            &create_primitive<slicing_operation>)};

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

    ///////////////////////////////////////////////////////////////////////////
    std::int64_t slicing_operation::extract_integer_value(
        primitive_argument_type const& val, std::int64_t default_value) const
    {
        if (valid(val))
        {
            return execution_tree::extract_scalar_integer_value(val, name_, codename_);
        }
        return default_value;
    }

    ///////////////////////////////////////////////////////////////////////////
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

    ///////////////////////////////////////////////////////////////////////////
    std::vector<std::int64_t> slicing_operation::extract_slicing_args_vector(
        std::vector<primitive_argument_type>&& args, std::size_t size) const
    {
        if (args.size() == 2)
        {
            return extract_slicing(std::move(args[1]), size);
        }
        else if (args.size() == 1)
        {
            return extract_slicing(primitive_argument_type{}, size);
        }

        HPX_THROW_EXCEPTION(hpx::bad_parameter,
            "phylanx::execution_tree::primitives::"
                "slicing_operation::slice_list",
            generate_error_message("too many arguments for slicing a vector"));
    }

    ///////////////////////////////////////////////////////////////////////////
    std::string slicing_operation::extract_function_name(
        std::string const& name) const
    {
        compiler::primitive_name_parts name_parts;
        if (!compiler::parse_primitive_name(name, name_parts))
        {
            std::string::size_type p = name.find_first_of("$");
            if (p != std::string::npos)
            {
                return name.substr(0, p);
            }
        }

        return name_parts.primitive;
    }

    void slicing_operation::extract_slicing_args_matrix(
        std::vector<primitive_argument_type>&& args,
        std::vector<std::int64_t>& extracted_row,
        std::vector<std::int64_t>& extracted_column,
        std::size_t rows, std::size_t columns) const
    {
        const auto& func_name = extract_function_name(name_);
        // If its column only slicing, extract the index for the column
        // and use all of the rows.
        if (func_name == "slice_column")
        {
            if (args.size() > 1)
            {
                extracted_column = extract_slicing(std::move(args[1]), columns);
            }
            else
            {
                extracted_column =
                    extract_slicing(primitive_argument_type{}, columns);
            }

            extracted_row.push_back(0);
            extracted_row.push_back(rows);
        }
        else
        {
            // Extract the list or the single integer index
            // from second argument (row-> start, stop, step)
            if (args.size() > 1)
            {
                extracted_row = extract_slicing(std::move(args[1]), rows);
            }
            else
            {
                extracted_row =
                    extract_slicing(primitive_argument_type{}, rows);
            }

            // Extract the list or the single integer index
            // from third argument (column-> start, stop, step)
            if (args.size() == 3)
            {
                extracted_column = extract_slicing(std::move(args[2]), columns);
            }
            else
            {
                extracted_column =
                    extract_slicing(primitive_argument_type{}, columns);
            }
        }
    }

    primitive_argument_type slicing_operation::handle_numeric_operand(
        std::vector<primitive_argument_type>&& args) const
    {
        // Extract the matrix i.e the first argument
        arg_type matrix_input =
            extract_numeric_value(std::move(args[0]), name_, codename_);

        std::size_t matrix_dims = matrix_input.num_dimensions();
        switch (matrix_dims)
        {
        case 0:
            return slicing0d(std::move(matrix_input));

        case 1:
            return slicing1d(std::move(matrix_input),
                extract_slicing_args_vector(
                    std::move(args), matrix_input.vector().size()));

        case 2:
            {
                std::vector<std::int64_t> extracted_row;
                std::vector<std::int64_t> extracted_column;
                extract_slicing_args_matrix(std::move(args), extracted_row,
                    extracted_column, matrix_input.matrix().rows(),
                    matrix_input.matrix().columns());
                return slicing2d(
                    std::move(matrix_input), extracted_row, extracted_column);
            }

        default:
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "row_slicing_operation::eval",
                generate_error_message(
                    "left hand side operand has unsupported "
                        "number of dimensions"));
        }
    }

    ///////////////////////////////////////////////////////////////////////////
    std::vector<std::int64_t> slicing_operation::extract_slicing(
        primitive_argument_type&& arg, std::size_t arg_size) const
    {
        std::vector<std::int64_t> indices;

        // Extract the list or the single integer index
        // from second argument (row-> start, stop, step)
        if (is_list_operand_strict(arg))
        {
            auto arg_list =
                extract_list_value(std::move(arg), name_, codename_);

            std::size_t size = arg_list.size();
            if (size > 3)
            {
                HPX_THROW_EXCEPTION(hpx::bad_parameter,
                    "phylanx::execution_tree::primitives::"
                        "slicing_operation::extract_list_slicing_args",
                    generate_error_message("too many indicies given"));
            }

            auto it = arg_list.begin();

            // default first index is '0'
            if (size > 0)
            {
                indices.push_back(extract_integer_value(*it, 0));
            }
            else
            {
                indices.push_back(0);
            }

            // default last index is 'size'
            if (size > 1)
            {
                indices.push_back(extract_integer_value(*++it, arg_size));
            }
            else
            {
                indices.push_back(arg_size);
            }

            // default step is '1'
            if (size > 2)
            {
                indices.push_back(extract_integer_value(*++it, 1ll));
            }
            else
            {
                indices.push_back(1ll);
            }
        }
        else if (!valid(arg))
        {
            // no arguments given means return all of the argument
            indices.push_back(0);
            indices.push_back(arg_size);
            indices.push_back(1ll);
        }
        else
        {
            indices.push_back(execution_tree::extract_scalar_integer_value(
                std::move(arg), name_, codename_));
        }

        return indices;
    }

    std::vector<std::int64_t> slicing_operation::extract_slicing_args_list(
        std::vector<primitive_argument_type>&& args, std::size_t size) const
    {
        if (args.size() == 2)
        {
            return extract_slicing(std::move(args[1]), size);
        }
        else if (args.size() == 1)
        {
            return extract_slicing(primitive_argument_type{}, size);
        }

        HPX_THROW_EXCEPTION(hpx::bad_parameter,
            "phylanx::execution_tree::primitives::"
                "slicing_operation::slice_list",
            generate_error_message("too many arguments for slicing a list"));
    }

    primitive_argument_type slicing_operation::slice_list(ir::range&& list,
        std::vector<std::int64_t> const& columns) const
    {
        if (columns.empty())
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "phylanx::execution_tree::primitives::"
                    "slicing_operation::slice_list",
                generate_error_message("column indicies can not be empty"));
        }
        if (columns.size() > 3)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "phylanx::execution_tree::primitives::"
                    "slicing_operation::slice_list",
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
                        "slicing_operation::slice_list",
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
        auto list =
            extract_list_value_strict(std::move(args[0]), name_, codename_);

        std::size_t size = list.size();
        return slice_list(
            std::move(list), extract_slicing_args_list(std::move(args), size));
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

        if (!valid(operands[0]))
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "slicing_operation::eval",
                generate_error_message(
                    "the slicing_operation primitive requires "
                        "that the first argument given is valid"));
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
        if (this->no_operands())
        {
            return eval(args, noargs);
        }
        return eval(this->operands(), args);
    }
}}}
