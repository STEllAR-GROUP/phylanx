//  Copyright (c) 2019 Hartmut Kaiser
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>
#include <phylanx/plugins/dist_matrixops/dist_matrixops.hpp>
#include <phylanx/plugins/plugin_factory.hpp>

#include <string>

PHYLANX_REGISTER_PLUGIN_MODULE();

PHYLANX_REGISTER_PLUGIN_FACTORY(dist_cannon_product_plugin,
    phylanx::dist_matrixops::primitives::dist_cannon_product::match_data);
PHYLANX_REGISTER_PLUGIN_FACTORY(dist_constant_plugin,
    phylanx::dist_matrixops::primitives::dist_constant::match_data);
PHYLANX_REGISTER_PLUGIN_FACTORY(dist_dot_operation_plugin,
    phylanx::dist_matrixops::primitives::dist_dot_operation::match_data);
PHYLANX_REGISTER_PLUGIN_FACTORY(dist_transpose_operation_plugin,
    phylanx::dist_matrixops::primitives::dist_transpose_operation::match_data);
