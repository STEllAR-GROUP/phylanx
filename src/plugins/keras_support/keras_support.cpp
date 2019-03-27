//  Copyright (c) 2018-2019 Hartmut Kaiser
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>
#include <phylanx/plugins/keras_support/keras_support.hpp>
#include <phylanx/plugins/plugin_factory.hpp>

#include <string>

PHYLANX_REGISTER_PLUGIN_MODULE();

PHYLANX_REGISTER_PLUGIN_FACTORY(batch_dot_operation_plugin,
    phylanx::execution_tree::primitives::batch_dot_operation::match_data);
PHYLANX_REGISTER_PLUGIN_FACTORY(elu_operation_plugin,
    phylanx::execution_tree::primitives::elu_operation::match_data);
PHYLANX_REGISTER_PLUGIN_FACTORY(hard_sigmoid_operation_plugin,
    phylanx::execution_tree::primitives::hard_sigmoid_operation::match_data);
PHYLANX_REGISTER_PLUGIN_FACTORY(l2_normalize_operation_plugin,
    phylanx::execution_tree::primitives::l2_normalize_operation::match_data);
PHYLANX_REGISTER_PLUGIN_FACTORY(one_hot_operation_plugin,
    phylanx::execution_tree::primitives::one_hot_operation::match_data);
PHYLANX_REGISTER_PLUGIN_FACTORY(sigmoid_operation_plugin,
    phylanx::execution_tree::primitives::sigmoid_operation::match_data);
PHYLANX_REGISTER_PLUGIN_FACTORY(softmax_operation_plugin,
    phylanx::execution_tree::primitives::softmax_operation::match_data);
PHYLANX_REGISTER_PLUGIN_FACTORY(cat_cross_operation_plugin,
    phylanx::execution_tree::primitives::cat_cross_operation::match_data);
PHYLANX_REGISTER_PLUGIN_FACTORY(softplus_operation_plugin,
    phylanx::execution_tree::primitives::softplus_operation::match_data);
