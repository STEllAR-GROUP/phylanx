//  Copyright (c) 2017 Hartmut Kaiser
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(PHYLANX_EXECUTION_TREE_ACTORS_HPP)
#define PHYLANX_EXECUTION_TREE_ACTORS_HPP

#include <phylanx/config.hpp>
#include <phylanx/execution_tree/primitives/base_primitive.hpp>

#include <hpx/include/util.hpp>
#include <hpx/runtime/launch_policy.hpp>

#include <array>
#include <cstddef>
#include <functional>
#include <list>
#include <utility>
#include <vector>

namespace phylanx { namespace execution_tree { namespace compiler
{
    ///////////////////////////////////////////////////////////////////////////
    using result_type = primitive_argument_type;
    using argument_type = primitive_argument_type;
    using arguments_type = std::vector<primitive_argument_type>;

    using stored_function =
        hpx::util::function_nonser<result_type(arguments_type&&)>;

    ///////////////////////////////////////////////////////////////////////////
    template <typename Derived>
    struct actor
    {
        // direct execution
        result_type operator()(arguments_type args) const
        {
            return derived().call(std::move(args));
        }

        result_type operator()() const
        {
            return derived().call(arguments_type{});
        }

        template <typename ... Ts>
        result_type operator()(Ts &&... ts) const
        {
            arguments_type elements = {std::forward<Ts>(ts)...};
            return derived().call(std::move(elements));
        }

    private:
        Derived const& derived() const
        {
            return *static_cast<Derived const*>(this);
        }
    };

    ///////////////////////////////////////////////////////////////////////////
    struct function : actor<function>
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

        result_type call(arguments_type && args) const
        {
            return value_operand_sync(arg_, std::move(args));
        }
        hpx::future<result_type> eval(arguments_type && args) const
        {
            return value_operand(arg_, std::move(args));
        }

        void set_name(std::string && name)
        {
#if defined(_DEBUG)
            name_ = std::move(name);
#endif
        }

        primitive_argument_type arg_;
#if defined(_DEBUG)
        std::string name_;
#endif
    };

    // defer the evaluation of a given function
//     struct c : actor<function>
//     {
//         function() = default;
//
//         function(stored_function const& f)
//           : f_(f)
//         {
//             HPX_ASSERT(!f_.empty());
//         }
//         function(stored_function && f)
//           : f_(std::move(f))
//         {
//             HPX_ASSERT(!f_.empty());
//         }
//
//         bool empty() const
//         {
//             return f_.empty();
//         }
//
//         result_type call(arguments_type && args) const
//         {
//             return f_(std::move(args));
//         }
//
//         stored_function f_;
//     };


    // this must be a list to ensure stable references
    using function_list = std::list<function>;

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

    ///////////////////////////////////////////////////////////////////////////
    // lambda
    struct lambda : actor<lambda>
    {
        function_list elements_;

        // we must hold f by reference because functions can be recursive
        std::reference_wrapper<function const> f_;

        lambda(function const& f, function_list const& elements)
          : elements_(elements)
          , f_(f)
        {}

        result_type call(arguments_type && args) const
        {
            if (!elements_.empty())
            {
                std::vector<result_type> fargs;
                fargs.reserve(elements_.size());

                for (auto const& element : elements_)
                {
                    fargs.push_back(element(std::move(args)));
                }

                return f_.get()(std::move(fargs));
            }

            return f_.get()();
        }
    };
}}}

#endif
