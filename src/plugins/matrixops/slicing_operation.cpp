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
#include <array>
#include <cmath>
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
    std::vector<match_pattern_type> const slicing_operation::match_data =
    {
        hpx::util::make_tuple("slice",
            std::vector<std::string>{
                "slice(_1)", "slice(_1, _2)", "slice(_1,_2,_3)"},
            &create_slicing_operation,
            &create_primitive<slicing_operation>),

        hpx::util::make_tuple("slice_row",
            std::vector<std::string>{
                "slice_row(_1)", "slice_row(_1, _2)"},
            &create_slicing_operation,
            &create_primitive<slicing_operation>),

        hpx::util::make_tuple("slice_column",
            std::vector<std::string>{
                "slice_column(_1)", "slice_column(_1, _2)"},
            &create_slicing_operation,
            &create_primitive<slicing_operation>)
    };

    ///////////////////////////////////////////////////////////////////////////
    slicing_operation::slicing_operation(
            std::vector<primitive_argument_type>&& operands,
            std::string const& name, std::string const& codename)
      : primitive_component_base(std::move(operands), name, codename)
      , column_slicing_(extract_function_name(name_) == "slice_column")
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
        primitive_argument_type&& val, std::int64_t default_value) const
    {
        if (valid(val))
        {
            auto&& nd = execution_tree::extract_integer_value(
                std::move(val), name_, codename_);

            if (nd.size() == 0)
            {
                HPX_THROW_EXCEPTION(hpx::bad_parameter,
                    "phylanx::execution_tree::primitives::"
                        "slicing_operation::extract_integer_value",
                    generate_error_message(
                        "slicing arguments cannot be empty"));
            }

            return nd[0];
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
        slicing_operation::indices const& extracted_rows) const
    {
        // handle single argument slicing parameters
        auto input_vector = arg.vector();
        std::int64_t row_start = extracted_rows.slice_[0];

        // handle single value slicing result
        if (extracted_rows.single_value_)
        {
            std::int64_t index = row_start;
            if (index < 0)
            {
                index = input_vector.size() + index;
            }

            return primitive_argument_type{input_vector[index]};
        }

        std::int64_t row_stop = extracted_rows.slice_[1];
        std::int64_t step = extracted_rows.slice_[2];

        if (step == 0)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "phylanx::execution_tree::primitives::"
                    "slicing_operation::slicing1d",
                generate_error_message("step can not be zero"));
        }

        auto init_list =
            create_list_slice(row_start, row_stop, step, input_vector.size());

        auto sv = blaze::elements(input_vector, init_list);
        storage1d_type v{sv};
        return primitive_argument_type{ir::node_data<double>(std::move(v))};
    }

    primitive_argument_type slicing_operation::slicing2d(
        arg_type&& arg, slicing_operation::indices const& extracted_rows,
        slicing_operation::indices const& extracted_columns) const
    {
        blaze::DynamicMatrix<double> input_matrix = arg.matrix();
        std::size_t num_matrix_rows = input_matrix.rows();
        std::size_t num_matrix_cols = input_matrix.columns();

        std::int64_t row_start = extracted_rows.slice_[0];
        std::int64_t row_stop = extracted_rows.slice_[1];
        std::int64_t row_step = extracted_rows.slice_[2];

        if (row_step == 0 && !extracted_rows.single_value_)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "phylanx::execution_tree::primitives::"
                    "row_slicing_operation::row_slicing_operation",
                generate_error_message("step can not be zero"));
        }

        std::int64_t col_start = extracted_columns.slice_[0];
        std::int64_t col_stop = extracted_columns.slice_[1];
        std::int64_t col_step = extracted_columns.slice_[2];

        if (col_step == 0 && !extracted_columns.single_value_)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "phylanx::execution_tree::primitives::"
                    "slicing_operation::slicing2d",
                generate_error_message("step can not be zero"));
        }

        // return a value and not a vector if you are not given a list
        if (extracted_rows.single_value_)
        {
            std::int64_t index = row_start;
            if (index < 0)
            {
                index = num_matrix_rows + index;
            }

            auto sv = blaze::trans(blaze::row(input_matrix, index));
            if (extracted_columns.single_value_)
            {
                std::int64_t index_col = col_start;
                if (index_col < 0)
                {
                    index_col = num_matrix_cols + index_col;
                }

                double value = sv[index_col];
                return primitive_argument_type{ir::node_data<double>{value}};
            }

            auto init_list_col = create_list_slice(
                col_start, col_stop, col_step, num_matrix_cols);

            auto final_vec = blaze::elements(sv, init_list_col);
            storage1d_type v{final_vec};
            return primitive_argument_type{ir::node_data<double>{std::move(v)}};
        }

        auto init_list =
            create_list_slice(row_start, row_stop, row_step, num_matrix_rows);

        auto sm = blaze::rows(input_matrix, init_list);
        if (extracted_columns.single_value_)
        {
            std::int64_t index_col = col_start;
            if (index_col < 0)
            {
                index_col = num_matrix_cols + index_col;
            }

            auto vec = blaze::column(sm, index_col);
            storage1d_type v{vec};
            return primitive_argument_type{ir::node_data<double>{std::move(v)}};
        }

        auto init_list_col =
            create_list_slice(col_start, col_stop, col_step, num_matrix_cols);

        auto final_mat = blaze::columns(sm, init_list_col);
        storage2d_type m{final_mat};
        return primitive_argument_type{ir::node_data<double>{std::move(m)}};
    }

    ///////////////////////////////////////////////////////////////////////////
    slicing_operation::indices slicing_operation::extract_slicing_args_vector(
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
        std::string const& name)
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
        slicing_operation::indices& extracted_rows,
        slicing_operation::indices& extracted_columns,
        std::size_t num_rows, std::size_t num_columns) const
    {
        // If its column only slicing, extract the index for the column
        // and use all of the rows.
        if (column_slicing_)
        {
            if (args.size() > 1)
            {
                extracted_columns =
                    extract_slicing(std::move(args[1]), num_columns);
            }
            else
            {
                extracted_columns =
                    extract_slicing(primitive_argument_type{}, num_columns);
            }

            extracted_rows.slice_[0] = 0;
            extracted_rows.slice_[1] = num_rows;
            extracted_rows.slice_[2] = 1;
        }
        else
        {
            // Extract the list or the single integer index
            // from second argument (row-> start, stop, step)
            if (args.size() > 1)
            {
                extracted_rows = extract_slicing(std::move(args[1]), num_rows);
            }
            else
            {
                extracted_rows =
                    extract_slicing(primitive_argument_type{}, num_rows);
            }

            // Extract the list or the single integer index
            // from third argument (column-> start, stop, step)
            if (args.size() == 3)
            {
                extracted_columns =
                    extract_slicing(std::move(args[2]), num_columns);
            }
            else
            {
                extracted_columns =
                    extract_slicing(primitive_argument_type{}, num_columns);
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
            {
                std::size_t size = matrix_input.vector().size();
                return slicing1d(std::move(matrix_input),
                    extract_slicing_args_vector(std::move(args), size));
            }

        case 2:
            {
                indices extracted_rows;
                indices extracted_columns;
                extract_slicing_args_matrix(std::move(args), extracted_rows,
                    extracted_columns, matrix_input.matrix().rows(),
                    matrix_input.matrix().columns());
                return slicing2d(
                    std::move(matrix_input), extracted_rows, extracted_columns);
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
    slicing_operation::indices slicing_operation::extract_slicing(
        primitive_argument_type&& arg, std::size_t arg_size) const
    {
        slicing_operation::indices indices;

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

            // if step is negative and start/stop are not given, then start and
            // stop must be swapped
            std::int64_t default_start = 0;
            std::int64_t default_stop = arg_size;
            std::int64_t step = 1;
            if (size == 3)
            {
                std::advance(it, 2);
                step = extract_integer_value(std::move(*it), 1ll);
                if (step < 0)
                {
                    // create_list_slice above will add list size to these values
                    default_start = -1;
                    default_stop = -arg_size - 1;
                }
            }

            // reinit iterator
            it = arg_list.begin();

            // default first index is '0'
            if (size > 0)
            {
                indices.slice_[0] =
                    extract_integer_value(std::move(*it), default_start);
                indices.single_value_ = true;
            }
            else
            {
                indices.slice_[0] = 0;
            }

            // default last index is 'size'
            if (size > 1)
            {
                indices.slice_[1] =
                    extract_integer_value(std::move(*++it), default_stop);
                indices.single_value_ = false;
            }
            else
            {
                indices.slice_[1] = arg_size;
            }

            // default step is '1'
            indices.slice_[2] = step;
        }
        else if (!valid(arg))
        {
            // no arguments given means return all of the argument
            indices.slice_[0] = 0;
            indices.slice_[1] = arg_size;
            indices.slice_[2] = 1;
        }
        else
        {
            // allow for the slicing parameters to be a single integer
            std::int64_t start = extract_integer_value(std::move(arg), 0);

            indices.slice_[0] = start;
            indices.single_value_ = true;
        }

        return indices;
    }

    slicing_operation::indices slicing_operation::extract_slicing_args_list(
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
        slicing_operation::indices const& columns) const
    {
        std::size_t list_size = list.size();

        std::int64_t start = columns.slice_[0];
        std::int64_t stop = columns.slice_[1];
        std::int64_t step = columns.slice_[2];

        if (step == 0 && !columns.single_value_)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "phylanx::execution_tree::primitives::"
                    "slicing_operation::slice_list",
                generate_error_message("step can not be zero"));
        }

        if (columns.single_value_)
        {
            std::int64_t index = start;
            if (index < 0)
            {
                index = list_size + index;
            }

            auto it = list.begin();
            std::advance(it, index);
            return primitive_argument_type{std::move(*it)};
        }

        // list of indices to extract
        std::vector<std::int64_t> index_list =
            create_list_slice(start, stop, step, list_size);

        std::vector<primitive_argument_type> result;
        result.reserve(index_list.size());

        auto idx_it = index_list.begin();
        auto idx_end = index_list.end();

        // extract elements from list at given indices
        if (start <= stop && step > 0)
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
        if (operands.empty() || operands.size() > 3)
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
