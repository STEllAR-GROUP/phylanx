//  Copyright (c) 2017-2019 Hartmut Kaiser
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(PHYLANX_EXECUTION_TREE_ACTORS_HPP)
#define PHYLANX_EXECUTION_TREE_ACTORS_HPP

#include <phylanx/config.hpp>
#include <phylanx/execution_tree/primitives/base_primitive.hpp>

#include <hpx/assertion.hpp>
#include <hpx/include/util.hpp>
#include <hpx/runtime/launch_policy.hpp>
#include <hpx/serialization/serialization_fwd.hpp>

#include <array>
#include <cstddef>
#include <functional>
#include <list>
#include <map>
#include <memory>
#include <set>
#include <string>
#include <type_traits>
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
        function()
          : num_named_args_(0)
        {}

        function(primitive_argument_type const& arg)
          : arg_(arg)
          , num_named_args_(0)
        {}
        function(primitive_argument_type && arg)
          : arg_(std::move(arg))
          , num_named_args_(0)
        {}

        function(primitive_argument_type const& arg, std::string name)
          : arg_(arg)
          , num_named_args_(0)
        {
            set_name(std::move(name));
        }
        function(primitive_argument_type && arg, std::string name)
          : arg_(std::move(arg))
          , num_named_args_(0)
        {
            set_name(std::move(name));
        }

        // direct execution
        result_type run(eval_context ctx) const
        {
            if (is_primitive_operand(arg_))
            {
                return extract_copy_value(
                    value_operand_sync(arg_, primitive_arguments_type{}, name_,
                        "<unknown>", std::move(ctx)), name_);
            }
            return extract_copy_value(arg_, name_);
        }

        ///////////////////////////////////////////////////////////////////////
        // interpret this function as an invocable (evaluate object itself and
        // use the returned value to evaluate the arguments)
        result_type operator()(
            arguments_type const& args, eval_context ctx) const
        {
            if (is_primitive_operand(arg_))
            {
                arguments_type params;
                params.reserve(args.size());
                for (auto const& arg : args)
                {
                    params.emplace_back(extract_ref_value(arg, name_));
                }

                return extract_copy_value(
                    value_operand_sync(arg_, std::move(params), name_), name_);
            }
            return extract_copy_value(arg_, name_);
        }

        result_type operator()(arguments_type&& args, eval_context ctx) const
        {
            if (is_primitive_operand(arg_))
            {
                arguments_type keep_alive(std::move(args));

                // construct argument-pack to use for actual call
                arguments_type params;
                params.reserve(keep_alive.size());
                for (auto const& arg : keep_alive)
                {
                    params.emplace_back(extract_ref_value(arg, name_));
                }

                return extract_copy_value(
                    value_operand_sync(arg_, std::move(params), name_,
                        "<unknown>", std::move(ctx)),
                    name_);
            }
            return extract_copy_value(arg_, name_);
        }

        using kwarguments_type = std::map<std::string, primitive_argument_type>;

        result_type operator()(arguments_type&& args, kwarguments_type&& kwargs,
            eval_context ctx) const
        {
            if (is_primitive_operand(arg_))
            {
                arguments_type keep_alive(std::move(args));
                kwarguments_type kw_keep_alive(std::move(kwargs));

                // construct argument-pack to use for actual call
                arguments_type params;
                params.reserve(keep_alive.size() + num_named_args_);
                for (auto const& arg : keep_alive)
                {
                    params.emplace_back(extract_ref_value(arg, name_));
                }

                // fill in the given named arguments at their correct positions
                for (auto const& kwarg : kw_keep_alive)
                {
                    auto it = std::find(named_args_.get(),
                        named_args_.get() + num_named_args_, kwarg.first);
                    if (it == named_args_.get() + num_named_args_)
                    {
                        HPX_THROW_EXCEPTION(hpx::bad_parameter,
                            "function::operator()",
                            hpx::util::format("cannot locate requested "
                                "named argument '{}'", kwarg.first));
                    }

                    std::ptrdiff_t kwarg_pos =
                        std::distance(named_args_.get(), it);
                    if (kwarg_pos >= std::ptrdiff_t(params.size()))
                    {
                        params.resize(kwarg_pos + 1);
                    }

                    params[kwarg_pos] = extract_ref_value(kwarg.second, name_);
                }

                return extract_copy_value(
                    value_operand_sync(arg_, std::move(params), name_,
                        "<unknown>", std::move(ctx)),
                    name_);
            }
            return extract_copy_value(arg_, name_);
        }

        template <typename T1, typename ... Ts>
        typename std::enable_if<
           !std::is_same<eval_context, typename std::decay<T1>::type>::value,
            result_type
        >::type
        operator()(T1 && t1, Ts &&... ts) const
        {
            if (is_primitive_operand(arg_))
            {
                // user-facing functions need to copy all arguments
                arguments_type keep_alive;
                keep_alive.reserve(sizeof...(Ts) + 1);

                int const sequencer_[] = {
                    (keep_alive.emplace_back(
                        extract_copy_value(primitive_argument_type(
                            std::forward<T1>(t1)
                        ), name_)), 0)
                  , (keep_alive.emplace_back(
                        extract_copy_value(primitive_argument_type(
                            std::forward<Ts>(ts)
                        ), name_)), 0)...
                };
                (void)sequencer_;

                // construct argument-pack to use for actual call
                arguments_type params;
                params.reserve(sizeof...(Ts) + 1);
                for (auto const& arg : keep_alive)
                {
                    params.emplace_back(extract_ref_value(arg, name_));
                }

                return extract_copy_value(
                    value_operand_sync(arg_, std::move(params), name_), name_);
            }

            return extract_copy_value(arg_, name_);
        }

        result_type operator()() const
        {
            if (is_primitive_operand(arg_))
            {
                return extract_copy_value(
                    value_operand_sync(arg_, primitive_argument_type{}, name_),
                    name_);
            }
            return extract_copy_value(arg_, name_);
        }

        template <typename ... Ts>
        result_type operator()(eval_context ctx, Ts &&... ts) const
        {
            if (is_primitive_operand(arg_))
            {
                // user-facing functions need to copy all arguments
                arguments_type keep_alive;
                keep_alive.reserve(sizeof...(Ts));

                int const sequencer_[] = {
                    0, (keep_alive.emplace_back(
                            extract_copy_value(primitive_argument_type(
                                std::forward<Ts>(ts)
                            ), name_)), 0)...
                };
                (void)sequencer_;

                // construct argument-pack to use for actual call
                arguments_type params;
                params.reserve(sizeof...(Ts));
                for (auto const& arg : keep_alive)
                {
                    params.emplace_back(extract_ref_value(arg, name_));
                }

                return extract_copy_value(
                    value_operand_sync(arg_, std::move(params), name_,
                        "<unknown>", std::move(ctx)),
                    name_);
            }

            return extract_copy_value(arg_, name_);
        }

        hpx::future<result_type> eval(
            arguments_type&& args, eval_context ctx) const
        {
            primitive const* p = util::get_if<primitive>(&arg_);
            if (p != nullptr)
            {
                // user-facing functions need to copy all arguments
                arguments_type keep_alive;
                keep_alive.reserve(args.size());

                for (auto && arg : std::move(args))
                {
                    keep_alive.emplace_back(
                        extract_value(std::move(arg), name_));
                }

                return p->eval(std::move(keep_alive), std::move(ctx));
            }
            return hpx::make_ready_future(arg_);
        }

        template <typename ... Ts>
        hpx::future<result_type> eval(Ts &&... ts) const
        {
            primitive const* p = util::get_if<primitive>(&arg_);
            if (p != nullptr)
            {
                // user-facing functions need to copy all arguments
                arguments_type keep_alive;
                keep_alive.reserve(sizeof...(Ts));

                int const sequencer_[] = {
                    0, (keep_alive.emplace_back(
                            extract_value(primitive_argument_type(
                                std::forward<Ts>(ts)
                            ))), 0)...
                };
                (void)sequencer_;

                return p->eval(std::move(keep_alive));
            }
            return hpx::make_ready_future(arg_);
        }

        ////////////////////////////////////////////////////////////////////////
        void set_name(std::string && name)
        {
            name_ = std::move(name);
        }

        void set_named_args(std::shared_ptr<std::string[]> named_args,
            std::size_t num_named_args)
        {
            num_named_args_ = num_named_args;
            named_args_ = std::move(named_args);
        }

        ////////////////////////////////////////////////////////////////////////
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
        std::size_t num_named_args_;
        std::shared_ptr<std::string[]> named_args_;

    private:
        friend class hpx::serialization::access;

        PHYLANX_EXPORT void serialize(hpx::serialization::output_archive& ar,
            unsigned);
        PHYLANX_EXPORT void serialize(hpx::serialization::input_archive& ar,
            unsigned);
    };

    ///////////////////////////////////////////////////////////////////////////
    struct entry_point
    {
        entry_point() = default;        // needed for serialization

        entry_point(std::string const& func_name, std::string const& name)
          : func_name_(func_name)
          , name_(name)
        {
        }

        void add_entry_point(function&& f)
        {
            code_.emplace_back(std::move(f));
        }

        // execute the code represented by this entry point
        function run(eval_context ctx = eval_context{}) const
        {
            auto last = code_.end();
            if (!code_.empty())
            {
                --last;
            }
            for (auto it = code_.begin(); it != code_.end(); ++it)
            {
                if (it == last)
                {
                    auto f = function{it->run(ctx), it->name_};
                    f.set_named_args(it->named_args_, it->num_named_args_);

                    return f;
                }
                it->run(ctx);
            }
            return function{};
        }

        std::list<function> const& functions() const
        {
            return code_;
        }

        topology get_expression_topology() const
        {
            std::set<std::string> functions;
            return code_.back().get_expression_topology(std::move(functions));
        }
        topology get_expression_topology(
            std::set<std::string>&& functions) const
        {
            return code_.back().get_expression_topology(std::move(functions));
        }
        topology get_expression_topology(std::set<std::string>&& functions,
            std::set<std::string>&& resolve_children) const
        {
            return code_.back().get_expression_topology(
                std::move(functions), std::move(resolve_children));
        }

        void serialize(hpx::serialization::input_archive& ar, unsigned);
        void serialize(hpx::serialization::output_archive& ar, unsigned);

        std::string name_;          // the name of this entry point
        std::string func_name_;     // the function name this was compiled from
        std::list<function> code_;  // the functions representing this
    };

    ///////////////////////////////////////////////////////////////////////////
    // this must be a list to ensure stable references
    struct program
    {
        program() = default;

        ///////////////////////////////////////////////////////////////////////
        // simply run the program, return whatever the last snippet has returned
        function run(eval_context ctx = eval_context{}) const
        {
            auto last = entrypoints_.end();
            if (!entrypoints_.empty())
            {
                --last;
            }
            for (auto it = entrypoints_.begin(); it != entrypoints_.end(); ++it)
            {
                if (it == last)
                {
                    return it->run(ctx);
                }
                it->run(ctx);
            }
            return function{};
        }

        ///////////////////////////////////////////////////////////////////////
        function& add_empty(std::string const& name)
        {
            auto it = scratchpad_.find(name);
            if (it == scratchpad_.end())
            {
                it = scratchpad_
                         .insert(std::make_pair(name, std::list<function>{}))
                         .first;
            }
            it->second.emplace_back(function{});
            return it->second.back();
        }

        compiler::entry_point const& add_entry_point(compiler::entry_point&& ep)
        {
            entrypoints_.emplace_back(std::move(ep));
            return entrypoints_.back();
        }

        compiler::entry_point entry_point() const
        {
            return entrypoints_.back();
        }

        bool has_entry_points() const
        {
            return !entrypoints_.back().functions().empty();
        }

        ///////////////////////////////////////////////////////////////////////
        std::map<std::string, std::list<function>> const& scratchpad() const
        {
            return scratchpad_;
        }

        std::list<compiler::entry_point> const& entry_points() const
        {
            return entrypoints_;
        }

        ///////////////////////////////////////////////////////////////////////
        topology get_expression_topology() const
        {
            HPX_ASSERT(has_entry_points());
            std::set<std::string> functions;
            return entrypoints_.back().get_expression_topology(
                std::move(functions));
        }
        topology get_expression_topology(
            std::set<std::string>&& functions) const
        {
            HPX_ASSERT(has_entry_points());
            return entrypoints_.back().get_expression_topology(
                std::move(functions));
        }
        topology get_expression_topology(std::set<std::string>&& functions,
            std::set<std::string>&& resolve_children) const
        {
            HPX_ASSERT(has_entry_points());
            return entrypoints_.back().get_expression_topology(
                std::move(functions), std::move(resolve_children));
        }


    private:
        friend class hpx::serialization::access;

        PHYLANX_EXPORT void serialize(hpx::serialization::output_archive& ar,
            unsigned);
        PHYLANX_EXPORT void serialize(hpx::serialization::input_archive& ar,
            unsigned);

        std::map<std::string, std::list<function>> scratchpad_;
        std::list<compiler::entry_point> entrypoints_;
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
