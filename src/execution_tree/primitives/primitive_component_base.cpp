//  Copyright (c) 2017-2018 Hartmut Kaiser
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>
#include <phylanx/execution_tree/compiler/primitive_name.hpp>
#include <phylanx/execution_tree/primitives/base_primitive.hpp>
#include <phylanx/execution_tree/primitives/primitive_component_base.hpp>
#include <phylanx/util/scoped_timer.hpp>

#include <hpx/include/lcos.hpp>
#include <hpx/include/util.hpp>
#include <hpx/runtime/config_entry.hpp>
#include <hpx/runtime/launch_policy.hpp>
#include <hpx/runtime/naming_fwd.hpp>
#include <hpx/throw_exception.hpp>

#include <cstddef>
#include <cstdint>
#include <set>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

namespace phylanx { namespace execution_tree { namespace primitives
{
    ///////////////////////////////////////////////////////////////////////////
    std::vector<primitive_argument_type> primitive_component_base::noargs{};

    primitive_component_base::primitive_component_base(
            std::vector<primitive_argument_type>&& params,
            std::string const& name, std::string const& codename,
            bool eval_direct)
      : operands_(std::move(params))
      , name_(name)
      , codename_(codename)
      , execute_directly_(eval_direct ? 1 : -1)
      , eval_count_(0ll)
      , eval_duration_(0ll)
    {
#if defined(HPX_HAVE_APEX)
        eval_name_ = name_ + "::eval";
#endif
    }

    namespace detail
    {
        template <typename T>
        struct keep_alive
        {
            template <typename T_>
            keep_alive(T_ && t)
              : t_(std::forward<T_>(t))
            {}

            void operator()() const {}

            T t_;
        };
    }

    template <typename T>
    detail::keep_alive<typename std::decay<T>::type> keep_alive(T && t)
    {
        return detail::keep_alive<typename std::decay<T>::type>(
            std::forward<T>(t));
    }

    hpx::future<primitive_argument_type> primitive_component_base::do_eval(
        std::vector<primitive_argument_type> const& params,
        eval_mode mode) const
    {
#if defined(HPX_HAVE_APEX)
        hpx::util::annotate_function annotate(eval_name_.c_str());
#endif

        util::scoped_timer<std::int64_t> timer(eval_duration_);
        ++eval_count_;

        auto f = this->eval(params, mode);
        if (!f.is_ready())
        {
            using shared_state_ptr =
                typename hpx::traits::detail::shared_state_ptr_for<
                    decltype(f)>::type;
            shared_state_ptr const& state =
                hpx::traits::future_access<decltype(f)>::get_shared_state(f);

            state->set_on_completed(keep_alive(std::move(timer)));
        }
        return f;
    }

    // eval_action
    hpx::future<primitive_argument_type> primitive_component_base::eval(
        std::vector<primitive_argument_type> const& params) const
    {
        return this->eval(params, eval_default);
    }

    hpx::future<primitive_argument_type> primitive_component_base::eval(
        std::vector<primitive_argument_type> const& params,
        eval_mode mode) const
    {
        return this->eval(params);
    }

    // store_action
    void primitive_component_base::store(primitive_argument_type&&)
    {
        HPX_THROW_EXCEPTION(hpx::invalid_status,
            "phylanx::execution_tree::primitives::primitive_component_base::"
                "store",
            generate_error_message(
                "store function should only be called for the primitives that "
                "support it (e.g. variables)"));
    }

    void primitive_component_base::store_set_1d(
        phylanx::ir::node_data<double>&&, std::vector<int64_t>&&)
    {
        HPX_THROW_EXCEPTION(hpx::invalid_status,
            "phylanx::execution_tree::primitives::primitive_component_base::"
            "store_set_1d",
            generate_error_message("store_set_1d function should only be "
                                   "called for the primitives that "
                                   "support it (e.g. variables)"));
    }

    void primitive_component_base::store_set_2d(
        phylanx::ir::node_data<double>&&, std::vector<int64_t>&&,
        std::vector<int64_t>&&)
    {
        HPX_THROW_EXCEPTION(hpx::invalid_status,
            "phylanx::execution_tree::primitives::primitive_component_base::"
            "store_set_1d",
            generate_error_message("store_set_2d function should only be "
                                   "called for the primitives that "
                                   "support it (e.g. variables)"));
    }

    // extract_topology_action
    topology primitive_component_base::expression_topology(
        std::set<std::string>&& functions,
        std::set<std::string>&& resolve_children) const
    {
        std::vector<hpx::future<topology>> results;
        results.reserve(operands_.size());

        for (auto& operand : operands_)
        {
            primitive const* p = util::get_if<primitive>(&operand);
            if (p != nullptr)
            {
                std::set<std::string> funcs{functions};
                std::set<std::string> resolve{resolve_children};
                results.push_back(p->expression_topology(
                    std::move(funcs), std::move(resolve)));
            }
        }

        std::vector<topology> children;
        if (!results.empty())
        {
            hpx::wait_all(results);

            for (auto& r : results)
            {
                children.emplace_back(r.get());
            }
        }

        return topology{std::move(children)};
    }

    // bind_action
    bool primitive_component_base::bind(
        std::vector<primitive_argument_type> const& params) const
    {
        HPX_THROW_EXCEPTION(hpx::invalid_status,
            "phylanx::execution_tree::primitives::"
                "primitive_component_base::bind",
            generate_error_message(
                "bind function should only be called for the "
                    "primitives that support it (e.g. variable/function)"));
        return true;
    }

    std::string primitive_component_base::generate_error_message(
        std::string const& msg) const
    {
        return execution_tree::generate_error_message(msg, name_, codename_);
    }

    std::int64_t primitive_component_base::get_eval_count(bool reset) const
    {
        return hpx::util::get_and_reset_value(eval_count_, reset);
    }

    std::int64_t primitive_component_base::get_eval_duration(bool reset) const
    {
        return hpx::util::get_and_reset_value(eval_duration_, reset);
    }

    std::int64_t primitive_component_base::get_direct_execution(bool reset) const
    {
        return hpx::util::get_and_reset_value(execute_directly_, reset);
    }

    ////////////////////////////////////////////////////////////////////////////
    // decide whether to execute eval directly
    bool primitive_component_base::get_sync_execution()
    {
        static bool sync_execution =
            hpx::get_config_entry("phylanx.sync_execution", "0") == "1";
        return sync_execution;
    }

    hpx::launch primitive_component_base::select_direct_eval_execution(
        hpx::launch policy) const
    {
        // always run this on an HPX thread
        if (hpx::threads::get_self_ptr() == nullptr)
        {
            return hpx::launch::async;
        }

        // always execute synchronously, if requested
        if (get_sync_execution())
        {
            return hpx::launch::sync;
        }

        if (eval_count_ != 0)
        {
            // check whether execution status needs to be changed (with some
            // hysteresis)
            std::int64_t exec_time = (eval_duration_ / eval_count_);
            if (exec_time > 300000)
            {
                execute_directly_ = 0;
            }
            else if (exec_time < 150000)
            {
                execute_directly_ = 1;
            }
        }

        if (execute_directly_ == 1)
        {
            return hpx::launch::sync;
        }
        else if (execute_directly_ == 0)
        {
            return hpx::launch::async;
        }

        return policy;
    }
}}}

namespace phylanx { namespace execution_tree
{
    std::string generate_error_message(std::string const& msg,
        compiler::primitive_name_parts const& parts,
        std::string const& codename)
    {
        return generate_error_message(msg,
            compiler::compose_primitive_name(parts), codename);
    }

    std::string generate_error_message(std::string const& msg,
        std::string const& name, std::string const& codename)
    {
        if (!name.empty())
        {
            compiler::primitive_name_parts parts;

            if (compiler::parse_primitive_name(name, parts))
            {
                std::string line_col;
                if (parts.tag1 != -1 && parts.tag2 != -1)
                {
                    line_col = hpx::util::format(
                        "(" PHYLANX_FORMAT_SPEC(1) ", "
                            PHYLANX_FORMAT_SPEC(2) ")",
                        parts.tag1, parts.tag2);
                }

                if (!parts.instance.empty())
                {
                    return hpx::util::format(
                        PHYLANX_FORMAT_SPEC(1) PHYLANX_FORMAT_SPEC(2)
                            ": " PHYLANX_FORMAT_SPEC(3)
                            "$"  PHYLANX_FORMAT_SPEC(4)
                            ":: " PHYLANX_FORMAT_SPEC(5),
                        codename.empty() ? "<unknown>" : codename, line_col,
                        parts.primitive, parts.instance, msg);
                }

                return hpx::util::format(
                    PHYLANX_FORMAT_SPEC(1) PHYLANX_FORMAT_SPEC(2)
                        ": " PHYLANX_FORMAT_SPEC(3)
                        ":: " PHYLANX_FORMAT_SPEC(4),
                    codename.empty() ? "<unknown>" : codename, line_col,
                    parts.primitive, msg);
            }

            return hpx::util::format(
                PHYLANX_FORMAT_SPEC(1)
                    ": "  PHYLANX_FORMAT_SPEC(2)
                    ":: " PHYLANX_FORMAT_SPEC(3),
                codename.empty() ? "<unknown>" : codename, name, msg);
        }

        return hpx::util::format(
            PHYLANX_FORMAT_SPEC(1) ": "  PHYLANX_FORMAT_SPEC(2),
            codename.empty() ? "<unknown>" : codename, msg);
    }
}}
