//  Copyright (c) 2007-2013 Hartmut Kaiser
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(PHYLANX_VERSION_AUG_18_2011_0854PM)
#define PHYLANX_VERSION_AUG_18_2011_0854PM

#include <phylanx/config.hpp>
#include <phylanx/config/export_definitions.hpp>
#include <phylanx/config/version.hpp>

#include <cstdint>
#include <string>

///////////////////////////////////////////////////////////////////////////////
namespace phylanx
{
    // Returns the major HPX version.
    PHYLANX_EXPORT std::uint8_t major_version();

    // Returns the minor HPX version.
    PHYLANX_EXPORT std::uint8_t minor_version();

    // Returns the sub-minor/patch-level HPX version.
    PHYLANX_EXPORT std::uint8_t subminor_version();

    // Returns the full HPX version.
    PHYLANX_EXPORT std::uint32_t full_version();

    // Returns the full HPX version.
    PHYLANX_EXPORT std::string full_version_as_string();
}

#endif
