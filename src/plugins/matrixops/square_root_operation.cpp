// Copyright (c) 2017 Parsa Amini
// Copyright (c) 2017-2018 Hartmut Kaiser
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>
#include <phylanx/ir/node_data.hpp>
#include <phylanx/plugins/matrixops/square_root_operation.hpp>

#include <hpx/include/lcos.hpp>
#include <hpx/include/naming.hpp>
#include <hpx/include/util.hpp>
#include <hpx/throw_exception.hpp>

#include <cmath>
#include <cstddef>
#include <memory>
#include <string>
#include <utility>
#include <vector>

///////////////////////////////////////////////////////////////////////////////
namespace phylanx { namespace execution_tree { namespace primitives
{
    ///////////////////////////////////////////////////////////////////////////
    match_pattern_type const square_root_operation::match_data =
    {
        hpx::util::make_tuple("square_root",
            std::vector<std::string>{"square_root(_1, _2)"},
            &create_square_root_operation,
            &create_primitive<square_root_operation>)
    };

    ///////////////////////////////////////////////////////////////////////////
    square_root_operation::square_root_operation(
            primitive_arguments_type&& operands,
            std::string const& name, std::string const& codename)
      : primitive_component_base(std::move(operands), name, codename)
    {}

    ///////////////////////////////////////////////////////////////////////////
    primitive_argument_type square_root_operation::square_root_0d(
        operand_type&& op) const
    {
        op = std::sqrt(op.scalar());
        return primitive_argument_type{std::move(op)};
    }

    primitive_argument_type square_root_operation::square_root_1d(
        operand_type&& op) const
    {
        if (op.is_ref())
        {
            op = blaze::sqrt(op.vector());
        }
        else
        {
            op.vector() = blaze::sqrt(op.vector());
        }
        return primitive_argument_type{std::move(op)};
    }

    primitive_argument_type square_root_operation::square_root_2d(
        operand_type&& op) const
    {
        if (op.is_ref())
        {
            op = blaze::sqrt(op.matrix());
        }
        else
        {
            op.matrix() = blaze::sqrt(op.matrix());
        }
        return primitive_argument_type{std::move(op)};
    }

    hpx::future<primitive_argument_type> square_root_operation::eval(
        primitive_arguments_type const& operands,
        primitive_arguments_type const& args) const
    {
        if (operands.size() != 1)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "square_root_operation::eval",
                util::generate_error_message(
                    "the square_root_operation primitive "
                        "requires exactly one operand",
                    name_, codename_));
        }

        if (!valid(operands[0]))
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "square_root_operation::eval",
                util::generate_error_message(
                    "the square_root_operation primitive "
                        "requires that the argument given "
                        "by the operands array is valid",
                    name_, codename_));
        }

        auto this_ = this->shared_from_this();
        return hpx::dataflow(hpx::launch::sync, hpx::util::unwrapping(
            [this_](operand_type&& op) -> primitive_argument_type
            {

                switch (op.num_dimensions())
                {
                case 0:
                    return this_->square_root_0d(std::move(op));

                case 1:
                    return this_->square_root_1d(std::move(op));

                case 2:
                    return this_->square_root_2d(std::move(op));

                default:
                    HPX_THROW_EXCEPTION(hpx::bad_parameter,
                        "square_root_operation::eval",
                        util::generate_error_message(
                            "left hand side operand has unsupported "
                                "number of dimensions",
                            this_->name_, this_->codename_));
                }
            }),
            numeric_operand(operands[0], args, name_, codename_));
    }

    ///////////////////////////////////////////////////////////////////////////
    hpx::future<primitive_argument_type> square_root_operation::eval(
        primitive_arguments_type const& args) const
    {
        if (this->no_operands())
        {
            return eval(args, noargs);
        }
        return eval(this->operands(), args);
    }
}}}
