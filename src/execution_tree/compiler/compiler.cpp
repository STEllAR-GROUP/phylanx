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
#include <phylanx/execution_tree/compiler/primitive_name.hpp>
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
            auto const& p = hpx::util::get<1>(patterns);
            if (!hpx::util::get<1>(p).empty())
            {
                result.define(hpx::util::get<0>(p),
                    builtin_function(hpx::util::get<2>(p), default_locality));
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
        for (auto const& patterns : patterns_list)
        {
            auto const& p = hpx::util::get<1>(patterns);
            for (auto const& pattern : hpx::util::get<1>(p))
            {
                auto exprs = ast::generate_ast(pattern);
                HPX_ASSERT(exprs.size() == 1);

                result.insert(expression_pattern_list::value_type(
                    hpx::util::get<0>(p),
                    hpx::util::make_tuple(
                        pattern, exprs[0], hpx::util::get<2>(p))));
            }
        }
        return result;
    }

    ///////////////////////////////////////////////////////////////////////////
    struct compiler
    {
        compiler(std::string const& name, function_list& snippets,
                environment& env, expression_pattern_list const& patterns,
                hpx::id_type const& default_locality)
          : name_(name)
          , env_(env)
          , snippets_(snippets)
          , patterns_(patterns)
          , default_locality_(default_locality)
        {}

    private:
        ///////////////////////////////////////////////////////////////////////
        static std::string generate_error_message(std::string const& msg,
            std::string const& name, ast::tagged const& id)
        {
            return hpx::util::format(
                PHYLANX_FORMAT_SPEC(1)
                    "(" PHYLANX_FORMAT_SPEC(2) ", " PHYLANX_FORMAT_SPEC(3) "): "
                    PHYLANX_FORMAT_SPEC(4), name, id.id, id.col, msg);
        }

        ///////////////////////////////////////////////////////////////////////
        template <typename Iterator>
        ast::expression extract_name(
            std::pair<Iterator, Iterator> const& p, ast::tagged const& id)
        {
            if (std::distance(p.first, p.second) < 2)
            {
                HPX_THROW_EXCEPTION(hpx::bad_parameter,
                    "phylanx::execution_tree::detail::extract_name",
                    generate_error_message(
                        "the define() operation requires at least 2 arguments",
                        name_, id));
            }

            if (!ast::detail::is_identifier(p.first->second))
            {
                HPX_THROW_EXCEPTION(hpx::bad_parameter,
                    "phylanx::execution_tree::detail::extract_name",
                    generate_error_message(
                        "the define() operation requires that the name of the "
                        "function to define is represented as a variable name "
                        "(not an expression)",
                        name_, id));
            }

            return p.first->second;
        }

        template <typename Iterator>
        std::vector<ast::expression> extract_define_arguments(
            std::pair<Iterator, Iterator> const& p, ast::tagged const& id)
        {
            std::ptrdiff_t size = std::distance(p.first, p.second);
            if (size < 2)
            {
                HPX_THROW_EXCEPTION(hpx::bad_parameter,
                    "phylanx::execution_tree::detail::"
                        "extract_define_arguments",
                    generate_error_message(
                        "the define() operation requires at least 2 arguments",
                        name_, id));
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
                        "phylanx::execution_tree::detail::"
                            "extract_define_arguments",
                        generate_error_message(
                            "the define() operation requires that all "
                            "arguments are represented as variable "
                            "names (not expressions)", name_, id));
                }
                args.push_back(it->second);
            }

            return args;
        }

        template <typename Iterator>
        std::vector<ast::expression> extract_lambda_arguments(
            std::pair<Iterator, Iterator> const& p, ast::tagged const& id)
        {
            std::ptrdiff_t size = std::distance(p.first, p.second);
            if (size == 0)
            {
                HPX_THROW_EXCEPTION(hpx::bad_parameter,
                    "phylanx::execution_tree::detail::"
                        "extract_lambda_arguments",
                    generate_error_message(
                        "the define() operation requires at least 1 arguments",
                        name_, id));
            }

            std::vector<ast::expression> args;
            args.reserve(size);

            auto first = p.first;
            auto last = p.second; --last;

            std::size_t count = 0;
            for (auto it = first; it != last; ++it, ++count)
            {
                if (count != 0 && count != size-1 &&
                    !ast::detail::is_identifier(it->second))
                {
                    HPX_THROW_EXCEPTION(hpx::bad_parameter,
                        "phylanx::execution_tree::detail::"
                            "extract_lambda_arguments",
                        generate_error_message(
                            "the lambda() operation requires that all "
                            "arguments are represented as variable "
                            "names (not expressions)", name_, id));
                }
                args.push_back(it->second);
            }

            return args;
        }

        template <typename Iterator>
        ast::expression extract_define_body(
            std::pair<Iterator, Iterator> const& p, ast::tagged const& id)
        {
            if (std::distance(p.first, p.second) < 2)
            {
                HPX_THROW_EXCEPTION(hpx::bad_parameter,
                    "phylanx::execution_tree::detail::extract_define_body",
                    generate_error_message(
                        "the define() operation requires at least "
                            "2 arguments",
                        name_, id));
            }

            auto last = p.second; --last;
            return last->second;
        }

        template <typename Iterator>
        ast::expression extract_lambda_body(
            std::pair<Iterator, Iterator> const& p, ast::tagged const& id)
        {
            if (std::distance(p.first, p.second) == 0)
            {
                HPX_THROW_EXCEPTION(hpx::bad_parameter,
                    "phylanx::execution_tree::detail::extract_lambda_body",
                    generate_error_message(
                        "the lambda() operation requires at least 1 "
                            "argument (the body of the lambda)",
                        name_, id));
            }

            auto last = p.second; --last;
            return last->second;
        }

        ///////////////////////////////////////////////////////////////////////
        function handle_lambda(
            std::vector<ast::expression> const& args,
            ast::expression const& body) const
        {
            std::size_t base_arg_num = 0; // env_.base_arg_num();
            environment env(&env_, args.size());
            for (std::size_t i = 0; i != args.size(); ++i)
            {
                access_argument arg(default_locality_);

                HPX_ASSERT(ast::detail::is_identifier(args[i]));
                env.define(ast::detail::identifier_name(args[i]),
                    hpx::util::bind(arg, i + base_arg_num,
                        hpx::util::placeholders::_2, name_));
            }
            return compile(
                name_, body, snippets_, env, patterns_, default_locality_);
        }

        function handle_lambda(
            std::multimap<std::string, ast::expression>& placeholders,
            ast::tagged const& lambda_id)
        {
            // we know that 'lambda()' uses '__1' to match arguments
            using iterator =
                typename std::multimap<std::string, ast::expression>::iterator;
            std::pair<iterator, iterator> p = placeholders.equal_range("__1");

            auto args = extract_lambda_arguments(p, lambda_id);
            auto body = extract_lambda_body(p, lambda_id);

            static std::string define_lambda_("define-lambda");
            std::size_t sequence_number =
                snippets_.sequence_numbers_[define_lambda_]++;

            // extract expressions representing the newly defined function
            // and store new function description for later use
            snippets_.snippets_.emplace_back(function{});
            function& f = snippets_.snippets_.back();

            primitive_name_parts name_parts("lambda", sequence_number,
                lambda_id.id, lambda_id.col, snippets_.compile_id_ - 1);

            std::string lambda_name = compose_primitive_name(name_parts);

            static std::string function_("call-function");
            sequence_number =
                snippets_.sequence_numbers_[function_]++;

            auto extf = call_function(f, default_locality_);

            // create define_function helper object
            f = primitive_function{default_locality_}(name_parts, name_);

            // set the body for the compiled function
            primitive_operand(f.arg_, lambda_name, name_).set_body(
                hpx::launch::sync, std::move(handle_lambda(args, body).arg_));

            return extf({}, name_parts, name_);
        }

        function handle_define(
            std::multimap<std::string, ast::expression>& placeholders,
            ast::tagged const& define_id)
        {
            // we know that 'define()' uses '__1' to match arguments
            using iterator =
                typename std::multimap<std::string, ast::expression>::iterator;
            std::pair<iterator, iterator> p = placeholders.equal_range("__1");

            // extract expressions representing the newly defined variable
            // and store new function description for later use
            snippets_.snippets_.emplace_back(function{});
            function& f = snippets_.snippets_.back();

            ast::expression name_expr = extract_name(p, define_id);
            std::string name = ast::detail::identifier_name(name_expr);

            // get global name of the component created
            static std::string variable_("variable");
            ast::tagged id = ast::detail::tagged_id(name_expr);

            primitive_name_parts name_parts(variable_,
                snippets_.sequence_numbers_[variable_]++,
                id.id, id.col, snippets_.compile_id_ - 1);
            name_parts.instance = name;

            auto args = extract_define_arguments(p, define_id);
            auto body = extract_define_body(p, define_id);

            // define(x, ...) creates a new variable that stores the value of x
            //
            // The symbol table will hold a compiler-function that returns an
            // object of type 'access-variable' that extracts the current value
            // of the variable it refers to.
            env_.define(std::move(name), access_variable(f, default_locality_));

            // now create the variable object
            std::string variable_name = compose_primitive_name(name_parts);
            f = function{primitive_argument_type{
                    create_primitive_component(
                        default_locality_, name_parts.primitive,
                        primitive_argument_type{}, variable_name, name_)
                }, variable_name};

            primitive_operand(f.arg_, variable_name, name_).store(
                hpx::launch::sync, std::move(handle_lambda(args, body).arg_));

            // the define-variable object is invoked whenever a define() is
            // executed
            function variable_ref = f;      // copy f as we need to move it
            return define_operation{default_locality_}(
                std::move(variable_ref.arg_), std::move(name_parts), name_);
        }

        function handle_variable_reference(std::string name,
            ast::expression const& expr)
        {
            ast::tagged id = ast::detail::tagged_id(expr);
            primitive_name_parts name_parts(name, -1, id.id, id.col);

            if (compiled_function* cf = env_.find(name))
            {
                name_parts.compile_id = snippets_.compile_id_ - 1;
                name_parts.sequence_number = snippets_.sequence_numbers_[name]++;

                return (*cf)(std::list<function>{}, std::move(name_parts), name_);
            }

            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "phylanx::execution_tree::compiler::handle_variable",
                generate_error_message(
                    "couldn't find variable '" + name + "' in symbol table",
                    name_, id));
        }

        function handle_function_call(std::string name,
            ast::expression const& expr)
        {
            ast::tagged id = ast::detail::tagged_id(expr);

            if (compiled_function* cf = env_.find(name))
            {
                std::vector<ast::expression> argexprs =
                    ast::detail::function_arguments(expr);

                static std::string call_function_("call-function");
                primitive_name_parts name_parts(call_function_,
                    snippets_.sequence_numbers_[call_function_]++,
                    id.id, id.col, snippets_.compile_id_ - 1);
                name_parts.instance = std::move(name);

                std::vector<primitive_argument_type> fargs;
                fargs.reserve(argexprs.size() + 1);

                fargs.push_back(
                    (*cf)(std::list<function>{}, name_parts, name_).arg_);
                for (auto const& argexpr : argexprs)
                {
                    environment env(&env_);
                    fargs.push_back(compile(name_, argexpr, snippets_, env,
                        patterns_, default_locality_).arg_);
                }

                std::string full_name = compose_primitive_name(name_parts);

                return function{
                    primitive_argument_type{
                        create_primitive_component(
                            default_locality_, name_parts.primitive,
                            std::move(fargs), full_name, name_)
                    },
                    full_name};
            }

            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "phylanx::execution_tree::compiler::handle_function_call",
                generate_error_message(
                    "couldn't find function '" + name + "' in symbol table",
                    name_, id));
        }

        function handle_placeholders(
            std::multimap<std::string, ast::expression>& placeholders,
            std::string const& name, ast::tagged id)
        {
            // add sequence number for this primitive component
            std::size_t sequence_number =
                snippets_.sequence_numbers_[name]++;

            // get global name of the component created
            primitive_name_parts name_parts(name, sequence_number, id.id,
                id.col, snippets_.compile_id_ - 1);

            if (compiled_function* cf = env_.find(name))
            {
                std::list<function> args;
                environment env(&env_);

                for (auto const& placeholder : placeholders)
                {
                    args.push_back(compile(name_, placeholder.second,
                        snippets_, env, patterns_, default_locality_));
                }

                // create primitive with given arguments
                return (*cf)(std::move(args), std::move(name_parts), name_);
            }

            // otherwise the match was not complete, bail out
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "phylanx::execution_tree::compiler::handle_placeholders",
                generate_error_message("couldn't find built-in function '" +
                        name + "' in compilation environment",
                    name_, id));
        }

    public:
        function operator()(ast::expression const& expr)
        {
            ast::tagged id = ast::detail::tagged_id(expr);
            if (ast::detail::is_function_call(expr))
            {
                // handle function calls separately
                std::string function_name = ast::detail::function_name(expr);

                expression_pattern_list::const_iterator cit =
                    patterns_.lower_bound(function_name);
                if (cit != patterns_.end())
                {
                    // Handle define(__1)
                    if (function_name == "define")
                    {
                        std::multimap<std::string, ast::expression> placeholders;
                        if (ast::match_ast(expr, hpx::util::get<1>((*cit).second),
                                ast::detail::on_placeholder_match{placeholders}))
                        {
                            return handle_define(placeholders, id);
                        }
                    }

                    // Handle lambda(__1)
                    if (function_name == "lambda")
                    {
                        std::multimap<std::string, ast::expression> placeholders;
                        if (ast::match_ast(expr, hpx::util::get<1>((*cit).second),
                                ast::detail::on_placeholder_match{placeholders}))
                        {
                            return handle_lambda(placeholders, id);
                        }
                    }

                    while (cit != patterns_.end() && (*cit).first == function_name)
                    {
                        std::multimap<std::string, ast::expression> placeholders;
                        if (!ast::match_ast(expr, hpx::util::get<1>((*cit).second),
                                ast::detail::on_placeholder_match{placeholders}))
                        {
                            ++cit;
                            continue;   // no match found for the current pattern
                        }

                        return handle_placeholders(placeholders, (*cit).first, id);
                    }
                }
                else
                {
                    return handle_function_call(function_name, expr);
                }
            }
            else
            {
                // this should handle all remaining constructs (non-function calls)
                for (auto const& pattern : patterns_)
                {
                    std::multimap<std::string, ast::expression> placeholders;
                    if (!ast::match_ast(expr, hpx::util::get<1>(pattern.second),
                            ast::detail::on_placeholder_match{placeholders}))
                    {
                        continue;   // no match found for the current pattern
                    }

                    return handle_placeholders(placeholders, pattern.first, id);
                }
            }

            // remaining expression could refer to a variable
            if (ast::detail::is_identifier(expr))
            {
                std::string name = ast::detail::identifier_name(expr);
                if (name == "nil")
                {
                    return literal_value(primitive_argument_type{});
                }
                else if (name == "false")
                {
                    return literal_value(primitive_argument_type{false});
                }
                else if (name == "true")
                {
                    return literal_value(primitive_argument_type{true});
                }
                return handle_variable_reference(name, expr);
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
                generate_error_message(
                    "couldn't fully pattern-match the given expression: " +
                        ast::to_string(expr),
                    name_, id));
        }

    private:
        std::string name_;          // file name of original code
        environment& env_;          // current compilation environment
        function_list& snippets_;   // list of compiled snippets
        expression_pattern_list const& patterns_;
        hpx::id_type default_locality_;
    };

    ///////////////////////////////////////////////////////////////////////////
    function compile(std::string const& codename, ast::expression const& expr,
        function_list& snippets, environment& env,
        expression_pattern_list const& patterns,
        hpx::id_type const& default_locality)
    {
        compiler comp{codename, snippets, env, patterns, default_locality};
        return comp(expr);
    }

    ///////////////////////////////////////////////////////////////////////////
    function define_variable(std::string const& codename,
        primitive_name_parts name_parts, function_list& snippets,
        environment& env, primitive_argument_type body,
        hpx::id_type const& default_locality)
    {
        snippets.snippets_.emplace_back(function{});
        function& f = snippets.snippets_.back();

        // get sequence number of this component
        env.define(name_parts.primitive, access_variable(f, default_locality));

        if (name_parts.instance.empty())
        {
            name_parts.instance = name_parts.primitive;
        }
        name_parts.primitive = "variable";
        name_parts.sequence_number =
            snippets.sequence_numbers_[name_parts.primitive]++;

        // now create the variable object
        std::string variable_name = compose_primitive_name(name_parts);
        f = function{primitive_argument_type{
                create_primitive_component(
                    default_locality, name_parts.primitive,
                    std::move(body), variable_name, codename)
            }, variable_name};

        // the define-variable object is invoked whenever a define() is
        // executed
        function variable_ref = f;      // copy f as we need to move it
        return define_operation{default_locality}(
            std::move(variable_ref.arg_), std::move(name_parts), codename);
    }
}}}

