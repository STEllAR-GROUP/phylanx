//  Copyright (c) 2018 Hartmut Kaiser
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>
#include <phylanx/execution_tree/primitives/primitive_argument_type.hpp>
#include <phylanx/plugins/controls/apply.hpp>

#include <hpx/include/lcos.hpp>
#include <hpx/include/naming.hpp>
#include <hpx/include/util.hpp>
#include <hpx/runtime/launch_policy.hpp>
#include <hpx/throw_exception.hpp>
#include <hpx/util/assert.hpp>

#include <cstddef>
#include <string>
#include <utility>
#include <vector>

///////////////////////////////////////////////////////////////////////////////
namespace phylanx { namespace execution_tree { namespace primitives
{
    ///////////////////////////////////////////////////////////////////////////
    match_pattern_type const apply::match_data =
    {
        hpx::util::make_tuple("apply",
            std::vector<std::string>{"apply(_1, _2)"},
            &create_apply, &create_primitive<apply>,
            R"(func, range
            Call function `func` one time using the values in `range`
            as the argument list.

            Args:

                func(function) : a function
                range(iter) : an iterator with at least as many values
                              as function `func` has arguments.

            Returns:

            The return value of function `func` when called with the
            arguments supplied by `range`.)"
            )
    };

    ///////////////////////////////////////////////////////////////////////////
    apply::apply(primitive_arguments_type&& operands,
            std::string const& name, std::string const& codename)
      : primitive_component_base(std::move(operands), name, codename)
    {}

    hpx::future<primitive_argument_type> apply::eval(
        primitive_arguments_type const& params, eval_context ctx) const
    {
        if (operands_.size() != 2)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "apply::eval",
                generate_error_message(
                    "the apply primitive requires exactly two operands"));
        }

        primitive const* p = util::get_if<primitive>(&operands_[0]);
        if (p == nullptr)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "apply::eval",
                generate_error_message(
                    "the first argument to apply must be an invocable object"));
        }

        auto this_ = this->shared_from_this();
        return hpx::dataflow(hpx::launch::sync, hpx::util::unwrapping(
            [this_ = std::move(this_), ctx](
                primitive_argument_type&& func, ir::range&& list)
            {
                if (list.is_ref())
                {
                    return value_operand_sync(func, std::move(list.args()),
                        this_->name_, this_->codename_, ctx);
                }
                return value_operand_sync(
                    func, list.copy(), this_->name_, this_->codename_, ctx);
            }),
            value_operand(operands_[0], params, name_, codename_,
                add_mode(ctx,
                    eval_mode(eval_dont_wrap_functions |
                        eval_dont_evaluate_partials |
                        eval_dont_evaluate_lambdas))),
            list_operand(operands_[1], params, name_, codename_, ctx));
    }
}}}
