// Copyright (c) 2017-2018 Hartmut Kaiser
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>
#include <phylanx/execution_tree/primitives/inverse_operation.hpp>
#include <phylanx/ir/node_data.hpp>

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
    primitive create_inverse_operation(hpx::id_type const& locality,
        std::vector<primitive_argument_type>&& operands,
        std::string const& name, std::string const& codename)
    {
        static std::string type("inverse");
        return create_primitive_component(
            locality, type, std::move(operands), name, codename);
    }

    match_pattern_type const inverse_operation::match_data =
    {
        hpx::util::make_tuple("inverse",
            std::vector<std::string>{"inverse(_1)"},
            &create_inverse_operation, &create_primitive<inverse_operation>)
    };

    ///////////////////////////////////////////////////////////////////////////
    inverse_operation::inverse_operation(
            std::vector<primitive_argument_type>&& operands,
            std::string const& name, std::string const& codename)
      : primitive_component_base(std::move(operands), name, codename)
    {}

    ///////////////////////////////////////////////////////////////////////////
    hpx::future<primitive_argument_type> inverse_operation::eval(
        std::vector<primitive_argument_type> const& operands,
        std::vector<primitive_argument_type> const& args) const
    {
        if (operands.size() != 1)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "inverse_operation::eval",
                execution_tree::generate_error_message(
                    "the inverse_operation primitive requires"
                        "exactly one operand",
                        name_, codename_));
        }

        if (!valid(operands[0]))
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "inverse_operation::eval",
                execution_tree::generate_error_message(
                    "the inverse_operation primitive requires that "
                        "the arguments given by the operands array "
                        "is valid",
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
                    return this_->inverse0d(std::move(ops));

                case 2:
                    return this_->inverse2d(std::move(ops));

                case 1: HPX_FALLTHROUGH;
                default:
                    HPX_THROW_EXCEPTION(hpx::bad_parameter,
                        "inverse_operation::eval",
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

    primitive_argument_type inverse_operation::inverse0d(
        operands_type&& ops) const
    {
        ops[0].scalar() = 1 / ops[0].scalar();
        return primitive_argument_type{std::move(ops[0])};
    }

    primitive_argument_type inverse_operation::inverse2d(
        operands_type&& ops) const
    {
        if (ops[0].dimension(0) != ops[0].dimension(1))
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "inverse::inverse2d",
                execution_tree::generate_error_message(
                    "matrices to inverse have to be quadratic",
                    name_, codename_));
        }

        if (ops[0].is_ref())
        {
            ops[0] = blaze::inv(ops[0].matrix());
        }
        else
        {
            blaze::invert(ops[0].matrix_non_ref());
        }
        return primitive_argument_type{std::move(ops[0])};
    }

    //////////////////////////////////////////////////////////////////////////
    hpx::future<primitive_argument_type> inverse_operation::eval(
        std::vector<primitive_argument_type> const& args) const
    {
        if (operands_.empty())
        {
            return eval(args, noargs);
        }
        return eval(operands_, args);
    }
}}}
