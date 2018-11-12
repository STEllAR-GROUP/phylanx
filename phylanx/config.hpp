//  Copyright (c) 2017-2018 Hartmut Kaiser
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(PHYLANX_CONFIG_AUG_25_2017_0711PM)
#define PHYLANX_CONFIG_AUG_25_2017_0711PM

#include <hpx/config.hpp>

#include <phylanx/config/defines.hpp>
#include <phylanx/config/export_definitions.hpp>
#include <phylanx/config/format.hpp>
#include <phylanx/config/version.hpp>

///////////////////////////////////////////////////////////////////////////////
// Make sure DEBUG macro is defined consistently across platforms
#if defined(_DEBUG) && !defined(DEBUG)
#  define DEBUG
#endif

///////////////////////////////////////////////////////////////////////////////
#if defined(DEBUG) && !defined(PHYLANX_DEBUG)
#  define PHYLANX_DEBUG
#endif

///////////////////////////////////////////////////////////////////////////////
// Blaze: use HPX threads. Do not use C++11 threads
#if !defined(BLAZE_USE_HPX_THREADS) || BLAZE_USE_HPX_THREADS != 1
#error "Please make sure Blaze is configured to use HPX threads for parallelization while compiling Phylanx"
#endif
#if !defined(BLAZE_USE_SHARED_MEMORY_PARALLELIZATION) || \
    BLAZE_USE_SHARED_MEMORY_PARALLELIZATION != 1
#error "Please make sure Blaze is configured to use shared memory parallelization while compiling Phylanx"
#endif

#undef BLAZE_USE_CPP_THREADS
#undef BLAZE_USE_BOOST_THREADS

///////////////////////////////////////////////////////////////////////////////
#if !defined(PHYLANX_HAVE_BLAZETENSOR)
#  define PHYLANX_MAX_DIMENSIONS 2
#else
#  define PHYLANX_MAX_DIMENSIONS 3
#endif

#endif // PHYLANX_CONFIG_AUG_25_2017_0711PM
