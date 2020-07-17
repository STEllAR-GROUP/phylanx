//  Copyright (c) 2018-2019 Hartmut Kaiser
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>
#include <phylanx/plugins/arithmetics/arithmetics.hpp>
#include <phylanx/plugins/plugin_factory.hpp>

#include <string>

PHYLANX_REGISTER_PLUGIN_MODULE();

PHYLANX_REGISTER_PLUGIN_FACTORY(add_operation_plugin,
    phylanx::execution_tree::primitives::add_operation::match_data);
PHYLANX_REGISTER_PLUGIN_FACTORY(cumsum_operation_plugin,
    phylanx::execution_tree::primitives::cumsum::match_data);
PHYLANX_REGISTER_PLUGIN_FACTORY(cumprod_operation_plugin,
    phylanx::execution_tree::primitives::cumprod::match_data);
PHYLANX_REGISTER_PLUGIN_FACTORY(div_operation_plugin,
    phylanx::execution_tree::primitives::div_operation::match_data);
PHYLANX_REGISTER_PLUGIN_FACTORY(maximum_plugin,
    phylanx::execution_tree::primitives::maximum::match_data);
PHYLANX_REGISTER_PLUGIN_FACTORY(minimum_plugin,
    phylanx::execution_tree::primitives::minimum::match_data);
PHYLANX_REGISTER_PLUGIN_FACTORY(mod_operation_plugin,
    phylanx::execution_tree::primitives::mod_operation::match_data);
PHYLANX_REGISTER_PLUGIN_FACTORY(mul_operation_plugin,
    phylanx::execution_tree::primitives::mul_operation::match_data);
PHYLANX_REGISTER_PLUGIN_FACTORY(sub_operation_plugin,
    phylanx::execution_tree::primitives::sub_operation::match_data);
PHYLANX_REGISTER_PLUGIN_FACTORY(unary_minus_operation_plugin,
    phylanx::execution_tree::primitives::unary_minus_operation::match_data);

namespace phylanx { namespace plugin
{
    struct generic_operation_plugin : plugin_base
    {
        void register_known_primitives(std::string const& fullpath) override
        {
            namespace pet = phylanx::execution_tree;

            std::string generic_operation_name("__gen");
            for (auto const& pattern :
                pet::primitives::generic_operation::match_data)
            {
                pet::register_pattern(
                    generic_operation_name, pattern.first, fullpath);
            }
        }
    };

    struct generic_operation_bool_plugin : plugin_base
    {
        void register_known_primitives(std::string const& fullpath) override
        {
            namespace pet = phylanx::execution_tree;

            std::string generic_operation_bool_name("__gen_bool");
            for (auto const& pattern :
                pet::primitives::generic_operation_bool::match_data)
            {
                pet::register_pattern(
                    generic_operation_bool_name, pattern, fullpath);
            }
        }
    };
}}

PHYLANX_REGISTER_PLUGIN_FACTORY(phylanx::plugin::generic_operation_plugin,
    generic_operation_plugin,
    phylanx::execution_tree::primitives::generic_operation::match_data,
    "__gen");

PHYLANX_REGISTER_PLUGIN_FACTORY(phylanx::plugin::generic_operation_bool_plugin,
    generic_operation_bool_plugin,
    phylanx::execution_tree::primitives::generic_operation_bool::match_data,
    "__gen_bool");
