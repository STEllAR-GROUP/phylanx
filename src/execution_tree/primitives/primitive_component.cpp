//  Copyright (c) 2017-2019 Hartmut Kaiser
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>
#include <phylanx/execution_tree/compile.hpp>
#include <phylanx/execution_tree/primitives/base_primitive.hpp>
#include <phylanx/execution_tree/primitives/primitive_component.hpp>

#include <hpx/include/actions.hpp>
#include <hpx/include/components.hpp>
#include <hpx/include/lcos.hpp>
#include <hpx/include/util.hpp>
#include <hpx/include/serialization.hpp>
#include <hpx/runtime/naming_fwd.hpp>
#include <hpx/runtime/launch_policy.hpp>

#include <cstddef>
#include <cstdint>
#include <map>
#include <memory>
#include <set>
#include <string>
#include <utility>
#include <vector>

///////////////////////////////////////////////////////////////////////////////
// Add factory registration functionality
HPX_REGISTER_COMPONENT_MODULE()

///////////////////////////////////////////////////////////////////////////////
// Serialization support for the base_file actions
typedef phylanx::execution_tree::primitives::primitive_component
    primitive_component_type;

HPX_REGISTER_ACTION(primitive_component_type::eval_action,
    phylanx_primitive_eval_action)
HPX_REGISTER_ACTION(primitive_component_type::eval_single_action,
    phylanx_primitive_eval_single_action)
HPX_REGISTER_ACTION(primitive_component_type::store_action,
    phylanx_primitive_store_action)
HPX_REGISTER_ACTION(primitive_component_type::store_single_action,
    phylanx_primitive_store_single_action)
HPX_REGISTER_ACTION(primitive_component_type::expression_topology_action,
    phylanx_primitive_expression_topology_action)
HPX_REGISTER_ACTION(primitive_component_type::bind_action,
    phylanx_primitive_bind_action)

//////////////////////////////////////////////////////////////////////////////
typedef hpx::components::component<primitive_component_type>
    phylanx_primitive_component_type;
HPX_REGISTER_COMPONENT(phylanx_primitive_component_type)

//////////////////////////////////////////////////////////////////////////////
namespace phylanx { namespace execution_tree { namespace primitives
{
    /////////////////////////////////////////////////////////////////////////
    namespace detail
    {
        using factories_map_type =
            std::map<std::string, primitive_factory_function_type>;

        struct fill_factories_map
        {
            fill_factories_map()
            {
                for (auto const& pattern : get_all_known_patterns())
                {
                    auto const& p = pattern.data_;
                    if (p.create_instance_ != nullptr)
                    {
                        instance_.insert(factories_map_type::value_type(
                            pattern.name_, p.create_instance_));
                    }
                }
            }

            factories_map_type instance_;
        };

        static factories_map_type& get_factories()
        {
            static fill_factories_map factories;
            return factories.instance_;
        }
    }

    /////////////////////////////////////////////////////////////////////////
    std::shared_ptr<primitive_component_base>
    primitive_component::create_primitive(std::string const& type,
        primitive_arguments_type&& args, std::string const& name,
        std::string const& codename)
    {
        auto const& factories = detail::get_factories();

        auto it = factories.find(type);
        if (it == factories.end())
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "primitive_component::create_primitive_no_name",
                "attempting to instantiate an unknown primitive type: " +
                    type);
        }

        return (*it).second(std::move(args), name, codename);
    }

    // eval_action
    hpx::future<primitive_argument_type> primitive_component::eval(
        primitive_arguments_type const& params,
        eval_context ctx) const
    {
        if ((ctx.mode_ & eval_dont_evaluate_partials) &&
            primitive_->operands_.empty())
        {
            // return a client referring to this component as the evaluation
            // result
            primitive this_{this->get_id()};
            return hpx::make_ready_future(
                primitive_argument_type{std::move(this_)});
        }
        return primitive_->do_eval(params, ctx);
    }

    hpx::future<primitive_argument_type> primitive_component::eval_single(
        primitive_argument_type && param, eval_context ctx) const
    {
        if ((ctx.mode_ & eval_dont_evaluate_partials) &&
            primitive_->operands_.empty())
        {
            // return a client referring to this component as the evaluation
            // result
            primitive this_{this->get_id()};
            return hpx::make_ready_future(
                primitive_argument_type{std::move(this_)});
        }
        return primitive_->do_eval(std::move(param), ctx);
    }

    // store_action
    void primitive_component::store(primitive_arguments_type&& args,
        primitive_arguments_type&& params, eval_context ctx)
    {
        primitive_->store(std::move(args), std::move(params), ctx);
    }

    void primitive_component::store_single(primitive_argument_type&& arg,
        primitive_arguments_type&& params, eval_context ctx)
    {
        primitive_->store(std::move(arg), std::move(params), ctx);
    }

    // extract_topology_action
    topology primitive_component::expression_topology(
        std::set<std::string>&& functions,
        std::set<std::string>&& resolve_children) const
    {
        return primitive_->expression_topology(
            std::move(functions), std::move(resolve_children));
    }

    // bind_action
    bool primitive_component::bind(
        primitive_arguments_type const& params, eval_context ctx) const
    {
        return primitive_->bind(params, ctx);
    }

    // access data for performance counter
    std::int64_t primitive_component::get_eval_count(bool reset) const
    {
        return primitive_->get_eval_count(reset);
    }

    std::int64_t primitive_component::get_eval_duration(bool reset) const
    {
        return primitive_->get_eval_duration(reset);
    }

    std::int64_t primitive_component::get_direct_execution(bool reset) const
    {
        return primitive_->get_direct_execution(reset);
    }

    void primitive_component::enable_measurements()
    {
        primitive_->enable_measurements();
    }

    hpx::launch primitive_component::select_direct_execution(
        primitive_component::eval_action, hpx::launch policy,
        hpx::naming::address_type lva)
    {
        auto this_ = hpx::get_lva<primitive_component>::call(lva);
#if defined(PHYLANX_HAVE_TASK_INLINING_POLICY) && defined(HPX_HAVE_APEX)
        return this_->primitive_->select_direct_eval_policy_thres(policy);
#else
        return this_->primitive_->select_direct_eval_execution(policy);
#endif
    }

    hpx::launch primitive_component::select_direct_execution(
        primitive_component::eval_single_action, hpx::launch policy,
        hpx::naming::address_type lva)
    {
        auto this_ = hpx::get_lva<primitive_component>::call(lva);
#if defined(PHYLANX_HAVE_TASK_INLINING_POLICY) && defined(HPX_HAVE_APEX)
        return this_->primitive_
            ->select_direct_eval_policy_thres(policy);
#else
        return this_->primitive_->select_direct_eval_execution(policy);
#endif
    }
}}}

namespace phylanx { namespace execution_tree
{
    primitive create_primitive_component(hpx::id_type const& locality,
        std::string const& type, primitive_arguments_type&& operands,
        std::string const& name, std::string const& codename,
        bool register_with_agas)
    {
        return primitive{
            hpx::new_<primitives::primitive_component>(
                locality, type, std::move(operands), name, codename),
            name, register_with_agas};
    }

    primitive create_primitive_component(hpx::id_type const& locality,
        std::string const& type, primitive_arguments_type&& operands,
        eval_context ctx, std::string const& name, std::string const& codename,
        bool register_with_agas)
    {
        return primitive{
            hpx::new_<primitives::primitive_component>(locality, type,
                std::move(operands), std::move(ctx), name, codename),
            name, register_with_agas};
    }

    primitive create_primitive_component(
        hpx::id_type const& locality, std::string const& type,
        primitive_argument_type operand, std::string const& name,
        std::string const& codename, bool register_with_agas)
    {
        primitive_arguments_type operands;
        operands.emplace_back(std::move(operand));

        return primitive{
            hpx::new_<primitives::primitive_component>(
                locality, type, std::move(operands), name, codename),
            name, register_with_agas};
    }
}}

