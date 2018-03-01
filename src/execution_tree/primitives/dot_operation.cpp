//  Copyright (c) 2017-2018 Hartmut Kaiser
//  Copyright (c) 2017 Parsa Amini
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>
#include <phylanx/execution_tree/primitives/dot_operation.hpp>
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
    primitive create_dot_operation(hpx::id_type const& locality,
        std::vector<primitive_argument_type>&& operands,
        std::string const& name, std::string const& codename)
    {
        static std::string type("dot");
        return create_primitive_component(
            locality, type, std::move(operands), name, codename);
    }

    match_pattern_type const dot_operation::match_data =
    {
        hpx::util::make_tuple("dot",
            std::vector<std::string>{"dot(_1, _2)"},
            &create_dot_operation, &create_primitive<dot_operation>)
    };

    ///////////////////////////////////////////////////////////////////////////
    dot_operation::dot_operation(
            std::vector<primitive_argument_type>&& operands,
            std::string const& name, std::string const& codename)
      : primitive_component_base(std::move(operands), name, codename)
    {}

    ///////////////////////////////////////////////////////////////////////////

    primitive_argument_type dot_operation::dot0d(operands_type && ops) const
    {
        if (ops[1].num_dimensions() != 0UL)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "dot_operation::dot0d",
                execution_tree::generate_error_message(
                    "the operands have incompatible number of "
                        "dimensions",
                    name_, codename_));
        }

        ops[0].scalar() *= ops[1].scalar();
        return primitive_argument_type{
            ir::node_data<double>{std::move(ops[0])}};
    }

    // lhs_num_dims == 1
    // Case 1: Inner product of two vectors
    // Case 2: Inner product of a vector and an array of vectors
    primitive_argument_type dot_operation::dot1d(operands_type && ops) const
    {
        operand_type& lhs = ops[0];
        operand_type& rhs = ops[1];

        switch (rhs.num_dimensions())
        {
        case 1:
            // If is_vector(lhs) && is_vector(rhs)
            return dot1d1d(lhs, rhs);

        case 2:
            // If is_vector(lhs) && is_matrix(rhs)
            return dot1d2d(lhs, rhs);

        case 0: HPX_FALLTHROUGH;
        default:
            // lhs_order == 1 && rhs_order != 2
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "dot_operation::dot1d",
                execution_tree::generate_error_message(
                    "the operands have incompatible number of "
                        "dimensions",
                    name_, codename_));
        }
    }

    primitive_argument_type dot_operation::dot1d1d(
        operand_type& lhs, operand_type& rhs) const
    {
        if (lhs.size() != rhs.size())
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "dot_operation::dot1d1d",
                execution_tree::generate_error_message(
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
        operand_type& lhs, operand_type& rhs) const
    {
        if (lhs.size() != rhs.dimension(0))
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "dot_operation::dot1d2d",
                execution_tree::generate_error_message(
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
    primitive_argument_type dot_operation::dot2d(operands_type && ops) const
    {
        operand_type& lhs = ops[0];
        operand_type& rhs = ops[1];

        switch (rhs.num_dimensions())
        {
        case 1:
            return dot2d1d(lhs, rhs);

        case 2:
            return dot2d2d(lhs, rhs);

        case 0: HPX_FALLTHROUGH;
        default:
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "dot_operation::dot2d",
                execution_tree::generate_error_message(
                    "the operands have incompatible number of "
                        "dimensions",
                    name_, codename_));
        }
    }

    primitive_argument_type dot_operation::dot2d1d(
        operand_type& lhs, operand_type& rhs) const
    {
        if (lhs.dimension(1) != rhs.size())
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "dot_operation::dot2d1d",
                execution_tree::generate_error_message(
                    "the operands have incompatible number of "
                        "dimensions",
                    name_, codename_));
        }

        rhs = lhs.matrix() * rhs.vector();
        return primitive_argument_type{
            ir::node_data<double>{std::move(rhs)}};
    }

    primitive_argument_type dot_operation::dot2d2d(
        operand_type& lhs, operand_type& rhs) const
    {
        if (lhs.dimension(1) != rhs.dimension(0))
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "dot_operation::dot2d2d",
                execution_tree::generate_error_message(
                    "the operands have incompatible number of "
                        "dimensions",
                    name_, codename_));
        }

        lhs = lhs.matrix() * rhs.matrix();
        return primitive_argument_type{
            ir::node_data<double>{std::move(lhs)}};
    }

    hpx::future<primitive_argument_type> dot_operation::eval(
        std::vector<primitive_argument_type> const& operands,
        std::vector<primitive_argument_type> const& args) const
    {
        if (operands.size() != 2)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "dot_operation::eval",
                execution_tree::generate_error_message(
                    "the dot_operation primitive requires exactly "
                        "two operands",
                    name_, codename_));
        }

        if (!valid(operands[0]) || !valid(operands[1]))
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "dot_operation::eval",
                execution_tree::generate_error_message(
                    "the dot_operation primitive requires that the "
                        "arguments given by the operands array are "
                        "valid",
                    name_, codename_));
        }

        auto this_ = this->shared_from_this();
        return hpx::dataflow(hpx::util::unwrapping(
            [this_](operands_type&& ops) -> primitive_argument_type
            {
                std::size_t dims = ops[0].num_dimensions();
                switch (dims)
                {
                case 0:
                    return this_->dot0d(std::move(ops));

                case 1:
                    return this_->dot1d(std::move(ops));

                case 2:
                    return this_->dot2d(std::move(ops));

                default:
                    HPX_THROW_EXCEPTION(hpx::bad_parameter,
                        "dot_operation::eval",
                        execution_tree::generate_error_message(
                            "left hand side operand has unsupported "
                                "number of dimensions",
                            this_->name_, this_->codename_));
                }
            }),
            detail::map_operands(
                operands, functional::numeric_operand{}, args,
                name_, codename_));
    }

    // implement 'dot' for all possible combinations of lhs and rhs
    hpx::future<primitive_argument_type> dot_operation::eval(
        std::vector<primitive_argument_type> const& args) const
    {
        if (operands_.empty())
        {
            return eval(args, noargs);
        }
        return eval(operands_, args);
    }
}}}
