// Copyright (c) 2017-2018 Parsa Amini
// Copyright (c) 2017-2018 Hartmut Kaiser
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>
#include <phylanx/ir/node_data.hpp>
#include <phylanx/plugins/matrixops/power_operation.hpp>

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
    match_pattern_type const power_operation::match_data =
    {
        hpx::util::make_tuple("power",
            std::vector<std::string>{"power(_1, _2)"},
            &create_power_operation, &create_primitive<power_operation>,
            "base, pow\n"
            "Args:\n"
            "\n"
            "    base (float) : the base of the exponent\n"
            "    pow (float) : the power of the exponent\n"
            "\n"
            "Returns:\n"
            "\n"
            "The value `base`**`pow`."
            )
    };

    ///////////////////////////////////////////////////////////////////////////
    power_operation::power_operation(
            primitive_arguments_type&& operands,
            std::string const& name, std::string const& codename)
      : primitive_component_base(std::move(operands), name, codename)
    {}

    ///////////////////////////////////////////////////////////////////////////
    primitive_argument_type power_operation::power0d(
        operand_type&& lhs, operand_type&& rhs) const
    {
        lhs = double(std::pow(lhs.scalar(), rhs[0]));
        return primitive_argument_type{std::move(lhs)};
    }

    primitive_argument_type power_operation::power1d(
        operand_type&& lhs, operand_type&& rhs) const
    {
        if (lhs.is_ref())
        {
            lhs = blaze::pow(lhs.vector(), rhs[0]);
        }
        else
        {
            lhs.vector() = blaze::pow(lhs.vector(), rhs[0]);
        }
        return primitive_argument_type{std::move(lhs)};
    }

    primitive_argument_type power_operation::power2d(
        operand_type&& lhs, operand_type&& rhs) const
    {
        if (lhs.is_ref())
        {
            lhs = blaze::pow(lhs.matrix(), rhs[0]);
        }
        else
        {
            lhs.matrix() = blaze::pow(lhs.matrix(), rhs[0]);
        }
        return primitive_argument_type{std::move(lhs)};
    }

    hpx::future<primitive_argument_type> power_operation::eval(
        primitive_arguments_type const& operands,
        primitive_arguments_type const& args, eval_context ctx) const
    {
        if (operands.size() != 2)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "power_operation::eval",
                generate_error_message(
                    "the power_operation primitive requires "
                        "exactly two operands"));
        }

        if (!valid(operands[0]) || !valid(operands[1]))
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "power_operation::eval",
                generate_error_message(
                    "the power_operation primitive requires "
                        "that the arguments given by the operands "
                        "array are valid"));
        }

        auto this_ = this->shared_from_this();
        return hpx::dataflow(hpx::launch::sync, hpx::util::unwrapping(
            [this_ = std::move(this_)](operand_type&& op1, operand_type&& op2)
            ->  primitive_argument_type
            {
                if (op2.num_dimensions() != 0)
                {
                    HPX_THROW_EXCEPTION(hpx::bad_parameter,
                        "power_operation::eval",
                        this_->generate_error_message(
                            "right hand side operand has to be a scalar value"));
                }

                switch (op1.num_dimensions())
                {
                case 0:
                    return this_->power0d(std::move(op1), std::move(op2));

                case 1:
                    return this_->power1d(std::move(op1), std::move(op2));

                case 2:
                    return this_->power2d(std::move(op1), std::move(op2));

                default:
                    HPX_THROW_EXCEPTION(hpx::bad_parameter,
                        "power_operation::eval",
                        this_->generate_error_message(
                            "left hand side operand has unsupported "
                                "number of dimensions"));
                }
            }),
            numeric_operand(operands[0], args, name_, codename_, ctx),
            numeric_operand(operands[1], args, name_, codename_, ctx));
    }
}}}
