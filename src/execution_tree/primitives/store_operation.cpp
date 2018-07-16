// Copyright (c) 2017 Alireza Kheirkhahan
// Copyright (c) 2018 Bibek Wagle
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>
#include <phylanx/ast/detail/is_literal_value.hpp>
#include <phylanx/execution_tree/primitives/store_operation.hpp>
#include <phylanx/ir/node_data.hpp>

#include <hpx/include/lcos.hpp>
#include <hpx/include/naming.hpp>
#include <hpx/include/util.hpp>
#include <hpx/throw_exception.hpp>

#include <cstddef>
#include <memory>
#include <string>
#include <utility>
#include <vector>

///////////////////////////////////////////////////////////////////////////////
namespace phylanx { namespace execution_tree { namespace primitives
{
    ///////////////////////////////////////////////////////////////////////////
    primitive create_store_operation(hpx::id_type const& locality,
        std::vector<primitive_argument_type>&& operands,
        std::string const& name, std::string const& codename)
    {
        static std::string type("store");
        return create_primitive_component(
            locality, type, std::move(operands), name, codename);
    }

    match_pattern_type const store_operation::match_data =
    {
        hpx::util::make_tuple("store",
            std::vector<std::string>{"store(_1, _2)",
            "store(_1, _2, _3)", "store(_1, _2, _3, _4)"},
            &create_store_operation, &create_primitive<store_operation>)
    };

    ///////////////////////////////////////////////////////////////////////////
    store_operation::store_operation(
            std::vector<primitive_argument_type> && operands,
            std::string const& name, std::string const& codename)
      : primitive_component_base(std::move(operands), name, codename, true)
    {}
    ////////////////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////
    bool store_operation::check_set_parameters(std::int64_t start,
        std::int64_t stop, std::int64_t step, std::size_t array_length) const
    {
        if (start < 0)
        {
            start = array_length + start;
        }

        if (stop < 0)
        {
            stop = array_length + stop;
        }

        if (step > 0)
        {
            return (stop > start);
        }
        else
        {
            return (start > stop);
        }
    }

    std::vector<std::int64_t> store_operation::create_list_set(
        std::int64_t start, std::int64_t stop, std::int64_t step,
        std::size_t array_length) const
    {
        HPX_ASSERT(step != 0);
        auto actual_start = 0;
        auto actual_stop = 0;

        if (start >= 0)
        {
            actual_start = start;
        }
        else    //(start < 0)
        {
            actual_start = array_length + start;
        }

        if (stop >= 0)
        {
            actual_stop = stop;
        }
        else    //(stop < 0)
        {
            actual_stop = array_length + stop;
        }

        std::vector<std::int64_t> result;

        if (step > 0)
        {
            HPX_ASSERT(actual_stop > actual_start);
            result.reserve((actual_stop - actual_start + step) / step);
            for (std::int64_t i = actual_start; i < actual_stop; i += step)
            {
                result.push_back(i);
            }
        }
        else    //(step < 0)
        {
            HPX_ASSERT(actual_start > actual_stop);
            result.reserve((actual_start - actual_stop - step) / (-step));
            for (std::int64_t i = actual_start; i > actual_stop; i += step)
            {
                result.push_back(i);
            }
        }

        if (result.empty())
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "phylanx::execution_tree::primitives::"
                "store_operation::create_list_set",
                execution_tree::generate_error_message(
                    "Set will produce empty result, please check your "
                    "parameters",
                    name_, codename_));
        }
        return result;
    }

    ///////////////////////////These need to be moved to util/////////////////////////

    std::int64_t store_operation::extract_integer_value(
        primitive_argument_type const& val, std::int64_t default_value) const
    {
        if (valid(val))
        {
            return execution_tree::extract_scalar_integer_value(
                val, name_, codename_);
        }
        return default_value;
    }

    std::vector<std::int64_t> store_operation::extract_slicing(
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

    //////////////////////////////////////////////////////////////////////////
    primitive_argument_type store_operation::set0d(
        std::vector<primitive_argument_type>&& args) const
    {
        HPX_THROW_EXCEPTION(hpx::bad_parameter,
            "phylanx::execution_tree::primitives::"
            "store_operation::set0d",
            execution_tree::generate_error_message(
                " 0d expects only two arguments, please use store(a,42).",
                name_, codename_));
    }

    void store_operation::set1d(std::vector<primitive_argument_type>&& args,
        ir::node_data<double>& data, std::vector<int64_t>& init_list) const
    {
        primitive_argument_type temp = args[1];
        auto input = extract_numeric_value(args[0]);
        auto size = input.size();
        auto input_vector = input.vector();
        std::vector<std::int64_t> extracted =
            extract_slicing(std::move(temp), size);
        std::int64_t row_start = extracted[0];
        std::int64_t row_stop = extracted[1];
        std::int64_t step = extracted[2];
        data = extract_numeric_value(args[2]);

        if (step == 0)
        {
            row_stop = row_start + 1;
            step = 1;
        }
        std::size_t value_dimnum = data.num_dimensions();

        if (value_dimnum == 2)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "phylanx::execution_tree::primitives::"
                "store_operation::set1d",
                execution_tree::generate_error_message(
                    "can not store matrix in a vetor", name_, codename_));
        }

        if (step == 0)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "phylanx::execution_tree::primitives::"
                "store_operation::set1d",
                execution_tree::generate_error_message(
                    "argument 'step' can not be zero", name_, codename_));
        }

        if (!check_set_parameters(row_start, row_stop, step, size))
        {
            std::ostringstream msg;
            msg << "argument 'start' or 'stop' are not valid: ";
            msg << "start=" << row_start << ", stop=" << row_stop
                << ", step=" << step;
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "phylanx::execution_tree::primitives::"
                "store_operation::set1d",
                execution_tree::generate_error_message(
                    msg.str(), name_, codename_));
        }

        init_list = create_list_set(row_start, row_stop, step, size);
    }

    void store_operation::set2d(std::vector<primitive_argument_type>&& args,
        ir::node_data<double>& data, std::vector<int64_t>& init_list_row,
        std::vector<int64_t>& init_list_col) const
    {
        std::size_t num_matrix_rows =
            extract_numeric_value(args[0]).dimensions()[0];
        std::size_t num_matrix_cols =
            extract_numeric_value(args[0]).dimensions()[1];

        data = extract_numeric_value(args[3]);
        std::size_t value_dimnum = data.num_dimensions();

        std::vector<std::int64_t> extracted_row;
        std::vector<std::int64_t> extracted_col;
        primitive_argument_type temp1 = args[1];
        primitive_argument_type temp2 = args[2];

        if (args.size() > 1)
        {
            extracted_row = extract_slicing(std::move(temp1), num_matrix_rows);
        }
        else
        {
            extracted_row =
                extract_slicing(primitive_argument_type{}, num_matrix_rows);
        }

        // Extract the list or the single integer index
        // from third argument (column-> start, stop, step)
        if (args.size() == 4)
        {
            extracted_col = extract_slicing(std::move(temp2), num_matrix_cols);
        }
        else
        {
            extracted_col =
                extract_slicing(primitive_argument_type{}, num_matrix_cols);
        }

        std::int64_t row_start = extracted_row[0];
        std::int64_t row_stop = extracted_row[1];
        std::int64_t step_row = extracted_row[2];

        std::int64_t col_start = extracted_col[0];
        std::int64_t col_stop = extracted_col[1];
        std::int64_t step_col = extracted_col[2];

        if (step_row == 0 || step_col == 0)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "phylanx::execution_tree::primitives::"
                "store_operation::set2d",
                execution_tree::generate_error_message(
                    "argument 'step_row' or 'step_col' can not be zero", name_,
                    codename_));
        }

        if (!check_set_parameters(
                row_start, row_stop, step_row, num_matrix_rows))
        {
            std::ostringstream msg;
            msg << "argument 'row_start' or 'row_stop' are not valid: ";
            msg << "start=" << row_start << ", stop=" << row_stop
                << ", step=" << step_row;
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "phylanx::execution_tree::primitives::"
                "store_operation::set2d",
                execution_tree::generate_error_message(
                    msg.str(), name_, codename_));
        }

        if (!check_set_parameters(
                col_start, col_stop, step_col, num_matrix_cols))
        {
            std::ostringstream msg;
            msg << "argument 'col_start' or 'col_stop' are not valid: ";
            msg << "start=" << col_start << ", stop=" << col_stop
                << ", step=" << step_col;
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "phylanx::execution_tree::primitives::"
                "store_operation::set2d",
                execution_tree::generate_error_message(
                    msg.str(), name_, codename_));
        }

        init_list_row =
            create_list_set(row_start, row_stop, step_row, num_matrix_rows);
        init_list_col =
            create_list_set(col_start, col_stop, step_col, num_matrix_cols);
    }

    ///////////////////////////////////////////////////////////////////////////
    hpx::future<primitive_argument_type> store_operation::eval(
        std::vector<primitive_argument_type> const& operands,
        std::vector<primitive_argument_type> const& args) const
    {
        std::size_t operands_size = operands_.size();
        if (operands_size < 2 || operands_size > 4)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "store_operation::eval",
                execution_tree::generate_error_message(
                    "the store_operation primitive requires either "
                        "two, three or four operands",
                    name_, codename_));
        }

        for (std::size_t i = 0; i < operands_size; ++i)
        {
            if (!valid(operands_[i]))
            {
                HPX_THROW_EXCEPTION(hpx::bad_parameter,
                    "store_operation::store_operation",
                    execution_tree::generate_error_message(
                        "the store_operation primitive requires that "
                        "the arguments given by the operands array "
                        "is valid",
                        name_, codename_));
            }
        }

        if (!is_primitive_operand(operands_[0]))
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "store_operation::store_operation",
                execution_tree::generate_error_message(
                    "the first argument of the store primitive must "
                        "refer to a another primitive and can't be a "
                        "literal value",
                    name_, codename_));
        }

        auto this_ = this->shared_from_this();

        if (operands_size == 2)
        {
            return literal_operand(operands[1], args, name_, codename_)
                .then(hpx::launch::sync,
                    hpx::util::unwrapping(
                        [this_, lhs = extract_ref_value(operands[0])](
                            primitive_argument_type&& val)
                            -> primitive_argument_type {
                            primitive_operand(
                                lhs, this_->name_, this_->codename_)
                                .store(hpx::launch::sync, std::move(val));
                            return primitive_argument_type{};
                        }));
        }
        else
        {
            return hpx::dataflow(hpx::launch::sync,
                hpx::util::unwrapping(
                    [this_, operands](
                        std::vector<primitive_argument_type>&& args)
                        -> primitive_argument_type {
                        std::size_t lhs_dims =
                            extract_numeric_value(args[0]).num_dimensions();
                        switch (lhs_dims)
                        {
                        case 0:
                            return this_->set0d(std::move(args));

                        case 1:
                        {
                            ir::node_data<double> data;
                            std::vector<int64_t> init_list;
                            this_->set1d(std::move(args), data, init_list);
                            primitive_operand(
                                operands[0], this_->name_, this_->codename_)
                                .store_set_1d(hpx::launch::sync,
                                    std::move(data),
                                    std::move(init_list));
                            return primitive_argument_type{};
                        }

                        case 2:
                        {
                            ir::node_data<double> data;
                            std::vector<int64_t> init_list_row;
                            std::vector<int64_t> init_list_col;
                            this_->set2d(std::move(args), data, init_list_row,
                                init_list_col);
                            primitive_operand(
                                operands[0], this_->name_, this_->codename_)
                                .store_set_2d(hpx::launch::sync,
                                    std::move(data),
                                    std::move(init_list_row),
                                    std::move(init_list_col));
                            return primitive_argument_type{};
                        }
                        default:
                            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                                "set_operation::eval",
                                execution_tree::generate_error_message(
                                    "left hand side operand has "
                                    "unsupported "
                                    "number of dimensions",
                                    this_->name_, this_->codename_));
                        }
                    }),
                detail::map_operands(operands, functional::literal_operand{},
                    args, name_, codename_));
        }
    }

    //////////////////////////////////////////////////////////////////////////
    hpx::future<primitive_argument_type> store_operation::eval(
        std::vector<primitive_argument_type> const& args) const
    {
        if (this->no_operands())
        {
            return eval(args, noargs);
        }
        return eval(this->operands(), args);
    }
}}}

