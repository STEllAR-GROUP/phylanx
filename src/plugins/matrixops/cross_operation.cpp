// Copyright (c) 2017 Parsa Amini
// Copyright (c) 2017-2018 Hartmut Kaiser
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>
#include <phylanx/ir/node_data.hpp>
#include <phylanx/execution_tree/primitives/node_data_helpers.hpp>
#include <phylanx/plugins/matrixops/cross_operation.hpp>

#include <hpx/include/lcos.hpp>
#include <hpx/include/naming.hpp>
#include <hpx/include/util.hpp>
#include <hpx/throw_exception.hpp>

#include <cstddef>
#include <memory>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

///////////////////////////////////////////////////////////////////////////////
namespace phylanx { namespace execution_tree { namespace primitives
{
    ///////////////////////////////////////////////////////////////////////////
    match_pattern_type const cross_operation::match_data =
    {
        match_pattern_type{"cross",
            std::vector<std::string>{"cross(_1, _2)"},
            &create_cross_operation, &create_primitive<cross_operation>, R"(
            v1, v2
            Args:

                v1 (vector) : a vector
                v2 (vector) : a vector

            Returns:

            The cross product of `v1` and `v2`.)"
        }
    };

    cross_operation::cross_operation(
            primitive_arguments_type&& operands,
            std::string const& name, std::string const& codename)
      : primitive_component_base(std::move(operands), name, codename)
    {}

    ///////////////////////////////////////////////////////////////////////////
    template <typename T>
    primitive_argument_type cross_operation::cross1d1d(
        ir::node_data<T>&& lhs, ir::node_data<T>&& rhs) const
    {
        std::size_t lhs_vector_dims = lhs.size();
        std::size_t rhs_vector_dims = rhs.size();

        if (lhs_vector_dims < 2ul || rhs_vector_dims < 2ul)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "cross_operation::cross1d1d",
                generate_error_message(
                    "operands have an invalid number of columns"));
        }

        // lhs vector has 2 elements
        if (lhs_vector_dims < 3ul)
        {
            if (lhs.is_ref())
            {
                blaze::DynamicVector<T> temp = lhs.vector();
                temp.resize(3ul);
                temp[2ul] = 0.0;
                lhs = std::move(temp);
            }
            else
            {
                lhs.vector_non_ref().resize(3ul);
                lhs[2ul] = 0.0;
            }
        }
        else if (lhs_vector_dims > 3)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "cross_operation::cross1d1d",
                generate_error_message(
                    "left hand side operand has more than 3 elements"));
        }

        // Only rhs has 2 elements
        if (rhs_vector_dims < 3ul)
        {
            if (rhs.is_ref())
            {
                blaze::DynamicVector<T> temp = rhs.vector();
                temp.resize(3ul);
                temp[2ul] = 0.0;
                rhs = std::move(temp);
            }
            else
            {
                rhs.vector_non_ref().resize(3ul);
                rhs[2ul] = 0.0;
            }
        }
        else if (rhs_vector_dims > 3)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "cross_operation::cross1d1d",
                generate_error_message(
                    "right hand side operand has more than 3 elements"));
        }

        // Both vectors have 3 elements
        lhs.vector() %= rhs.vector();
        return primitive_argument_type{ir::node_data<T>{std::move(lhs)}};
    }

    template <typename T>
    primitive_argument_type cross_operation::cross1d2d(
        ir::node_data<T>&& lhs, ir::node_data<T>&& rhs) const
    {
        if (lhs.size() == 2ul)
        {
            if (lhs.is_ref())
            {
                blaze::DynamicVector<T> temp = lhs.vector();
                temp.resize(3ul);
                temp[2ul] = 0.0;
                lhs = std::move(temp);
            }
            else
            {
                lhs.vector_non_ref().resize(3ul);
                lhs[2ul] = 0.0;
            }
        }

        // lhs has to have 3 elements
        if (lhs.size() == 3ul)
        {
            // rhs has 2 columns per vector
            if (rhs.dimension(1) == 2ul)
            {
                if (rhs.is_ref())
                {
                    blaze::DynamicMatrix<T> temp = rhs.matrix();
                    temp.resize(rhs.dimension(0), 3ul);
                    blaze::column(temp, 2ul) = 0.0;
                    rhs = std::move(temp);
                }
                else
                {
                    rhs.matrix_non_ref().resize(rhs.dimension(0), 3ul);
                    blaze::column(rhs.matrix_non_ref(), 2ul) = 0.0;
                }
            }

            for (std::size_t i = 0ul; i != rhs.dimension(0); ++i)
            {
                blaze::DynamicVector<T> rhs_op_vec{
                    rhs.at(i, 0ul), rhs.at(i, 1ul), rhs.at(i, 2ul)};

                auto i_vector = lhs.vector() % rhs_op_vec;

                rhs.at(i, 0ul) = i_vector[0ul];
                rhs.at(i, 1ul) = i_vector[1ul];
                rhs.at(i, 2ul) = i_vector[2ul];
            }

            return primitive_argument_type{ir::node_data<T>{std::move(rhs)}};
        }

        HPX_THROW_EXCEPTION(hpx::bad_parameter,
            "cross_operation::cross1d2d",
            generate_error_message(
                "operand vectors have an invalid number of elements"));
    }

    template <typename T>
    primitive_argument_type cross_operation::cross1d(
        ir::node_data<T>&& lhs, ir::node_data<T>&& rhs) const
    {
        switch (rhs.num_dimensions())
        {
        case 1:
            return cross1d1d(std::move(lhs), std::move(rhs));

        case 2:
            return cross1d2d(std::move(lhs), std::move(rhs));

        default:
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "cross_operation::cross1d",
                generate_error_message(
                    "right hand side operand has unsupported number of "
                        "dimensions"));
        }
    }

    ///////////////////////////////////////////////////////////////////////////
    template <typename T>
    primitive_argument_type cross_operation::cross2d1d(
        ir::node_data<T>&& lhs, ir::node_data<T>&& rhs) const
    {
        if (rhs.size() == 2ul)
        {
            if (rhs.is_ref())
            {
                blaze::DynamicVector<T> temp = rhs.vector();
                temp.resize(3ul);
                temp[2ul] = 0.0;
                rhs = std::move(temp);
            }
            else
            {
                rhs.vector_non_ref().resize(3ul);
                rhs[2ul] = 0.0;
            }
        }

        // rhs has to have 3 elements
        if (rhs.size() == 3ul)
        {
            // lhs has 2 columns per vector
            if (lhs.dimension(1) == 2ul)
            {
                if (lhs.is_ref())
                {
                    blaze::DynamicMatrix<T> temp = lhs.matrix();
                    temp.resize(lhs.dimension(0), 3ul);
                    blaze::column(temp, 2ul) = 0.0;
                    lhs = std::move(temp);
                }
                else
                {
                    lhs.matrix_non_ref().resize(lhs.dimension(0), 3ul);
                    blaze::column(lhs.matrix_non_ref(), 2ul) = 0.0;
                }
            }

            auto lhsm = lhs.matrix();
            blaze::DynamicMatrix<T> rhs_op_mat{{rhs[0ul], rhs[1ul], rhs[2ul]}};
            blaze::DynamicMatrix<T> temp = lhs.matrix();

            for (std::size_t i = 0ul; i != lhs.dimension(0); ++i)
            {
                blaze::row(temp, i) = blaze::cross(
                    blaze::row(lhsm, i),
                    blaze::row(rhs_op_mat, 0ul));
            }

            return primitive_argument_type{ir::node_data<T>{std::move(temp)}};
        }

        HPX_THROW_EXCEPTION(hpx::bad_parameter,
            "cross_operation::cross2d1d",
            generate_error_message(
                "operand vectors have an invalid number of elements"));
    }

    template <typename T>
    primitive_argument_type cross_operation::cross2d2d(
        ir::node_data<T>&& lhs, ir::node_data<T>&& rhs) const
    {
        // Both matrices have to have the same number of rows
        if (lhs.dimension(0) != rhs.dimension(0))
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "cross_operation::cross2d",
                generate_error_message(
                    "operands have non-matching number of rows"));
        }

        // If both have 2 elements per vector
        if (lhs.dimension(1) == 2ul)
        {
            // If rhs has 2 elements per vector
            if (rhs.dimension(1) == 2ul)
            {
                auto lhsm = lhs.matrix();
                auto rhsm = rhs.matrix();
                blaze::DynamicMatrix<T> temp = lhs.matrix();

                for (std::size_t idx_row = 0; idx_row != lhs.dimension(0);
                     ++idx_row)
                {
                    blaze::row(temp, idx_row) = blaze::cross(
                        blaze::row(lhsm, idx_row),
                        blaze::row(rhsm, idx_row));
                }

                return primitive_argument_type{
                    ir::node_data<T>{std::move(temp)}};
            }

            // If only lhs has 2 elements per vector
            else
            {
                if (rhs.is_ref())
                {
                    blaze::DynamicMatrix<T> temp = lhs.matrix();
                    temp.resize(lhs.dimension(0), 3ul);
                    blaze::column(temp, 2ul) = 0.0;
                    lhs = std::move(temp);
                }
                else
                {
                    lhs.matrix_non_ref().resize(lhs.dimension(0), 3ul);
                    blaze::column(lhs.matrix_non_ref(), 2ul) = 0.0;
                }
            }
        }

        // If lhs has 3 elements per vector
        if (lhs.dimension(1) == 3ul)
        {
            // If rhs has 2 elements per vector
            if (rhs.dimension(1) == 2ul)
            {
                if (rhs.is_ref())
                {
                    blaze::DynamicMatrix<T> temp = rhs.matrix();
                    temp.resize(rhs.dimension(0), 3ul);
                    blaze::column(temp, 2ul) = 0.0;
                    rhs = std::move(temp);
                }
                else
                {
                    rhs.matrix_non_ref().resize(rhs.dimension(0), 3ul);
                    blaze::column(rhs.matrix_non_ref(), 2ul) = 0.0;
                }
            }

            // Both have 3 elements per vector
            auto lhsm = lhs.matrix();
            auto rhsm = rhs.matrix();
            blaze::DynamicMatrix<T> temp = lhs.matrix();

            for (std::size_t idx_row = 0; idx_row != lhs.dimension(0);
                ++idx_row)
            {
                blaze::row(temp, idx_row) = blaze::cross(
                    blaze::row(lhsm, idx_row),
                    blaze::row(rhsm, idx_row));
            }

            return primitive_argument_type{ir::node_data<T>{std::move(temp)}};
        }

        HPX_THROW_EXCEPTION(hpx::bad_parameter,
            "cross_operation::cross2d2d",
            generate_error_message(
                "operand vectors have an invalid number of elements"));
    }

    template <typename T>
    primitive_argument_type cross_operation::cross2d(
        ir::node_data<T>&& lhs, ir::node_data<T>&& rhs) const
    {
        switch (rhs.num_dimensions())
        {
        case 1:
            return cross2d1d(std::move(lhs), std::move(rhs));

        case 2:
            return cross2d2d(std::move(lhs), std::move(rhs));

        default:
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "cross_operation::cross2d",
                generate_error_message(
                    "right hand side operand has unsupported number of "
                        "dimensions"));
        }
    }

    ///////////////////////////////////////////////////////////////////////////
    primitive_argument_type cross_operation::cross1d(
        primitive_argument_type&& lhs, primitive_argument_type&& rhs) const
    {
        switch (extract_common_type(lhs, rhs))
        {
        case node_data_type_bool:
            return cross1d(
                extract_boolean_value(std::move(lhs), name_, codename_),
                extract_boolean_value(std::move(rhs), name_, codename_));

        case node_data_type_int64:
            return cross1d(
                extract_integer_value(std::move(lhs), name_, codename_),
                extract_integer_value(std::move(rhs), name_, codename_));

        case node_data_type_unknown: HPX_FALLTHROUGH;
        case node_data_type_double:
            return cross1d(
                extract_numeric_value(std::move(lhs), name_, codename_),
                extract_numeric_value(std::move(rhs), name_, codename_));

        default:
            break;
        }

        HPX_THROW_EXCEPTION(hpx::bad_parameter,
            "cross_operation::cross1d",
            generate_error_message(
                "the cross primitive requires for all arguments to "
                    "be numeric data types"));
    }

    primitive_argument_type cross_operation::cross2d(
        primitive_argument_type&& lhs, primitive_argument_type&& rhs) const
    {
        switch (extract_common_type(lhs, rhs))
        {
        case node_data_type_bool:
            return cross2d(
                extract_boolean_value(std::move(lhs), name_, codename_),
                extract_boolean_value(std::move(rhs), name_, codename_));

        case node_data_type_int64:
            return cross2d(
                extract_integer_value(std::move(lhs), name_, codename_),
                extract_integer_value(std::move(rhs), name_, codename_));

        case node_data_type_unknown: HPX_FALLTHROUGH;
        case node_data_type_double:
            return cross2d(
                extract_numeric_value(std::move(lhs), name_, codename_),
                extract_numeric_value(std::move(rhs), name_, codename_));

        default:
            break;
        }

        HPX_THROW_EXCEPTION(hpx::bad_parameter,
            "cross_operation::cross2d",
            generate_error_message(
                "the cross primitive requires for all arguments to "
                    "be numeric data types"));
    }

    ///////////////////////////////////////////////////////////////////////////
    hpx::future<primitive_argument_type> cross_operation::eval(
        primitive_arguments_type const& operands,
        primitive_arguments_type const& args, eval_context ctx) const
    {
        if (operands.size() != 2)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "cross_operation::eval",
                generate_error_message(
                    "the cross_operation primitive requires exactly two "
                        "operands"));
        }

        if (!valid(operands[0]) || !valid(operands[1]))
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "cross_operation::eval",
                generate_error_message(
                    "the cross_operation primitive requires that the "
                        "arguments given by the operands array are valid"));
        }

        auto&& op0 = value_operand(operands[0], args, name_, codename_, ctx);

        auto this_ = this->shared_from_this();
        return hpx::dataflow(hpx::launch::sync,
            [this_ = std::move(this_)](
                    hpx::future<primitive_argument_type>&& f1,
                    hpx::future<primitive_argument_type>&& f2)
            ->  primitive_argument_type
            {
                auto&& op1 = f1.get();
                auto&& op2 = f2.get();

                switch (extract_numeric_value_dimension(
                    op1, this_->name_, this_->codename_))
                {
                case 1:
                    return this_->cross1d(std::move(op1), std::move(op2));

                case 2:
                    return this_->cross2d(std::move(op1), std::move(op2));

                default:
                    HPX_THROW_EXCEPTION(hpx::bad_parameter,
                        "cross_operation::eval",
                        this_->generate_error_message(
                            "left hand side operand has unsupported "
                                "number of dimensions"));
                }
            },
            std::move(op0),
            value_operand(operands[1], args, name_, codename_, std::move(ctx)));
    }
}}}

