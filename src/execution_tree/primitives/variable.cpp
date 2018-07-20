//  Copyright (c) 2017-2018 Hartmut Kaiser
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>
#include <phylanx/execution_tree/primitives/variable.hpp>

#include <hpx/include/lcos.hpp>
#include <hpx/include/util.hpp>
#include <hpx/throw_exception.hpp>
#include <hpx/util/unlock_guard.hpp>

#include <cstddef>
#include <memory>
#include <mutex>
#include <set>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

///////////////////////////////////////////////////////////////////////////////
namespace phylanx { namespace execution_tree { namespace primitives
{
    ///////////////////////////////////////////////////////////////////////////
    primitive create_variable(hpx::id_type const& locality,
        primitive_argument_type&& operand, std::string const& name,
        std::string const& codename)
    {
        static std::string type("variable");
        return create_primitive_component(
            locality, type, std::move(operand), name, codename);
    }

    match_pattern_type const variable::match_data =
    {
        hpx::util::make_tuple("variable",
            std::vector<std::string>{},
            nullptr, &create_primitive<variable>)
    };

    ///////////////////////////////////////////////////////////////////////////
    variable::variable(std::vector<primitive_argument_type>&& operands,
            std::string const& name, std::string const& codename)
      : primitive_component_base(std::move(operands), name, codename, true)
      , value_set_(false)
    {
        if (operands_.size() > 1)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "variable::variable",
                generate_error_message(
                    "the variable primitive requires no more than one operand"));
        }

        if (!operands_.empty())
        {
            operands_[0] = extract_copy_value(std::move(operands_[0]));
            value_set_ = true;
        }
    }

    hpx::future<primitive_argument_type> variable::eval(
        std::vector<primitive_argument_type> const& args) const
    {
        if (!value_set_ && !valid(bound_value_))
        {
            HPX_THROW_EXCEPTION(hpx::invalid_status,
                "variable::eval",
                generate_error_message(
                    "the expression representing the variable target "
                        "has not been initialized"));
        }

        primitive_argument_type const& target =
            valid(bound_value_) ? bound_value_ : operands_[0];
        return hpx::make_ready_future(extract_ref_value(target));
    }

    bool variable::bind(std::vector<primitive_argument_type> const& args) const
    {
        if (!value_set_)
        {
            HPX_THROW_EXCEPTION(hpx::invalid_status,
                "variable::bind",
                generate_error_message(
                    "the expression representing the variable target "
                        "has not been initialized"));
        }

        primitive const* p = util::get_if<primitive>(&operands_[0]);
        if (p != nullptr)
        {
            bound_value_ = extract_copy_value(p->eval(hpx::launch::sync, args));
        }
        else
        {
            bound_value_ = extract_ref_value(operands_[0]);
        }

        return true;
    }

    void variable::store(primitive_argument_type&& data)
    {
        if (!value_set_ || !valid(operands_[0]))
        {
            operands_[0] = extract_copy_value(std::move(data));
            value_set_ = true;
        }
        else
        {
            bound_value_ = extract_copy_value(std::move(data));
        }
    }

    void variable::store_set_1d(
        ir::node_data<double>&& data_to_set, std::vector<int64_t>&& list)
    {
        auto input_vector = extract_numeric_value(bound_value_).vector();
        std::size_t value_dimnum = data_to_set.num_dimensions();

        auto sv = blaze::elements(input_vector, list);

        if (value_dimnum == 0)
        {
            blaze::DynamicVector<double> temp(sv.size(), data_to_set.scalar());
            sv = temp;
        }
        else
        {
            auto temp = data_to_set.vector();

            if (sv.size() != temp.size())
            {
                std::ostringstream msg;
                msg << "Size mismatch, " << sv.size() << " != " << temp.size()
                    << ", please check your parameters or set vector";
                HPX_THROW_EXCEPTION(hpx::bad_parameter,
                    "phylanx::execution_tree::primitives::"
                    "store_operation::set1d",
                    execution_tree::generate_error_message(
                        msg.str(), name_, codename_));
            }

            sv = temp;
        }
    }

    void variable::store_set_2d(ir::node_data<double>&& data_to_set,
        std::vector<int64_t>&& list_row, std::vector<int64_t>&& list_col)
    {
        auto matrix_data = extract_numeric_value(bound_value_).matrix();
        std::size_t value_dimnum = data_to_set.num_dimensions();
        auto sm_row = blaze::rows(matrix_data, list_row);
        auto sm = blaze::columns(sm_row, list_col);

        if (value_dimnum == 0)
        {
            blaze::DynamicMatrix<double> data(
                sm.rows(), sm.columns(), data_to_set.scalar());
            sm = data;
        }
        else
        {
            if (value_dimnum == 1)
            {
                auto input_vector = data_to_set.vector();
                auto data = blaze::trans(input_vector);
                std::size_t data_size = data.size();
                std::size_t num_cols = sm.columns();
                std::size_t num_rows = sm.rows();
                blaze::DynamicMatrix<double> temp(sm.rows(), sm.columns());

                if (data_size != num_cols)
                {
                    HPX_THROW_EXCEPTION(hpx::bad_parameter,
                        "phylanx::execution_tree::primitives::"
                        "store_operation::set2d",
                        execution_tree::generate_error_message(
                            "size of set vector does not match the number "
                            "of columns in the input matrix",
                            name_, codename_));
                }

                for (std::size_t j = 0; j < num_rows; j++)
                {
                    blaze::row(temp, j) = data;
                }

                sm = temp;
            }
            else
            {
                auto data = data_to_set.matrix();
                std::size_t data_rows = data.rows();
                std::size_t data_cols = data.columns();
                std::size_t num_cols = sm.columns();
                std::size_t num_rows = sm.rows();
                if (data_rows != num_rows || data_cols != num_cols)
                {
                    std::ostringstream msg;
                    msg << "matrix sizes don't match: ";
                    msg << " data.shape()==[" << data.rows() << ","
                        << data.columns() << "]";
                    msg << " sm.shape()==[" << sm.rows() << "," << sm.columns()
                        << "]";
                    HPX_THROW_EXCEPTION(hpx::bad_parameter,
                        "phylanx::execution_tree::primitives::"
                        "store_operation::set2d",
                        execution_tree::generate_error_message(
                            msg.str(), name_, codename_));
                }
                blaze::DynamicMatrix<double> temp(data);
                sm = temp;
            }
        }
    }

    topology variable::expression_topology(std::set<std::string>&& functions,
        std::set<std::string>&& resolve_children) const
    {
        if (functions.find(name_) != functions.end())
        {
            return {};      // avoid recursion
        }

        primitive const* p = util::get_if<primitive>(&operands_[0]);
        if (p != nullptr)
        {
            functions.insert(name_);
            return p->expression_topology(hpx::launch::sync,
                std::move(functions), std::move(resolve_children));
        }
        return {};
    }
}}}

