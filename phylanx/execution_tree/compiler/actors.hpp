//  Copyright (c) 2017-2018 Hartmut Kaiser
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(PHYLANX_EXECUTION_TREE_ACTORS_HPP)
#define PHYLANX_EXECUTION_TREE_ACTORS_HPP

#include <phylanx/config.hpp>
#include <phylanx/execution_tree/primitives/base_primitive.hpp>

#include <hpx/include/util.hpp>
#include <hpx/runtime/launch_policy.hpp>
#include <hpx/util/assert.hpp>

#include <array>
#include <cstddef>
#include <functional>
#include <list>
#include <map>
#include <set>
#include <string>
#include <utility>
#include <vector>

namespace phylanx { namespace execution_tree { namespace compiler
{
    ///////////////////////////////////////////////////////////////////////////
    using result_type = primitive_argument_type;
    using argument_type = primitive_argument_type;
    using arguments_type = primitive_arguments_type;

    using stored_function =
        hpx::util::function_nonser<result_type(arguments_type&&)>;

    ///////////////////////////////////////////////////////////////////////////
    template <typename Derived>
    struct actor
    {
        // direct execution
        result_type operator()(arguments_type const& args) const
        {
            arguments_type params;
            params.reserve(args.size());
            for (auto const& arg : args)
            {
                params.emplace_back(extract_ref_value(arg));
            }
            return derived().call(std::move(params));
        }

        result_type operator()(arguments_type && args) const
        {
            arguments_type params;
            params.reserve(args.size());
            for (auto && arg : args)
            {
                params.emplace_back(extract_ref_value(std::move(arg)));
            }
            return derived().call(std::move(params));
        }

        result_type operator()() const
        {
            return derived().call(arguments_type{});
        }

        template <typename ... Ts>
        result_type operator()(Ts &&... ts) const
        {
            arguments_type elements;
            elements.reserve(sizeof...(Ts));
            int const sequencer_[] = {
                0, (elements.emplace_back(
                        extract_ref_value(std::forward<Ts>(ts))), 0)...
            };
            (void)sequencer_;
            return derived().call(std::move(elements));
        }

    private:
        Derived const& derived() const
        {
            return *static_cast<Derived const*>(this);
        }
    };

    ///////////////////////////////////////////////////////////////////////////
    struct function
    {
        function() = default;

        function(primitive_argument_type const& arg)
          : arg_(arg)
        {}
        function(primitive_argument_type && arg)
          : arg_(std::move(arg))
        {}

        function(primitive_argument_type const& arg, std::string name)
          : arg_(arg)
        {
            set_name(std::move(name));
        }
        function(primitive_argument_type && arg, std::string name)
          : arg_(std::move(arg))
        {
            set_name(std::move(name));
        }

        // direct execution
        result_type run() const
        {
            if (is_primitive_operand(arg_))
            {
                return extract_copy_value(value_operand_sync(arg_,
                    primitive_arguments_type{}, name_));
            }
            return extract_ref_value(arg_);
        }

        // interpret this function as an invocable (evaluate object itself and
        // use the returned value to evaluate the arguments)
        result_type operator()(arguments_type const& args) const
        {
            if (is_primitive_operand(arg_))
            {
                arguments_type params;
                params.reserve(args.size());
                for (auto const& arg : args)
                {
                    params.emplace_back(extract_ref_value(arg));
                }

                return extract_copy_value(
                    value_operand_sync(arg_, std::move(params), name_));
            }
            return extract_ref_value(arg_);
        }

        result_type operator()(arguments_type && args) const
        {
            if (is_primitive_operand(arg_))
            {
                arguments_type keep_alive(std::move(args));

                // construct argument-pack to use for actual call
                arguments_type params;
                params.reserve(keep_alive.size());
                for (auto const& arg : keep_alive)
                {
                    params.emplace_back(extract_ref_value(arg));
                }

                return extract_copy_value(
                    value_operand_sync(arg_, std::move(params), name_));
            }
            return extract_ref_value(arg_);
        }

        template <typename ... Ts>
        result_type operator()(Ts &&... ts) const
        {
            if (is_primitive_operand(arg_))
            {
                // user-facing functions need to copy all arguments
                arguments_type keep_alive;
                keep_alive.reserve(sizeof...(Ts));

                int const sequencer_[] = {
                    0, (keep_alive.emplace_back(
                            extract_copy_value(primitive_argument_type{
                                std::forward<Ts>(ts)
                            })), 0)...
                };
                (void)sequencer_;

                // construct argument-pack to use for actual call
                arguments_type params;
                params.reserve(sizeof...(Ts));
                for (auto const& arg : keep_alive)
                {
                    params.emplace_back(extract_ref_value(arg));
                }

                return extract_copy_value(
                    value_operand_sync(arg_, std::move(params), name_));
            }

            return extract_ref_value(arg_);
        }

        hpx::future<result_type> eval(arguments_type && args) const
        {
            primitive const* p = util::get_if<primitive>(&arg_);
            if (p != nullptr)
            {
                // user-facing functions need to copy all arguments
                arguments_type keep_alive;
                keep_alive.reserve(args.size());
                for (auto && arg : std::move(args))
                {
                    keep_alive.emplace_back(extract_value(std::move(arg)));
                }

                return p->eval(std::move(keep_alive));
            }
            return hpx::make_ready_future(arg_);
        }

        void set_name(std::string && name)
        {
            name_ = std::move(name);
        }

        topology get_expression_topology() const
        {
            primitive const* p = util::get_if<primitive>(&arg_);
            if (p != nullptr)
            {
                return p->expression_topology(hpx::launch::sync,
                    std::set<std::string>{}, std::set<std::string>{});
            }
            return {};
        }

        topology get_expression_topology(std::set<std::string> && functions) const
        {
            primitive const* p = util::get_if<primitive>(&arg_);
            if (p != nullptr)
            {
                return p->expression_topology(hpx::launch::sync,
                    std::move(functions), std::set<std::string>{});
            }
            return {};
        }

        topology get_expression_topology(std::set<std::string>&& functions,
            std::set<std::string>&& resolve_children) const
        {
            primitive const* p = util::get_if<primitive>(&arg_);
            if (p != nullptr)
            {
                return p->expression_topology(hpx::launch::sync,
                    std::move(functions), std::move(resolve_children));
            }
            return {};
        }

        explicit operator bool() const
        {
            return bool(arg_);
        }

        primitive_argument_type arg_;
        std::string name_;
    };

    // this must be a list to ensure stable references
    struct program
    {
        program() = default;

        program(std::list<function> const& code)
          : code_(code)
        {
        }
        program(std::list<function>&& code)
          : code_(std::move(code))
        {
        }

        ///////////////////////////////////////////////////////////////////////
        // simply run the program, return whatever the last snippet has returned
        primitive_argument_type run() const
        {
            primitive_argument_type result;
            for (auto const& f : code_)
            {
                result = f.run();
            }
            return result;
        }

        ///////////////////////////////////////////////////////////////////////
        function& add_empty()
        {
            scratchpad_.emplace_back(function{});
            return scratchpad_.back();
        }
        function& add(function&& f)
        {
            scratchpad_.emplace_back(std::move(f));
            return scratchpad_.back();
        }

        function& add_entry_point(function&& f)
        {
            code_.emplace_back(std::move(f));
            return code_.back();
        }

        function const& entry_point() const
        {
            return code_.back();
        }

        std::list<function> const& entry_points() const
        {
            return code_;
        }

        void store_entry_points()
        {
            // move all current entry points to scratch pad
            scratchpad_.splice(scratchpad_.end(), code_);
        }

        bool has_entry_points() const
        {
            return !code_.empty();
        }

        ///////////////////////////////////////////////////////////////////////
        std::list<function> const& scratchpad() const
        {
            return scratchpad_;
        }

        ///////////////////////////////////////////////////////////////////////
        topology get_expression_topology() const
        {
            HPX_ASSERT(has_entry_points());
            std::set<std::string> functions;
            return entry_point().get_expression_topology(
                std::move(functions));
        }
        topology get_expression_topology(
            std::set<std::string>&& functions) const
        {
            HPX_ASSERT(has_entry_points());
            return entry_point().get_expression_topology(
                std::move(functions));
        }
        topology get_expression_topology(std::set<std::string>&& functions,
            std::set<std::string>&& resolve_children) const
        {
            HPX_ASSERT(has_entry_points());
            return entry_point().get_expression_topology(
                std::move(functions), std::move(resolve_children));
        }

    private:
        std::list<function> code_;
        std::list<function> scratchpad_;
    };

    struct function_list
    {
        function_list()
          : compile_id_(0)
        {}

        function_list(function_list const&) = delete;
        function_list(function_list &&) = delete;

        function_list& operator=(function_list const&) = delete;
        function_list& operator=(function_list &&) = delete;

        std::size_t compile_id_;    // sequence number of this compiler invocation
        program program_;           // storage for top-level code
        std::map<std::string, std::size_t> sequence_numbers_;
    };

    ///////////////////////////////////////////////////////////////////////////
    // arguments
    struct argument_function : actor<argument_function>
    {
        explicit argument_function(std::size_t n)
          : n(n)
        {}

        result_type call(arguments_type && args) const
        {
            return value_operand_sync(args[n], std::move(args));
        }
        hpx::future<result_type> eval(arguments_type && args) const
        {
            return value_operand(args[n], std::move(args));
        }

    private:
        std::size_t n;
    };
}}}

#endif
