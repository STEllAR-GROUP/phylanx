//  Copyright (c) 2017-2018 Hartmut Kaiser
//  Copyright (c) 2017 Parsa Amini
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>
#include <phylanx/ir/node_data.hpp>
#include <phylanx/plugins/matrixops/dot_operation.hpp>

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
    match_pattern_type const dot_operation::match_data =
    {
        hpx::util::make_tuple("dot",
            std::vector<std::string>{"dot(_1, _2)"},
            &create_dot_operation, &create_primitive<dot_operation>,
            "v1, v2\n"
            "Args:\n"
            "\n"
            "    v1 (vector) : a vector\n"
            "    v2 (vector) : a vector\n"
            "\n"
            "Returns:\n"
            "\n"
            "The dot product of vector `v1` and `v2`."
            )
    };

    ///////////////////////////////////////////////////////////////////////////
    dot_operation::dot_operation(
            primitive_arguments_type&& operands,
            std::string const& name, std::string const& codename)
      : primitive_component_base(std::move(operands), name, codename)
    {}

    ///////////////////////////////////////////////////////////////////////////
    primitive_argument_type dot_operation::dot0d0d(
        operand_type&& lhs, operand_type&& rhs) const
    {
        lhs.scalar() *= rhs.scalar();
        return primitive_argument_type{ir::node_data<double>{std::move(lhs)}};
    }

    primitive_argument_type dot_operation::dot0d1d(
        operand_type&& lhs, operand_type&& rhs) const
    {
        if (rhs.is_ref())
        {
            rhs = rhs.vector() * lhs.scalar();
        }
        else
        {
            rhs.vector() *= lhs.scalar();
        }

        return primitive_argument_type{std::move(rhs)};
    }

    primitive_argument_type dot_operation::dot0d2d(
        operand_type&& lhs, operand_type&& rhs) const
    {
        if (rhs.is_ref())
        {
            rhs = rhs.matrix() * lhs.scalar();
        }
        else
        {
            rhs.matrix() *= lhs.scalar();
        }

        return primitive_argument_type{std::move(rhs)};
    }

    primitive_argument_type dot_operation::dot0d(
        operand_type&& lhs, operand_type&& rhs) const
    {
        switch (rhs.num_dimensions())
        {
        case 0:
            // If is_scalar(lhs) && is_scalar(rhs)
            return dot0d0d(std::move(lhs), std::move(rhs));

        case 1:
            // If is_scalar(lhs) && is_vector(rhs)
            return dot0d1d(std::move(lhs), std::move(rhs));

        case 2:
            // If is_scalar(lhs) && is_matrix(rhs)
            return dot0d2d(std::move(lhs), std::move(rhs));

        default:
            // lhs_order == 1 && rhs_order != 2
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "dot_operation::dot0d",
                util::generate_error_message(
                    "the operands have incompatible number of "
                    "dimensions",
                    name_, codename_));
        }
    }

    // lhs_num_dims == 1
    // Case 1: Inner product of two vectors
    // Case 2: Inner product of a vector and an array of vectors
    primitive_argument_type dot_operation::dot1d(
        operand_type&& lhs, operand_type&& rhs) const
    {
        switch (rhs.num_dimensions())
        {
        case 0:
            // If is_vector(lhs) && is_scalar(rhs)
            return dot1d0d(std::move(lhs), std::move(rhs));

        case 1:
            // If is_vector(lhs) && is_vector(rhs)
            return dot1d1d(std::move(lhs), std::move(rhs));

        case 2:
            // If is_vector(lhs) && is_matrix(rhs)
            return dot1d2d(std::move(lhs), std::move(rhs));

        default:
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "dot_operation::dot1d",
                util::generate_error_message(
                    "the operands have incompatible number of "
                        "dimensions",
                    name_, codename_));
        }
    }

    primitive_argument_type dot_operation::dot1d0d(
        operand_type&& lhs, operand_type&& rhs) const
    {
        if (lhs.is_ref())
        {
            lhs = lhs.vector() * rhs.scalar();
        }
        else
        {
            lhs.vector() *= rhs.scalar();
        }

        return primitive_argument_type{ir::node_data<double>{std::move(lhs)}};
    }

    primitive_argument_type dot_operation::dot1d1d(
        operand_type&& lhs, operand_type&& rhs) const
    {
        if (lhs.size() != rhs.size())
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "dot_operation::dot1d1d",
                util::generate_error_message(
                    "the operands have incompatible number of "
                        "dimensions",
                    name_, codename_));
        }

        // lhs.dimension(0) == rhs.dimension(0)
        lhs = double(blaze::dot(lhs.vector(), rhs.vector()));
        return primitive_argument_type{
            ir::node_data<double>{std::move(lhs)}};
    }

    primitive_argument_type dot_operation::dot1d2d(
        operand_type&& lhs, operand_type&& rhs) const
    {
        if (lhs.size() != rhs.dimension(0))
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "dot_operation::dot1d2d",
                util::generate_error_message(
                    "the operands have incompatible number of "
                        "dimensions",
                    name_, codename_));
        }

        lhs = blaze::trans(blaze::trans(lhs.vector()) * rhs.matrix());
        return primitive_argument_type{
            ir::node_data<double>{std::move(lhs)}};
    }

    // lhs_num_dims == 2
    // Multiply a matrix with a vector
    // Regular matrix multiplication
    primitive_argument_type dot_operation::dot2d(
        operand_type&& lhs, operand_type&& rhs) const
    {
        switch (rhs.num_dimensions())
        {
        case 0:
            // If is_matrix(lhs) && is_scalar(rhs)
            return dot2d0d(std::move(lhs), std::move(rhs));

        case 1:
            // If is_matrix(lhs) && is_vector(rhs)
            return dot2d1d(std::move(lhs), std::move(rhs));

        case 2:
            // If is_matrix(lhs) && is_matrix(rhs)
            return dot2d2d(std::move(lhs), std::move(rhs));

        default:
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "dot_operation::dot2d",
                util::generate_error_message(
                    "the operands have incompatible number of "
                        "dimensions",
                    name_, codename_));
        }
    }

    primitive_argument_type dot_operation::dot2d0d(
        operand_type&& lhs, operand_type&& rhs) const
    {
        lhs = lhs.matrix() * rhs.scalar();
        return primitive_argument_type{ir::node_data<double>{std::move(lhs)}};
    }

    primitive_argument_type dot_operation::dot2d1d(
        operand_type&& lhs, operand_type&& rhs) const
    {
        if (lhs.dimension(1) != rhs.size())
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "dot_operation::dot2d1d",
                util::generate_error_message(
                    "the operands have incompatible number of "
                        "dimensions",
                    name_, codename_));
        }

        rhs = lhs.matrix() * rhs.vector();
        return primitive_argument_type{
            ir::node_data<double>{std::move(rhs)}};
    }

    primitive_argument_type dot_operation::dot2d2d(
        operand_type&& lhs, operand_type&& rhs) const
    {
        if (lhs.dimension(1) != rhs.dimension(0))
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "dot_operation::dot2d2d",
                util::generate_error_message(
                    "the operands have incompatible number of "
                        "dimensions",
                    name_, codename_));
        }

        lhs = lhs.matrix() * rhs.matrix();
        return primitive_argument_type{
            ir::node_data<double>{std::move(lhs)}};
    }

    hpx::future<primitive_argument_type> dot_operation::eval(
        primitive_arguments_type const& operands,
        primitive_arguments_type const& args) const
    {
        if (operands.size() != 2)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "dot_operation::eval",
                util::generate_error_message(
                    "the dot_operation primitive requires exactly "
                        "two operands",
                    name_, codename_));
        }

        if (!valid(operands[0]) || !valid(operands[1]))
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "dot_operation::eval",
                util::generate_error_message(
                    "the dot_operation primitive requires that the "
                        "arguments given by the operands array are "
                        "valid",
                    name_, codename_));
        }

        auto this_ = this->shared_from_this();
        return hpx::dataflow(hpx::launch::sync, hpx::util::unwrapping(
            [this_ = std::move(this_)](operand_type&& op1, operand_type&& op2)
            ->  primitive_argument_type
            {
                std::size_t dims = op1.num_dimensions();
                switch (dims)
                {
                case 0:
                    return this_->dot0d(std::move(op1), std::move(op2));

                case 1:
                    return this_->dot1d(std::move(op1), std::move(op2));

                case 2:
                    return this_->dot2d(std::move(op1), std::move(op2));

                default:
                    HPX_THROW_EXCEPTION(hpx::bad_parameter,
                        "dot_operation::eval",
                        util::generate_error_message(
                            "left hand side operand has unsupported "
                                "number of dimensions",
                            this_->name_, this_->codename_));
                }
            }),
            numeric_operand(operands[0], args, name_, codename_),
            numeric_operand(operands[1], args, name_, codename_));
    }

    // implement 'dot' for all possible combinations of lhs and rhs
    hpx::future<primitive_argument_type> dot_operation::eval(
        primitive_arguments_type const& args, eval_context) const
    {
        if (this->no_operands())
        {
            return eval(args, noargs);
        }
        return eval(this->operands(), args);
    }
}}}
