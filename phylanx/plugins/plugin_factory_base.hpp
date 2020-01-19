//  Copyright (c) 2016-2018 Hartmut Kaiser
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(PHYLANX_PLUGIN_FACTORY_BASE_APR_06_2018_1248PM)
#define PHYLANX_PLUGIN_FACTORY_BASE_APR_06_2018_1248PM

#include <phylanx/config.hpp>
#include <phylanx/plugins/plugin_base.hpp>

#include <hpx/plugin.hpp>
#include <hpx/plugins/plugin_factory_base.hpp>
#include <hpx/type_support/pack.hpp>

///////////////////////////////////////////////////////////////////////////////
namespace phylanx { namespace plugin
{
    ///////////////////////////////////////////////////////////////////////////
    /// The \a plugin_factory_base has to be used as a base class for all
    /// plugin factories.
    struct plugin_factory_base : hpx::plugins::plugin_factory_base
    {
        ~plugin_factory_base() override {}

        // Create a new instance of the plugin
        virtual plugin_base* create() = 0;
    };
}}

namespace hpx { namespace util { namespace plugin
{
    ///////////////////////////////////////////////////////////////////////////
    // The following specialization of the virtual_constructor template
    // defines the argument list for the constructor of the concrete plugin
    // factory (derived from the component_factory_base above). This magic is
    // needed because we use hpx::plugin for the creation of instances of
    // derived types using the example_plugin_factory_base virtual base class
    // only (essentially implementing a virtual constructor).
    //
    // All derived component factories have to expose a constructor with the
    // matching signature. For instance:
    //
    //     class my_factory : public plugin_factory_base
    //     {
    //     public:
    //         my_factory (hpx::util::section const*, hpx::util::section const*, bool)
    //         {}
    //     };
    //
    template <>
    struct virtual_constructor<phylanx::plugin::plugin_factory_base>
    {
        typedef
            hpx::util::pack<
                hpx::util::section const*, hpx::util::section const*, bool
            > type;
    };
}}}

////////////////////////////////////////////////////////////////////////////////
/// This macro is used to register the given component factory with
/// Hpx.Plugin. This macro has to be used for each of the component factories.
#define PHYLANX_REGISTER_PLUGIN_FACTORY_BASE(FactoryType, pluginname)          \
    HPX_PLUGIN_EXPORT(HPX_PLUGIN_PLUGIN_PREFIX,                                \
        hpx::plugins::plugin_factory_base, FactoryType, pluginname,            \
        phylanx_primitive_factory)                                             \
    /**/

#define PHYLANX_REGISTER_PLUGIN_MODULE()                                       \
    HPX_PLUGIN_EXPORT_LIST(                                                    \
        HPX_PLUGIN_PLUGIN_PREFIX, phylanx_primitive_factory);                  \
    HPX_REGISTER_PLUGIN_REGISTRY_MODULE_DYNAMIC()                              \
    /**/

#endif

