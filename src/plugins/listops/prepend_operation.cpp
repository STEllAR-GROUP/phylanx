// Copyright (c) 2018 Weile Wei
// Copyright (c) 2018 R. Tohid
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>
#include <phylanx/ir/node_data.hpp>
#include <phylanx/ir/ranges.hpp>
#include <phylanx/plugins/listops/prepend_operation.hpp>

#include <hpx/include/lcos.hpp>
#include <hpx/include/naming.hpp>
#include <hpx/include/util.hpp>
#include <hpx/throw_exception.hpp>

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <iterator>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include <blaze/Math.h>

///////////////////////////////////////////////////////////////////////////////
namespace phylanx { namespace execution_tree { namespace primitives
{
    ///////////////////////////////////////////////////////////////////////////
    match_pattern_type const prepend_operation::match_data =
    {
        hpx::util::make_tuple("prepend",
            std::vector<std::string>{"prepend(_1, _2)"},
            &create_prepend_operation, &create_primitive<prepend_operation>,
            R"(val,li
            Args:

                val (object) : a value to prepend to the list 'li'
                li (list) : a list to which a value should be pre-pended

            Returns:

            A new list with the value `val` pre-pended. Note that `li` is
            not modified by this operation.)")
    };

    ///////////////////////////////////////////////////////////////////////////
    prepend_operation::prepend_operation(
        primitive_arguments_type && operands,
        std::string const& name, std::string const& codename)
      : primitive_component_base(std::move(operands), name, codename)
    {
    }

    primitive_argument_type prepend_operation::handle_list_operands(
        primitive_argument_type && op1, primitive_argument_type && lhs) const
    {
        ir::range rhs =
            extract_list_value_strict(std::move(op1), name_, codename_);

        if (rhs.is_ref())
        {
            primitive_arguments_type result;
            result.reserve(rhs.size() + 1);
            result.emplace_back(std::move(lhs));
            std::copy(rhs.begin(), rhs.end(), std::back_inserter(result));
            return primitive_argument_type{std::move(result)};
        }

        rhs.args().insert(rhs.args().begin(), std::move(lhs));
        return primitive_argument_type{std::move(rhs)};
    }

    ///////////////////////////////////////////////////////////////////////////
    hpx::future<primitive_argument_type> prepend_operation::eval(
        primitive_arguments_type const& operands,
        primitive_arguments_type const& args, eval_context ctx) const
    {
        if (operands.size() != 2)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "phylanx::execution_tree::primitives::prepend_operation::eval",
                generate_error_message(
                    "prepend_operation accepts exactly two arguments"));
        }

        if (!valid(operands[0]) || !valid(operands[1]))
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter, "prepend_operation::eval",
                generate_error_message(
                    "the prepend_operation primitive requires that the "
                    "arguments given by the operands array are valid"));
        }

        auto&& op0 = value_operand(operands[0], args, name_, codename_, ctx);

        auto this_ = this->shared_from_this();
        return hpx::dataflow(hpx::launch::sync,
            [this_ = std::move(this_)](
                    hpx::future<primitive_argument_type>&& lhs,
                    hpx::future<primitive_argument_type>&& rhs)
            -> primitive_argument_type
            {
                auto&& rhs_data = rhs.get();

                if (is_list_operand_strict(rhs_data))
                {
                    return this_->handle_list_operands(
                        std::move(rhs_data), lhs.get());
                }

                HPX_THROW_EXCEPTION(hpx::bad_parameter,
                    "phylanx::execution_tree::primitives::"
                        "prepend_operation::eval",
                    this_->generate_error_message(
                        "prepend_operation accepts a list "
                        "value as its second operand only"));
            },
            std::move(op0),
            value_operand(operands[1], args, name_, codename_, std::move(ctx)));
    }
}}}
