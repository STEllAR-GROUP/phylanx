// Copyright (c) 2017 Alireza Kheirkhahan
// Copyright (c) 2018 Bibek Wagle
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>
#include <phylanx/ast/detail/is_literal_value.hpp>
#include <phylanx/execution_tree/primitives/store_operation.hpp>
#include <phylanx/ir/node_data.hpp>
#include <phylanx/util/slicing_helpers.hpp>

#include <hpx/include/lcos.hpp>
#include <hpx/include/naming.hpp>
#include <hpx/include/util.hpp>
#include <hpx/throw_exception.hpp>
#include <hpx/util/assert.hpp>

#include <cstddef>
#include <cstdint>
#include <memory>
#include <sstream>
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
        auto input = extract_numeric_value(args[0]);
        auto size = input.size();
        auto input_vector = input.vector();

        data = extract_numeric_value(args[2]);
        std::size_t value_dimnum = data.num_dimensions();

        std::vector<std::int64_t> extracted_row =
            util::slicing_helpers::extract_slicing(std::move(args[1]), size);

        if (extracted_row.empty())
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                                "phylanx::execution_tree::primitives::"
                                "store_operation::store1d",
                                generate_error_message("rows can not be empty"));
        }

        if (extracted_row.size() == 1 && value_dimnum == 0)
        {
            std::int64_t index = extracted_row[0];
            if (index < 0)
            {
                index = input_vector.size() + index;
            }
            init_list.push_back(index);
        }
        else
        {
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
                        "store_operation::store1d",
                        generate_error_message("step can not be zero"));
                }
            }

            if (value_dimnum == 2)
            {
                HPX_THROW_EXCEPTION(hpx::bad_parameter,
                    "phylanx::execution_tree::primitives::"
                    "store_operation::set1d",
                    execution_tree::generate_error_message(
                        "can not store matrix in a vetor", name_, codename_));
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

        if (args.size() > 1)
        {
            extracted_row = util::slicing_helpers::extract_slicing(
                std::move(args[1]), num_matrix_rows);
        }
        else
        {
            extracted_row = util::slicing_helpers::extract_slicing(
                primitive_argument_type{}, num_matrix_rows);
        }

        // Extract the list or the single integer index
        // from third argument (column-> start, stop, step)
        if (args.size() == 4)
        {
            extracted_col = util::slicing_helpers::extract_slicing(
                std::move(args[2]), num_matrix_cols);
        }
        else
        {
            extracted_col = util::slicing_helpers::extract_slicing(
                primitive_argument_type{}, num_matrix_cols);
        }

        if (extracted_col.empty() || extracted_row.empty())
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                                "phylanx::execution_tree::primitives::"
                                "store_operation::set2d",
                                generate_error_message("columns/rows can not be empty"));
        }

        bool row_set = false;
        bool col_set = false;

        if (extracted_row.size() == 1)
        {
            std::int64_t index = extracted_row[0];
            if (index < 0)
            {
                index = num_matrix_rows + index;
            }
            init_list_row.push_back(index);
            row_set = true;
        }

        if (extracted_col.size() == 1)
        {
            std::int64_t index = extracted_col[0];
            if (index < 0)
            {
                index = num_matrix_cols + index;
            }
            init_list_col.push_back(index);
            col_set = true;
        }

        if (!row_set) {
            std::int64_t row_start = extracted_row[0];
            std::int64_t row_stop = extracted_row[1];
            std::int64_t step_row = 1;

            if (extracted_row.size() == 3)
            {
                step_row = extracted_row[2];
                if (step_row == 0)
                {
                    HPX_THROW_EXCEPTION(hpx::bad_parameter,
                                        "phylanx::execution_tree::primitives::"
                                        "store_operation::store2d",
                                        generate_error_message("step can not be zero"));
                }
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
            init_list_row =
                create_list_set(row_start, row_stop, step_row, num_matrix_rows);
        }

        if (!col_set) {
            std::int64_t col_start = extracted_col[0];
            std::int64_t col_stop = extracted_col[1];
            std::int64_t step_col = 1;

            if (extracted_col.size() == 3) {
                step_col = extracted_col[2];
                if (step_col == 0) {
                    HPX_THROW_EXCEPTION(hpx::bad_parameter,
                                        "phylanx::execution_tree::primitives::"
                                        "store_operation::store2d",
                                        generate_error_message("step can not be zero"));
                }
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
            init_list_col =
                create_list_set(col_start, col_stop, step_col, num_matrix_cols);
        }


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

