//  Copyright (c) 2017-2018 Hartmut Kaiser
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
    {
        // operands_[0] holds the target function/variable
        if (operands_.empty() || !valid(operands_[0]))
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "target_reference::target_reference",
                generate_error_message("no target given"));
        }

        // try to bind to the function object locally
        primitive* p = util::get_if<primitive>(&operands_[0]);
        if (p != nullptr)
        {
            hpx::error_code ec(hpx::lightweight);
            target_ = hpx::get_ptr<primitive_component>(
                hpx::launch::sync, p->get_id(), ec);
        }
    }

    ///////////////////////////////////////////////////////////////////////////
    hpx::future<primitive_argument_type> target_reference::eval(
        primitive_arguments_type const& params, eval_context ctx) const
    {
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
                fargs.emplace_back(extract_value(param, name_, codename_));
            }

            if (target_)
            {
                return target_->eval(
                    std::move(fargs), set_mode(std::move(ctx), eval_default));
            }

            return value_operand(operands_[0], std::move(fargs), name_,
                codename_, set_mode(std::move(ctx), eval_default));
        }

        if (target_)
        {
            return target_->eval(
                params, add_mode(std::move(ctx), eval_dont_wrap_functions));
        }

        return value_operand(operands_[0], params, name_, codename_,
            add_mode(std::move(ctx), eval_dont_wrap_functions));
    }

    hpx::future<primitive_argument_type> target_reference::eval(
        primitive_argument_type && param, eval_context) const
    {
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

            if (target_)
            {
                return target_->eval(fargs, eval_context{});
            }

            return value_operand(
                operands_[0], std::move(fargs), name_, codename_);
        }

        if (target_)
        {
            return target_->eval_single(
                std::move(param), eval_context(eval_dont_wrap_functions));
        }

        return value_operand(operands_[0], std::move(param), name_, codename_,
            eval_context(eval_dont_wrap_functions));
    }

    void target_reference::store(primitive_arguments_type&& data,
        primitive_arguments_type&& params)
    {
        if (target_)
        {
            target_->store(std::move(data), std::move(params));
        }
        else
        {
            primitive* p = util::get_if<primitive>(&operands_[0]);
            if (p != nullptr)
            {
                p->store(std::move(data), std::move(params));
            }
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

