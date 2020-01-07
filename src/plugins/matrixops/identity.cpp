//   Copyright (c) 2017 Shahrzad Shirzad
//
//   Distributed under the Boost Software License, Version 1.0. (See accompanying
//   file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>
#include <phylanx/execution_tree/primitives/node_data_helpers.hpp>
#include <phylanx/ir/node_data.hpp>
#include <phylanx/plugins/matrixops/identity.hpp>

#include <hpx/include/lcos.hpp>
#include <hpx/include/naming.hpp>
#include <hpx/include/util.hpp>
#include <hpx/errors/throw_exception.hpp>

#include <cmath>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include <blaze/Math.h>

///////////////////////////////////////////////////////////////////////////////
namespace phylanx { namespace execution_tree { namespace primitives
{
    ///////////////////////////////////////////////////////////////////////////
    match_pattern_type const identity::match_data =
    {
        match_pattern_type{"identity",
            std::vector<std::string>{"identity(_1, __arg(_2_dtype, nil))"},
            &create_identity, &create_primitive<identity>, R"(
            n, dtype
            Args:

                n (int) : the size of a created (n x n) matrix
                dtype (optional, string) : the data-type of the returned array,
                  defaults to 'float'.

            Returns:

            An identity matrix of size `sz` by `sz`.)"
        }
    };

    ///////////////////////////////////////////////////////////////////////////
    identity::identity(primitive_arguments_type && operands,
            std::string const& name, std::string const& codename)
      : primitive_component_base(std::move(operands), name, codename)
    {
    }

    ///////////////////////////////////////////////////////////////////////////
    template <typename T>
    primitive_argument_type identity::identity_helper(std::int64_t&& op) const
    {
        if (op < 0)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "identity::identity_helper",
                generate_error_message("input should be greater than zero"));
        }
        std::size_t size = static_cast<std::size_t>(op);
        return primitive_argument_type{
            ir::node_data<T>{blaze::IdentityMatrix<T>(size)}};
    }

    primitive_argument_type identity::identity_nd(
        std::int64_t&& op, node_data_type t) const
    {
        switch (t)
        {
        case node_data_type_bool:
            return identity_helper<std::uint8_t>(std::move(op));

        case node_data_type_int64:
            return identity_helper<std::int64_t>(std::move(op));

        case node_data_type_unknown:
            HPX_FALLTHROUGH;
        case node_data_type_double:
            return identity_helper<double>(std::move(op));

        default:
            break;
        }

        HPX_THROW_EXCEPTION(hpx::bad_parameter,
            "phylanx::execution_tree::primitives::"
                "identity::identity_nd",
            generate_error_message(
                "the identity primitive requires for all arguments to "
                    "be numeric data types"));
    }

    hpx::future<primitive_argument_type> identity::eval(
        primitive_arguments_type const& operands,
        primitive_arguments_type const& args, eval_context ctx) const
    {
        if (operands.size() != 1 && operands.size() != 2)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "identity::eval",
                generate_error_message(
                    "the identity primitive requires at most two operands"));
        }

        if (!valid(operands[0]))
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "identity::eval",
                generate_error_message(
                    "the identity primitive requires that the "
                        "arguments given by the operands array are valid"));
        }

        if (operands.size() > 1 && !valid(operands[1]) &&
            !is_explicit_nil(operands[1]))
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "identity::eval",
                generate_error_message(
                    "the identity primitive requires that the "
                        "arguments given by the operands array are valid"));
        }

        auto this_ = this->shared_from_this();
        if (operands.size() > 1 && valid(operands[1]))
        {
            auto dtype =
                string_operand_strict(operands[1], args, name_, codename_, ctx);

            return hpx::dataflow(hpx::launch::sync,
                [this_ = std::move(this_)](
                    hpx::future<std::int64_t>&& op0,
                    hpx::future<std::string>&& dtype)
                -> primitive_argument_type
                {
                    return this_->identity_nd(op0.get(), map_dtype(dtype.get()));
                },
                scalar_integer_operand_strict(
                    operands[0], args, name_, codename_, std::move(ctx)),
                std::move(dtype));
        }

        return hpx::dataflow(hpx::launch::sync,
            [this_ = std::move(this_)](hpx::future<std::int64_t>&& op0)
            -> primitive_argument_type
            {
                return this_->identity_nd(op0.get(), node_data_type_double);
            },
            scalar_integer_operand_strict(
                operands[0], args, name_, codename_, std::move(ctx)));
    }
}}}
