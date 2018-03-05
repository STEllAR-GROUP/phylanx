// Copyright (c) 2017-2018 Hartmut Kaiser
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>
#include <phylanx/ast/detail/is_literal_value.hpp>
#include <phylanx/execution_tree/primitives/transpose_operation.hpp>
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
    primitive create_transpose_operation(hpx::id_type const& locality,
        std::vector<primitive_argument_type>&& operands,
        std::string const& name, std::string const& codename)
    {
        static std::string type("transpose");
        return create_primitive_component(
            locality, type, std::move(operands), name, codename);
    }

    match_pattern_type const transpose_operation::match_data =
    {
        hpx::util::make_tuple("transpose",
            std::vector<std::string>{"transpose(_1)"},
            &create_transpose_operation, &create_primitive<transpose_operation>)
    };

    ///////////////////////////////////////////////////////////////////////////
    transpose_operation::transpose_operation(
            std::vector<primitive_argument_type>&& operands,
            std::string const& name, std::string const& codename)
      : primitive_component_base(std::move(operands), name, codename)
    {}

    ///////////////////////////////////////////////////////////////////////////
    hpx::future<primitive_argument_type> transpose_operation::eval(
        std::vector<primitive_argument_type> const& operands,
        std::vector<primitive_argument_type> const& args,
        std::string const& name, std::string const& codename) const
    {
        if (operands.size() != 1)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "transpose_operation::transpose_operation",
                execution_tree::generate_error_message(
                    "the transpose_operation primitive requires"
                        "exactly one operand",
                    name, codename));
        }

        if (!valid(operands[0]))
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "transpose_operation::transpose_operation",
                execution_tree::generate_error_message(
                    "the transpose_operation primitive requires "
                        "that the arguments given by the operands "
                        "array is valid",
                    name, codename));
        }

        auto this_ = this->shared_from_this();
        return hpx::dataflow(hpx::util::unwrapping(
            [this_, name, codename](operands_type&& ops)
            -> primitive_argument_type
            {
                std::size_t dims = ops[0].num_dimensions();
                switch (dims)
                {
                case 0:
                    HPX_FALLTHROUGH;

                case 1:
                    return this_->transpose0d1d(std::move(ops));

                case 2:
                    return this_->transpose2d(std::move(ops));

                default:
                    HPX_THROW_EXCEPTION(hpx::bad_parameter,
                        "transpose_operation::eval",
                        execution_tree::generate_error_message(
                            "left hand side operand has unsupported "
                                "number of dimensions",
                            name, codename));
                }
            }),
            detail::map_operands(
                operands, functional::numeric_operand{}, args,
                name, codename));
    }

    primitive_argument_type transpose_operation::transpose0d1d(operands_type&& ops) const
    {
        return primitive_argument_type{std::move(ops[0])};       // no-op
    }

    primitive_argument_type transpose_operation::transpose2d(operands_type && ops) const
    {
        if (ops[0].is_ref())
        {
            ops[0] = blaze::trans(ops[0].matrix());
        }
        else
        {
            blaze::transpose(ops[0].matrix_non_ref());
        }
        return primitive_argument_type{std::move(ops[0])};
    }

    //////////////////////////////////////////////////////////////////////////
    hpx::future<primitive_argument_type> transpose_operation::eval(
        std::vector<primitive_argument_type> const& args) const
    {
        if (operands_.empty())
        {
            return eval(args, noargs, name_, codename_);
        }
        return eval(operands_, args, name_, codename_);
    }
}}}
