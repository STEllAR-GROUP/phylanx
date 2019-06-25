// Copyright (c) 2019 Hartmut Kaiser
// Copyright (c) 2019 Bita Hasheminezhad
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(PHYLANX_KERAS_SUPPORT_POOL_INDICES_HELPER)
#define PHYLANX_KERAS_SUPPORT_POOL_INDICES_HELPER

#include <cstdint>

#include <blaze/Math.h>

struct sizes
{
    std::int64_t image_beg_;
    std::int64_t size_;
};

///////////////////////////////////////////////////////////////////////
inline sizes get_subsizes(std::int64_t image_size,
    std::int64_t kernel_size, std::int64_t relative_position)
{
    if (relative_position < 0)
    {
        if (kernel_size + relative_position > image_size)
            return sizes{0, image_size};

        return sizes{0, kernel_size + relative_position};
    }

    if (relative_position > image_size - kernel_size)
    {
        return sizes{relative_position, image_size - relative_position};
    }

    return sizes{relative_position, kernel_size};
}

#endif
