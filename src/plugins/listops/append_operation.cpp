// Copyright (c) 2018 Weile Wei
// Copyright (c) 2018 R. Tohid
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>
#include <phylanx/ir/node_data.hpp>
#include <phylanx/ir/ranges.hpp>
#include <phylanx/plugins/listops/append_operation.hpp>

#include <hpx/include/lcos.hpp>
#include <hpx/include/naming.hpp>
#include <hpx/include/util.hpp>
#include <hpx/throw_exception.hpp>

#include <cstddef>
#include <cstdint>
#include <memory>
#include <numeric>
#include <string>
#include <utility>
#include <vector>

#include <blaze/Math.h>

///////////////////////////////////////////////////////////////////////////////
namespace phylanx { namespace execution_tree { namespace primitives
{
    ///////////////////////////////////////////////////////////////////////////
    match_pattern_type const append_operation::match_data = {
        hpx::util::make_tuple("append",
            std::vector<std::string>{"append(_1, _2)"},
            &create_append_operation, &create_primitive<append_operation>)};

    ///////////////////////////////////////////////////////////////////////////
    append_operation::append_operation(
        std::vector<primitive_argument_type> && operands,
        std::string const& name, std::string const& codename)
      : primitive_component_base(std::move(operands), name, codename)
    {
    }

    primitive_argument_type append_operation::handle_list_operands(
        primitive_argument_type && op1, primitive_argument_type && rhs) const
    {
        ir::range lhs =
            extract_list_value_strict(std::move(op1), name_, codename_);

        if (lhs.is_ref())
        {
            auto result = lhs.copy();
            result.emplace_back(std::move(rhs));
            return primitive_argument_type{std::move(result)};
        }

        lhs.args().emplace_back(std::move(rhs));
        return primitive_argument_type{std::move(lhs)};
    }

    ///////////////////////////////////////////////////////////////////////////
    hpx::future<primitive_argument_type> append_operation::eval(
        std::vector<primitive_argument_type> const& operands,
        std::vector<primitive_argument_type> const& args) const
    {
        if (operands.size() != 2)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "phylanx::execution_tree::primitives::append_operation::eval",
                execution_tree::generate_error_message(
                    "append_operation accepts exactly two arguments", name_,
                    codename_));
        }

        if (!valid(operands[0]) || !valid(operands[1]))
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter, "append_operation::eval",
                generate_error_message(
                    "the append_operation primitive requires that the "
                    "arguments "
                    "given by the operands array are valid"));
        }
        auto this_ = this->shared_from_this();
        return hpx::dataflow(hpx::launch::sync,
            hpx::util::unwrapping(
                [this_](primitive_argument_type&& lhs,
                    primitive_argument_type&& rhs) -> primitive_argument_type {
                    if (is_list_operand_strict(lhs))
                    {
                        return this_->handle_list_operands(
                            std::move(lhs), std::move(rhs));
                    }
                    HPX_THROW_EXCEPTION(hpx::bad_parameter,
                        "phylanx::execution_tree::primitives::append_operation:"
                        ":eval",
                        this_->generate_error_message(
                            "append_operation accepts a list "
                            "value as its lhs operand only"));
                }),
            value_operand(operands[0], args, name_, codename_),
            value_operand(operands[1], args, name_, codename_));
    }

    hpx::future<primitive_argument_type> append_operation::eval(
        std::vector<primitive_argument_type> const& args) const
    {
        if (this->no_operands())
        {
            return eval(args, noargs);
        }
        return eval(this->operands(), args);
    }
}}}
