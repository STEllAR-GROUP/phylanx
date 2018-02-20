//  Copyright (c) 2017-2018 Hartmut Kaiser
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// phylanxinspect:noinclude:HPX_ASSERT

#if !defined(PHYLANX_EXECUTION_TREE_COMPILER_HPP)
#define PHYLANX_EXECUTION_TREE_COMPILER_HPP

#include <phylanx/config.hpp>
#include <phylanx/execution_tree/compiler/actors.hpp>
#include <phylanx/execution_tree/primitives/base_primitive.hpp>
#include <phylanx/execution_tree/primitives/primitive_component_base.hpp>

#include <hpx/include/util.hpp>
#include <hpx/include/naming.hpp>
#include <hpx/throw_exception.hpp>

#include <cstddef>
#include <functional>
#include <list>
#include <map>
#include <string>
#include <utility>
#include <vector>

namespace phylanx { namespace execution_tree { namespace compiler
{
    ///////////////////////////////////////////////////////////////////////////
    class environment;

    ///////////////////////////////////////////////////////////////////////////
    using expression_pattern = hpx::util::tuple<
        std::string, std::string,
        ast::expression, factory_function_type>;
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
            std::list<function> elements;
            int const sequencer_[] = {
                0, (elements.emplace_back(std::forward<Ts>(ts)), 0)...
            };
            (void)sequencer_;
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
                primitive_argument_type{
                    (*f_)(this->locality_, std::move(fargs), name)
                }, name};
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
            static std::string type("define-variable");
            return function{
                primitive_argument_type{create_primitive_component_with_name(
                    locality_, type, std::move(arg), name)},
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
            return function{ast::nil{}, "always-nil"};
        }
        function operator()(std::string && name) const
        {
            return function{ast::nil{}, "always-nil$ " + name};
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
            static std::string type("define-function");

            return function{
                primitive_argument_type{
                    create_primitive_component_with_name(locality_, type,
                        std::vector<primitive_argument_type>{}, name)},
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
            static std::size_t sequence_number = 0;
            static std::string type("access-argument");
            std::string full_name = type + "$" +
                std::to_string(sequence_number++) + "$" + name;

            return function{
                primitive_argument_type{create_primitive_component(locality_,
                    type, primitive_argument_type{std::int64_t(n)}, full_name)},
                full_name};
        }

        hpx::id_type locality_;
    };

    // compose an external variable
    struct external_variable : compiled_actor<external_variable>
    {
        // we must hold f by reference
        std::reference_wrapper<function const> f_;

        explicit external_variable(function const& f,
                hpx::id_type const& locality = hpx::find_here())
          : compiled_actor<external_variable>(locality)
          , f_(f)
        {}

        function compose(std::list<function> && elements,
            std::string const& name) const
        {
            static std::size_t sequence_number = 0;
            static std::string type("access-variable");
            std::string full_name = type + "$" +
                std::to_string(sequence_number++) + "$" + name;

            return function{
                primitive_argument_type{create_primitive_component_with_name(
                    this->locality_, type, f_.get().arg_, full_name)},
                full_name};
        }
    };

    // compose an external function
    struct external_function : compiled_actor<external_function>
    {
        // we must hold f by reference because functions can be recursive
        std::reference_wrapper<function const> f_;
        std::size_t sequence_number_;

        explicit external_function(function const& f,
                std::size_t sequence_number = std::size_t(-1),
                hpx::id_type const& locality = hpx::find_here())
          : compiled_actor<external_function>(locality)
          , sequence_number_(sequence_number)
          , f_(f)
        {}

        function compose(std::list<function> && elements,
            std::string const& name) const
        {
            arguments_type fargs;
            fargs.reserve(elements.size() + 1);

            fargs.push_back(f_.get().arg_);
            for (auto const& arg : elements)
            {
                fargs.push_back(arg.arg_);
            }

            static std::string type("call-function");
            std::string full_name = type + "$" +
                std::to_string(sequence_number_) + "$" + name;

            return function{
                primitive_argument_type{create_primitive_component_with_name(
                    this->locality_, type, std::move(fargs), full_name)},
                full_name};
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
        environment(environment* outer = nullptr, std::size_t base_arg_num = 0)
          : outer_(outer)
          , base_arg_num_(outer != nullptr ?
                    outer->base_arg_num_ + base_arg_num :
                    base_arg_num)
        {}

        template <typename F>
        compiled_function* define(std::string name, F && f)
        {
            if (definitions_.find(name) != definitions_.end())
            {
                HPX_THROW_EXCEPTION(hpx::bad_parameter,
                    "phylanx::execution_tree::environment::define",
                    "given name was already defined: " + name);
            }

            auto result = definitions_.emplace(value_type(
                std::move(name), compiled_function(std::forward<F>(f))));

            if (!result.second)
            {
                HPX_THROW_EXCEPTION(hpx::bad_parameter,
                    "phylanx::execution_tree::environment::define",
                    "couldn't insert name into symbol table");
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
            if (outer_ != nullptr)
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

        std::size_t base_arg_num() const
        {
            return base_arg_num_;
        }

    private:
        environment* outer_;
        std::map<std::string, compiled_function> definitions_;
        std::size_t base_arg_num_;
    };

    ///////////////////////////////////////////////////////////////////////////
    PHYLANX_EXPORT environment default_environment(
        pattern_list const& patterns_list,
        hpx::id_type const& default_locality);

    ///////////////////////////////////////////////////////////////////////////
    /// Compile the given AST instance and generate an expression tree
    /// corresponding to its structure. Return a function object that - when
    /// executed - will evaluate the generated execution tree.
    PHYLANX_EXPORT function compile(std::string const& name,
        ast::expression const& expr, function_list& snippets, environment& env,
        expression_pattern_list const& patterns,
        hpx::id_type const& default_locality);

    /// Add the given variable to the compilation environment
    PHYLANX_EXPORT function define_variable(std::string name,
        function_list& snippets, environment& env, primitive_argument_type body,
        hpx::id_type const& default_locality);
}}}

#endif
