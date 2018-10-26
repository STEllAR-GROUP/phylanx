// Copyright (c) 2018 Parsa Amini
// Copyright (c) 2018 Hartmut Kaiser
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>

#include <cstdint>
#include <random>

#if !defined(PHYLANX_PRIMITIVES_RANDOM_UTILS)
#define PHYLANX_PRIMITIVES_RANDOM_UTILS

namespace phylanx { namespace util
{
    extern std::uint32_t seed_;     // The current seed for the generator.

    std::uint32_t default_seed();

    PHYLANX_EXPORT extern std::mt19937 rng_;    // The Mersenne twister generator.

    PHYLANX_EXPORT void set_seed(std::uint32_t seed);

    PHYLANX_EXPORT std::uint32_t get_seed();
}}

#endif
