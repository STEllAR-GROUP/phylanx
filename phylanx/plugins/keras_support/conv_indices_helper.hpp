// Copyright (c) 2019 Hartmut Kaiser
// Copyright (c) 2019 Bita Hasheminezhad
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(PHYLANX_KERAS_SUPPORT_CONV_INDICES_HELPER)
#define PHYLANX_KERAS_SUPPORT_CONV_INDICES_HELPER

#include <cstdint>

#include <blaze/Math.h>

struct sizes
{
    std::int64_t image_beg_;
    std::int64_t kernel_beg_;
    std::int64_t size_;
};

///////////////////////////////////////////////////////////////////////
inline sizes get_subsizes(std::int64_t image_size,
    std::int64_t kernel_size, std::int64_t relative_position)
{
    if (relative_position < 0)
    {
        if (kernel_size + relative_position > image_size)
            return sizes{0, -relative_position, image_size};

        return sizes{
            0, -relative_position, kernel_size + relative_position};
    }

    if (relative_position > image_size - kernel_size)
    {
        return sizes{
            relative_position, 0, image_size - relative_position};
    }

    return sizes{relative_position, 0, kernel_size};
}

inline sizes get_subsizes_dilated(std::int64_t image_size,
    std::int64_t kernel_size, std::int64_t relative_position,
    std::int64_t dilation_rate)
{
    if (relative_position < 0)
    {
        std::int64_t remainder = relative_position % dilation_rate;
        remainder = remainder >= 0 ? remainder : dilation_rate + remainder;
        std::int64_t corrected_kernel_size = 0;
        if (dilation_rate - remainder <= image_size)
        {
            if ((kernel_size - 1) * dilation_rate + relative_position >=
                image_size)
            {
                corrected_kernel_size = blaze::ceil(
                    static_cast<double>(image_size - remainder) /
                    dilation_rate);
            }
            else
            {
                corrected_kernel_size = kernel_size +
                    blaze::floor(
                        static_cast<double>(relative_position) /
                        dilation_rate);
            }
        }
        std::int64_t kernel_beg_ = blaze::ceil(
            static_cast<double>(-relative_position) / dilation_rate);
        return sizes{remainder, kernel_beg_, corrected_kernel_size};
    }

    if (relative_position >= image_size - (kernel_size - 1) * dilation_rate)
    {
        std::int64_t corrected_kernel_size = blaze::ceil(
            static_cast<double>(image_size - relative_position) /
            dilation_rate);
        return sizes{relative_position, 0, corrected_kernel_size};
    }

    return sizes{relative_position, 0, kernel_size};
}

#endif
