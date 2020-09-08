//  Copyright (c) 2020 Hartmut Kaiser
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>
#include <phylanx/execution_tree/primitives/timer.hpp>

#include <hpx/errors/throw_exception.hpp>
#include <hpx/include/lcos.hpp>
#include <hpx/include/naming.hpp>
#include <hpx/include/util.hpp>

#include <cstddef>
#include <cstdint>
#include <memory>
#include <string>
#include <utility>
#include <vector>

///////////////////////////////////////////////////////////////////////////////
namespace phylanx { namespace execution_tree { namespace primitives {

    ///////////////////////////////////////////////////////////////////////////
    primitive create_timer(hpx::id_type const& locality,
        primitive_arguments_type&& operands, std::string const& name,
        std::string const& codename)
    {
        static std::string type("timer");
        return create_primitive_component(
            locality, type, std::move(operands), name, codename);
    }

    match_pattern_type const timer::match_data = {

        hpx::util::make_tuple("timer",
            std::vector<std::string>{"timer(_1, _2)"}, &create_timer,
            &create_primitive<timer>,
            R"(arg, done
            This primitive allows to measure the execution time of the embedded
            expression 'arg' and reports it to the supplied reporting function
            'done'.

            Args:

                arg (expression) : the operation to measure the execution time for
                done (function) : a function that will be called with the measured time

            Returns:

            whatever the evaluation of 'arg' returns)"
        )};

    ///////////////////////////////////////////////////////////////////////////
    timer::timer(primitive_arguments_type&& operands, std::string const& name,
        std::string const& codename)
      : primitive_component_base(std::move(operands), name, codename)
    {
    }

    hpx::future<primitive_argument_type> timer::eval(
        primitive_arguments_type const& operands,
        primitive_arguments_type const& args, eval_context ctx) const
    {
        if (operands.size() != 2)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "timer::eval",
                generate_error_message(
                    "the timer primitive requires exactly two operands", ctx));
        }

        if (!valid(operands[0]) || !valid(operands[1]))
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "timer::eval",
                generate_error_message(
                    "the timer primitive requires that the "
                    "arguments given by the operands array are valid", ctx));
        }

        auto this_ = this->shared_from_this();
        return hpx::dataflow(hpx::launch::sync,
            [this_ = std::move(this_), ctx, func = operands[0], args](
                    hpx::future<primitive_argument_type>&& l) mutable
                -> primitive_argument_type
            {
                hpx::util::high_resolution_timer t;

                // execute body
                primitive const* p = util::get_if<primitive>(&func);
                if (p == nullptr)
                {
                    HPX_THROW_EXCEPTION(hpx::bad_parameter,
                        "timer::eval",
                        this_->generate_error_message(
                            "the first argument to timer must be an "
                            "invocable object", ctx));
                }

                primitive_argument_type result = p->eval(hpx::launch::sync,
                    primitive_argument_type{}, ctx);

                double elapsed = t.elapsed();

                // report timings
                auto&& arg = l.get();

                primitive const* r = util::get_if<primitive>(&arg);
                if (p == nullptr)
                {
                    HPX_THROW_EXCEPTION(hpx::bad_parameter,
                        "timer::eval",
                        this_->generate_error_message(
                            "the second argument to timer must be an "
                            "invocable object", ctx));
                }

                r->eval(hpx::launch::sync, primitive_argument_type{elapsed},
                    std::move(ctx));

                // return overall result
                return result;
            },
            value_operand(operands[1], args, name_, codename_,
                add_mode(ctx, eval_dont_evaluate_lambdas)));
    }
}}}    // namespace phylanx::execution_tree::primitives
