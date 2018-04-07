//  Copyright (c) 2018 Hartmut Kaiser
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>
#include <phylanx/plugins/booleans/booleans.hpp>
#include <phylanx/plugins/plugin_factory.hpp>

PHYLANX_REGISTER_PLUGIN_MODULE();

PHYLANX_REGISTER_PLUGIN_FACTORY(and_operation_plugin,
    phylanx::execution_tree::primitives::and_operation::match_data);
PHYLANX_REGISTER_PLUGIN_FACTORY(equal_plugin,
    phylanx::execution_tree::primitives::equal::match_data);
PHYLANX_REGISTER_PLUGIN_FACTORY(greater_plugin,
    phylanx::execution_tree::primitives::greater::match_data);
PHYLANX_REGISTER_PLUGIN_FACTORY(greater_equal_plugin,
    phylanx::execution_tree::primitives::greater_equal::match_data);
PHYLANX_REGISTER_PLUGIN_FACTORY(less_plugin,
    phylanx::execution_tree::primitives::less::match_data);
PHYLANX_REGISTER_PLUGIN_FACTORY(less_equal_plugin,
    phylanx::execution_tree::primitives::less_equal::match_data);
PHYLANX_REGISTER_PLUGIN_FACTORY(not_equal_plugin,
    phylanx::execution_tree::primitives::not_equal::match_data);
PHYLANX_REGISTER_PLUGIN_FACTORY(or_operation_plugin,
    phylanx::execution_tree::primitives::or_operation::match_data);
PHYLANX_REGISTER_PLUGIN_FACTORY(unary_not_operation_plugin,
    phylanx::execution_tree::primitives::unary_not_operation::match_data);
