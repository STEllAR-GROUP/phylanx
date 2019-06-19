//   Copyright (c) 2019 Hartmut Kaiser
//
//   Distributed under the Boost Software License, Version 1.0. (See accompanying
//   file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>
#include <phylanx/ir/node_data.hpp>
#include <phylanx/plugins/matrixops/astype.hpp>

#include <hpx/include/lcos.hpp>
#include <hpx/include/naming.hpp>
#include <hpx/include/util.hpp>
#include <hpx/throw_exception.hpp>

#include <cstddef>
#include <cstdint>
#include <memory>
#include <string>
#include <utility>
#include <vector>

///////////////////////////////////////////////////////////////////////////////
namespace phylanx { namespace execution_tree { namespace primitives
{
    ///////////////////////////////////////////////////////////////////////////
    match_pattern_type const astype::match_data =
    {
        match_pattern_type{"astype",
            std::vector<std::string>{"astype(_1_x, _2_dtype)"},
            &create_astype, &create_primitive<astype>, R"(
            x, dtype
            Args:

                x (array) : the array to be cast to the specified type
                dtype : the data-type to which the array is cast

            Returns:

            Copy of the array, cast to a specified type.)"
        }
    };

    ///////////////////////////////////////////////////////////////////////////
    astype::astype(primitive_arguments_type && operands,
            std::string const& name, std::string const& codename)
      : primitive_component_base(std::move(operands), name, codename    )
    {
    }

    ///////////////////////////////////////////////////////////////////////////
    template <typename T>
    primitive_argument_type astype::astype_helper(ir::node_data<T>&& op) const
    {
        return primitive_argument_type{ir::node_data<T>{std::move(op)}};
    }

    primitive_argument_type astype::astype_nd(
        primitive_argument_type&& op, node_data_type dtype) const
    {
        switch (dtype)
        {
        case node_data_type_bool:
            return astype_helper(extract_node_data<std::uint8_t>(
                std::move(op), name_, codename_));

        case node_data_type_int64:
            return astype_helper(extract_node_data<std::int64_t>(
                std::move(op), name_, codename_));

        case node_data_type_unknown: HPX_FALLTHROUGH;
        case node_data_type_double:
            return astype_helper(
                extract_node_data<double>(std::move(op), name_, codename_));

        default:
            break;
        }

        HPX_THROW_EXCEPTION(hpx::bad_parameter,
            "phylanx::execution_tree::primitives::astype::astype_nd",
            generate_error_message(
                "the astype primitive requires for all arguments to "
                    "be numeric data types"));
    }

    hpx::future<primitive_argument_type> astype::eval(
        primitive_arguments_type const& operands,
        primitive_arguments_type const& args, eval_context ctx) const
    {
        if (operands.size() != 2)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "astype::eval",
                generate_error_message(
                    "the astype primitive requires exactly two operands"));
        }

        if (!valid(operands[0]) || !valid(operands[1]))
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "astype::eval",
                generate_error_message(
                    "the astype primitive requires that the "
                        "arguments given by the operands array are valid"));
        }

        auto&& op0 = value_operand(operands[0], args, name_, codename_, ctx);

        auto this_ = this->shared_from_this();
        return hpx::dataflow(hpx::launch::sync,
            [this_ = std::move(this_)](
                hpx::future<primitive_argument_type>&& op0,
                hpx::future<std::string>&& dtype_op)
            -> primitive_argument_type
            {
                return this_->astype_nd(op0.get(), map_dtype(dtype_op.get()));
            },
            std::move(op0),
            string_operand(operands[1], args, name_, codename_, std::move(ctx)));
    }
}}}
