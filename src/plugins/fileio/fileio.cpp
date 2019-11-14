//  Copyright (c) 2018 Hartmut Kaiser
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>
#include <phylanx/plugins/fileio/fileio.hpp>
#include <phylanx/plugins/plugin_factory.hpp>

PHYLANX_REGISTER_PLUGIN_MODULE();

PHYLANX_REGISTER_PLUGIN_FACTORY(file_read_plugin,
    phylanx::execution_tree::primitives::file_read::match_data);
PHYLANX_REGISTER_PLUGIN_FACTORY(file_write_plugin,
    phylanx::execution_tree::primitives::file_write::match_data);
PHYLANX_REGISTER_PLUGIN_FACTORY(file_read_csv_plugin,
    phylanx::execution_tree::primitives::file_read_csv::match_data);
PHYLANX_REGISTER_PLUGIN_FACTORY(file_read_csv_d_plugin,
    phylanx::execution_tree::primitives::file_read_csv_d::match_data);
PHYLANX_REGISTER_PLUGIN_FACTORY(file_write_csv_plugin,
    phylanx::execution_tree::primitives::file_write_csv::match_data);

#if defined(PHYLANX_HAVE_HIGHFIVE)
PHYLANX_REGISTER_PLUGIN_FACTORY(file_read_hdf5_plugin,
    phylanx::execution_tree::primitives::file_read_hdf5::match_data);
PHYLANX_REGISTER_PLUGIN_FACTORY(file_write_hdf5_plugin,
    phylanx::execution_tree::primitives::file_write_hdf5::match_data);
#endif
