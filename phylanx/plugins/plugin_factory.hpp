//  Copyright (c) 2016-2018 Hartmut Kaiser
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(PHYLANX_PLUGIN_FACTORY_APR_06_2018_1233PM)
#define PHYLANX_PLUGIN_FACTORY_APR_06_2018_1233PM

#include <phylanx/config.hpp>

#include <hpx/plugins/unique_plugin_name.hpp>
#include <hpx/plugins/plugin_registry.hpp>

#include <phylanx/plugins/plugin_factory_base.hpp>

///////////////////////////////////////////////////////////////////////////////
namespace phylanx { namespace plugin
{
    ///////////////////////////////////////////////////////////////////////////
    template <typename Plugin>
    struct plugin_factory : public plugin_factory_base
    {
        plugin_factory(hpx::util::section const* global_cfg,
                hpx::util::section const* plugin_cfg, bool is_enabled)
          : is_enabled_(is_enabled)
        {
            // store the configuration settings
            if (nullptr != global_cfg)
                global_settings_ = *global_cfg;
            if (nullptr != plugin_cfg)
                local_settings_ = *plugin_cfg;
        }

        // Create a new instance of a Phylanx primitives plugin
        //
        // \returns Returns the newly created instance of the plugin
        //          supported by this factory
        plugin_base* create() override
        {
            if (is_enabled_)
            {
                return new Plugin{};
            }
            return nullptr;
        }

    protected:
        hpx::util::section global_settings_;
        hpx::util::section local_settings_;
        bool is_enabled_;
    };
}}

////////////////////////////////////////////////////////////////////////////////
// This macro is used create and to register a primitive factory with
// Hpx.Plugin.
#define PHYLANX_REGISTER_PLUGIN_FACTORY(Plugin, pluginname)                    \
    PHYLANX_REGISTER_PLUGIN_FACTORY_BASE(                                      \
        phylanx::plugin::plugin_factory< Plugin>, pluginname)                  \
    HPX_DEF_UNIQUE_PLUGIN_NAME(                                                \
        phylanx::plugin::plugin_factory< Plugin>, pluginname)                  \
    template struct phylanx::plugin::plugin_factory< Plugin>;                  \
    HPX_REGISTER_PLUGIN_REGISTRY_2(Plugin, pluginname)                         \
    /**/

#endif

