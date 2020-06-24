//  Copyright (c) 2019-2020 Hartmut Kaiser
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>
#include <phylanx/plugins/dist_matrixops/dist_matrixops.hpp>
#include <phylanx/plugins/plugin_factory.hpp>

#include <string>

PHYLANX_REGISTER_PLUGIN_MODULE();

PHYLANX_REGISTER_PLUGIN_FACTORY(all_gather_plugin,
    phylanx::dist_matrixops::primitives::all_gather::match_data);
PHYLANX_REGISTER_PLUGIN_FACTORY(dist_argmax_plugin,
    phylanx::dist_matrixops::primitives::dist_argmax::match_data);
PHYLANX_REGISTER_PLUGIN_FACTORY(dist_argmin_plugin,
    phylanx::dist_matrixops::primitives::dist_argmin::match_data);
PHYLANX_REGISTER_PLUGIN_FACTORY(dist_cannon_product_plugin,
    phylanx::dist_matrixops::primitives::dist_cannon_product::match_data);
PHYLANX_REGISTER_PLUGIN_FACTORY(dist_constant_plugin,
    phylanx::dist_matrixops::primitives::dist_constant::match_data);
PHYLANX_REGISTER_PLUGIN_FACTORY(dist_conv1d_plugin,
    phylanx::dist_matrixops::primitives::dist_conv1d::match_data);
PHYLANX_REGISTER_PLUGIN_FACTORY(dist_diag_plugin,
    phylanx::dist_matrixops::primitives::dist_diag::match_data);
PHYLANX_REGISTER_PLUGIN_FACTORY(dist_dot_operation_plugin,
    phylanx::dist_matrixops::primitives::dist_dot_operation::match_data);
PHYLANX_REGISTER_PLUGIN_FACTORY(dist_identity_plugin,
    phylanx::dist_matrixops::primitives::dist_identity::match_data)
PHYLANX_REGISTER_PLUGIN_FACTORY(dist_inverse_operation_plugin,
    phylanx::dist_matrixops::primitives::dist_inverse::match_data);
PHYLANX_REGISTER_PLUGIN_FACTORY(dist_random_plugin,
    phylanx::dist_matrixops::primitives::dist_random::match_data)
PHYLANX_REGISTER_PLUGIN_FACTORY(dist_transpose_operation_plugin,
    phylanx::dist_matrixops::primitives::dist_transpose_operation::match_data);
PHYLANX_REGISTER_PLUGIN_FACTORY(retile_annotations_plugin,
    phylanx::dist_matrixops::primitives::retile_annotations::match_data);
