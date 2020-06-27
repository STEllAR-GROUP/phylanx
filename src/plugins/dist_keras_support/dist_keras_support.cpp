//  Copyright (c) 2020 Hartmut Kaiser
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>
#include <phylanx/plugins/dist_keras_support/dist_keras_support.hpp>
#include <phylanx/plugins/plugin_factory.hpp>

#include <string>

PHYLANX_REGISTER_PLUGIN_MODULE();

PHYLANX_REGISTER_PLUGIN_FACTORY(dist_conv1d_plugin,
    phylanx::dist_keras_support::primitives::dist_conv1d::match_data);
