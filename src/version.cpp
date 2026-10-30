//  Copyright (c) 2017 Hartmut Kaiser
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>
#include <phylanx/config/user_main_config.hpp>
#include <phylanx/version.hpp>

#include <hpx/modules/filesystem.hpp>
#include <hpx/modules/format.hpp>
#include <hpx/preprocessor/expand.hpp>
#include <hpx/preprocessor/stringize.hpp>

#if !defined(_WIN32) && !defined(__WIN32__) && !defined(WIN32)
#include <dlfcn.h>
#endif

#include <cstdint>
#include <cstdlib>
#include <string>
#include <vector>

namespace {

    // glibc exposes RTLD_DI_ORIGIN as an enumerator, not a macro, so
    // hpx::util::find_prefix records the HPX install prefix. hpx.location is
    // the host executable, which for the Python extension is python itself.
    // Primitive plugins are always in the phylanx directory next to this
    // component library.
    std::string module_plugin_directory()
    {
#if defined(_WIN32) || defined(__WIN32__) || defined(WIN32)
        return {};
#else
        Dl_info info{};
        if (::dladdr(reinterpret_cast<void*>(&module_plugin_directory),
                &info) == 0 ||
            info.dli_fname == nullptr)
        {
            return {};
        }

        return (hpx::filesystem::path(info.dli_fname).parent_path() / "phylanx")
            .string();
#endif
    }

}    // namespace

///////////////////////////////////////////////////////////////////////////////
namespace phylanx
{
    std::uint8_t major_version()
    {
        return PHYLANX_VERSION_MAJOR;
    }

    std::uint8_t minor_version()
    {
        return PHYLANX_VERSION_MINOR;
    }

    std::uint8_t subminor_version()
    {
        return PHYLANX_VERSION_SUBMINOR;
    }

    std::uint32_t full_version()
    {
        return PHYLANX_VERSION_FULL;
    }

    std::string full_version_as_string()
    {
        std::string result(HPX_PP_STRINGIZE(HPX_PP_EXPAND(PHYLANX_VERSION_MAJOR)));
        result += ".";
        result += HPX_PP_STRINGIZE(HPX_PP_EXPAND(PHYLANX_VERSION_MINOR));
        result += ".";
        result += HPX_PP_STRINGIZE(HPX_PP_EXPAND(PHYLANX_VERSION_SUBMINOR));
        return result;
    }

    ///////////////////////////////////////////////////////////////////////////
    char const PHYLANX_CHECK_VERSION[] =
        HPX_PP_STRINGIZE(HPX_PP_EXPAND(PHYLANX_CHECK_VERSION));
    char const PHYLANX_CHECK_BOOST_VERSION[] =
        HPX_PP_STRINGIZE(HPX_PP_EXPAND(PHYLANX_CHECK_VERSION));

    ///////////////////////////////////////////////////////////////////////////
    namespace detail
    {
        static std::vector<std::string> (*prev_user_main_config_function)(
            std::vector<std::string> const&) = nullptr;

        std::vector<std::string> user_main_config(
            std::vector<std::string> const& config)
        {
            std::vector<std::string> cfg(config);

            // add directory for this shared library to the component_path
            cfg.emplace_back(hpx::util::format(
                "hpx.component_base_paths=$[hpx.component_base_paths]{1}{2}",
                HPX_INI_PATH_DELIMITER,
                hpx::util::find_prefix(HPX_COMPONENT_STRING)));

            // add additional search suffix for plugins to configuration
            cfg.emplace_back(hpx::util::format(
                "hpx.component_path_suffixes="
                    "$[hpx.component_path_suffixes]{1}{2}{1}{3}",
                    HPX_INI_PATH_DELIMITER, "/lib/phylanx", "/bin/phylanx"));

            // add plugin directories to search path
            char const* path = std::getenv("PHYLANX_PLUGINS_PATH");
            if (path != nullptr)
            {
                cfg.emplace_back(hpx::util::format(
                    "hpx.component_paths=$[hpx.component_paths]{1}{2}",
                    HPX_INI_PATH_DELIMITER, std::string(path)));
            }

            std::string const plugin_dir = module_plugin_directory();
            if (!plugin_dir.empty())
            {
                cfg.emplace_back(hpx::util::format(
                    "hpx.component_paths=$[hpx.component_paths]{1}{2}",
                    HPX_INI_PATH_DELIMITER, plugin_dir));
            }

            // If there was another config function registered, call it
            if (prev_user_main_config_function)
                return prev_user_main_config_function(cfg);

            return cfg;
        }

        // make sure user_main_config is registered
        register_user_main_config cfg;

        register_user_main_config::register_user_main_config()
        {
            prev_user_main_config_function =
                hpx_startup::user_main_config_function;
            hpx_startup::user_main_config_function = &user_main_config;
        }
    }
}

