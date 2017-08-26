//  Copyright (c) 2017 Hartmut Kaiser
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// Make HPX inspect tool happy: hpxinspect:nounnamed

#if !defined(PHYLANX_CONFIG_VERSION_HPP)
#define PHYLANX_CONFIG_VERSION_HPP

#include <phylanx/config.hpp>
#include <phylanx/config/export_definitions.hpp>

#include <hpx/util/detail/pp/cat.hpp>
#include <hpx/util/detail/pp/expand.hpp>

#include <boost/version.hpp>

///////////////////////////////////////////////////////////////////////////////
//  The version of PHYLANX
//
//  PHYLANX_VERSION_FULL & 0x0000FF is the sub-minor version
//  PHYLANX_VERSION_FULL & 0x00FF00 is the minor version
//  PHYLANX_VERSION_FULL & 0xFF0000 is the major version
//
//  PHYLANX_VERSION_DATE   YYYYMMDD is the date of the release
//                               (estimated release date for master branch)
//
#define PHYLANX_VERSION_FULL         0x000001

#define PHYLANX_VERSION_MAJOR        0
#define PHYLANX_VERSION_MINOR        0
#define PHYLANX_VERSION_SUBMINOR     1

#define PHYLANX_VERSION_DATE         20170825

#define PHYLANX_VERSION_TAG          ""

#if !defined(PHYLANX_HAVE_GIT_COMMIT)
    #define PHYLANX_HAVE_GIT_COMMIT  "unknown"
#endif

///////////////////////////////////////////////////////////////////////////////
// The version check enforces the major and minor version numbers to match for
// every compilation unit to be compiled.
#define PHYLANX_CHECK_VERSION                                                 \
    HPX_PP_CAT(phylanx_check_version_,                                        \
        HPX_PP_CAT(PHYLANX_VERSION_MAJOR,                                     \
            HPX_PP_CAT(_, PHYLANX_VERSION_MINOR)))                            \
    /**/

// The version check enforces the major and minor version numbers to match for
// every compilation unit to be compiled.
#define PHYLANX_CHECK_BOOST_VERSION                                           \
    HPX_PP_CAT(hpx_check_boost_version_, HPX_PP_EXPAND(BOOST_VERSION))        \
    /**/

///////////////////////////////////////////////////////////////////////////////
namespace phylanx
{
    // Helper data structures allowing to automatically detect version problems
    // between applications and the core libraries.
    PHYLANX_EXPORT extern char const PHYLANX_CHECK_VERSION[];
    PHYLANX_EXPORT extern char const PHYLANX_CHECK_BOOST_VERSION[];
}

///////////////////////////////////////////////////////////////////////////////
#if !defined(PHYLANX_EXPORTS) && !defined(PHYLANX_NO_VERSION_CHECK)
    // This is instantiated for each translation unit outside of the PHYLANX
    // core library, forcing to resolve the variable PHYLANX_CHECK_VERSION.
    namespace
    {
        // Note: this function is never executed.
#if defined(__GNUG__)
        __attribute__ ((unused))
#endif
        char const* check_phylanx_version()
        {
            char const* versions[] = {
                phylanx::PHYLANX_CHECK_VERSION,
                phylanx::PHYLANX_CHECK_BOOST_VERSION
            };
            return versions[0];
        }
    }
#endif

#endif
