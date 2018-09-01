//   Copyright (c) 2017-2018 Hartmut Kaiser
//
//   Distributed under the Boost Software License, Version 1.0. (See accompanying
//   file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>
#include <phylanx/ir/node_data.hpp>
#include <phylanx/plugins/matrixops/determinant.hpp>

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

#include <blaze/Math.h>

///////////////////////////////////////////////////////////////////////////////
namespace phylanx { namespace execution_tree { namespace primitives
{
    ///////////////////////////////////////////////////////////////////////////
    match_pattern_type const determinant::match_data =
    {
        hpx::util::make_tuple("determinant",
            std::vector<std::string>{"determinant(_1)"},
            &create_determinant, &create_primitive<determinant>)
    };

    ///////////////////////////////////////////////////////////////////////////
    determinant::determinant(primitive_arguments_type&& operands,
            std::string const& name, std::string const& codename)
      : primitive_component_base(std::move(operands), name, codename)
    {}

    ///////////////////////////////////////////////////////////////////////////

    hpx::future<primitive_argument_type> determinant::eval(
        primitive_arguments_type const& operands,
        primitive_arguments_type const& args) const
    {
        if (operands.size() != 1)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "determinant::eval",
                util::generate_error_message(
                    "the determinant primitive requires"
                        "exactly one operand",
                    name_, codename_));
        }

        if (!valid(operands[0]))
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "determinant::eval",
                util::generate_error_message(
                    "the determinant primitive requires that the "
                        "argument given by the operands array is "
                        "valid",
                    name_, codename_));
        }

        auto this_ = this->shared_from_this();
        return hpx::dataflow(hpx::launch::sync, hpx::util::unwrapping(
            [this_](operand_type&& op) -> primitive_argument_type
            {
                std::size_t dims = op.num_dimensions();
                switch (dims)
                {
                case 0:
                    return this_->determinant0d(std::move(op));

                case 2:
                    return this_->determinant2d(std::move(op));

                case 1: HPX_FALLTHROUGH;
                default:
                    HPX_THROW_EXCEPTION(hpx::bad_parameter,
                        "determinant::eval",
                        util::generate_error_message(
                            "left hand side operand has unsupported "
                                "number of dimensions",
                            this_->name_, this_->codename_));
                }
            }),
            numeric_operand(operands[0], args, name_, codename_));
    }

    primitive_argument_type determinant::determinant0d(operand_type&& op) const
    {
        return primitive_argument_type{std::move(op)};       // no-op
    }

    primitive_argument_type determinant::determinant2d(operand_type&& op) const
    {
        double d = blaze::det(op.matrix());
        return primitive_argument_type{operand_type(d)};
    }

    hpx::future<primitive_argument_type> determinant::eval(
        primitive_arguments_type const& args) const
    {
        if (this->no_operands())
        {
            return eval(args, noargs);
        }
        return eval(this->operands(), args);
    }
}}}
