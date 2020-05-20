//  Copyright (c) 2017 Hartmut Kaiser
//  Copyright (c) 2017 Adrian Serio
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>
#include <phylanx/plugins/controls/if_conditional.hpp>

#include <hpx/include/lcos.hpp>
#include <hpx/include/naming.hpp>
#include <hpx/include/util.hpp>
#include <hpx/errors/throw_exception.hpp>

#include <cstddef>
#include <cstdint>
#include <memory>
#include <numeric>
#include <string>
#include <utility>
#include <vector>

///////////////////////////////////////////////////////////////////////////////
namespace phylanx { namespace execution_tree { namespace primitives
{
    ///////////////////////////////////////////////////////////////////////////
    match_pattern_type const if_conditional::match_data =
    {
        hpx::util::make_tuple("if",
            std::vector<std::string>{"if(_1, _2, _3)", "if(_1, _2)"},
            &create_if_conditional, &create_primitive<if_conditional>,
            R"(cond, thenf, elsef
            This primitive implements the if statement in Python.
            The statement `thenf` is evaluated if `cond` is True and
            the statement `elsef` is evaluated otherwise.

            Args:

                cond (boolean expression) : a boolean expression
                thenf (statement) : a statement
                elsef (statement, optional) : a statement

            Returns:

              The value returned by the statement that was executed, `nil`
              otherwise)"
            )
    };

    ///////////////////////////////////////////////////////////////////////////
    if_conditional::if_conditional(
            primitive_arguments_type&& operands,
            std::string const& name, std::string const& codename)
      : primitive_component_base(std::move(operands), name, codename)
    {}

    ///////////////////////////////////////////////////////////////////////////
    hpx::future<primitive_argument_type> if_conditional::eval(
        primitive_arguments_type const& operands,
        primitive_arguments_type const& args, eval_context ctx) const
    {
        if (operands_.size() != 3 && operands_.size() != 2)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "if_conditional::if_conditional",
                util::generate_error_message(
                    "the if_conditional primitive requires three "
                    "operands",
                    name_, codename_));
        }

        bool arguments_valid = true;
        for (std::size_t i = 0; i != operands_.size(); ++i)
        {
            if (!valid(operands_[i]))
            {
                arguments_valid = false;
            }
        }

        if (!arguments_valid)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "if_conditional::if_conditional",
                util::generate_error_message(
                    "the if_conditional primitive requires that the "
                    "arguments given by the operands array "
                    "is valid",
                    name_, codename_));
        }

        // Keep data alive with a shared pointer
        auto f =
            scalar_boolean_operand(operands_[0], args, name_, codename_, ctx);
        auto this_ = this->shared_from_this();
        return f.then(hpx::launch::sync,
            [this_ = std::move(this_), args = args, ctx = std::move(ctx)](
                    hpx::future<std::uint8_t>&& cond_eval) mutable
            ->  hpx::future<primitive_argument_type>
            {
                if (cond_eval.get() != 0)
                {
                    return value_operand(this_->operands_[1], std::move(args),
                        this_->name_, this_->codename_, std::move(ctx));
                }
                if (this_->operands_.size() > 2)
                {
                    return value_operand(this_->operands_[2], std::move(args),
                        this_->name_, this_->codename_, std::move(ctx));
                }
                return hpx::make_ready_future(primitive_argument_type{});
            });
    }
}}}
