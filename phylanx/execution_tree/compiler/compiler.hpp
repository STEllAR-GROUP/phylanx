// Copyright (c) 2017-2018 Hartmut Kaiser
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// phylanxinspect:noinclude:HPX_ASSERT

#if !defined(PHYLANX_EXECUTION_TREE_COMPILER_HPP)
#define PHYLANX_EXECUTION_TREE_COMPILER_HPP

#include <phylanx/config.hpp>
#include <phylanx/execution_tree/compiler/actors.hpp>
#include <phylanx/execution_tree/compiler/primitive_name.hpp>
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
        std::string, ast::expression, factory_function_type>;
    using expression_pattern_list =
        std::multimap<std::string, expression_pattern>;

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
            primitive_name_parts name_parts,
            std::string const& codename = "<unknown>") const
        {
            return derived().compose(
                std::move(elements), std::move(name_parts), codename);
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

        function compose(std::list<function>&& args,
            primitive_name_parts&& name_parts,
            std::string const& codename) const
        {
            arguments_type fargs;
            fargs.reserve(args.size());

            for (auto const& arg : args)
            {
                fargs.push_back(arg.arg_);
            }

            std::string full_name = compose_primitive_name(name_parts);

            auto f = function{
                primitive_argument_type{
                    (*f_)(this->locality_, std::move(fargs), full_name,
                        codename)
                }, full_name };

            // A built-in function can be directly referenced by its name, in
            // which case we wrap it into a variable object to avoid premature
            // evaluation.
            if (name_parts.primitive != "call-function" && args.empty())
            {
                HPX_ASSERT(name_parts.instance.empty());
                name_parts.instance = std::move(name_parts.primitive);
                name_parts.primitive = "variable";

                full_name = compose_primitive_name(name_parts);

                f = function{primitive_argument_type{
                        create_primitive_component(
                            this->locality_, name_parts.primitive,
                            std::move(f.arg_), full_name, codename)
                    }, full_name};
            }

            return f;
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

    // compose a define-variable object
    struct define_operation : compiled_actor<define_operation>
    {
        explicit define_operation(
                hpx::id_type const& locality = hpx::find_here())
          : compiled_actor<define_operation>(locality)
        {}

        function operator()(argument_type && arg,
            primitive_name_parts&& name_parts,
            std::string const& codename = "<unknown>") const
        {
            if (name_parts.instance.empty())
            {
                name_parts.instance = std::move(name_parts.primitive);
            }
            name_parts.primitive = "define-variable";

            std::string full_name = compose_primitive_name(name_parts);
            return function{primitive_argument_type{
                create_primitive_component(this->locality_,
                    name_parts.primitive, std::move(arg), full_name, codename)
                }, full_name};
        }
    };

    // compose a primitive::variable
    struct primitive_function
    {
        explicit primitive_function(
                hpx::id_type const& locality = hpx::find_here())
          : locality_(locality)
        {}

        function operator()(primitive_name_parts name_parts,
            std::string const& codename = "<unknown>") const
        {
            std::string name = compose_primitive_name(name_parts);

            static std::string type("define-function");

            return function{
                primitive_argument_type{
                    create_primitive_component(locality_, type,
                        std::vector<primitive_argument_type>{}, name,
                        codename)},
                name};
        }

    private:
        hpx::id_type locality_;
    };

    // compose an argument selector
    struct access_argument : compiled_actor<access_argument>
    {
        access_argument(hpx::id_type const& locality = hpx::find_here())
          : compiled_actor<access_argument>(locality)
        {
        }

        function operator()(std::size_t n, primitive_name_parts&& name_parts,
            std::string const& codename = "<unknown>") const
        {
            static std::size_t sequence_number = 0;

            if (name_parts.instance.empty())
            {
                name_parts.instance = std::move(name_parts.primitive);
            }
            name_parts.primitive = "access-argument";
            name_parts.sequence_number = sequence_number++;

            std::string const full_name =
                compose_primitive_name(name_parts);

            return function{
                primitive_argument_type{create_primitive_component(
                    this->locality_, name_parts.primitive,
                    primitive_argument_type{std::int64_t(n)}, full_name,
                    codename)},
                full_name};
        }
    };

    // compose an object that accesses an existing variable or function
    struct access_target : compiled_actor<access_target>
    {
        // we must hold f by reference
        std::reference_wrapper<function const> f_;
        std::string target_name_;

        explicit access_target(function const& f,
                std::string && target_name,
                hpx::id_type const& locality = hpx::find_here())
          : compiled_actor<access_target>(locality)
          , f_(f)
          , target_name_(std::move(target_name))
        {}

        function compose(std::list<function>&& elements,
            primitive_name_parts&& name_parts,
            std::string const& codename) const
        {
            if (name_parts.instance.empty())
            {
                name_parts.instance = name_parts.primitive;
            }
            name_parts.primitive = target_name_;

            std::string full_name = compose_primitive_name(name_parts);

            return function{
                primitive_argument_type{
                    create_primitive_component(
                        this->locality_, name_parts.primitive, f_.get().arg_,
                        full_name, codename)
                },
                full_name};
        }
    };

    struct access_variable : access_target
    {
        explicit access_variable(function const& f,
                hpx::id_type const& locality = hpx::find_here())
          : access_target(f, "access-variable", locality)
        {}
    };

    // compose a call-function object
    struct call_function : compiled_actor<call_function>
    {
        // we must hold f by reference because functions can be recursive
        std::reference_wrapper<function const> f_;

        explicit call_function(function const& f,
                hpx::id_type const& locality = hpx::find_here())
          : compiled_actor<call_function>(locality)
          , f_(f)
        {}

        function compose(std::list<function>&& elements,
            primitive_name_parts&& name_parts,
            std::string const& codename) const
        {
            HPX_ASSERT(name_parts.instance.empty());
            name_parts.instance = std::move(name_parts.primitive);
            name_parts.primitive = "call-function";

            std::vector<primitive_argument_type> fargs;
            fargs.reserve(elements.size() + 1);

            fargs.push_back(f_.get().arg_);
            for (auto && elem : elements)
            {
                fargs.push_back(elem.arg_);
            }

            std::string full_name = compose_primitive_name(name_parts);

            auto cf = create_primitive_component(this->locality_,
                name_parts.primitive, std::move(fargs), full_name, codename);

            return function{primitive_argument_type{std::move(cf)}, full_name};
        }
    };

    ///////////////////////////////////////////////////////////////////////////
    using compiled_function = hpx::util::function_nonser<function(
            std::list<function> const&, primitive_name_parts, std::string const&
        )>;

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
            auto existing = definitions_.find(name);
            if (existing != definitions_.end())
            {
                definitions_.erase(existing);
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
    PHYLANX_EXPORT function define_variable(std::string const& codename,
        primitive_name_parts name_parts, function_list& snippets, environment& env,
        primitive_argument_type body, hpx::id_type const& default_locality);
}}}

#endif
