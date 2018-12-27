//  Copyright (c) 2018 Hartmut Kaiser
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>
#include <phylanx/plugins/statistics/statistics.hpp>
#include <phylanx/plugins/plugin_factory.hpp>

PHYLANX_REGISTER_PLUGIN_MODULE();

PHYLANX_REGISTER_PLUGIN_FACTORY(max_operation_plugin,
    phylanx::execution_tree::primitives::max_operation::match_data);
PHYLANX_REGISTER_PLUGIN_FACTORY(mean_operation_plugin,
    phylanx::execution_tree::primitives::mean_operation::match_data);
PHYLANX_REGISTER_PLUGIN_FACTORY(min_operation_plugin,
    phylanx::execution_tree::primitives::min_operation::match_data);
PHYLANX_REGISTER_PLUGIN_FACTORY(prod_operation_plugin,
    phylanx::execution_tree::primitives::prod_operation::match_data);
PHYLANX_REGISTER_PLUGIN_FACTORY(std_operation_plugin,
    phylanx::execution_tree::primitives::std_operation::match_data);
PHYLANX_REGISTER_PLUGIN_FACTORY(sum_operation_plugin,
    phylanx::execution_tree::primitives::sum_operation::match_data);
PHYLANX_REGISTER_PLUGIN_FACTORY(var_operation_plugin,
    phylanx::execution_tree::primitives::var_operation::match_data);
