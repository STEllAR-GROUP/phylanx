//  Copyright (c) 2018-2020 Hartmut Kaiser
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>
#include <phylanx/plugins/controls/controls.hpp>
#include <phylanx/plugins/plugin_factory.hpp>

PHYLANX_REGISTER_PLUGIN_MODULE();

PHYLANX_REGISTER_PLUGIN_FACTORY(apply_plugin,
    phylanx::execution_tree::primitives::apply::match_data);
PHYLANX_REGISTER_PLUGIN_FACTORY(async_operation_plugin,
    phylanx::execution_tree::primitives::async_operation::match_data);
PHYLANX_REGISTER_PLUGIN_FACTORY(block_operation_plugin,
    phylanx::execution_tree::primitives::block_operation::match_data);
PHYLANX_REGISTER_PLUGIN_FACTORY(filter_operation_plugin,
    phylanx::execution_tree::primitives::filter_operation::match_data);
PHYLANX_REGISTER_PLUGIN_FACTORY(fold_left_operation_plugin,
    phylanx::execution_tree::primitives::fold_left_operation::match_data);
PHYLANX_REGISTER_PLUGIN_FACTORY(fold_right_operation_plugin,
    phylanx::execution_tree::primitives::fold_right_operation::match_data);
PHYLANX_REGISTER_PLUGIN_FACTORY(for_each_plugin,
    phylanx::execution_tree::primitives::for_each::match_data);
PHYLANX_REGISTER_PLUGIN_FACTORY(for_operation_plugin,
    phylanx::execution_tree::primitives::for_operation::match_data);
PHYLANX_REGISTER_PLUGIN_FACTORY(if_conditional_plugin,
    phylanx::execution_tree::primitives::if_conditional::match_data);
PHYLANX_REGISTER_PLUGIN_FACTORY(fmap_operation_plugin,
    phylanx::execution_tree::primitives::fmap_operation::match_data);
PHYLANX_REGISTER_PLUGIN_FACTORY(parallel_block_operation_plugin,
    phylanx::execution_tree::primitives::parallel_block_operation::match_data);
PHYLANX_REGISTER_PLUGIN_FACTORY(parallel_map_operation_plugin,
    phylanx::execution_tree::primitives::parallel_map_operation::match_data);
PHYLANX_REGISTER_PLUGIN_FACTORY(range_operation_plugin,
    phylanx::execution_tree::primitives::range_operation::match_data);
PHYLANX_REGISTER_PLUGIN_FACTORY(while_operation_plugin,
    phylanx::execution_tree::primitives::while_operation::match_data);

