//  Copyright (c) 2017-2018 Hartmut Kaiser
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// phylanxinspect:noinclude:HPX_ASSERT

#include <phylanx/config.hpp>
#include <phylanx/ast/detail/is_function_call.hpp>
#include <phylanx/ast/detail/is_identifier.hpp>
#include <phylanx/ast/detail/is_literal_value.hpp>
#include <phylanx/ast/detail/is_placeholder.hpp>
#include <phylanx/ast/detail/is_placeholder_ellipses.hpp>
#include <phylanx/ast/detail/tagged_id.hpp>
#include <phylanx/ast/generate_ast.hpp>
#include <phylanx/ast/match_ast.hpp>
#include <phylanx/ast/node.hpp>
#include <phylanx/execution_tree/compile.hpp>
#include <phylanx/execution_tree/compiler/actors.hpp>
#include <phylanx/execution_tree/compiler/compiler.hpp>
#include <phylanx/execution_tree/primitives/base_primitive.hpp>
#include <phylanx/ir/node_data.hpp>

#include <hpx/include/naming.hpp>
#include <hpx/include/util.hpp>

#include <cstddef>
#include <cstdint>
#include <list>
#include <map>
#include <string>
#include <vector>
#include <utility>

namespace phylanx { namespace execution_tree { namespace compiler
{
    ///////////////////////////////////////////////////////////////////////
    environment default_environment(pattern_list const& patterns_list,
        hpx::id_type const& default_locality)
    {
        environment result;

        for (auto const& patterns : patterns_list)
        {
            if (!hpx::util::get<1>(patterns).empty())
            {
            result.define(hpx::util::get<0>(patterns),
                builtin_function(
                    hpx::util::get<2>(patterns), default_locality));
        }
        }

        return result;
    }

    environment default_environment(hpx::id_type const& default_locality)
    {
        return default_environment(get_all_known_patterns(), default_locality);
    }

    ///////////////////////////////////////////////////////////////////////////
    expression_pattern_list generate_patterns(pattern_list const& patterns_list)
    {
        expression_pattern_list result;
        result.reserve(patterns_list.size());

        for (auto const& patterns : patterns_list)
        {
            for (auto const& pattern : hpx::util::get<1>(patterns))
            {
                auto exprs = ast::generate_ast(pattern);
                HPX_ASSERT(exprs.size() == 1);

                result.push_back(
                    hpx::util::make_tuple(
                        hpx::util::get<0>(patterns), pattern,
                        exprs[0], hpx::util::get<2>(patterns)));
            }
        }

        return result;
    }

    ///////////////////////////////////////////////////////////////////////////
    template <typename Iterator>
    ast::expression extract_name(std::pair<Iterator, Iterator> const& p)
    {
        if (std::distance(p.first, p.second) < 2)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "phylanx::execution_tree::detail::extract_name",
                "the define() operation requires at least 2 arguments");
        }

        if (!ast::detail::is_identifier(p.first->second))
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "phylanx::execution_tree::detail::extract_name",
                "the define() operation requires that the name of the "
                    "function to define is represented as a variable name "
                    "(not an expression)");
        }

        return p.first->second;
    }

    template <typename Iterator>
    std::vector<ast::expression> extract_arguments(
        std::pair<Iterator, Iterator> const& p)
    {
        std::ptrdiff_t size = std::distance(p.first, p.second);
        if (size < 2)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "phylanx::execution_tree::detail::extract_arguments",
                "the define() operation requires at least 2 arguments");
        }

        std::vector<ast::expression> args;
        args.reserve(size);

        auto first = p.first; ++first;
        auto last = p.second; --last;

        std::size_t count = 0;
        for (auto it = first; it != last; ++it, ++count)
        {
            if (count != 0 && count != size-1 &&
                !ast::detail::is_identifier(it->second))
            {
                HPX_THROW_EXCEPTION(hpx::bad_parameter,
                    "phylanx::execution_tree::detail::extract_arguments",
                    "the define() operation requires that all arguments "
                        "are represented as variable names (not "
                        "expressions)");
            }
            args.push_back(it->second);
        }

        return args;
    }

    template <typename Iterator>
    ast::expression extract_body(std::pair<Iterator, Iterator> const& p)
    {
        if (std::distance(p.first, p.second) < 2)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "phylanx::execution_tree::detail::extract_body",
                "the define() operation requires at least 2 arguments");
        }

        auto last = p.second; --last;
        return last->second;
    }

    ///////////////////////////////////////////////////////////////////////////
    struct compiler
    {
        compiler(function_list& snippets, environment& env,
                expression_pattern_list const& patterns,
                hpx::id_type const& default_locality)
          : env_(env)
          , snippets_(snippets)
          , patterns_(patterns)
          , default_locality_(default_locality)
        {}

    private:
        std::string annotation(ast::tagged const& id)
        {
            // Note: the compile-id needs to be adjusted to be zero-based.
            std::string result = "/" +
                std::to_string(snippets_.compile_id_ - 1) + "$" +
                std::to_string(id.id);

            if (id.col != -1)
            {
                result += '$' + std::to_string(id.col);
            }

            return result;
        }

        function handle_lambda(
            std::vector<ast::expression> const& args,
            ast::expression const& body) const
        {
            std::size_t base_arg_num = env_.base_arg_num();
            environment env(&env_, args.size());
            for (std::size_t i = 0; i != args.size(); ++i)
            {
                // get sequence number of this component
                argument arg(default_locality_);

                HPX_ASSERT(ast::detail::is_identifier(args[i]));
                env.define(ast::detail::identifier_name(args[i]),
                    hpx::util::bind(
                        arg, i + base_arg_num, hpx::util::placeholders::_2));
            }
            return compile(body, snippets_, env, patterns_, default_locality_);
        }

        function handle_define(
            std::multimap<std::string, ast::expression>& placeholders,
            expression_pattern const& pattern)
        {
            // we know that 'define()' uses '__1' to match arguments
            using iterator =
                typename std::multimap<std::string, ast::expression>::iterator;
            std::pair<iterator, iterator> p = placeholders.equal_range("__1");

            // extract expressions representing the newly defined function
            // and store new function description for later use
            snippets_.snippets_.emplace_back(function{});
            function& f = snippets_.snippets_.back();

            ast::expression name_expr = extract_name(p);
            std::string name = ast::detail::identifier_name(name_expr);

            // get global name of the component created
            std::string full_name = name;
            ast::tagged id = ast::detail::tagged_id(name_expr);
            if (id.id >= 0)
            {
                full_name += annotation(id);
            }

            auto args = extract_arguments(p);
            auto body = extract_body(p);
            if (args.empty())
            {
                // get sequence number of this component
                env_.define(
                    std::move(name), external_variable(f, default_locality_));

                static std::string define_variable("define-variable");
                std::size_t sequence_number =
                    snippets_.sequence_numbers_[define_variable]++;

                // define variable
                environment env(&env_);
                function bf = compile(body, snippets_, env, patterns_,
                    default_locality_);

                f = primitive_variable{default_locality_}(
                        std::move(bf.arg_), define_variable + "$" +
                            std::to_string(sequence_number) + "$" + full_name);
                return f;
            }

            // NOTE: Check the consistency of names: "function" vs "call-function"
            // get sequence number of this component
            static std::string function_("call-function");
            std::size_t sequence_number =
                snippets_.sequence_numbers_[function_]++;

            // two-step initialization of the wrapped_function to support
            // recursion
            env_.define(std::move(name),
                external_function(f, sequence_number, default_locality_));

            static std::string define_function_("define-function");
            sequence_number =
                snippets_.sequence_numbers_[define_function_]++;

            // create define_function helper object
            std::string define_function_name =
                std::to_string(sequence_number) + "$" + full_name;

            f = primitive_function{default_locality_}(
                define_function_ + "$" + define_function_name);

            // set the body for the compiled function
            primitive_operand(f.arg_).set_body(
                hpx::launch::sync, std::move(handle_lambda(args, body).arg_));

            // define-function shouldn't return a function that evaluates
            // to itself, let it return nil{} instead
            return always_nil{}(std::move(define_function_name));
        }

        function handle_variable_reference(std::string name,
            ast::expression const& expr)
        {
            if (compiled_function* cf = env_.find(name))
            {
                ast::tagged id = ast::detail::tagged_id(expr);
                if (id.id >= 0)
                {
                    name += annotation(id);
                }
                return (*cf)(std::list<function>{}, name);
            }

            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "phylanx::execution_tree::compiler::handle_variable",
                "couldn't find given name in symbol table: " + name);
        }

        function handle_function_call(std::string name,
            ast::expression const& expr)
        {
            if (compiled_function* cf = env_.find(name))
            {
                std::vector<ast::expression> argexprs =
                    ast::detail::function_arguments(expr);

                std::list<function> args;
                for (auto const& argexpr : argexprs)
                {
                    environment env(&env_);
                    args.push_back(compile(argexpr, snippets_, env, patterns_,
                        default_locality_));
                }

                ast::tagged id = ast::detail::tagged_id(expr);
                if (id.id >= 0)
                {
                    name += annotation(id);
                }
                return (*cf)(std::move(args), name);
            }

            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "phylanx::execution_tree::compiler::handle_function_call",
                "couldn't find given name in symbol table: " + name);
        }

        function handle_placeholders(
            std::multimap<std::string, ast::expression>& placeholders,
            std::string const& name, std::string const& global_name)
        {
            if (compiled_function* cf = env_.find(name))
            {
                std::list<function> args;
                environment env(&env_);

                for (auto const& placeholder : placeholders)
                {
                    args.push_back(compile(placeholder.second,
                        snippets_, env, patterns_, default_locality_));
                }

                // create primitive with given arguments
                return (*cf)(std::move(args), global_name);
            }

            // otherwise the match was not complete, bail out
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "phylanx::execution_tree::compiler::handle_placeholders",
                "couldn't find built-in function in environment: " + name);
        }

    public:
        function operator()(ast::expression const& expr)
        {
            for (auto const& pattern : patterns_)
            {
                std::multimap<std::string, ast::expression> placeholders;
                if (!ast::match_ast(expr, hpx::util::get<2>(pattern),
                        ast::detail::on_placeholder_match{placeholders}))
                {
                    continue;   // no match found for the current pattern
                }

                // Handle define(__1)
                std::string name = hpx::util::get<0>(pattern);
                if (name == "define")
                {
                    return handle_define(placeholders, pattern);
                }

                // add sequence number for this primitive component
                std::size_t sequence_number =
                    snippets_.sequence_numbers_[name]++;
                name += "$" + std::to_string(sequence_number);

                // get global name of the component created
                ast::tagged id = ast::detail::tagged_id(expr);
                if (id.id >= 0)
                {
                    name += annotation(id);
                }

                return handle_placeholders(
                    placeholders, hpx::util::get<0>(pattern), name);
            }

            // remaining expression could refer to a variable
            if (ast::detail::is_identifier(expr))
            {
                return handle_variable_reference(
                    ast::detail::identifier_name(expr), expr);
            }

            // ... or a function call
            if (ast::detail::is_function_call(expr))
            {
                return handle_function_call(
                    ast::detail::function_name(expr), expr);
            }

            // alternatively it could refer to a literal value
            if (ast::detail::is_literal_value(expr))
            {
                return literal_value(to_primitive_value_type(
                    ast::detail::literal_value(expr)));
            }

            // otherwise the match was not complete, bail out
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "phylanx::execution_tree::compiler::operator()()",
                "couldn't fully pattern-match the given expression: " +
                    ast::to_string(expr));
        }

    private:
        environment& env_;
        function_list& snippets_;
        expression_pattern_list const& patterns_;
        hpx::id_type default_locality_;
    };

    ///////////////////////////////////////////////////////////////////////////
    function compile(ast::expression const& expr,
        function_list& snippets, environment& env,
        expression_pattern_list const& patterns,
        hpx::id_type const& default_locality)
    {
        compiler comp{snippets, env, patterns, default_locality};
        return comp(expr);
    }

    ///////////////////////////////////////////////////////////////////////////
    function define_variable(std::string name, function_list& snippets,
        environment& env, primitive_argument_type body,
        hpx::id_type const& default_locality)
    {
        std::string full_name = name;
        auto p = name.find_first_of("$/");
        if (p != std::string::npos)
        {
            name.erase(p);
        }

        snippets.snippets_.emplace_back(function{});
        function& f = snippets.snippets_.back();

        // get sequence number of this component
        env.define(std::move(name), external_variable(f, default_locality));

        static std::string define_variable("define-variable");
        std::size_t sequence_number =
            snippets.sequence_numbers_[define_variable]++;

        f = primitive_variable{default_locality}(
                std::move(body), define_variable + "$" +
                    std::to_string(sequence_number) + "$" + full_name);
        return f;
    }
}}}

