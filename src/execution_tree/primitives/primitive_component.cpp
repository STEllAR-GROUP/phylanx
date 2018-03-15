//  Copyright (c) 2017-2018 Hartmut Kaiser
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
HPX_REGISTER_ACTION(primitive_component_type::eval_direct_action,
    phylanx_primitive_eval_direct_action)
HPX_REGISTER_ACTION(primitive_component_type::store_action,
    phylanx_primitive_store_action)
HPX_REGISTER_ACTION(primitive_component_type::expression_topology_action,
    phylanx_primitive_expression_topology_action)
HPX_REGISTER_ACTION(primitive_component_type::set_body_action,
    phylanx_primitive_set_body_action)

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
                    if (hpx::util::get<3>(pattern) != nullptr)
                    {
                        instance_.insert(factories_map_type::value_type(
                            hpx::util::get<0>(pattern),
                            hpx::util::get<3>(pattern)));
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
        std::vector<primitive_argument_type>&& args, std::string const& name,
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
        std::vector<primitive_argument_type> const& params) const
    {
        return primitive_->do_eval(params);
    }

    // direct_eval_action
    primitive_argument_type primitive_component::eval_direct(
        std::vector<primitive_argument_type> const& params) const
    {
        return primitive_->do_eval_direct(params);
    }

    // store_action
    void primitive_component::store(primitive_argument_type && arg)
    {
        primitive_->store(std::move(arg));
    }

    // extract_topology_action
    topology primitive_component::expression_topology(
        std::set<std::string>&& functions) const
    {
        return primitive_->expression_topology(std::move(functions));
    }

    // set_body_action (define_function only)
    void primitive_component::set_body(primitive_argument_type&& target)
    {
        primitive_->set_body(std::move(target));
    }

    // access data for performance counter
    std::int64_t primitive_component::get_eval_count(
        bool reset, bool direct) const
    {
        return primitive_->get_eval_count(reset, direct);
    }

    std::int64_t primitive_component::get_eval_duration(
        bool reset, bool direct) const
    {
        return primitive_->get_eval_duration(reset, direct);
    }
}}}

namespace phylanx { namespace execution_tree
{
    primitive create_primitive_component(
        hpx::id_type const& locality, std::string const& type,
        std::vector<primitive_argument_type>&& operands,
        std::string const& name, std::string const& codename)
    {
        return primitive{
            hpx::new_<primitives::primitive_component>(
                locality, type, std::move(operands), name, codename),
            name};
    }

    primitive create_primitive_component(
        hpx::id_type const& locality, std::string const& type,
        primitive_argument_type operand, std::string const& name,
        std::string const& codename)
    {
        std::vector<primitive_argument_type> operands;
        operands.emplace_back(std::move(operand));

        return primitive{
            hpx::new_<primitives::primitive_component>(
                locality, type, std::move(operands), name, codename),
            name};
    }
}}

