//   Copyright (c) 2017-2018 Hartmut Kaiser
//
//   Distributed under the Boost Software License, Version 1.0. (See accompanying
//   file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>
#include <phylanx/ir/node_data.hpp>
#include <phylanx/execution_tree/primitives/node_data_helpers.hpp>
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
        match_pattern_type{"determinant",
            std::vector<std::string>{"determinant(_1)"},
            &create_determinant, &create_primitive<determinant>, R"(
            arg
            Args:

                arg (matrix) : a square matrix of numbers

            Returns:

            The determinant of the matrix represented by `arg`.)"
        }
    };

    ///////////////////////////////////////////////////////////////////////////
    determinant::determinant(primitive_arguments_type&& operands,
            std::string const& name, std::string const& codename)
      : primitive_component_base(std::move(operands), name, codename)
    {}

    ///////////////////////////////////////////////////////////////////////////
    template <typename T>
    primitive_argument_type determinant::determinant0d(
        ir::node_data<T>&& op) const
    {
        return primitive_argument_type{std::move(op)};       // no-op
    }

    primitive_argument_type determinant::determinant0d(
        primitive_argument_type&& op) const
    {
        switch (extract_common_type(op))
        {
        case node_data_type_bool:
            return determinant0d(
                extract_boolean_value_strict(std::move(op), name_, codename_));

        case node_data_type_int64:
            return determinant0d(
                extract_integer_value_strict(std::move(op), name_, codename_));

        case node_data_type_double:
            return determinant0d(
                extract_numeric_value_strict(std::move(op), name_, codename_));

        case node_data_type_unknown:
            return determinant0d(
                extract_numeric_value(std::move(op), name_, codename_));

        default:
            break;
        }

        HPX_THROW_EXCEPTION(hpx::bad_parameter,
            "determinant::determinant0d",
            generate_error_message(
                "the determinant primitive requires for all arguments to "
                    "be numeric data types"));
    }

    ///////////////////////////////////////////////////////////////////////////
    template <typename T>
    primitive_argument_type determinant::determinant2d(
        ir::node_data<T>&& op) const
    {
        double d = blaze::det(op.matrix());
        return primitive_argument_type{ir::node_data<T>(d)};
    }

    primitive_argument_type determinant::determinant2d(
        primitive_argument_type&& op) const
    {
        switch (extract_common_type(op))
        {
        case node_data_type_double:
            return determinant2d(
                extract_numeric_value_strict(std::move(op), name_, codename_));

        case node_data_type_int64:  HPX_FALLTHROUGH;
        case node_data_type_bool:   HPX_FALLTHROUGH;
        case node_data_type_unknown:
            return determinant2d(
                extract_numeric_value(std::move(op), name_, codename_));

        default:
            break;
        }

        HPX_THROW_EXCEPTION(hpx::bad_parameter,
            "determinant::determinant2d",
            generate_error_message(
                "the determinant primitive requires for all arguments to "
                    "be numeric data types"));
    }

    ///////////////////////////////////////////////////////////////////////////
    hpx::future<primitive_argument_type> determinant::eval(
        primitive_arguments_type const& operands,
        primitive_arguments_type const& args, eval_context ctx) const
    {
        if (operands.size() != 1)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "determinant::eval",
                generate_error_message(
                    "the determinant primitive requires exactly one operand"));
        }

        if (!valid(operands[0]))
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "determinant::eval",
                generate_error_message(
                    "the determinant primitive requires that the "
                        "argument given by the operands array is valid"));
        }

        auto this_ = this->shared_from_this();
        return hpx::dataflow(hpx::launch::sync,
            [this_ = std::move(this_)](
                    hpx::future<primitive_argument_type>&& f)
            -> primitive_argument_type
            {
                auto&& op = f.get();

                switch (extract_numeric_value_dimension(
                    op, this_->name_, this_->codename_))
                {
                case 0:
                    return this_->determinant0d(std::move(op));

                case 2:
                    return this_->determinant2d(std::move(op));

                case 1: HPX_FALLTHROUGH;
                default:
                    HPX_THROW_EXCEPTION(hpx::bad_parameter,
                        "determinant::eval",
                        this_->generate_error_message(
                            "operand has unsupported number of dimensions"));
                }
            },
            value_operand(operands[0], args, name_, codename_, std::move(ctx)));
    }
}}}
