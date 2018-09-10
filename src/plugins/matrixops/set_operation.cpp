// Copyright (c) 2018 Bibek Wagle
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>
#include <phylanx/ir/node_data.hpp>
#include <phylanx/plugins/matrixops/set_operation.hpp>

#include <hpx/include/lcos.hpp>
#include <hpx/include/naming.hpp>
#include <hpx/include/util.hpp>
#include <hpx/throw_exception.hpp>
#include <hpx/util/assert.hpp>

#include <cstddef>
#include <cstdint>
#include <memory>
#include <numeric>
#include <string>
#include <utility>
#include <vector>
#include <sstream>

#include <blaze/Math.h>
#include <blaze/math/Elements.h>

///////////////////////////////////////////////////////////////////////////////
namespace phylanx { namespace execution_tree { namespace primitives
{
    ///////////////////////////////////////////////////////////////////////////
    match_pattern_type const set_operation::match_data =
    {
        hpx::util::make_tuple(
            "set", std::vector<std::string>{"set(_1,_2,_3,_4,_5,_6,_7,_8)"},
            &create_set_operation, &create_primitive<set_operation>,
            "m, b1, e1, s1, b2, e2, s2,v\n"
            "Args:\n"
            "\n"
            "    m (matrix) : a matrix of values\n"
            "    b1 (int) : the start of a Python range\n"
            "    e1 (int) : the end of a Python range\n"
            "    s1 (int) : the step of a Python range\n"
            "    b2 (int) : the start of a Python range\n"
            "    e2 (int) : the end of a Python range\n"
            "    s2 (int) : the step of a Python range\n"
            "    v (number) : the value to set.\n"
            "\n"
            "Returns:\n"
            )
    };

    ///////////////////////////////////////////////////////////////////////////
    set_operation::set_operation(
            primitive_arguments_type&& operands,
            std::string const& name, std::string const& codename)
      : primitive_component_base(std::move(operands), name, codename)
    {}

    ///////////////////////////////////////////////////////////////////////////
    bool set_operation::check_set_parameters(std::int64_t start, std::int64_t stop,
        std::int64_t step, std::size_t array_length) const
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

    std::vector<std::int64_t> set_operation::create_list_set(std::int64_t start,
        std::int64_t stop, std::int64_t step,
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
            for (std::int64_t i = actual_start; i < actual_stop;
                    i += step)
            {
                result.push_back(i);
            }
        }
        else    //(step < 0)
        {
            HPX_ASSERT(actual_start > actual_stop);
            result.reserve(
                (actual_start - actual_stop - step) / (-step));
            for (std::int64_t i = actual_start; i > actual_stop;
                    i += step)
            {
                result.push_back(i);
            }
        }

        if (result.empty())
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "phylanx::execution_tree::primitives::"
                "set_operation::create_list_set",
                util::generate_error_message(
                    "Set will produce empty result, please check your "
                    "parameters",
                    name_, codename_));
        }
        return result;
    }

    primitive_argument_type set_operation::set0d(args_type&& args) const
    {
        HPX_THROW_EXCEPTION(hpx::bad_parameter,
            "phylanx::execution_tree::primitives::"
            "set_operation::set0d",
            util::generate_error_message(
                "use store operation for setting value to a variable",
                name_, codename_));
    }

    primitive_argument_type set_operation::set1d(args_type&& args) const
    {
        auto input_vector = args[0].vector();
        std::int64_t row_start = args[1].scalar();
        std::int64_t row_stop = args[2].scalar();
        std::int64_t step = args[3].scalar();

        if(step == 0)
        {
            row_stop = row_start + 1;
            step = 1;
        }
        std::size_t value_dimnum = args[7].num_dimensions();

        if (value_dimnum == 2)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "phylanx::execution_tree::primitives::"
                "set_operation::set1d",
                util::generate_error_message(
                    "can not store matrix in a vetor", name_,
                    codename_));
        }

        if (step == 0)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "phylanx::execution_tree::primitives::"
                "set_operation::set1d",
                util::generate_error_message(
                    "argument 'step' can not be zero", name_,
                    codename_));
        }

        if (!check_set_parameters(
                row_start, row_stop, step, args[0].size()))
        {
            std::ostringstream msg;
            msg << "argument 'start' or 'stop' are not valid: ";
            msg << "start=" << row_start << ", stop=" << row_stop << ", step=" << step;
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "phylanx::execution_tree::primitives::"
                "set_operation::set1d",
                util::generate_error_message(
                    msg.str(),
                    name_, codename_));
        }

        auto init_list =
            create_list_set(row_start, row_stop, step, args[0].size());

        auto sv = blaze::elements(input_vector, init_list);

        if (value_dimnum == 0)
        {
            blaze::DynamicVector<double> temp(
                sv.size(), args[7].scalar());
            sv = temp;
            return primitive_argument_type{
                ir::node_data<double>(args[0].vector())};
        }

        auto temp = args[7].vector();

        if (sv.size() != temp.size())
        {
            std::ostringstream msg;
            msg << "Size mismatch, " << sv.size() << " != " << temp.size()
                << ", please check your parameters or set vector";
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "phylanx::execution_tree::primitives::"
                "set_operation::set1d",
                util::generate_error_message(msg.str(),
                    name_, codename_));
        }

        sv = temp;
        return primitive_argument_type{ir::node_data<double>(args[0].vector())};
    }

    primitive_argument_type set_operation::set2d(args_type&& args) const
    {
        std::int64_t row_start = args[1].scalar();
        std::int64_t row_stop = args[2].scalar();
        std::int64_t step_row = args[3].scalar();
        if(step_row == 0)
        {
            row_stop = row_start + 1;
            step_row = 1;
        }
        std::int64_t col_start = args[4].scalar();
        std::int64_t col_stop = args[5].scalar();
        std::int64_t step_col = args[6].scalar();
        if(step_col == 0)
        {
            col_stop = col_start + 1;
            step_col = 1;
        }
        std::size_t num_matrix_rows = args[0].dimensions()[0];
        std::size_t num_matrix_cols = args[0].dimensions()[1];
        std::size_t value_dimnum = args[7].num_dimensions();
        auto matrix_data = args[0].matrix();

        if (step_row == 0 || step_col == 0)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "phylanx::execution_tree::primitives::"
                "set_operation::set2d",
                util::generate_error_message(
                    "argument 'step_row' or 'step_col' can not be zero",
                    name_, codename_));
        }

        if (!check_set_parameters(
                row_start, row_stop, step_row, num_matrix_rows))
        {
            std::ostringstream msg;
            msg << "argument 'row_start' or 'row_stop' are not valid: ";
            msg << "start=" << row_start << ", stop="
                << row_stop << ", step=" << step_row;
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "phylanx::execution_tree::primitives::"
                "set_operation::set2d",
                util::generate_error_message(
                    msg.str(),
                    name_, codename_));
        }

        if (!check_set_parameters(
                col_start, col_stop, step_col, num_matrix_cols))
        {
            std::ostringstream msg;
            msg << "argument 'col_start' or 'col_stop' are not valid: ";
            msg << "start=" << col_start << ", stop="
                << col_stop << ", step=" << step_col;
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "phylanx::execution_tree::primitives::"
                "set_operation::set2d",
                util::generate_error_message(
                    msg.str(),
                    name_, codename_));
        }

        auto init_list_row = create_list_set(
            row_start, row_stop, step_row, num_matrix_rows);
        auto init_list_col = create_list_set(
            col_start, col_stop, step_col, num_matrix_cols);

        auto sm_row = blaze::rows(matrix_data, init_list_row);
        auto sm = blaze::columns(sm_row, init_list_col);

        if (value_dimnum == 0)
        {
            blaze::DynamicMatrix<double> data(
                sm.rows(), sm.columns(), args[7].scalar());
            sm = data;
            return primitive_argument_type{
                ir::node_data<double>(args[0].matrix())};
        }

        if (value_dimnum == 1)
        {
            auto input_vector = args[7].vector();
            auto data = blaze::trans(input_vector);
            std::size_t data_size = data.size();
            std::size_t num_cols = sm.columns();
            std::size_t num_rows = sm.rows();
            blaze::DynamicMatrix<double> temp(sm.rows(), sm.columns());

            if (data_size != num_cols)
            {
                HPX_THROW_EXCEPTION(hpx::bad_parameter,
                    "phylanx::execution_tree::primitives::"
                    "set_operation::set2d",
                    util::generate_error_message(
                        "size of set vector does not match the number "
                        "of columns in the input matrix",
                        name_, codename_));
            }

            for (std::size_t j = 0; j < num_rows; j++)
            {
                blaze::row(temp, j) = data;
            }

            sm = temp;

            return primitive_argument_type{
                ir::node_data<double>(args[0].matrix())};
        }

        auto data = args[7].matrix();
        std::size_t data_rows = data.rows();
        std::size_t data_cols = data.columns();
        std::size_t num_cols = sm.columns();
        std::size_t num_rows = sm.rows();
        if (data_rows != num_rows || data_cols != num_cols)
        {
            std::ostringstream msg;
            msg << "matrix sizes don't match: ";
            msg << " data.shape()==[" << data.rows() << "," << data.columns() << "]";
            msg << " sm.shape()==[" << sm.rows() << "," << sm.columns() << "]";
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "phylanx::execution_tree::primitives::"
                "set_operation::set2d",
                util::generate_error_message(
                    msg.str(), name_, codename_));
        }
        blaze::DynamicMatrix<double> temp(data);
        sm = temp;
        return primitive_argument_type{
            ir::node_data<double>(args[0].matrix())};
    }

    hpx::future<primitive_argument_type> set_operation::eval(
        primitive_arguments_type const& operands,
        primitive_arguments_type const& args) const
    {
        if (operands.size() != 8)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "phylanx::execution_tree::primitives::"
                "set_operation::set_operation",
                util::generate_error_message(
                    "the set_operation primitive requires "
                    "eight arguments",
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
                "set_operation::eval",
                util::generate_error_message(
                    "the set_operation primitive requires "
                    "that the arguments given by the operands "
                    "array are valid",
                    name_, codename_));
        }

        auto this_ = this->shared_from_this();
        return hpx::dataflow(hpx::launch::sync,
            hpx::util::unwrapping(
                [this_](args_type&& args) -> primitive_argument_type {
                    std::size_t lhs_dims = args[0].num_dimensions();
                    switch (lhs_dims)
                    {
                    case 0:
                        return this_->set0d(std::move(args));

                    case 1:
                        return this_->set1d(std::move(args));

                    case 2:
                        return this_->set2d(std::move(args));

                    default:
                        HPX_THROW_EXCEPTION(hpx::bad_parameter,
                            "set_operation::eval",
                            util::generate_error_message(
                                "left hand side operand has "
                                "unsupported "
                                "number of dimensions",
                                this_->name_, this_->codename_));
                    }
                }),
            detail::map_operands(
                operands, functional::numeric_operand{}, args,
                name_, codename_));
    }

    //////////////////////////////////////////////////////////////////////////
    hpx::future<primitive_argument_type> set_operation::eval(
        primitive_arguments_type const& args) const
    {
        if (this->no_operands())
        {
            return eval(args, noargs);
        }
        return eval(this->operands(), args);
    }
}}}
