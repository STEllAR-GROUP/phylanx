//  Copyright (c) 2014-2018 Hartmut Kaiser
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(PHYLANX_UTIL_DIRECT_EXECUTION_POLICY_FEB_04_2018_0155PM)
#define PHYLANX_UTIL_DIRECT_EXECUTION_POLICY_FEB_04_2018_0155PM

#include <phylanx/config.hpp>

#include <hpx/config.hpp>
#include <hpx/lcos/detail/async_colocated.hpp>
#include <hpx/lcos/detail/async_colocated_callback.hpp>
#include <hpx/lcos/detail/async_implementations.hpp>
#include <hpx/lcos/future.hpp>
#include <hpx/runtime/applier/detail/apply_colocated_callback_fwd.hpp>
#include <hpx/runtime/applier/detail/apply_colocated_fwd.hpp>
#include <hpx/runtime/applier/detail/apply_implementations.hpp>
#include <hpx/runtime/components/client_base.hpp>
#include <hpx/runtime/components/stubs/stub_base.hpp>
#include <hpx/runtime/launch_policy.hpp>
#include <hpx/runtime/find_here.hpp>
#include <hpx/runtime/naming/id_type.hpp>
#include <hpx/runtime/naming/name.hpp>
#include <hpx/runtime/serialization/serialization_fwd.hpp>
#include <hpx/traits/extract_action.hpp>
#include <hpx/traits/is_distribution_policy.hpp>
#include <hpx/traits/promise_local_result.hpp>

#include <algorithm>
#include <cstddef>
#include <type_traits>
#include <utility>
#include <vector>

namespace phylanx { namespace util
{
    namespace detail
    {
        struct direct_execution_policy_creator;
    }

    struct direct_execution_policy
    {
    private:
        friend struct detail::direct_execution_policy_creator;

    public:
        ///////////////////////////////////////////////////////////////////////
        // implement only async functionality as apply can't be executed
        // synchronously anyways
        template <typename Action, typename ...Ts>
        hpx::future<
            typename hpx::traits::promise_local_result<
                typename hpx::traits::extract_action<Action>::remote_result_type
            >::type>
        async(hpx::launch policy, Ts&&... vs) const
        {
            return hpx::detail::async_impl<Action>(
                direct_execution_ ? hpx::launch::sync : policy, id_,
                std::forward<Ts>(vs)...);
        }

        template <typename Action, typename Callback, typename ...Ts>
        hpx::future<
            typename hpx::traits::promise_local_result<
                typename hpx::traits::extract_action<Action>::remote_result_type
            >::type>
        async_cb(hpx::launch policy, Callback&& cb, Ts&&... vs) const
        {
            return hpx::detail::async_cb_impl<Action>(
                direct_execution_ ? hpx::launch::sync : policy, id_,
                std::forward<Callback>(cb), std::forward<Ts>(vs)...);
        }

    protected:
        /// \cond NOINTERNAL
        direct_execution_policy(hpx::id_type const& id, bool direct_execution)
          : id_(id)
          , direct_execution_(direct_execution)
        {}

        friend class hpx::serialization::access;

        template <typename Archive>
        void serialize(Archive& ar, unsigned int const)
        {
            ar & id_ & direct_execution_;
        }

        hpx::id_type id_;
        bool direct_execution_;
        /// \endcond
    };

    namespace detail
    {
        struct direct_execution_policy_creator
        {
            direct_execution_policy_creator() = default;

            direct_execution_policy operator()(
                hpx::id_type const& id, bool direct_execution = true) const
            {
                return direct_execution_policy(id, direct_execution);
            }

            template <typename Client, typename Stub>
            direct_execution_policy operator()(
                hpx::components::client_base<Client, Stub> const& client,
                bool direct_execution = true) const
            {
                return direct_execution_policy(client.get_id(), direct_execution);
            }
        };
    }

    static constexpr detail::direct_execution_policy_creator const
        direct_execution{};
}}

namespace hpx { namespace traits
{
    template <>
    struct is_distribution_policy<phylanx::util::direct_execution_policy>
        : std::true_type
    {};
}}

#endif
