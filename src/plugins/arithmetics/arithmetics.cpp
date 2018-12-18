//  Copyright (c) 2018 Hartmut Kaiser
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>
#include <phylanx/plugins/arithmetics/arithmetics.hpp>
#include <phylanx/plugins/plugin_factory.hpp>

PHYLANX_REGISTER_PLUGIN_MODULE();

PHYLANX_REGISTER_PLUGIN_FACTORY(add_operation_plugin,
    phylanx::execution_tree::primitives::add_operation::match_data);
PHYLANX_REGISTER_PLUGIN_FACTORY(cumsum_operation_plugin,
    phylanx::execution_tree::primitives::cumsum::match_data);
PHYLANX_REGISTER_PLUGIN_FACTORY(div_operation_plugin,
    phylanx::execution_tree::primitives::div_operation::match_data);
PHYLANX_REGISTER_PLUGIN_FACTORY(mul_operation_plugin,
    phylanx::execution_tree::primitives::mul_operation::match_data);
PHYLANX_REGISTER_PLUGIN_FACTORY(prod_operation_plugin,
    phylanx::execution_tree::primitives::prod_operation::match_data);
PHYLANX_REGISTER_PLUGIN_FACTORY(sub_operation_plugin,
    phylanx::execution_tree::primitives::sub_operation::match_data);
PHYLANX_REGISTER_PLUGIN_FACTORY(sum_operation_plugin,
    phylanx::execution_tree::primitives::sum_operation::match_data);
PHYLANX_REGISTER_PLUGIN_FACTORY(unary_minus_operation_plugin,
    phylanx::execution_tree::primitives::unary_minus_operation::match_data);
