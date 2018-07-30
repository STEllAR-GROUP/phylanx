//  Copyright (c) 2018 Hartmut Kaiser
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(PHYLANX_UTIL_SMALL_VECTOR_JUL_18_2018_0744PM)
#define PHYLANX_UTIL_SMALL_VECTOR_JUL_18_2018_0744PM

#include <phylanx/config.hpp>

#include <vector>

// before Boost V1.59 small_vector appears to be broken
#if BOOST_VERSION >= 105900
#include <boost/container/small_vector.hpp>
#endif

namespace phylanx { namespace util
{
#if BOOST_VERSION < 105900
    template <typename T>
    using small_vector = std::vector<T>;
#else
    template <typename T>
    using small_vector = boost::container::small_vector<T, 3>;
#endif
}}

#endif

