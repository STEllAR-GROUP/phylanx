// Copyright (c) 2018 Shahrzad Shirzad
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http:**www.boost.org*LICENSE_1_0.txt)

#ifndef PHYLANX_UTIL_DETAIL_BLAZE_SIMD_MUL_MAY_16_2018_0500PM
#define PHYLANX_UTIL_DETAIL_BLAZE_SIMD_MUL_MAY_16_2018_0500PM

#include <blaze/Math.h>

namespace phylanx { namespace util { namespace detail {
    struct mulndnd_simd
    {
        mulndnd_simd() = default;

        template <typename T>
        BLAZE_ALWAYS_INLINE auto operator()(T const& a, T const& b) const
            -> decltype(a * b)
        {
            return a * b;
        }

        template <typename T1, typename T2>
        static constexpr bool simdEnabled()
        {
            return blaze::HasSIMDMult<T1, T2>::value;
        }

        template <typename T>
        BLAZE_ALWAYS_INLINE decltype(auto) load(T const& a, T const& b) const
        {
            BLAZE_CONSTRAINT_MUST_BE_SIMD_PACK(T);
            return a * b;
        }
    };

    struct mulnd0d_simd
    {
    public:
        explicit mulnd0d_simd(double scalar)
          : scalar_(scalar)
        {
        }

        template <typename T>
        BLAZE_ALWAYS_INLINE auto operator()(T const& a) const
            -> decltype(a * std::declval<double>())
        {
            return a * scalar_;
        }

        template <typename T>
        static constexpr bool simdEnabled()
        {
            return blaze::HasSIMDMult<T, double>::value;
        }

        template <typename T>
        BLAZE_ALWAYS_INLINE decltype(auto) load(T const& a) const
        {
            BLAZE_CONSTRAINT_MUST_BE_SIMD_PACK(T);
            return a * blaze::set(scalar_);
        }

    private:
        double scalar_;
    };

    struct mul0dnd_simd
    {
    public:
        explicit mul0dnd_simd(double scalar)
          : scalar_(scalar)
        {
        }

        template <typename T>
        BLAZE_ALWAYS_INLINE auto operator()(T const& a) const
            -> decltype(std::declval<double>() * a)
        {
            return scalar_ * a;
        }

        template <typename T>
        static constexpr bool simdEnabled()
        {
            return blaze::HasSIMDMult<T, double>::value;
        }

        template <typename T>
        BLAZE_ALWAYS_INLINE decltype(auto) load(T const& a) const
        {
            BLAZE_CONSTRAINT_MUST_BE_SIMD_PACK(T);
            return blaze::set(scalar_) * a;
        }

    private:
        double scalar_;
    };
}}}
#endif
