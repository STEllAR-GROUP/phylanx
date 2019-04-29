//  Copyright (c) 2017-2019 Hartmut Kaiser
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
#include <hpx/include/future.hpp>
#include <hpx/include/util.hpp>
#include <hpx/runtime/launch_policy.hpp>

#include <cstddef>
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
            primitive_arguments_type&& params, std::string const& name,
            std::string const& codename);

    public:
        primitive_component() = default;

        primitive_component(std::string const& type,
                primitive_arguments_type&& operands,
                std::string const& name, std::string const& codename)
          : primitive_(
                create_primitive(type, std::move(operands), name, codename))
        {
        }

        primitive_component(std::string const& type,
                primitive_arguments_type&& operands, eval_context ctx,
                std::string const& name, std::string const& codename)
          : primitive_(
                create_primitive(type, std::move(operands), name, codename))
        {
            primitive_->set_eval_context(std::move(ctx));
        }

        // eval_action
        PHYLANX_EXPORT hpx::future<primitive_argument_type> eval(
            primitive_arguments_type const& params,
            eval_context ctx) const;

        PHYLANX_EXPORT hpx::future<primitive_argument_type> eval_single(
            primitive_argument_type&& param, eval_context ctx) const;

        // store_action
        PHYLANX_EXPORT void store(primitive_arguments_type&&,
            primitive_arguments_type&&, eval_context ctx);

        PHYLANX_EXPORT void store_single(primitive_argument_type&&,
            primitive_arguments_type&&, eval_context ctx);

        // extract_topology_action
        PHYLANX_EXPORT topology expression_topology(
            std::set<std::string>&& functions,
            std::set<std::string>&& resolve_children) const;

        // bind an invocable object
        PHYLANX_EXPORT bool bind(
            primitive_arguments_type const& params, eval_context ctx) const;

        HPX_DEFINE_COMPONENT_ACTION(
            primitive_component, eval, eval_action);
        HPX_DEFINE_COMPONENT_ACTION(
            primitive_component, eval_single, eval_single_action);
        HPX_DEFINE_COMPONENT_ACTION(
            primitive_component, expression_topology,
            expression_topology_action);
        HPX_DEFINE_COMPONENT_DIRECT_ACTION(
            primitive_component, bind, bind_action);
        HPX_DEFINE_COMPONENT_ACTION(
            primitive_component, store, store_action);
        HPX_DEFINE_COMPONENT_ACTION(
            primitive_component, store_single, store_single_action);

        // access data for performance counter
        PHYLANX_EXPORT std::int64_t get_eval_count(bool reset) const;
        PHYLANX_EXPORT std::int64_t get_eval_duration(bool reset) const;
        PHYLANX_EXPORT std::int64_t get_direct_execution(bool reset) const;

        PHYLANX_EXPORT void enable_measurements();

        PHYLANX_EXPORT hpx::launch select_direct_eval_execution(
            hpx::launch policy) const;

        // decide whether to execute eval directly
        PHYLANX_EXPORT static hpx::launch select_direct_execution(
            eval_action, hpx::launch policy, hpx::naming::address_type lva);
        PHYLANX_EXPORT static hpx::launch select_direct_execution(
            eval_single_action, hpx::launch policy,
            hpx::naming::address_type lva);

    private:
        std::shared_ptr<primitive_component_base> primitive_;
    };
}}}

// Declaration of serialization support for the local_file actions
HPX_REGISTER_ACTION_DECLARATION(
    phylanx::execution_tree::primitives::primitive_component::eval_action,
    phylanx_primitive_eval_action);
HPX_REGISTER_ACTION_DECLARATION(
    phylanx::execution_tree::primitives::primitive_component::eval_single_action,
    phylanx_primitive_eval_single_action);
HPX_REGISTER_ACTION_DECLARATION(
    phylanx::execution_tree::primitives::primitive_component::store_action,
    phylanx_primitive_store_action);
HPX_REGISTER_ACTION_DECLARATION(
    phylanx::execution_tree::primitives::primitive_component::store_single_action,
    phylanx_primitive_store_single_action);
HPX_REGISTER_ACTION_DECLARATION(
    phylanx::execution_tree::primitives::
        primitive_component::expression_topology_action,
    phylanx_primitive_expression_topology_action);
HPX_REGISTER_ACTION_DECLARATION(
    phylanx::execution_tree::primitives::primitive_component::bind_action,
    phylanx_primitive_bind_action);

#endif
