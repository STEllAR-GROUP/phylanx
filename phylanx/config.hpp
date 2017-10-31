//  Copyright (c) 2017 Hartmut Kaiser
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(PHYLANX_CONFIG_AUG_25_2017_0711PM)
#define PHYLANX_CONFIG_AUG_25_2017_0711PM

#include <hpx/config.hpp>

#include <phylanx/config/export_definitions.hpp>
#include <phylanx/config/version.hpp>

// TODO: If windows.h is included, ensure min and max macros are not defined
// after here or the code will fail to compile pointing at header files in
// Blaze. Therefore, the ordering of the includes matter. This needs a
// permanent fix
#if defined(_MSC_VER) && !defined(NOMINMAX)
#define NOMINMAX
#endif // NOMINMAX

// Use HPX threads. Do not use C++11 threads
#if !defined(BLAZE_USE_HPX_THREADS)
#define BLAZE_USE_HPX_THREADS 1

#if !defined(BLAZE_USE_SHARED_MEMORY_PARALLELIZATION) || BLAZE_USE_SHARED_MEMORY_PARALLELIZATION != 1
#define BLAZE_USE_SHARED_MEMORY_PARALLELIZATION 1
#endif BLAZE_USE_SHARED_MEMORY_PARALLELIZATION

#ifdef BLAZE_USE_CPP_THREADS
#undef BLAZE_USE_CPP_THREADS
#endif // BLAZE_USE_CPP_THREADS

#ifdef BLAZE_USE_BOOST_THREADS
#undef BLAZE_USE_BOOST_THREADS
#endif

#endif // BLAZE_USE_HPX_THREADS

#endif // PHYLANX_CONFIG_AUG_25_2017_0711PM
