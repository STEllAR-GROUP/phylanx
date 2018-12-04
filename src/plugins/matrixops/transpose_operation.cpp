// Copyright (c) 2017-2018 Hartmut Kaiser
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>
#include <phylanx/ir/node_data.hpp>
#include <phylanx/plugins/matrixops/transpose_operation.hpp>

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
    match_pattern_type const transpose_operation::match_data =
    {
        hpx::util::make_tuple("transpose",
            std::vector<std::string>{"transpose(_1)"},
            &create_transpose_operation, &create_primitive<transpose_operation>,
            "m\n"
            "Args:\n"
            "\n"
            "    m (matrix) : a matrix\n"
            "\n"
            "Returns:\n"
            "\n"
            "The transpose of `m`.")
    };

    ///////////////////////////////////////////////////////////////////////////
    transpose_operation::transpose_operation(
            primitive_arguments_type&& operands,
            std::string const& name, std::string const& codename)
      : primitive_component_base(std::move(operands), name, codename)
    {}

    ///////////////////////////////////////////////////////////////////////////
    primitive_argument_type transpose_operation::transpose0d1d(
        operands_type&& ops) const
    {
        return primitive_argument_type{std::move(ops[0])};       // no-op
    }

    primitive_argument_type transpose_operation::transpose2d(
        operands_type&& ops) const
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

    ///////////////////////////////////////////////////////////////////////////
    hpx::future<primitive_argument_type> transpose_operation::eval(
        primitive_arguments_type const& operands,
        primitive_arguments_type const& args, eval_context ctx) const
    {
        if (operands.size() != 1)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "transpose_operation::transpose_operation",
                generate_error_message(
                    "the transpose_operation primitive requires"
                        "exactly one operand"));
        }

        if (!valid(operands[0]))
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "transpose_operation::transpose_operation",
                generate_error_message(
                    "the transpose_operation primitive requires "
                        "that the arguments given by the operands "
                        "array is valid"));
        }

        auto this_ = this->shared_from_this();
        return hpx::dataflow(hpx::launch::sync, hpx::util::unwrapping(
            [this_ = std::move(this_)](operands_type&& ops)
            -> primitive_argument_type
            {
                std::size_t dims = ops[0].num_dimensions();
                switch (dims)
                {
                case 0: HPX_FALLTHROUGH;
                case 1:
                    return this_->transpose0d1d(std::move(ops));

                case 2:
                    return this_->transpose2d(std::move(ops));

                default:
                    HPX_THROW_EXCEPTION(hpx::bad_parameter,
                        "transpose_operation::eval",
                        this_->generate_error_message(
                            "left hand side operand has unsupported "
                                "number of dimensions"));
                }
            }),
            detail::map_operands(
                operands, functional::numeric_operand{}, args,
                name_, codename_, std::move(ctx)));
    }
}}}
