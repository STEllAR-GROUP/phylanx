////////////////////////////////////////////////////////////////////////////////
//  Copyright (c) 2016 Thomas Heller
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
////////////////////////////////////////////////////////////////////////////////

#ifndef PHYLANX_UTIL_SCOPED_TIMER_HPP
#define PHYLANX_UTIL_SCOPED_TIMER_HPP

#include <hpx/util/high_resolution_clock.hpp>

#include <cstdint>

namespace phylanx { namespace util
{
    template <typename T>
    struct scoped_timer
    {
        scoped_timer(T& t)
          : started_at_(hpx::util::high_resolution_clock::now())
          , t_(&t)
        {}

        scoped_timer(scoped_timer const&) = default;
        scoped_timer(scoped_timer && rhs)
          : started_at_(rhs.started_at_)
          , t_(rhs.t_)
        {
            rhs.t_ = nullptr;
        }

        ~scoped_timer()
        {
            if (t_ != nullptr)
            {
                *t_ += (hpx::util::high_resolution_clock::now() - started_at_);
            }
        }

        scoped_timer& operator=(scoped_timer const& rhs) = default;
        scoped_timer& operator=(scoped_timer && rhs)
        {
            started_at_ = rhs.started_at_;
            t_ = rhs.t_;
            rhs.t_ = nullptr;
            return *this;
        }

    private:
        std::uint64_t started_at_;
        T* t_;
    };
}}

#endif
