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
#include <hpx/throw_exception.hpp>

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
            "cond,thenf,elsef"
            "This primitive implements the if statement in Python.\n"
            "The statement `thenf` is evaluated if `cond` is True and \n"
            "the statement `elsef` is evaluated otherwise.\n"
            "\n"
            "Args:\n"
            "\n"
            "    cond (boolean expression)  : a boolean expression\n"
            "    thenf (statement) : a statement\n"
            "    elsef (statement) : a statement\n"
            "\n"
            "Returns:"
            )
    };

    ///////////////////////////////////////////////////////////////////////////
    if_conditional::if_conditional(
            std::vector<primitive_argument_type>&& operands,
            std::string const& name, std::string const& codename)
      : primitive_component_base(std::move(operands), name, codename)
    {}

    ///////////////////////////////////////////////////////////////////////////
    hpx::future<primitive_argument_type> if_conditional::eval(
        std::vector<primitive_argument_type> const& operands,
        std::vector<primitive_argument_type> const& args) const
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
        auto f = boolean_operand(operands_[0], args, name_, codename_);
        auto this_ = this->shared_from_this();
        return f.then(hpx::launch::sync,
            [this_, args = std::move(args)](
                hpx::future<std::uint8_t>&& cond_eval) mutable
            -> hpx::future<primitive_argument_type>
            {
                if (cond_eval.get() != 0)
                {
                    return literal_operand(this_->operands_[1],
                        std::move(args), this_->name_, this_->codename_);
                }
                if (this_->operands_.size() > 2)
                {
                    return literal_operand(this_->operands_[2],
                        std::move(args), this_->name_, this_->codename_);
                }
                return hpx::make_ready_future(primitive_argument_type{});
            });
    }

    ///////////////////////////////////////////////////////////////////////////
    // Evaluate 'true_case' or 'false_case' based on 'cond'
    hpx::future<primitive_argument_type> if_conditional::eval(
        std::vector<primitive_argument_type> const& args) const
    {
        if (this->no_operands())
        {
            return eval(args, noargs);
        }
        return eval(this->operands(), args);
    }
}}}
