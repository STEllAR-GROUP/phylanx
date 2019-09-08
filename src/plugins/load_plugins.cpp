//  Copyright (c) 2016-2018 Hartmut Kaiser
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// This dynamically loads shared libraries and asks to create instances for
// each of the factory types it supports. It then uses each of the factories to
// create a corresponding plugin instance.

#include <phylanx/config.hpp>
#include <phylanx/plugins/plugin_factory.hpp>

#include <hpx/include/runtime.hpp>
#include <hpx/include/util.hpp>

#include <hpx/filesystem.hpp>
#include <boost/algorithm/string/case_conv.hpp>
#include <boost/tokenizer.hpp>

#include <memory>
#include <string>
#include <utility>
#include <vector>

///////////////////////////////////////////////////////////////////////////////
namespace phylanx { namespace plugin
{
    ///////////////////////////////////////////////////////////////////////////
    bool load_plugins(plugin_map_type& plugins)
    {
        hpx::util::section& ini = hpx::get_runtime().get_config();

        // load all components as described in the configuration information
        if (!ini.has_section("phylanx.plugins"))
        {
            LRT_(info) << "No plugins found/loaded." << std::endl;
            return true;     // no plugins to load
        }

        // each shared library containing plugins may have an ini section
        //
        // # mandatory section describing the plugin module
        // [hpx.plugins.instance_name]
        //  name = ...           # the name of this plugin module
        //  path = ...           # the path where to find this plugin module
        //  enabled = false      # optional (default is assumed to be true)
        //
        // # optional section defining additional properties for this module
        // [hpx.plugins.instance_name.settings]
        //  key = value
        //
        hpx::util::section* sec = ini.get_section("phylanx.plugins");
        if (nullptr == sec)
        {
            return false;     // something bad happened
        }

        hpx::util::section::section_map& s = (*sec).get_sections();
        using iterator = hpx::util::section::section_map::iterator;

        iterator end = s.end();
        for (iterator i = s.begin(); i != end; ++i)
        {
            // the section name is the instance name of the component
            hpx::util::section& sect = i->second;
            std::string instance (sect.get_name());
            std::string component;

            if (i->second.has_entry("name"))
            {
                component = sect.get_entry("name");
            }
            else
            {
                component = instance;
            }

            if (sect.has_entry("enabled"))
            {
                std::string tmp = sect.get_entry("enabled");
                boost::algorithm::to_lower(tmp);
                if (tmp == "no" || tmp == "false" || tmp == "0")
                {
                    LRT_(info) << "plugin factory disabled: " << instance;
                    continue;     // this plugin has been disabled
                }
            }

            // initialize the factory instance using the preferences from the
            // ini files
            hpx::util::section const* glob_ini = nullptr;
            if (ini.has_section("settings"))
            {
                glob_ini = ini.get_section("settings");
            }

            hpx::util::section const* plugin_ini = nullptr;
            std::string plugin_section("phylanx.plugins." + instance);
            if (ini.has_section(plugin_section))
            {
                plugin_ini = ini.get_section(plugin_section);
            }

            hpx::filesystem::path lib_path;
            std::string component_path = sect.get_entry("path");

            using tokenizer = boost::tokenizer<boost::char_separator<char> >;
            boost::char_separator<char> sep(HPX_INI_PATH_DELIMITER);
            tokenizer tokens(component_path, sep);
            hpx::filesystem::error_code fsec;
            for (tokenizer::iterator it = tokens.begin(); it != tokens.end();
                 ++it)
            {
                hpx::filesystem::path dir = hpx::filesystem::path(*it);
                lib_path = dir / std::string(HPX_MAKE_DLL_STRING(component));
                if (hpx::filesystem::exists(lib_path, fsec))
                {
                    break;
                }
                lib_path.clear();
            }

            if (lib_path.string().empty())
            {
                continue;       // didn't find this plugin
            }

            hpx::util::plugin::dll module(
                lib_path.string(), HPX_MANGLE_STRING(component));

            // Get the factory (get base class as hpx::util::any compares the
            // full typeid of the held pointer, requiring us to pretend its a
            // hpx::plugins::plugin_factory_base* and cast it up below).
            hpx::util::plugin::plugin_factory<
                    hpx::plugins::plugin_factory_base
                > pf(module, "phylanx_primitive_factory");

            try {
                // create the plugin factory object, if not disabled
                hpx::error_code ec(hpx::lightweight);
                std::shared_ptr<phylanx::plugin::plugin_factory_base> factory(
                    static_cast<phylanx::plugin::plugin_factory_base*>(
                        pf.create(instance, ec, glob_ini, plugin_ini, true)));

                if (!ec)
                {
                    // store component factory and module for later use
                    phylanx::plugin::plugin_factory_data data(factory, module);
                    std::pair<plugin_map_type::iterator, bool> p =
                        plugins.insert(
                            plugin_map_type::value_type(instance, data));

                    if (!p.second)
                    {
                        LRT_(fatal) << "duplicate plugin type: " << instance;
                        return false;
                    }

                    LRT_(info)
                        << "dynamic loading succeeded: " << lib_path.string()
                        << ": " << instance;

                    // use factory to create an instance of the plugin
                    std::shared_ptr<phylanx::plugin::plugin_base> plugin(
                        factory->create());

                    plugin->register_known_primitives(lib_path.string());

                    // store the full path of this plugin
                    sect.add_entry(
                        std::string("loaded_from_path"), lib_path.string());
                }
            }
            catch (...) {
                // different type of factory (not "example_factory"), ignore here
            }
        }
        return true;
    }
}}

