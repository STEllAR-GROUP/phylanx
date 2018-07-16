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
            std::vector<primitive_argument_type>&& params,
            std::string const& name, std::string const& codename);

    public:
        primitive_component() = default;

        primitive_component(std::string const& type,
                std::vector<primitive_argument_type>&& operands,
                std::string const& name, std::string const& codename)
          : primitive_(
                create_primitive(type, std::move(operands), name, codename))
        {
        }

        PHYLANX_EXPORT hpx::future<primitive_argument_type> eval(
            std::vector<primitive_argument_type> const& params,
            eval_mode mode) const;

        // store_action
        PHYLANX_EXPORT void store(primitive_argument_type&&);
        PHYLANX_EXPORT void store_set_1d(
            phylanx::ir::node_data<double>&&, std::vector<int64_t>&&);
        PHYLANX_EXPORT void store_set_2d(phylanx::ir::node_data<double>&&,
            std::vector<int64_t>&&, std::vector<int64_t>&&);

        // extract_topology_action
        PHYLANX_EXPORT topology expression_topology(
            std::set<std::string>&& functions,
            std::set<std::string>&& resolve_children) const;

        // bind an invocable object
        PHYLANX_EXPORT bool bind(
            std::vector<primitive_argument_type> const& params) const;

        HPX_DEFINE_COMPONENT_ACTION(
            primitive_component, eval, eval_action);
        HPX_DEFINE_COMPONENT_ACTION(
            primitive_component, expression_topology,
            expression_topology_action);
        HPX_DEFINE_COMPONENT_DIRECT_ACTION(
            primitive_component, bind, bind_action);
        HPX_DEFINE_COMPONENT_ACTION(
            primitive_component, store, store_action);
        HPX_DEFINE_COMPONENT_ACTION(
            primitive_component, store_set_1d, store_set_1d_action);
        HPX_DEFINE_COMPONENT_ACTION(
            primitive_component, store_set_2d, store_set_2d_action);

        // access data for performance counter
        PHYLANX_EXPORT std::int64_t get_eval_count(bool reset) const;
        PHYLANX_EXPORT std::int64_t get_eval_duration(bool reset) const;
        PHYLANX_EXPORT std::int64_t get_direct_execution(bool reset) const;

        // decide whether to execute eval directly
        PHYLANX_EXPORT static hpx::launch select_direct_execution(eval_action,
            hpx::launch policy, hpx::naming::address_type lva);

    private:
        std::shared_ptr<primitive_component_base> primitive_;
    };
}}}

// Declaration of serialization support for the local_file actions
HPX_REGISTER_ACTION_DECLARATION(
    phylanx::execution_tree::primitives::primitive_component::eval_action,
    phylanx_primitive_eval_action);
HPX_REGISTER_ACTION_DECLARATION(
    phylanx::execution_tree::primitives::primitive_component::store_action,
    phylanx_primitive_store_action);
HPX_REGISTER_ACTION_DECLARATION(
    phylanx::execution_tree::primitives::primitive_component::store_set_1d_action,
    phylanx_primitive_store_set_1d_action);
HPX_REGISTER_ACTION_DECLARATION(
    phylanx::execution_tree::primitives::primitive_component::store_set_2d_action,
    phylanx_primitive_store_set_2d_action);
HPX_REGISTER_ACTION_DECLARATION(
    phylanx::execution_tree::primitives::
        primitive_component::expression_topology_action,
    phylanx_primitive_expression_topology_action);
HPX_REGISTER_ACTION_DECLARATION(
    phylanx::execution_tree::primitives::primitive_component::bind_action,
    phylanx_primitive_bind_action);

#endif
