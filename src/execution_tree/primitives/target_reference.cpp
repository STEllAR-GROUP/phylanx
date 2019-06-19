//  Copyright (c) 2017-2019 Hartmut Kaiser
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>
#include <phylanx/execution_tree/primitives/target_reference.hpp>
#include <phylanx/execution_tree/primitives/primitive_component.hpp>

#include <hpx/include/lcos.hpp>
#include <hpx/include/naming.hpp>
#include <hpx/include/util.hpp>
#include <hpx/throw_exception.hpp>

#include <algorithm>
#include <iterator>
#include <set>
#include <string>
#include <utility>
#include <vector>

///////////////////////////////////////////////////////////////////////////////
namespace phylanx { namespace execution_tree { namespace primitives
{
    ///////////////////////////////////////////////////////////////////////////
    match_pattern_type const target_reference::match_data =
    {
        hpx::util::make_tuple("target-reference",
            std::vector<std::string>{},
            nullptr, &create_primitive<target_reference>,
            "Internal")
    };

    ///////////////////////////////////////////////////////////////////////////
    target_reference::target_reference(
            primitive_arguments_type&& args, std::string const& name,
            std::string const& codename)
      : primitive_component_base(std::move(args), name, codename)
      , ctx_(eval_context::noinit)
    {
        // operands_[0] holds the target function/variable
        if (operands_.empty() || !valid(operands_[0]))
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "target_reference::target_reference",
                generate_error_message("no target given"));
        }
    }

    // initialize evaluation context
    void target_reference::set_eval_context(eval_context ctx)
    {
        ctx_ = std::move(ctx);
    }

    ///////////////////////////////////////////////////////////////////////////
    hpx::future<primitive_argument_type> target_reference::eval(
        primitive_arguments_type const& params, eval_context ctx) const
    {
        // use stored evaluation context, if available
        if (ctx_)
        {
            ctx = ctx_;
        }

        if (operands_.size() > 1)
        {
            // the function has pre-bound arguments
            primitive_arguments_type fargs;
            fargs.reserve(operands_.size() - 1 + params.size());

            for (auto it = operands_.begin() + 1; it != operands_.end(); ++it)
            {
                fargs.emplace_back(extract_ref_value(*it, name_, codename_));
            }
            for (auto const& param : params)
            {
                fargs.emplace_back(
                    extract_value(std::move(param), name_, codename_));
            }

            return value_operand(operands_[0], std::move(fargs), name_,
                codename_, add_frame(set_mode(std::move(ctx), eval_default)));
        }

        return value_operand(operands_[0], std::move(params), name_, codename_,
            add_frame(set_mode(std::move(ctx), eval_dont_wrap_functions)));
    }

    hpx::future<primitive_argument_type> target_reference::eval(
        primitive_argument_type&& param, eval_context ctx) const
    {
        // use stored evaluation context, if available
        if (ctx_)
        {
            ctx = ctx_;
        }

        if (operands_.size() > 1)
        {
            // the function has pre-bound arguments
            primitive_arguments_type fargs;
            fargs.reserve(operands_.size());

            for (auto it = operands_.begin() + 1; it != operands_.end(); ++it)
            {
                fargs.emplace_back(extract_ref_value(*it, name_, codename_));
            }
            fargs.emplace_back(
                extract_value(std::move(param), name_, codename_));

            return value_operand(operands_[0], std::move(fargs), name_,
                codename_, add_frame(set_mode(std::move(ctx), eval_default)));
        }

        return value_operand(operands_[0], std::move(param), name_, codename_,
            add_frame(set_mode(std::move(ctx), eval_dont_wrap_functions)));
    }

    bool target_reference::bind(
        primitive_arguments_type const& args, eval_context ctx) const
    {
        return true;
    }

    void target_reference::store(primitive_arguments_type&& data,
        primitive_arguments_type&& params, eval_context ctx)
    {
        primitive* p = util::get_if<primitive>(&operands_[0]);
        if (p != nullptr)
        {
            p->store(hpx::launch::sync, std::move(data), std::move(params),
                std::move(ctx));
        }
    }

    topology target_reference::expression_topology(
        std::set<std::string>&& functions,
        std::set<std::string>&& resolve_children) const
    {
        primitive const* p = util::get_if<primitive>(&operands_[0]);
        if (p != nullptr)
        {
            return p->expression_topology(hpx::launch::sync,
                std::move(functions), std::move(resolve_children));
        }
        return {};
    }
}}}

