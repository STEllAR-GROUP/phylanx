// Copyright (c) 2018 Shahrzad Shirzad
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef PHYLANX_UTIL_DETAIL_BLAZE_SIMD_SUB_MAY_16_2018_0500PM
#define PHYLANX_UTIL_DETAIL_BLAZE_SIMD_SUB_MAY_16_2018_0500PM

#include <blaze/Math.h>

namespace phylanx { namespace util { namespace detail {
    struct sub0dnd_simd
    {
    public:
        explicit sub0dnd_simd(double scalar)
          : scalar_(scalar)
        {
        }

        template <typename T>
        BLAZE_ALWAYS_INLINE auto operator()(T const& a) const
            -> decltype(std::declval<double>() - a)
        {
            return scalar_ - a;
        }

        template <typename T>
        static constexpr bool simdEnabled()
        {
            return blaze::HasSIMDSub<T, double>::value;
        }

        template <typename T>
        BLAZE_ALWAYS_INLINE decltype(auto) load(T const& a) const
        {
            BLAZE_CONSTRAINT_MUST_BE_SIMD_PACK(T);
            return blaze::set(scalar_) - a;
        }

    private:
        double scalar_;
    };

    struct subnd0d_simd
    {
    public:
        explicit subnd0d_simd(double scalar)
          : scalar_(scalar)
        {
        }

        template <typename T>
        BLAZE_ALWAYS_INLINE auto operator()(T const& a) const
            -> decltype(a - std::declval<double>())
        {
            return a - scalar_;
        }

        template <typename T>
        static constexpr bool simdEnabled()
        {
            return blaze::HasSIMDSub<T, double>::value;
        }

        template <typename T>
        BLAZE_ALWAYS_INLINE decltype(auto) load(T const& a) const
        {
            BLAZE_CONSTRAINT_MUST_BE_SIMD_PACK(T);
            return a - blaze::set(scalar_);
        }

    private:
        double scalar_;
    };
}}}
#endif
