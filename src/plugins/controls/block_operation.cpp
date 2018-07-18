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
            &create_block_operation, &create_primitive<block_operation>)
    };

    ///////////////////////////////////////////////////////////////////////////

    block_operation::block_operation(
            std::vector<primitive_argument_type>&& operands,
            std::string const& name, std::string const& codename)
      : primitive_component_base(std::move(operands), name, codename)
    {}

    void block_operation::next(std::size_t i,
        std::vector<primitive_argument_type> && args,
        hpx::lcos::local::promise<primitive_argument_type> && result) const
    {
        // skip statements that don't return anything
        while (i != operands_.size() && !valid(operands_[i]))
        {
            if (i == operands_.size() - 1)
            {
                result.set_value(primitive_argument_type{});
            }
            ++i;
        }

        if (i == operands_.size())
            return;

        auto this_ = this->shared_from_this();
        auto f = value_operand(operands_[i], args, name_, codename_);
        f.then(
            hpx::launch::sync,
            [this_, i, args = std::move(args), result = std::move(result)](
                hpx::future<primitive_argument_type>&& step) mutable -> void
            {
                try
                {
                    // the value of the last step is returned
                    if (i == this_->operands_.size() - 1)
                    {
                        result.set_value(step.get());
                        return;
                    }

                    step.get();    // rethrow exception

                    // trigger next step
                    this_->next(i + 1, std::move(args), std::move(result));
                }
                catch (...)
                {
                    result.set_exception(std::current_exception());
                }
            });
    }

    hpx::future<primitive_argument_type> block_operation::eval(
        std::vector<primitive_argument_type> const& operands,
        std::vector<primitive_argument_type> args) const
    {
        // Empty blocks are allowed (Issue #278)
        if (this->no_operands())
        {
            return hpx::make_ready_future(primitive_argument_type{ast::nil{}});
        }
        hpx::lcos::local::promise<primitive_argument_type> result;
        auto f = result.get_future();
        next(0, std::move(args), std::move(result));    // trigger first step
        return f;
    }

    //////////////////////////////////////////////////////////////////////////

    // start iteration over given block statement
    hpx::future<primitive_argument_type> block_operation::eval(
        std::vector<primitive_argument_type> const& args) const
    {
        if (this->no_operands())
        {
            return eval(args, noargs);
        }
        return eval(this->operands(), args);
    }
}}}
