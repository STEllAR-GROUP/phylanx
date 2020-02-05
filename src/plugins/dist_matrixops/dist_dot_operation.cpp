//  Copyright (c) 2017-2019 Hartmut Kaiser
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>
#include <phylanx/execution_tree/localities_annotation.hpp>
#include <phylanx/execution_tree/primitives/node_data_helpers.hpp>
#include <phylanx/ir/node_data.hpp>
#include <phylanx/plugins/common/dot_operation_nd.hpp>
#include <phylanx/plugins/dist_matrixops/dist_dot_operation.hpp>

#include <hpx/include/lcos.hpp>
#include <hpx/include/naming.hpp>
#include <hpx/include/util.hpp>
#include <hpx/errors/throw_exception.hpp>

#include <cstddef>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include <blaze/Math.h>
#include <blaze_tensor/Math.h>

///////////////////////////////////////////////////////////////////////////////
namespace phylanx { namespace dist_matrixops { namespace primitives
{
    ///////////////////////////////////////////////////////////////////////////
    execution_tree::match_pattern_type const dist_dot_operation::match_data =
    {
        execution_tree::match_pattern_type{
            "dot_d", std::vector<std::string>{"dot_d(_1, _2)"},
            &create_dist_dot_operation,
            &execution_tree::create_primitive<dist_dot_operation>,
            R"(a, b
            Args:

                a (array) : a scalar, vector, matrix or a tensor
                b (array) : a scalar, vector, matrix or a tensor

            Returns:

            The dot product of two arrays: `a` and `b`. The dot product of an
            N-D array and an M-D array is of dimension N+M-2)"
        }
    };

    ////////////////////////////////////////////////////////////////////////////
    dist_dot_operation::dist_dot_operation(
            execution_tree::primitive_arguments_type&& operands,
            std::string const& name, std::string const& codename)
      : execution_tree::primitives::primitive_component_base(
            std::move(operands), name, codename)
    {}

    ////////////////////////////////////////////////////////////////////////////
    execution_tree::primitive_argument_type dist_dot_operation::dot0d(
        execution_tree::primitive_argument_type&& lhs,
        execution_tree::primitive_argument_type&& rhs) const
    {
        using namespace execution_tree;

        if (!lhs.has_annotation() && !rhs.has_annotation())
        {
            return common::dot0d(
                std::move(lhs), std::move(rhs), name_, codename_);
        }

        switch (extract_common_type(lhs, rhs))
        {
        case node_data_type_bool:
            return dot0d(
                extract_boolean_value(std::move(lhs), name_, codename_),
                extract_boolean_value(std::move(rhs), name_, codename_));

        case node_data_type_int64:
            return dot0d(
                extract_integer_value(std::move(lhs), name_, codename_),
                extract_integer_value(std::move(rhs), name_, codename_));

        case node_data_type_unknown: HPX_FALLTHROUGH;
        case node_data_type_double:
            return dot0d(
                extract_numeric_value(std::move(lhs), name_, codename_),
                extract_numeric_value(std::move(rhs), name_, codename_));

        default:
            break;
        }

        HPX_THROW_EXCEPTION(hpx::bad_parameter,
            "dist_dot_operation::dot0d",
            generate_error_message(
                "the dot primitive requires for all arguments to "
                    "be numeric data types"));
    }

    execution_tree::primitive_argument_type dist_dot_operation::dot1d(
        execution_tree::primitive_argument_type&& lhs,
        execution_tree::primitive_argument_type&& rhs) const
    {
        using namespace execution_tree;

        if (!lhs.has_annotation() && !rhs.has_annotation())
        {
            return common::dot1d(
                std::move(lhs), std::move(rhs), name_, codename_);
        }

        execution_tree::localities_information lhs_localities =
            extract_localities_information(lhs, name_, codename_);
        execution_tree::localities_information rhs_localities =
            extract_localities_information(rhs, name_, codename_);

        switch (extract_common_type(lhs, rhs))
        {
        case node_data_type_bool:
            return dot1d(
                extract_boolean_value(std::move(lhs), name_, codename_),
                extract_boolean_value(std::move(rhs), name_, codename_),
                std::move(lhs_localities), rhs_localities);

        case node_data_type_int64:
            return dot1d(
                extract_integer_value(std::move(lhs), name_, codename_),
                extract_integer_value(std::move(rhs), name_, codename_),
                std::move(lhs_localities), rhs_localities);

        case node_data_type_unknown: HPX_FALLTHROUGH;
        case node_data_type_double:
            return dot1d(
                extract_numeric_value(std::move(lhs), name_, codename_),
                extract_numeric_value(std::move(rhs), name_, codename_),
                std::move(lhs_localities), rhs_localities);

        default:
            break;
        }

        HPX_THROW_EXCEPTION(hpx::bad_parameter,
            "dist_dot_operation::dot1d",
            generate_error_message(
                "the distributed dot primitive requires for all arguments to "
                    "be numeric data types"));
    }

    execution_tree::primitive_argument_type dist_dot_operation::dot2d(
        execution_tree::primitive_argument_type&& lhs,
        execution_tree::primitive_argument_type&& rhs) const
    {
        using namespace execution_tree;

        if (!lhs.has_annotation() && !rhs.has_annotation())
        {
            return common::dot2d(
                std::move(lhs), std::move(rhs), name_, codename_);
        }

        execution_tree::localities_information lhs_localities =
            extract_localities_information(lhs, name_, codename_);
        execution_tree::localities_information rhs_localities =
            extract_localities_information(rhs, name_, codename_);

        switch (extract_common_type(lhs, rhs))
        {
        case node_data_type_bool:
            return dot2d(
                extract_boolean_value(std::move(lhs), name_, codename_),
                extract_boolean_value(std::move(rhs), name_, codename_),
                std::move(lhs_localities), rhs_localities);

        case node_data_type_int64:
            return dot2d(
                extract_integer_value(std::move(lhs), name_, codename_),
                extract_integer_value(std::move(rhs), name_, codename_),
                std::move(lhs_localities), rhs_localities);

        case node_data_type_unknown: HPX_FALLTHROUGH;
        case node_data_type_double:
            return dot2d(
                extract_numeric_value(std::move(lhs), name_, codename_),
                extract_numeric_value(std::move(rhs), name_, codename_),
                std::move(lhs_localities), rhs_localities);

        default:
            break;
        }

        HPX_THROW_EXCEPTION(hpx::bad_parameter,
            "dist_dot_operation::dot2d",
            generate_error_message(
                "the distributed dot primitive requires for all arguments to "
                    "be numeric data types"));
    }

    execution_tree::primitive_argument_type dist_dot_operation::dot3d(
        execution_tree::primitive_argument_type&& lhs,
        execution_tree::primitive_argument_type&& rhs) const
    {
        using namespace execution_tree;

        if (!lhs.has_annotation() && !rhs.has_annotation())
        {
            return common::dot3d(
                std::move(lhs), std::move(rhs), name_, codename_);
        }

        execution_tree::localities_information lhs_localities =
            extract_localities_information(lhs, name_, codename_);
        execution_tree::localities_information rhs_localities =
            extract_localities_information(rhs, name_, codename_);

        switch (extract_common_type(lhs, rhs))
        {
        case node_data_type_bool:
            return dot3d(
                extract_boolean_value(std::move(lhs), name_, codename_),
                extract_boolean_value(std::move(rhs), name_, codename_),
                lhs_localities, rhs_localities);

        case node_data_type_int64:
            return dot3d(
                extract_integer_value(std::move(lhs), name_, codename_),
                extract_integer_value(std::move(rhs), name_, codename_),
                lhs_localities, rhs_localities);

        case node_data_type_unknown: HPX_FALLTHROUGH;
        case node_data_type_double:
            return dot3d(
                extract_numeric_value(std::move(lhs), name_, codename_),
                extract_numeric_value(std::move(rhs), name_, codename_),
                lhs_localities, rhs_localities);

        default:
            break;
        }

        HPX_THROW_EXCEPTION(hpx::bad_parameter,
            "dist_dot_operation::dot0d",
            generate_error_message(
                "the distributed dot primitive requires for all arguments to "
                    "be numeric data types"));
    }

    ////////////////////////////////////////////////////////////////////////////
    execution_tree::primitive_argument_type dist_dot_operation::dot_nd(
        execution_tree::primitive_argument_type&& lhs,
        execution_tree::primitive_argument_type&& rhs) const
    {
        using namespace execution_tree;

        switch (extract_numeric_value_dimension(lhs, name_, codename_))
        {
        case 0:
            return dot0d(std::move(lhs), std::move(rhs));

        case 1:
            return dot1d(std::move(lhs), std::move(rhs));

        case 2:
            return dot2d(std::move(lhs), std::move(rhs));

        case 3:
            return dot3d(std::move(lhs), std::move(rhs));

        default:
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "dot_operation::dot_nd",
                generate_error_message("left hand side operand has unsupported "
                                       "number of dimensions"));
        }
    }
    ////////////////////////////////////////////////////////////////////////////
    hpx::future<execution_tree::primitive_argument_type>
    dist_dot_operation::eval(
        execution_tree::primitive_arguments_type const& operands,
        execution_tree::primitive_arguments_type const& args,
        execution_tree::eval_context ctx) const
    {
        using namespace execution_tree;

        if (operands.size() != 2 && operands.size() != 3)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "dist_dot_operation::eval",
                generate_error_message(
                    "the dist_dot_operation primitive requires exactly "
                        "two or three operands"));
        }

        if (!valid(operands[0]) || !valid(operands[1]))
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "dist_dot_operation::eval",
                generate_error_message(
                    "the dist_dot_operation primitive requires that the "
                        "arguments given by the operands array are valid"));
        }

        auto f = value_operand(operands[0], args, name_, codename_, ctx);

        auto this_ = this->shared_from_this();
        return hpx::dataflow(hpx::launch::sync,
            [this_ = std::move(this_)](
                    hpx::future<primitive_argument_type>&& op1,
                    hpx::future<primitive_argument_type>&& op2)
            -> primitive_argument_type
            {
                return this_->dot_nd(op1.get(), op2.get());
            },
            std::move(f),
            value_operand(operands[1], args, name_, codename_, std::move(ctx)));
    }
}}}
