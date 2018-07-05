//  Copyright (c) 2017-2018 Hartmut Kaiser
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>
#include <phylanx/execution_tree/primitives/call_function.hpp>

#include <hpx/include/lcos.hpp>
#include <hpx/include/naming.hpp>
#include <hpx/include/util.hpp>
#include <hpx/throw_exception.hpp>

#include <cstddef>
#include <memory>
#include <set>
#include <string>
#include <utility>
#include <vector>

///////////////////////////////////////////////////////////////////////////////
namespace phylanx { namespace execution_tree { namespace primitives
{
    ///////////////////////////////////////////////////////////////////////////
    match_pattern_type const call_function::match_data =
    {
        hpx::util::make_tuple("call-function",
            std::vector<std::string>{},
            nullptr, &create_primitive<call_function>)
    };

    ///////////////////////////////////////////////////////////////////////////
    call_function::call_function(
            std::vector<primitive_argument_type>&& args,
            std::string const& name, std::string const& codename)
      : primitive_component_base(std::move(args), name, codename)
    {
        // the first entry of operands represents the target
        if (this->no_operands())
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "call_function::call_function",
                generate_error_message("no target given"));
        }

        if (valid(operands_[0]))
        {
            operands_[0] = extract_copy_value(std::move(operands_[0]));
        }
    }

    ///////////////////////////////////////////////////////////////////////////
    hpx::future<primitive_argument_type> call_function::eval(
        std::vector<primitive_argument_type> const& params) const
    {
        if (!valid(operands_[0]))
        {
            HPX_THROW_EXCEPTION(hpx::invalid_status,
                "call_function::eval",
                generate_error_message(
                    "the expression representing the function target "
                        "has not been initialized"));
        }

        std::vector<primitive_argument_type> fargs;
        fargs.reserve(operands_.size() - 1);

        // pass along pre-bound arguments
        for (auto it = operands_.begin() + 1; it != operands_.end(); ++it)
        {
            fargs.push_back(extract_ref_value(*it));
        }

        auto this_ = this->shared_from_this();
        return hpx::dataflow(hpx::launch::sync, hpx::util::unwrapping(
                [this_](primitive_argument_type&& func,
                    std::vector<primitive_argument_type>&& args)
                {
                    return value_operand_sync(std::move(func),
                        std::move(args), this_->name_, this_->codename_);
                }),
            value_operand(operands_[0], params, name_, codename_,
                eval_mode(eval_dont_wrap_functions |
                    eval_dont_evaluate_partials |
                    eval_dont_evaluate_lambdas)),
            detail::map_operands(std::move(fargs), functional::value_operand{},
                params, name_, codename_, eval_dont_evaluate_partials));
    }

    bool call_function::bind(
        std::vector<primitive_argument_type> const& params) const
    {
        if (!valid(operands_[0]))
        {
            HPX_THROW_EXCEPTION(hpx::invalid_status,
                "call_function::bind",
                generate_error_message(
                    "the expression representing the function target "
                        "has not been initialized"));
        }

        // evaluation of the define-function yields the function body
        primitive const* p = util::get_if<primitive>(&operands_[0]);
        if (p != nullptr)
        {
            if (operands_.size() > 1)
            {
                std::vector<primitive_argument_type> fargs;
                fargs.reserve(operands_.size() - 1);

                // handle pre-bound arguments
                for (auto it = operands_.begin() + 1; it != operands_.end();
                     ++it)
                {
                    // bind pre-bound arguments using actual ones
                    primitive const* arg = util::get_if<primitive>(&*it);
                    if (arg != nullptr)
                    {
                        if (arg->bind(params))
                        {
                            fargs.push_back(
                                arg->eval(hpx::launch::sync, params));
                        }
                    }
                    else
                    {
                        fargs.push_back(extract_ref_value(*it));
                    }
                }

                if (p->bind(fargs))
                {
                    return true;
                }
            }

            if (p->bind(params))
            {
                return true;
            }
        }

        return false;
    }

    void call_function::store(primitive_argument_type&& data)
    {
        if (valid(operands_[0]))
        {
            HPX_THROW_EXCEPTION(hpx::invalid_status,
                "call_function::store",
                generate_error_message(
                    "the expression representing the function target "
                        "has already been initialized"));
        }

        operands_[0] = extract_copy_value(std::move(data));
    }

    topology call_function::expression_topology(
        std::set<std::string>&& functions) const
    {
        primitive const* p = util::get_if<primitive>(&operands_[0]);
        if (p != nullptr)
        {
            return p->expression_topology(
                hpx::launch::sync, std::move(functions));
        }
        return {};
    }
}}}

