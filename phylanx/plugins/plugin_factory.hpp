//  Copyright (c) 2016-2018 Hartmut Kaiser
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(PHYLANX_PLUGIN_FACTORY_APR_06_2018_1233PM)
#define PHYLANX_PLUGIN_FACTORY_APR_06_2018_1233PM

#include <phylanx/config.hpp>
#include <phylanx/plugins/plugin_base.hpp>
#include <phylanx/plugins/plugin_factory_base.hpp>

#include <hpx/plugins/plugin_factory_base.hpp>
#include <hpx/plugins/plugin_registry.hpp>
#include <hpx/plugins/unique_plugin_name.hpp>

#include <hpx/util/detail/pp/cat.hpp>
#include <hpx/util/detail/pp/expand.hpp>
#include <hpx/util/detail/pp/nargs.hpp>

#include <map>
#include <memory>
#include <string>

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
            {
                global_settings_ = *global_cfg;
            }
            if (nullptr != plugin_cfg)
            {
                local_settings_ = *plugin_cfg;
            }
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

    struct plugin_factory_data
    {
        plugin_factory_data(
                std::shared_ptr<hpx::plugins::plugin_factory_base> const& f,
                hpx::util::plugin::dll const& d)
          : first(f)
          , second(d)
        {}

        std::shared_ptr<hpx::plugins::plugin_factory_base> first;
        hpx::util::plugin::dll second;
    };

    using plugin_map_type = std::map<std::string, plugin_factory_data>;

    // traverse the configuration database and load all primitive plugins
    bool load_plugins(plugin_map_type& plugins);
}}

////////////////////////////////////////////////////////////////////////////////
// This macro is used create and to register a primitive factory with
// Hpx.Plugin.
#define PHYLANX_REGISTER_PLUGIN_FACTORY(...)                                   \
    PHYLANX_REGISTER_PLUGIN_FACTORY_(__VA_ARGS__)                              \
    /**/

#define PHYLANX_REGISTER_PLUGIN_FACTORY_(...)                                  \
    HPX_PP_EXPAND(HPX_PP_CAT(                                                  \
        PHYLANX_REGISTER_PLUGIN_FACTORY_, HPX_PP_NARGS(__VA_ARGS__)            \
    )(__VA_ARGS__))                                                            \
/**/

#define PHYLANX_REGISTER_PLUGIN_FACTORY_2(pluginname, match_data)              \
    PHYLANX_REGISTER_PLUGIN_FACTORY_3(                                         \
        pluginname, match_data, hpx::util::get<0>(match_data))                 \
    /**/

#define PHYLANX_REGISTER_PLUGIN_FACTORY_3(pluginname, match_data, name)        \
    namespace phylanx {                                                        \
        namespace plugin {                                                     \
            struct pluginname : plugin_base                                    \
            {                                                                  \
                void register_known_primitives() override                      \
                {                                                              \
                    phylanx::execution_tree::register_pattern(                 \
                        name, match_data);                                     \
                }                                                              \
            };                                                                 \
        }                                                                      \
    }                                                                          \
    PHYLANX_REGISTER_PLUGIN_FACTORY_4(                                         \
        phylanx::plugin::pluginname, pluginname, match_data, name)             \
    /**/

/**/

#define PHYLANX_REGISTER_PLUGIN_FACTORY_4(                                     \
        plugintype, pluginname, match_data, name)                              \
    PHYLANX_REGISTER_PLUGIN_FACTORY_BASE(                                      \
        phylanx::plugin::plugin_factory< plugintype>, pluginname)              \
    HPX_DEF_UNIQUE_PLUGIN_NAME(                                                \
        phylanx::plugin::plugin_factory< plugintype>, pluginname)              \
    template struct phylanx::plugin::plugin_factory< plugintype>;              \
    HPX_REGISTER_PLUGIN_REGISTRY_4(                                            \
        plugintype, pluginname, "phylanx", "phylanx")                          \
    /**/

#endif

