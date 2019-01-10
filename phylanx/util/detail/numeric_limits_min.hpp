// Copyright (c) 2018 Hartmut Kaiser
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef PHYLANX_UTIL_DETAIL_NUMERIC_LIMITS_MIN_JAN_09_2019_1000
#define PHYLANX_UTIL_DETAIL_NUMERIC_LIMITS_MIN_JAN_09_2019_1000

#include <cstdint>
#include <limits>

namespace phylanx { namespace util { namespace detail
{
    template <typename T>
    T numeric_limits_min()
    {
        return -(std::numeric_limits<T>::max)();
    }

    template <>
    inline std::uint8_t numeric_limits_min<std::uint8_t>()
    {
        return 0;
    }
}}}

#endif
