// Copyright (c) 2017-2018 Hartmut Kaiser
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>
#include <phylanx/plugins/controls/block_operation.hpp>

#include <hpx/include/lcos.hpp>
#include <hpx/include/naming.hpp>
#include <hpx/include/util.hpp>
#include <hpx/throw_exception.hpp>

#include <cstddef>
#include <memory>
#include <string>
#include <utility>
#include <vector>

///////////////////////////////////////////////////////////////////////////////
namespace phylanx { namespace execution_tree { namespace primitives
{
    ///////////////////////////////////////////////////////////////////////////
    match_pattern_type const block_operation::match_data =
    {
        hpx::util::make_tuple("block",
            std::vector<std::string>{"block(__1)"},
            &create_block_operation, &create_primitive<block_operation>,
            R"(stmt
            Args:

                *stmt (statement list) :  a list of statements.

            Returns:

            The value of the last statement.)"
            )
    };

    ///////////////////////////////////////////////////////////////////////////

    block_operation::block_operation(
            primitive_arguments_type&& operands,
            std::string const& name, std::string const& codename)
      : primitive_component_base(std::move(operands), name, codename)
    {}

    hpx::future<primitive_argument_type> block_operation::eval(
        primitive_arguments_type const& operands,
        primitive_arguments_type const& args, eval_context ctx) const
    {
        // Empty blocks are allowed (Issue #278)
        if (this->no_operands())
        {
            return hpx::make_ready_future(primitive_argument_type{});
        }

        hpx::future<primitive_argument_type> f;

        std::size_t size = operands_.size();
        for (std::size_t i = 0; i != size; ++i)
        {
            if (i == size - 1)
            {
                f = value_operand(
                    operands_[i], args, name_, codename_, std::move(ctx));
            }
            else
            {
                value_operand_sync(operands_[i], args, name_, codename_, ctx);
            }
        }

        return f;
    }
}}}
