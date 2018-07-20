// Copyright (c) 2018 Hartmut Kaiser
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>
#include <phylanx/plugins/listops/listops.hpp>
#include <phylanx/plugins/plugin_factory.hpp>

#include <string>

PHYLANX_REGISTER_PLUGIN_MODULE();

PHYLANX_REGISTER_PLUGIN_FACTORY(list_plugin,
    phylanx::execution_tree::primitives::make_list::match_data[0]);
PHYLANX_REGISTER_PLUGIN_FACTORY(make_list_plugin,
    phylanx::execution_tree::primitives::make_list::match_data[1]);
PHYLANX_REGISTER_PLUGIN_FACTORY(len_operation_plugin,
    phylanx::execution_tree::primitives::len_operation::match_data);
PHYLANX_REGISTER_PLUGIN_FACTORY(append_operation_plugin,
    phylanx::execution_tree::primitives::append_operation::match_data);

namespace phylanx { namespace plugin {
    struct car_cdr_plugin : plugin_base
    {
        void register_known_primitives() override
        {
            namespace pet = phylanx::execution_tree;

            std::string car_cdr_name("car_cdr");
            for (auto const& pattern :
                pet::primitives::car_cdr_operation::match_data)
            {
                pet::register_pattern(car_cdr_name, pattern);
            }
        }
    };
}}

PHYLANX_REGISTER_PLUGIN_FACTORY(phylanx::plugin::car_cdr_plugin, car_cdr_plugin,
    phylanx::execution_tree::primitives::make_list::match_data,
    "car_cdr_plugin");
