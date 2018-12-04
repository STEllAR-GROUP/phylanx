//   Copyright (c) 2017 Shahrzad Shirzad
//
//   Distributed under the Boost Software License, Version 1.0. (See accompanying
//   file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>
#include <phylanx/ir/node_data.hpp>
#include <phylanx/plugins/matrixops/identity.hpp>

#include <hpx/include/lcos.hpp>
#include <hpx/include/naming.hpp>
#include <hpx/include/util.hpp>
#include <hpx/throw_exception.hpp>

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
            std::vector<std::string>{"identity(_1)"},
            &create_identity, &create_primitive<identity>, R"(
            n
            Args:

                n (int) : the size of a created (n x n) matrix

            Returns:

            An identity matrix of size `sz` by `sz`.
            )",
            true
        }
    };

    ///////////////////////////////////////////////////////////////////////////
    identity::identity(primitive_arguments_type && operands,
            std::string const& name, std::string const& codename)
      : primitive_component_base(std::move(operands), name, codename    )
      , dtype_(extract_dtype(name_))
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

    primitive_argument_type identity::identity_nd(std::int64_t&& op) const
    {
        node_data_type t = dtype_;

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
        if (operands.size() != 1)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "identity::eval",
                generate_error_message(
                    "the identity primitive requires at most one operand"));
        }

        if (!valid(operands[0]))
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "identity::eval",
                generate_error_message(
                    "the identity primitive requires that the "
                        "arguments given by the operands array are valid"));
        }

        auto this_ = this->shared_from_this();
        return hpx::dataflow(hpx::launch::sync,
            hpx::util::unwrapping(
                [this_ = std::move(this_)](
                    std::int64_t&& op0) -> primitive_argument_type {
                    return this_->identity_nd(std::move(op0));
                }),
            scalar_integer_operand_strict(
                operands[0], args, name_, codename_, std::move(ctx)));
    }
}}}
