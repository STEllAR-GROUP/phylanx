// Copyright (c) 2017-2018 Hartmut Kaiser
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>
#include <phylanx/ir/node_data.hpp>
#include <phylanx/plugins/arithmetics/unary_minus_operation.hpp>

#include <hpx/include/lcos.hpp>
#include <hpx/include/naming.hpp>
#include <hpx/include/util.hpp>
#include <hpx/throw_exception.hpp>

#include <cstddef>
#include <memory>
#include <numeric>
#include <string>
#include <utility>
#include <vector>

#include <blaze/Math.h>

///////////////////////////////////////////////////////////////////////////////
namespace phylanx { namespace execution_tree { namespace primitives
{
    ///////////////////////////////////////////////////////////////////////////
    match_pattern_type const unary_minus_operation::match_data =
    {
        hpx::util::make_tuple("__minus",
            std::vector<std::string>{"-_1", "__minus(_1)"},
            &create_unary_minus_operation,
            &create_primitive<unary_minus_operation>)
    };

    ///////////////////////////////////////////////////////////////////////////
    unary_minus_operation::unary_minus_operation(
            std::vector<primitive_argument_type>&& operands,
            std::string const& name, std::string const& codename)
      : primitive_component_base(std::move(operands), name, codename)
    {}

    ///////////////////////////////////////////////////////////////////////////
    primitive_argument_type unary_minus_operation::neg0d(
        operand_type&& op) const
    {
        op.scalar() = -op.scalar();
        return primitive_argument_type(std::move(op));
    }

    primitive_argument_type unary_minus_operation::neg1d(
        operand_type&& op) const
    {
        if (op.is_ref())
        {
            op = -op.vector();
        }
        else
        {
            op.vector() = -op.vector();
        }
        return primitive_argument_type(std::move(op));
    }

    primitive_argument_type unary_minus_operation::neg2d(
        operand_type&& op) const
    {
        if (op.is_ref())
        {
            op = -op.matrix();
        }
        else
        {
            op.matrix() = -op.matrix();
        }
        return primitive_argument_type(std::move(op));
    }

    hpx::future<primitive_argument_type> unary_minus_operation::eval(
        std::vector<primitive_argument_type> const& operands,
        std::vector<primitive_argument_type> const& args) const
    {
        if (operands.size() != 1)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "unary_minus_operation::eval",
                util::generate_error_message(
                    "the unary_minus_operation primitive requires "
                        "exactly one operand",
                    name_, codename_));
        }

        if (!valid(operands[0]))
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "unary_minus_operation::eval",
                util::generate_error_message(
                    "the unary_minus_operation primitive requires "
                        "that the argument given by the operands "
                        "array is valid",
                    name_, codename_));
        }

        auto this_ = this->shared_from_this();
        return hpx::dataflow(hpx::launch::sync, hpx::util::unwrapping(
            [this_](operand_type && op) -> primitive_argument_type
            {
                std::size_t lhs_dims = op.num_dimensions();
                switch (lhs_dims)
                {
                case 0:
                    return this_->neg0d(std::move(op));

                case 1:
                    return this_->neg1d(std::move(op));

                case 2:
                    return this_->neg2d(std::move(op));

                default:
                    HPX_THROW_EXCEPTION(hpx::bad_parameter,
                        "unary_minus_operation::eval",
                        util::generate_error_message(
                            "operand has unsupported number of "
                                "dimensions",
                            this_->name_, this_->codename_));
                }
            }),
            numeric_operand(operands[0], args, name_, codename_));
    }

    //////////////////////////////////////////////////////////////////////////
    // Implement unary '-' for all possible combinations of lhs and rhs
    hpx::future<primitive_argument_type> unary_minus_operation::eval(
        std::vector<primitive_argument_type> const& args) const
    {
        if (this->no_operands())
        {
            return eval(args, noargs);
        }
        return eval(this->operands(), args);
    }
}}}
