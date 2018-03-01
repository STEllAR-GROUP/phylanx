//  Copyright (c) 2017-2018 Hartmut Kaiser
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(PHYLANX_PRIMITIVES_PRIMITIVE_COMPONENT_FEB_10_2018_0122PM)
#define PHYLANX_PRIMITIVES_PRIMITIVE_COMPONENT_FEB_10_2018_0122PM

#include <phylanx/config.hpp>
#include <phylanx/execution_tree/primitives/base_primitive.hpp>
#include <phylanx/execution_tree/primitives/primitive_component_base.hpp>

#include <hpx/include/actions.hpp>
#include <hpx/include/components.hpp>
#include <hpx/include/util.hpp>

#include <cstdint>
#include <memory>
#include <set>
#include <string>
#include <utility>
#include <vector>

namespace phylanx { namespace execution_tree { namespace primitives
{
    ///////////////////////////////////////////////////////////////////////////
    struct primitive_component_base;

    ///////////////////////////////////////////////////////////////////////////
    class primitive_component
      : public hpx::components::component_base<primitive_component>
    {
    private:
        PHYLANX_EXPORT static std::shared_ptr<primitive_component_base>
        create_primitive(std::string const& type,
            std::vector<primitive_argument_type>&& params,
            std::string const& name, std::string const& codename);

    public:
        primitive_component() = default;

        primitive_component(std::string const& type,
                std::vector<primitive_argument_type>&& operands,
                std::string const& name, std::string const& codename)
          : primitive_(
                create_primitive(type, std::move(operands), name, codename))
          , eval_count_(0ll)
          , eval_duration_(0ll)
          , eval_direct_count_(0ll)
          , eval_direct_duration_(0ll)
        {
        }

        PHYLANX_EXPORT hpx::future<primitive_argument_type> eval(
            std::vector<primitive_argument_type> const& params) const;

        // direct_eval_action
        PHYLANX_EXPORT primitive_argument_type eval_direct(
            std::vector<primitive_argument_type> const& params) const;

        // store_action
        PHYLANX_EXPORT void store(primitive_argument_type &&);

        // extract_topology_action
        PHYLANX_EXPORT topology expression_topology(
            std::set<std::string>&& functions) const;

        // set_body_action (define_function only)
        PHYLANX_EXPORT void set_body(primitive_argument_type&& target);

#if defined(PHYLANX_DEBUG)
        HPX_DEFINE_COMPONENT_DIRECT_ACTION(
            primitive_component, eval, eval_action);
        HPX_DEFINE_COMPONENT_DIRECT_ACTION(primitive_component,
            expression_topology, expression_topology_action);
#else
        HPX_DEFINE_COMPONENT_ACTION(
            primitive_component, eval, eval_action);
        HPX_DEFINE_COMPONENT_ACTION(
            primitive_component, expression_topology,
            expression_topology_action);
#endif
        HPX_DEFINE_COMPONENT_DIRECT_ACTION(
            primitive_component, eval_direct, eval_direct_action);
        HPX_DEFINE_COMPONENT_ACTION(
            primitive_component, store, store_action);
        HPX_DEFINE_COMPONENT_ACTION(
            primitive_component, set_body, set_body_action);

        // access data for performance counter
        std::int64_t get_eval_count(bool reset, bool direct) const
        {
            if (!direct)
            {
                return hpx::util::get_and_reset_value(eval_count_, reset);
            }
            return hpx::util::get_and_reset_value(eval_direct_count_, reset);
        }

        std::int64_t get_eval_duration(bool reset, bool direct) const
        {
            if (!direct)
            {
                return hpx::util::get_and_reset_value(eval_duration_, reset);
            }
            return hpx::util::get_and_reset_value(eval_direct_duration_, reset);
        }

    private:
        std::shared_ptr<primitive_component_base> primitive_;

        // Performance counter data
        mutable std::int64_t eval_count_;
        mutable std::int64_t eval_duration_;
        mutable std::int64_t eval_direct_count_;
        mutable std::int64_t eval_direct_duration_;
    };
}}}

// Declaration of serialization support for the local_file actions
HPX_REGISTER_ACTION_DECLARATION(
    phylanx::execution_tree::primitives::primitive_component::eval_action,
    phylanx_primitive_eval_action);
HPX_REGISTER_ACTION_DECLARATION(
    phylanx::execution_tree::primitives::primitive_component::eval_direct_action,
    phylanx_primitive_eval_direct_action);
HPX_REGISTER_ACTION_DECLARATION(
    phylanx::execution_tree::primitives::primitive_component::store_action,
    phylanx_primitive_store_action);
HPX_REGISTER_ACTION_DECLARATION(
    phylanx::execution_tree::primitives::
        primitive_component::expression_topology_action,
    phylanx_primitive_expression_topology_action);
HPX_REGISTER_ACTION_DECLARATION(
    phylanx::execution_tree::primitives::primitive_component::set_body_action,
    phylanx_primitive_set_body_action);

#endif
