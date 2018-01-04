//  Copyright (c) 2017 Hartmut Kaiser
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(PHYLANX_EXECUTION_TREE_COMPILER_HPP)
#define PHYLANX_EXECUTION_TREE_COMPILER_HPP

#include <phylanx/config.hpp>
#include <phylanx/execution_tree/compiler/actors.hpp>
#include <phylanx/execution_tree/primitives/access_argument.hpp>
#include <phylanx/execution_tree/primitives/base_primitive.hpp>
#include <phylanx/execution_tree/primitives/define_variable.hpp>
#include <phylanx/execution_tree/primitives/define_function.hpp>
#include <phylanx/execution_tree/primitives/variable.hpp>
#include <phylanx/execution_tree/primitives/wrapped_function.hpp>

#include <hpx/include/components.hpp>
#include <hpx/include/util.hpp>
#include <hpx/runtime/find_here.hpp>

#include <cstddef>
#include <functional>
#include <map>
#include <vector>
#include <string>
#include <utility>

namespace phylanx { namespace execution_tree { namespace compiler
{
    ///////////////////////////////////////////////////////////////////////////
    class environment;

    ///////////////////////////////////////////////////////////////////////////
    using expression_pattern =
        hpx::util::tuple<
            std::string, std::string, ast::expression, factory_function_type
        >;
    using expression_pattern_list = std::vector<expression_pattern>;

    PHYLANX_EXPORT expression_pattern_list generate_patterns(
        pattern_list const& patterns_list);

    /// Create default compilation environment based on the given list of
    /// patterns and using the given default locality.
    PHYLANX_EXPORT environment default_environment(
        pattern_list const& patterns_list,
        hpx::id_type const& default_locality = hpx::find_here());

    /// Create default compilation environment using the given default locality.
    PHYLANX_EXPORT environment default_environment(
        hpx::id_type const& default_locality = hpx::find_here());

    ///////////////////////////////////////////////////////////////////////////
    // compiled functions
    template <typename Derived>
    struct compiled_actor
    {
    private:
        Derived const& derived() const
        {
            return *static_cast<Derived const*>(this);
        }

    public:
        explicit compiled_actor(hpx::id_type const& locality)
          : locality_(locality)
        {}

        function operator()(std::list<function> elements,
            std::string const& name) const
        {
            return derived().compose(std::move(elements), name);
        }

        template <typename ... Ts>
        function operator()(Ts &&... ts) const
        {
            std::list<function> elements = {std::forward<Ts>(ts)...};
            return derived().compose(std::move(elements));
        }

    protected:
        hpx::id_type locality_;
    };

    // compose a built-in function
    struct builtin_function : compiled_actor<builtin_function>
    {
        explicit builtin_function(factory_function_type f,
                hpx::id_type const& locality = hpx::find_here())
          : compiled_actor<builtin_function>(locality)
          , f_(f)
        {}

        function compose(std::list<function> && args,
            std::string const& name) const
        {
            arguments_type fargs;
            fargs.reserve(args.size());

            for (auto const& arg : args)
            {
                fargs.push_back(arg.arg_);
            }

            return function{
                (*f_)(this->locality_, std::move(fargs), name),
                name};
        }

    private:
        factory_function_type f_;
    };

    // compose a literal value
    struct primitive_literal_value
    {
        primitive_literal_value() = default;

        function operator()(argument_type && arg) const
        {
            return function{std::move(arg), "primitive_literal_value"};
        }
        function operator()(arguments_type && args) const
        {
            HPX_ASSERT(!args.empty());

            return function{std::move(args[0]), "primitive_literal_value"};
        }
    };

    static const primitive_literal_value literal_value = {};

    // compose a primitive::variable
    struct primitive_variable
    {
        explicit primitive_variable(
                hpx::id_type const& locality = hpx::find_here())
          : locality_(locality)
        {}

        function operator()(argument_type && arg, std::string const& name) const
        {
            return function{
                    primitive(
                        hpx::new_<primitives::define_variable>(
                            locality_, std::move(arg), name),
                        name),
                    name};
        }

    private:
        hpx::id_type locality_;
    };

    // compose a  nil{} value
    struct always_nil
    {
        always_nil() = default;

        function operator()() const
        {
            return function{ast::nil{}, "always_nil"};
        }
        function operator()(std::string && name) const
        {
            return function{ast::nil{}, "always_nil: " + name};
        }
    };

    // compose a primitive::variable
    struct primitive_function
    {
        explicit primitive_function(
                hpx::id_type const& locality = hpx::find_here())
          : locality_(locality)
        {}

        function operator()(std::string const& name) const
        {
            return function{
                primitive(
                    hpx::new_<primitives::define_function>(locality_, name),
                    name),
                name};
        }

    private:
        hpx::id_type locality_;
    };

    // compose an argument selector
    struct argument
    {
        argument(hpx::id_type const& locality = hpx::find_here())
          : locality_(locality)
        {
        }

        function operator()(std::size_t n, std::string const& name) const
        {
            std::string full_name = "argument:" + name;
            return function{
                    primitive(
                        hpx::new_<primitives::access_argument>(locality_, n),
                        full_name),
                    full_name
                };
        }

        hpx::id_type locality_;
    };

    // compose an external function
    struct external_function : compiled_actor<external_function>
    {
        // we must hold f by reference because functions can be recursive
        std::reference_wrapper<function const> f_;

        explicit external_function(function const& f,
                hpx::id_type const& locality = hpx::find_here())
          : compiled_actor<external_function>(locality)
          , f_(f)
        {}

        function compose(std::list<function> && elements,
            std::string const& name) const
        {
            if (elements.empty())
            {
                return function{f_.get().arg_, name};
            }

            arguments_type fargs;
            fargs.reserve(elements.size());

            for (auto const& arg : elements)
            {
                fargs.push_back(arg.arg_);
            }

            return function{
                    hpx::new_<primitives::wrapped_function>(
                        this->locality_, f_.get().arg_, std::move(fargs), name),
                    name
                };
        }
    };

    ///////////////////////////////////////////////////////////////////////////
    typedef hpx::util::function_nonser<
            function(std::list<function> const&, std::string const&)
        > compiled_function;

    class environment
    {
        using iterator = std::map<std::string, compiled_function>::iterator;
        using value_type = std::map<std::string, compiled_function>::value_type;

    public:
        environment(environment* outer = nullptr)
          : outer_(outer)
        {}

        template <typename F>
        compiled_function* define(std::string const& name, F && f)
        {
            if (definitions_.find(name) != definitions_.end())
            {
                HPX_THROW_EXCEPTION(hpx::bad_parameter,
                    "phylanx::execution_tree::environment::define",
                    "given name was already defined: " + name);
            }

            auto result = definitions_.emplace(
                value_type(name, compiled_function(std::forward<F>(f))));

            if (!result.second)
            {
                HPX_THROW_EXCEPTION(hpx::bad_parameter,
                    "phylanx::execution_tree::environment::define",
                    "couldn't insert name into symbol table: " + name);
            }

            return &result.first->second;
        }

        compiled_function* find(std::string const& name)
        {
            iterator it = definitions_.find(name);
            if (it != definitions_.end())
            {
                return &it->second;
            }
            else if (outer_ != nullptr)
            {
                return outer_->find(name);
            }
            return nullptr;
        }

        environment* parent() const { return outer_; }

        std::size_t size() const
        {
            std::size_t count = definitions_.size();
            if (outer_ != nullptr)
                count += outer_->size();
            return count;
        }

    private:
        environment* outer_;
        std::map<std::string, compiled_function> definitions_;
    };

    ///////////////////////////////////////////////////////////////////////////
    PHYLANX_EXPORT environment default_environment(
        pattern_list const& patterns_list,
        hpx::id_type const& default_locality);

    /// Compile the given AST instance and generate an expression tree
    /// corresponding to its structure. Return a function object that - when
    /// executed - will evaluate the generated execution tree.
    PHYLANX_EXPORT function compile(ast::expression const& expr,
        function_list& snippets, environment& env,
        expression_pattern_list const& patterns,
        hpx::id_type const& default_locality);

    /// Compile the given list of AST instances and generate an expression tree
    /// corresponding to its structure. Return a function object that - when
    /// executed - will evaluate the generated execution tree.
    PHYLANX_EXPORT function compile(std::vector<ast::expression> const& exprs,
        function_list& snippets, environment& env,
        expression_pattern_list const& patterns,
        hpx::id_type const& default_locality);
}}}

#endif
