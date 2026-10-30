// Copyright (c) 2026 STE||AR Group
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// Names removed or moved between the HPX that Phylanx last built with (1.8)
// and HPX 1.11. Keep call sites on the old names.

#if !defined(PHYLANX_CONFIG_HPX_COMPAT_HPP)
#define PHYLANX_CONFIG_HPX_COMPAT_HPP

#include <hpx/chrono.hpp>
#include <hpx/parallel/algorithms/for_loop.hpp>
#include <hpx/collectives/barrier.hpp>
#include <hpx/components/iostreams/manipulators.hpp>
#include <hpx/errors/exception_fwd.hpp>
#include <hpx/execution.hpp>
#include <hpx/concurrency/spinlock.hpp>
#include <hpx/functional/bind.hpp>
#include <hpx/functional/bind_front.hpp>
#include <hpx/functional/function.hpp>
#include <hpx/future.hpp>
#include <hpx/synchronization/condition_variable.hpp>

#include <optional>

namespace hpx {

    // throwmode is an enum class. A using-declaration of its enumerators
    // requires C++20, and Phylanx builds as C++17.
    inline constexpr throwmode lightweight = throwmode::lightweight;
    inline constexpr throwmode plain = throwmode::plain;
    inline constexpr throwmode rethrow = throwmode::rethrow;

    inline constexpr iostreams::flush_type flush{};
    inline constexpr iostreams::endl_type endl{};
    inline constexpr iostreams::async_flush_type async_flush{};
    inline constexpr iostreams::async_endl_type async_endl{};

    // HPX 1.9 moved the algorithm function object out of namespace hpx.
    using experimental::for_loop;

}    // namespace hpx

namespace hpx::util {

    template <typename Signature>
    using function_nonser = hpx::function<Signature>;

    using hpx::bind;
    using hpx::bind_front;

    using hpx::chrono::steady_duration;
    using hpx::chrono::steady_time_point;

    template <typename T>
    using optional = std::optional<T>;

}    // namespace hpx::util

namespace hpx::lcos {

    using barrier = hpx::distributed::barrier;
    using future_status = hpx::future_status;

    template <typename R>
    using future = hpx::future<R>;

    namespace local {

        using condition_variable_any = hpx::condition_variable_any;
        using spinlock = hpx::util::spinlock;

    }    // namespace local

}    // namespace hpx::lcos

namespace hpx::parallel::execution {

    using hpx::execution::par;
    using hpx::execution::par_unseq;
    using hpx::execution::seq;

}    // namespace hpx::parallel::execution

#endif
