//  Copyright (c) 2018 Hartmut Kaiser
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(PHYLANX_PYTHON_GIL_HOOK_JUN_07_2018_0214AM)
#define PHYLANX_PYTHON_GIL_HOOK_JUN_07_2018_0214AM

#include <phylanx/config.hpp>
#include <phylanx/util/runs_in_python.hpp>

#include <hpx/runtime/get_lva.hpp>
#include <hpx/runtime/threads/coroutines/coroutine.hpp>
#include <hpx/traits/action_decorate_function.hpp>
#include <hpx/assertion.hpp>
#include <hpx/util/bind.hpp>
#include <hpx/util/bind_front.hpp>

#include <utility>
#include <type_traits>

namespace phylanx { namespace execution_tree { namespace primitives
{
    // This hook can be inserted into the derivation chain of any component
    // allowing to automatically handle the Python GIL while suspending HPX
    // threads.
    template <typename BaseComponent>
    struct python_gil_hook : BaseComponent
    {
    private:
        using base_type = BaseComponent;
        using this_component_type = typename base_type::this_component_type;

    public:
        template <typename ...Arg>
        python_gil_hook(Arg &&... arg)
          : base_type(std::forward<Arg>(arg)...)
        {}

        python_gil_hook(python_gil_hook const& rhs) = default;
        python_gil_hook(python_gil_hook && rhs) = default;

        using decorates_action = void;

        // This is the hook implementation for decorate_action which locks
        // the component ensuring that only one action is executed at a time
        // for this component instance.
        template <typename F>
        static hpx::threads::thread_function_type
        decorate_action(hpx::naming::address::address_type lva, F && f)
        {
            if (phylanx::util::get_runs_in_python())
            {
                return hpx::util::bind(
                    hpx::util::one_shot(&python_gil_hook::thread_function),
                    hpx::get_lva<this_component_type>::call(lva),
                    hpx::util::placeholders::_1,
                    hpx::traits::component_decorate_function<base_type>::call(
                        lva, std::forward<F>(f)));
            }

            return std::forward<F>(f);
        }

    protected:
        using yield_decorator_type =
            hpx::util::function_nonser<hpx::threads::thread_arg_type(
                hpx::threads::thread_result_type)>;

        struct decorate_wrapper
        {
            template <typename F, typename Enable = typename
                std::enable_if<!std::is_same<typename hpx::util::decay<F>::type,
                    decorate_wrapper>::value>::type>
            decorate_wrapper(F && f)
            {
                hpx::threads::get_self().decorate_yield(std::forward<F>(f));
            }

            ~decorate_wrapper()
            {
                hpx::threads::get_self().undecorate_yield();
            }
        };

        // Execute the wrapped action. This locks the mutex ensuring a thread
        // safe action invocation.
        hpx::threads::thread_result_type thread_function(
            hpx::threads::thread_arg_type state,
            hpx::threads::thread_function_type f)
        {
            HPX_ASSERT(util::get_runs_in_python());

            hpx::threads::thread_result_type result(hpx::threads::unknown,
                hpx::threads::invalid_thread_id);

            {
                // acquire GIL when running this HPX thread
                phylanx::util::detail::gil_scoped_acquire acquire;

                // register our yield decorator
                decorate_wrapper yield_decorator(hpx::util::bind_front(
                    &python_gil_hook::yield_function, this));

                result = f(state);

                (void)yield_decorator;       // silence gcc warnings
            }

            return result;
        }

        struct undecorate_wrapper
        {
            undecorate_wrapper()
              : yield_decorator_(hpx::threads::get_self().undecorate_yield())
            {}

            ~undecorate_wrapper()
            {
                hpx::threads::get_self().decorate_yield(
                    std::move(yield_decorator_));
            }

            yield_decorator_type yield_decorator_;
        };

        // The yield decorator unlocks the mutex and calls the system yield
        // which gives up control back to the thread manager.
        hpx::threads::thread_arg_type yield_function(
            hpx::threads::thread_result_type state)
        {
            HPX_ASSERT(phylanx::util::get_runs_in_python());

            // We un-decorate the yield function as the lock handling may
            // suspend, which causes an infinite recursion otherwise.
            undecorate_wrapper yield_decorator;
            hpx::threads::thread_arg_type result = hpx::threads::wait_unknown;

            {
                phylanx::util::detail::gil_scoped_release release;
                result = hpx::threads::get_self().yield_impl(state);
            }

            return result;
        }
    };
}}}

#endif
