// Copyright (c) 2017-2018 Hartmut Kaiser
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>
#include <phylanx/ir/node_data.hpp>
#include <phylanx/execution_tree/primitives/node_data_helpers.hpp>
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
            &create_transpose_operation,
            &create_primitive<transpose_operation>, R"(
            arg
            Args:

                arg (arr) : an array

            Returns:

            The transpose of `arg`.)")
    };

    ///////////////////////////////////////////////////////////////////////////
    transpose_operation::transpose_operation(
            primitive_arguments_type&& operands,
            std::string const& name, std::string const& codename)
      : primitive_component_base(std::move(operands), name, codename)
    {}

    ///////////////////////////////////////////////////////////////////////////
    primitive_argument_type transpose_operation::transpose0d1d(
        primitive_argument_type&& arg) const
    {
        return primitive_argument_type{std::move(arg)};       // no-op
    }

    template <typename T>
    primitive_argument_type transpose_operation::transpose2d(
        ir::node_data<T>&& arg) const
    {
        if (arg.is_ref())
        {
            arg = blaze::trans(arg.matrix());
        }
        else
        {
            blaze::transpose(arg.matrix_non_ref());
        }

        return primitive_argument_type{std::move(arg)};
    }

    primitive_argument_type transpose_operation::transpose2d(
        primitive_argument_type&& arg) const
    {
        switch (extract_common_type(arg))
        {
        case node_data_type_bool:
            return transpose2d(extract_boolean_value_strict(std::move(arg)));

        case node_data_type_int64:
            return transpose2d(extract_integer_value_strict(std::move(arg)));

        case node_data_type_unknown: HPX_FALLTHROUGH;
        case node_data_type_double:
            return transpose2d(extract_numeric_value(std::move(arg)));

        default:
            break;
        }

        HPX_THROW_EXCEPTION(hpx::bad_parameter,
            "transpose_operation::transpose2d",
            generate_error_message(
                "the transpose primitive requires for its argument to "
                    "be numeric data type"));
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
            [this_ = std::move(this_)](primitive_argument_type&& arg)
            -> primitive_argument_type
            {
                switch (extract_numeric_value_dimension(
                    arg, this_->name_, this_->codename_))
                {
                case 0: HPX_FALLTHROUGH;
                case 1:
                    return this_->transpose0d1d(std::move(arg));

                case 2:
                    return this_->transpose2d(std::move(arg));

                default:
                    HPX_THROW_EXCEPTION(hpx::bad_parameter,
                        "transpose_operation::eval",
                        this_->generate_error_message(
                            "left hand side operand has unsupported "
                                "number of dimensions"));
                }
            }),
            value_operand(operands[0], args, name_, codename_, std::move(ctx)));
    }
}}}
