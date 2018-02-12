//  Copyright (c) 2017-2018 Hartmut Kaiser
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>
#include <phylanx/execution_tree/primitives/base_primitive.hpp>
#include <phylanx/execution_tree/primitives/primitive_component_base.hpp>

#include <hpx/include/lcos.hpp>
#include <hpx/throw_exception.hpp>

#include <utility>
#include <vector>

namespace phylanx { namespace execution_tree { namespace primitives
{
    ///////////////////////////////////////////////////////////////////////////
    std::vector<primitive_argument_type> primitive_component_base::noargs{};

    primitive_component_base::primitive_component_base(
            std::vector<primitive_argument_type>&& params)
      : operands_(std::move(params))
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
    topology primitive_component_base::expression_topology() const
    {
        std::vector<hpx::future<topology>> results;
        results.reserve(operands_.size());

        for (auto& operand : operands_)
        {
            primitive const* p = util::get_if<primitive>(&operand);
            if (p != nullptr)
            {
                results.push_back(p->expression_topology());
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
}}}
