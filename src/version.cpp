//  Copyright (c) 2017 Hartmut Kaiser
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>
#include <phylanx/version.hpp>

#include <hpx/util/detail/pp/expand.hpp>
#include <hpx/util/detail/pp/stringize.hpp>

#include <cstdint>
#include <string>

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
        std::string result(HPX_PP_STRINGIZE(HPX_PP_EXPAND(HPX_VERSION_MAJOR)));
        result += ".";
        result += HPX_PP_STRINGIZE(HPX_PP_EXPAND(HPX_VERSION_MINOR));
        result += ".";
        result += HPX_PP_STRINGIZE(HPX_PP_EXPAND(HPX_VERSION_SUBMINOR));
        return result;
    }

    ///////////////////////////////////////////////////////////////////////////
    char const PHYLANX_CHECK_VERSION[] =
        HPX_PP_STRINGIZE(HPX_PP_EXPAND(PHYLANX_CHECK_VERSION));
    char const PHYLANX_CHECK_BOOST_VERSION[] =
        HPX_PP_STRINGIZE(HPX_PP_EXPAND(PHYLANX_CHECK_VERSION));
}

