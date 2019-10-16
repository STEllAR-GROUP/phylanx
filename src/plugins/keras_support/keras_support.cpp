//  Copyright (c) 2018-2019 Hartmut Kaiser
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>
#include <phylanx/plugins/keras_support/keras_support.hpp>
#include <phylanx/plugins/plugin_factory.hpp>

#include <string>

PHYLANX_REGISTER_PLUGIN_MODULE();

PHYLANX_REGISTER_PLUGIN_FACTORY(avg_pool2d_operation_plugin,
    phylanx::execution_tree::primitives::avg_pool2d_operation::match_data);
PHYLANX_REGISTER_PLUGIN_FACTORY(avg_pool3d_operation_plugin,
    phylanx::execution_tree::primitives::avg_pool3d_operation::match_data);

PHYLANX_REGISTER_PLUGIN_FACTORY(batch_dot_operation_plugin,
    phylanx::execution_tree::primitives::batch_dot_operation::match_data);
PHYLANX_REGISTER_PLUGIN_FACTORY(bias_add_operation_plugin,
    phylanx::execution_tree::primitives::bias_add_operation::match_data);
PHYLANX_REGISTER_PLUGIN_FACTORY(bin_cross_operation_plugin,
    phylanx::execution_tree::primitives::bin_cross_operation::match_data);
PHYLANX_REGISTER_PLUGIN_FACTORY(cat_cross_operation_plugin,
    phylanx::execution_tree::primitives::cat_cross_operation::match_data);

PHYLANX_REGISTER_PLUGIN_FACTORY(conv1d_operation_plugin,
    phylanx::execution_tree::primitives::conv1d_operation::match_data);
PHYLANX_REGISTER_PLUGIN_FACTORY(conv2d_operation_plugin,
    phylanx::execution_tree::primitives::conv2d_operation::match_data);
PHYLANX_REGISTER_PLUGIN_FACTORY(ctc_decode_operation_plugin,
    phylanx::execution_tree::primitives::ctc_decode_operation::match_data);

PHYLANX_REGISTER_PLUGIN_FACTORY(elu_operation_plugin,
    phylanx::execution_tree::primitives::elu_operation::match_data);
PHYLANX_REGISTER_PLUGIN_FACTORY(hard_sigmoid_operation_plugin,
    phylanx::execution_tree::primitives::hard_sigmoid_operation::match_data);
PHYLANX_REGISTER_PLUGIN_FACTORY(l2_normalize_operation_plugin,
    phylanx::execution_tree::primitives::l2_normalize_operation::match_data);
PHYLANX_REGISTER_PLUGIN_FACTORY(max_pool2d_operation_plugin,
    phylanx::execution_tree::primitives::max_pool2d_operation::match_data);
PHYLANX_REGISTER_PLUGIN_FACTORY(max_pool3d_operation_plugin,
    phylanx::execution_tree::primitives::max_pool3d_operation::match_data);

PHYLANX_REGISTER_PLUGIN_FACTORY(one_hot_operation_plugin,
    phylanx::execution_tree::primitives::one_hot_operation::match_data);
PHYLANX_REGISTER_PLUGIN_FACTORY(relu_operation_plugin,
    phylanx::execution_tree::primitives::relu_operation::match_data);
PHYLANX_REGISTER_PLUGIN_FACTORY(resize_operation_plugin,
    phylanx::execution_tree::primitives::resize_operation::match_data);

PHYLANX_REGISTER_PLUGIN_FACTORY(separable_conv1d_operation_plugin,
    phylanx::execution_tree::primitives::separable_conv1d_operation::match_data);

PHYLANX_REGISTER_PLUGIN_FACTORY(sigmoid_operation_plugin,
    phylanx::execution_tree::primitives::sigmoid_operation::match_data);
PHYLANX_REGISTER_PLUGIN_FACTORY(softmax_operation_plugin,
    phylanx::execution_tree::primitives::softmax_operation::match_data);
PHYLANX_REGISTER_PLUGIN_FACTORY(softplus_operation_plugin,
    phylanx::execution_tree::primitives::softplus_operation::match_data);
PHYLANX_REGISTER_PLUGIN_FACTORY(softsign_operation_plugin,
    phylanx::execution_tree::primitives::softsign_operation::match_data);
PHYLANX_REGISTER_PLUGIN_FACTORY(spatial_2d_padding_operation_plugin,
    phylanx::execution_tree::primitives::spatial_2d_padding_operation::match_data);
PHYLANX_REGISTER_PLUGIN_FACTORY(switch_operation_plugin,
    phylanx::execution_tree::primitives::switch_operation::match_data);
