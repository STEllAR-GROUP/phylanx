// Copyright (c) 2017-2018 Hartmut Kaiser
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(PHYLANX_UTIL_FUTURE_OR_VALUE_JUL_23_2018_0117PM)
#define PHYLANX_UTIL_FUTURE_OR_VALUE_JUL_23_2018_0117PM

#include <phylanx/config.hpp>
#include <phylanx/util/variant.hpp>

#include <hpx/async.hpp>
#include <hpx/futures/future.hpp>
#include <hpx/memory/intrusive_ptr.hpp>
#include <hpx/traits/acquire_future.hpp>
#include <hpx/traits/future_traits.hpp>
#include <hpx/traits/is_future.hpp>
#include <hpx/util/steady_clock.hpp>

#include <type_traits>
#include <utility>

namespace phylanx { namespace util
{
    template <typename T>
    struct future_or_value
    {
        future_or_value() = default;

        future_or_value(T const& val)
          : data_(val)
        {}
        future_or_value(T && val)
          : data_(std::move(val))
        {}
        future_or_value(hpx::future<T> && f)
          : data_(std::move(f))
        {}

        future_or_value& operator=(T const& val)
        {
            data_ = val;
            return *this;
        }
        future_or_value& operator=(T && val)
        {
            data_ = std::move(val);
            return *this;
        }
        future_or_value& operator=(hpx::future<T> && f)
        {
            data_ = std::move(f);
            return *this;
        }

        ///////////////////////////////////////////////////////////////////////
        T get()
        {
            if (data_.index() == 0)
            {
                return std::move(util::get<0>(data_));
            }
            return util::get<1>(data_).get();
        }

        ///////////////////////////////////////////////////////////////////////
        bool is_ready() const
        {
            return data_.index() == 0 || util::get<1>(data_).is_ready();
        }

        bool valid() const
        {
            return data_.index() == 0 || util::get<1>(data_).valid();
        }

        bool has_value() const
        {
            return data_.index() == 0 || util::get<1>(data_).has_value();
        }

        bool has_exception() const
        {
            return data_.index() != 0 && util::get<1>(data_).has_exception();
        }

        ///////////////////////////////////////////////////////////////////////
        void wait() const
        {
            if (data_.index() == 1)
            {
                util::get<1>(data_).wait();
            }
        }

        hpx::lcos::future_status wait_until(
            hpx::util::steady_time_point const& abs_time) const
        {
            if (data_.index() == 1)
            {
                return util::get<1>(data_).wait_until(abs_time);
            }
            return hpx::lcos::future_status::ready;
        }

        hpx::lcos::future_status wait_for(
            hpx::util::steady_duration const& rel_time) const
        {
            if (data_.index() == 1)
            {
                return util::get<1>(data_).wait_for(rel_time);
            }
            return hpx::lcos::future_status::ready;
        }

        ///////////////////////////////////////////////////////////////////////
        template <typename F>
        struct future_then_dispatch
            : hpx::lcos::detail::future_then_dispatch<future_or_value, F>
        {};

        template <typename F>
        typename hpx::traits::future_then_result<future_or_value, F>::type
        then(hpx::launch policy, F && f)
        {
            if (data_.index() == 1)
            {
                if (!get_shared_state())
                {
                    HPX_THROW_EXCEPTION(hpx::no_state,
                        "future_or_value<T>::then",
                        "this future_or_value has no valid shared state");

                    using result_type =
                        typename hpx::traits::future_then_result<
                            future_or_value, F>::type;

                    return result_type();
                }

                return future_then_dispatch<hpx::launch>::call(
                    std::move(util::get<1>(data_)), policy, std::forward<F>(f));
            }

            return hpx::async(policy,
                [f = std::forward<F>(f)](T&& val) mutable
                {
                    return std::forward<F>(f)(std::move(val));
                },
                std::move(util::get<0>(data_)));
        }

    private:
        template <typename Future>
        friend struct hpx::traits::future_access;

        template <typename Future, typename Enable>
        friend struct hpx::traits::detail::future_access_customization_point;

        typename hpx::traits::detail::shared_state_ptr<T>::type const&
        get_shared_state() const
        {
            return hpx::traits::future_access<hpx::future<T>>::
                get_shared_state(util::get<1>(data_));
        }

        typename hpx::traits::detail::shared_state_ptr<T>::type::element_type*
        detach_shared_state()
        {
            return hpx::traits::future_access<hpx::future<T>>::
                detach_shared_state(util::get<1>(std::move(data_)));
        }

        phylanx::util::variant<T, hpx::future<T>> data_;
    };
}}

// define traits that make look this type like as if it was a future
namespace hpx { namespace traits
{
    namespace detail
    {
        ///////////////////////////////////////////////////////////////////////
        // traits::is_unique_future<>
        template <typename T>
        struct is_unique_future<phylanx::util::future_or_value<T>>
          : std::true_type
        {};

        ///////////////////////////////////////////////////////////////////////
        // traits::is_future<>
        template <typename T>
        struct is_future_customization_point<phylanx::util::future_or_value<T>>
          : std::true_type
        {};

        ///////////////////////////////////////////////////////////////////////
        // traits::future_traits<>
        template <typename T>
        struct future_traits_customization_point<
            phylanx::util::future_or_value<T>>
        {
            using type = T;
            using result_type = T;
        };

        ///////////////////////////////////////////////////////////////////////
        // traits::acquire_future
        template <typename T>
        struct acquire_future_impl<phylanx::util::future_or_value<T>>
        {
            typedef phylanx::util::future_or_value<T> type;

            HPX_FORCEINLINE phylanx::util::future_or_value<T>
            operator()(phylanx::util::future_or_value<T>& future) const
            {
                return std::move(future);
            }

            HPX_FORCEINLINE phylanx::util::future_or_value<T>
            operator()(phylanx::util::future_or_value<T>&& future) const
            {
                return std::move(future);
            }
        };

        ///////////////////////////////////////////////////////////////////////
        // traits::future_access
        template <typename T>
        struct future_access_customization_point<
            phylanx::util::future_or_value<T>>
        {
            template <typename SharedState>
            static phylanx::util::future_or_value<T>
            create(hpx::intrusive_ptr<SharedState> const& shared_state)
            {
                return phylanx::util::future_or_value<T>(
                    hpx::future<T>(shared_state));
            }

            template <typename SharedState>
            static phylanx::util::future_or_value<T>
            create(hpx::intrusive_ptr<SharedState>&& shared_state)
            {
                return phylanx::util::future_or_value<T>(
                    hpx::future<T>(std::move(shared_state)));
            }

            template <typename SharedState>
            static phylanx::util::future_or_value<T>
            create(SharedState* shared_state)
            {
                return phylanx::util::future_or_value<T>(hpx::future<T>(
                    hpx::intrusive_ptr<SharedState>(shared_state)));
            }

            HPX_FORCEINLINE static
            typename hpx::traits::detail::shared_state_ptr<T>::type const&
            get_shared_state(phylanx::util::future_or_value<T> const& f)
            {
                return f.get_shared_state();
            }

            HPX_FORCEINLINE static
            typename hpx::traits::detail::shared_state_ptr<T>::type::element_type*
            detach_shared_state(phylanx::util::future_or_value<T> && f)
            {
                return f.detach_shared_state();
            }
        };
    }
}}

#endif
