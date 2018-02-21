//  Copyright (c) 2017-2018 Hartmut Kaiser
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>
#include <phylanx/execution_tree/compiler/primitive_name.hpp>
#include <phylanx/execution_tree/primitives/base_primitive.hpp>
#include <phylanx/execution_tree/primitives/primitive_component_base.hpp>

#include <hpx/include/lcos.hpp>
#include <hpx/include/util.hpp>
#include <hpx/throw_exception.hpp>

#include <set>
#include <string>
#include <utility>
#include <vector>

namespace phylanx { namespace execution_tree { namespace primitives
{
    ///////////////////////////////////////////////////////////////////////////
    std::vector<primitive_argument_type> primitive_component_base::noargs{};

    primitive_component_base::primitive_component_base(
            std::vector<primitive_argument_type>&& params,
            std::string const& name, std::string const& codename)
      : operands_(std::move(params))
      , name_(name)
      , codename_(codename)
    {
    }

    // eval_action
    hpx::future<primitive_argument_type> primitive_component_base::eval(
        std::vector<primitive_argument_type> const& params) const
    {
        return hpx::make_ready_future(eval_direct(params));
    }

    // direct_eval_action
    primitive_argument_type primitive_component_base::eval_direct(
        std::vector<primitive_argument_type> const& params) const
    {
        return eval(params).get();
    }

    // store_action
    void primitive_component_base::store(primitive_argument_type &&)
    {
        HPX_THROW_EXCEPTION(hpx::invalid_status,
            "phylanx::execution_tree::primitives::primitive_component_base",
            "store function should only be called for the store_primitive");
    }

    // extract_topology_action
    topology primitive_component_base::expression_topology(
        std::set<std::string>&& functions) const
    {
        std::vector<hpx::future<topology>> results;
        results.reserve(operands_.size());

        for (auto& operand : operands_)
        {
            primitive const* p = util::get_if<primitive>(&operand);
            if (p != nullptr)
            {
                std::set<std::string> funcs{functions};
                results.push_back(p->expression_topology(std::move(funcs)));
            }
        }

        std::vector<topology> children;
        if (!results.empty())
        {
            hpx::wait_all(results);

            for (auto& r : results)
            {
                children.emplace_back(r.get());
            }
        }

        return topology{std::move(children)};
    }

    // set_body_action (define_function only)
    void primitive_component_base::set_body(primitive_argument_type&& target)
    {
        HPX_THROW_EXCEPTION(hpx::invalid_status,
            "phylanx::execution_tree::primitives::primitive_component_base",
            "set_body function should only be called for the "
                "define_function_primitive");
    }

    std::string primitive_component_base::generate_error_message(
        std::string const& msg) const
    {
        return execution_tree::generate_error_message(msg, name_, codename_);
    }
}}}

namespace phylanx { namespace execution_tree
{
    std::string generate_error_message(std::string const& msg,
        std::string const& name, std::string const& codename)
    {
        if (!name.empty())
        {
            auto parts = compiler::parse_primitive_name(name);

            std::string line_col;
            if (parts.tag1 != -1 && parts.tag2 != -1)
            {
                line_col = hpx::util::format("(%1%, %2%)", parts.tag1, parts.tag2);
            }

            if (!parts.instance.empty())
            {
                return hpx::util::format("%1%%2%: %3%$%4%:: %5%",
                    codename.empty() ? "<unknown>" : codename,
                    line_col, parts.primitive, parts.instance, msg);
            }

            return hpx::util::format("%1%%2%: %3%:: %4%",
                codename.empty() ? "<unknown>" : codename, line_col,
                parts.primitive, msg);
        }

        return hpx::util::format(
            "%1%: %2%", codename.empty() ? "<unknown>" : codename, msg);
    }
}}
