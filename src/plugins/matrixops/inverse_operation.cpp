// Copyright (c) 2017-2018 Hartmut Kaiser
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>
#include <phylanx/ir/node_data.hpp>
#include <phylanx/plugins/matrixops/inverse_operation.hpp>

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
    match_pattern_type const inverse_operation::match_data =
    {
        hpx::util::make_tuple("inverse",
            std::vector<std::string>{"inverse(_1)"},
            &create_inverse_operation, &create_primitive<inverse_operation>,
            "m\n"
            "Args:\n"
            "\n"
            "    m (matrix): a matrix\n"
            "\n"
            "Returns:\n"
            "\n"
            "The inverse of m."
            )
    };

    ///////////////////////////////////////////////////////////////////////////
    inverse_operation::inverse_operation(
            primitive_arguments_type&& operands,
            std::string const& name, std::string const& codename)
      : primitive_component_base(std::move(operands), name, codename)
    {}

    ///////////////////////////////////////////////////////////////////////////
    primitive_argument_type inverse_operation::inverse0d(
        operand_type&& op) const
    {
        op.scalar() = 1 / op.scalar();
        return primitive_argument_type{std::move(op)};
    }

    primitive_argument_type inverse_operation::inverse2d(
        operand_type&& op) const
    {
        if (op.dimension(0) != op.dimension(1))
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "inverse::inverse2d",
                util::generate_error_message(
                    "matrices to inverse have to be quadratic",
                    name_, codename_));
        }

        if (op.is_ref())
        {
            op = blaze::inv(op.matrix());
        }
        else
        {
            blaze::invert(op.matrix_non_ref());
        }
        return primitive_argument_type{std::move(op)};
    }

    ///////////////////////////////////////////////////////////////////////////
    hpx::future<primitive_argument_type> inverse_operation::eval(
        primitive_arguments_type const& operands,
        primitive_arguments_type const& args, eval_context ctx) const
    {
        if (operands.size() != 1)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "inverse_operation::eval",
                generate_error_message(
                    "the inverse_operation primitive requires"
                        "exactly one operand"));
        }

        if (!valid(operands[0]))
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "inverse_operation::eval",
                generate_error_message(
                    "the inverse_operation primitive requires that "
                        "the arguments given by the operands array is valid"));
        }

        auto this_ = this->shared_from_this();
        return hpx::dataflow(hpx::launch::sync, hpx::util::unwrapping(
            [this_ = std::move(this_)](operand_type&& op)
            -> primitive_argument_type
            {
                std::size_t dims = op.num_dimensions();
                switch (dims)
                {
                case 0:
                    return this_->inverse0d(std::move(op));

                case 2:
                    return this_->inverse2d(std::move(op));

                case 1: HPX_FALLTHROUGH;
                default:
                    HPX_THROW_EXCEPTION(hpx::bad_parameter,
                        "inverse_operation::eval",
                        this_->generate_error_message(
                            "left hand side operand has unsupported "
                                "number of dimensions"));
                }
            }),
            numeric_operand(operands[0], args,
                name_, codename_, std::move(ctx)));
    }
}}}
