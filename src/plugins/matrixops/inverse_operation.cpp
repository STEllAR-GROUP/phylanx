// Copyright (c) 2017-2018 Hartmut Kaiser
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>
#include <phylanx/ir/node_data.hpp>
#include <phylanx/execution_tree/primitives/node_data_helpers.hpp>
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

#include <blaze/Math.h>
#if defined(PHYLANX_HAVE_BLAZE_TENSOR)
#include <blaze_tensor/Math.h>
#endif

///////////////////////////////////////////////////////////////////////////////
namespace phylanx { namespace execution_tree { namespace primitives
{
    ///////////////////////////////////////////////////////////////////////////
    match_pattern_type const inverse_operation::match_data =
    {
        hpx::util::make_tuple("inverse",
            std::vector<std::string>{"inverse(_1)"},
            &create_inverse_operation,
            &create_primitive<inverse_operation>, R"(
            m
            Args:

                m (matrix): a scalar, matrix, or tensor

            Returns:

            The multiplicative inverse of m.)")
    };

    ///////////////////////////////////////////////////////////////////////////
    inverse_operation::inverse_operation(
            primitive_arguments_type&& operands,
            std::string const& name, std::string const& codename)
      : primitive_component_base(std::move(operands), name, codename)
    {}

    ///////////////////////////////////////////////////////////////////////////
    template <typename T>
    primitive_argument_type inverse_operation::inverse0d(
        ir::node_data<T>&& op) const
    {
        op.scalar() = 1 / op.scalar();
        return primitive_argument_type{std::move(op)};
    }

    template <typename T>
    primitive_argument_type inverse_operation::inverse2d(
        ir::node_data<T>&& op) const
    {
        if (op.dimension(0) != op.dimension(1))
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "inverse::inverse2d",
                generate_error_message(
                    "matrices to inverse have to be quadratic"));
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

#if defined(PHYLANX_HAVE_BLAZE_TENSOR)
    template <typename T>
    primitive_argument_type inverse_operation::inverse3d(
        ir::node_data<T>&& op) const
    {
        if (op.dimension(1) != op.dimension(2))
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "inverse::inverse3d",
                generate_error_message(
                    "matrices to inverse have to be quadratic"));
        }

        if (op.is_ref())
        {
            auto t = op.tensor();
            blaze::DynamicTensor<T> result(t.pages(), t.rows(), t.columns());
            for (std::size_t p = 0; p != t.pages(); ++p)
            {
                blaze::pageslice(result, p) = blaze::inv(blaze::pageslice(t, p));
            }
            return primitive_argument_type{std::move(result)};
        }

        auto& t = op.tensor_non_ref();
        for (std::size_t p = 0; p != t.pages(); ++p)
        {
            auto ps = blaze::pageslice(t, p);
            blaze::invert(ps);
        }
        return primitive_argument_type{std::move(op)};
    }
#endif

    ///////////////////////////////////////////////////////////////////////////
    primitive_argument_type inverse_operation::inverse0d(
        primitive_argument_type&& op) const
    {
        switch (extract_common_type(op))
        {
        case node_data_type_bool:
            return inverse0d(
                extract_boolean_value(std::move(op), name_, codename_));

        case node_data_type_int64:
            return inverse0d(
                extract_integer_value(std::move(op), name_, codename_));

        case node_data_type_double:
            return inverse0d(extract_numeric_value_strict(
                std::move(op), name_, codename_));

        case node_data_type_unknown:
            return inverse0d(
                extract_numeric_value(std::move(op), name_, codename_));

        default:
            break;
        }

        HPX_THROW_EXCEPTION(hpx::bad_parameter,
            "inverse_operation::inverse0d",
            generate_error_message(
                "the inverse primitive requires for all arguments to "
                    "be numeric data types"));
    }

    primitive_argument_type inverse_operation::inverse2d(
        primitive_argument_type&& op) const
    {
        switch (extract_common_type(op))
        {
        case node_data_type_double:
            return inverse2d(extract_numeric_value_strict(
                std::move(op), name_, codename_));

        case node_data_type_bool:
        case node_data_type_int64:
        case node_data_type_unknown:
            return inverse2d(extract_numeric_value(
                std::move(op), name_, codename_));

        default:
            break;
        }

        HPX_THROW_EXCEPTION(hpx::bad_parameter,
            "inverse_operation::inverse2d",
            generate_error_message(
                "the inverse primitive requires for all arguments to "
                    "be numeric data types"));
    }

#if defined(PHYLANX_HAVE_BLAZE_TENSOR)
    primitive_argument_type inverse_operation::inverse3d(
        primitive_argument_type&& op) const
    {
        switch (extract_common_type(op))
        {
        case node_data_type_double:
            return inverse3d(extract_numeric_value_strict(
                std::move(op), name_, codename_));

        case node_data_type_bool:
        case node_data_type_int64:
        case node_data_type_unknown:
            return inverse3d(extract_numeric_value(
                std::move(op), name_, codename_));

        default:
            break;
        }

        HPX_THROW_EXCEPTION(hpx::bad_parameter,
            "inverse_operation::inverse3d",
            generate_error_message(
                "the inverse primitive requires for all arguments to "
                    "be numeric data types"));
    }
#endif

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
                    return this_->inverse0d(std::move(op));

                case 2:
                    return this_->inverse2d(std::move(op));

#if defined(PHYLANX_HAVE_BLAZE_TENSOR)
                case 3:
                    return this_->inverse3d(std::move(op));
#endif
                case 1: HPX_FALLTHROUGH;
                default:
                    HPX_THROW_EXCEPTION(hpx::bad_parameter,
                        "inverse_operation::eval",
                        this_->generate_error_message(
                            "left hand side operand has unsupported "
                                "number of dimensions"));
                }
            },
            value_operand(operands[0], args, name_, codename_, std::move(ctx)));
    }
}}}
