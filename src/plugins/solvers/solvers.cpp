//  Copyright (c) 2018 Hartmut Kaiser
//  Copyright (c) 2018 Shahrzad Shirzad
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>
#include <phylanx/plugins/plugin_factory.hpp>
#include <phylanx/plugins/solvers/solvers.hpp>

#include <string>

PHYLANX_REGISTER_PLUGIN_MODULE();

namespace phylanx { namespace plugin
{
    struct linear_solver_plugin : plugin_base
    {
        void register_known_primitives(std::string const& fullpath) override
        {
            namespace pet = phylanx::execution_tree;

            std::string linear_solver_name("_linear_solver");
            for (auto const& pattern :
                pet::primitives::linear_solver::match_data)
            {
                pet::register_pattern(linear_solver_name, pattern, fullpath);
            }
        }
    };

    struct decomposition_plugin : plugin_base
    {
        void register_known_primitives(std::string const& fullpath) override
        {
            namespace pet = phylanx::execution_tree;

            std::string decomposition_name("_decomposition");
            for (auto const& pattern :
                pet::primitives::decomposition::match_data)
            {
                pet::register_pattern(decomposition_name, pattern, fullpath);
            }
        }
    };
}}

PHYLANX_REGISTER_PLUGIN_FACTORY(phylanx::plugin::linear_solver_plugin,
    linear_solver_plugin,
    phylanx::execution_tree::primitives::make_list::match_data,
    "_linear_solver");

PHYLANX_REGISTER_PLUGIN_FACTORY(phylanx::plugin::decomposition_plugin,
    decomposition_plugin,
    phylanx::execution_tree::primitives::make_list::match_data,
    "_decomposition");
