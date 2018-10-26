// Copyright (c) 2018 Parsa Amini
// Copyright (c) 2018 Hartmut Kaiser
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/util/random.hpp>

#include <cstdint>
#include <random>

namespace phylanx { namespace util
{
    std::uint32_t seed_ = 0;     // The current seed for the generator.

    std::uint32_t default_seed()
    {
        static const std::uint32_t seed =
            static_cast<std::uint32_t>(std::random_device{}());
        return seed;
    }

    std::mt19937 rng_{default_seed()};    // The Mersenne twister generator.

    void set_seed(std::uint32_t seed)
    {
        seed_ = seed;
        rng_.seed(seed_);
    }

    std::uint32_t get_seed()
    {
        return seed_;
    }
}}
