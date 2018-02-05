//  Copyright (c) 2017-2018 Hartmut Kaiser
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// phylanxinspect:noinclude:HPX_ASSERT

#if !defined(PHYLANX_EXECUTION_TREE_COMPILER_HPP)
#define PHYLANX_EXECUTION_TREE_COMPILER_HPP

#include <phylanx/config.hpp>
#include <phylanx/execution_tree/compiler/actors.hpp>
#include <phylanx/execution_tree/primitives/access_argument.hpp>
#include <phylanx/execution_tree/primitives/base_primitive.hpp>
#include <phylanx/execution_tree/primitives/define_function.hpp>
#include <phylanx/execution_tree/primitives/define_variable.hpp>
#include <phylanx/execution_tree/primitives/variable.hpp>
#include <phylanx/execution_tree/primitives/wrapped_function.hpp>
#include <phylanx/execution_tree/primitives/wrapped_variable.hpp>

#include <hpx/include/components.hpp>
#include <hpx/include/util.hpp>
#include <hpx/runtime/find_here.hpp>
#include <hpx/util/assert.hpp>

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
            return function{ast::nil{}, "always-nil"};
        }
        function operator()(std::string && name) const
        {
            return function{ast::nil{}, "always-nil# " + name};
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
        argument(std::size_t sequence_number,
                hpx::id_type const& locality = hpx::find_here())
          : sequence_number_(sequence_number),
            locality_(locality)
        {
        }

        function operator()(std::size_t n, std::string const& name) const
        {
            std::string full_name = "access-argument#" +
                std::to_string(sequence_number_) + "#" + name;

            return function{
                    primitive(
                        hpx::new_<primitives::access_argument>(locality_, n),
                        full_name),
                    full_name
                };
        }

        std::size_t sequence_number_;
        hpx::id_type locality_;
    };

    // compose an external variable
    struct external_variable : compiled_actor<external_variable>
    {
        // we must hold f by reference
        std::reference_wrapper<function const> f_;
        std::size_t sequence_number_;

        explicit external_variable(function const& f,
                std::size_t sequence_number = std::size_t(-1),
                hpx::id_type const& locality = hpx::find_here())
          : compiled_actor<external_variable>(locality)
          ,sequence_number_(sequence_number)
          , f_(f)
        {}

        function compose(std::list<function> && elements,
            std::string const& name) const
        {
            std::string full_name = "access-variable#" +
                std::to_string(sequence_number_) + "#" + name;

            return function{
                    primitive(
                        hpx::new_<primitives::wrapped_variable>(
                            this->locality_, f_.get().arg_,
                            full_name),
                        full_name),
                    full_name
                };
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
            fargs.reserve(elements.size());

            for (auto const& arg : elements)
            {
                fargs.push_back(arg.arg_);
            }

            std::string full_name = "function#" +
                std::to_string(sequence_number_) + "#" + name;

            return function{
                    primitive(
                        hpx::new_<primitives::wrapped_function>(
                            this->locality_, f_.get().arg_, std::move(fargs),
                            full_name),
                        full_name),
                    full_name
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
        environment(environment* outer = nullptr, std::size_t base_arg_num = 0)
          : outer_(outer)
          , base_arg_num_(outer != nullptr ?
                    outer->base_arg_num_ + base_arg_num :
                    base_arg_num)
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
