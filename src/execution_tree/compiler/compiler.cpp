//  Copyright (c) 2017-2019 Hartmut Kaiser
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
#include <phylanx/ast/traverse.hpp>
#include <phylanx/execution_tree/compile.hpp>
#include <phylanx/execution_tree/compiler/actors.hpp>
#include <phylanx/execution_tree/compiler/compiler.hpp>
#include <phylanx/execution_tree/compiler/locality_attribute.hpp>
#include <phylanx/execution_tree/compiler/primitive_name.hpp>
#include <phylanx/execution_tree/primitives/base_primitive.hpp>
#include <phylanx/ir/node_data.hpp>

#include <hpx/include/naming.hpp>
#include <hpx/include/util.hpp>
#include <hpx/runtime/get_num_localities.hpp>

#include <boost/fusion/include/std_pair.hpp>
#include <boost/spirit/include/qi_attr.hpp>
#include <boost/spirit/include/qi_char.hpp>
#include <boost/spirit/include/qi_operator.hpp>
#include <boost/spirit/include/qi_parse.hpp>
#include <boost/spirit/include/qi_sequence.hpp>

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <list>
#include <map>
#include <memory>
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
            auto const& p = patterns.data_;
            if (!p.patterns_.empty())
            {
                result.define(p.primitive_type_,
                    builtin_function(p.create_primitive_, default_locality));

                if (p.supports_dtype_)
                {
                    result.define(p.primitive_type_ + "__bool",
                        builtin_function(p.create_primitive_, default_locality));
                    result.define(p.primitive_type_ + "__int",
                        builtin_function(p.create_primitive_, default_locality));
                    result.define(p.primitive_type_ + "__float",
                        builtin_function(p.create_primitive_, default_locality));
                }
            }
        }

        return result;
    }

    environment default_environment(
        compiler::expression_pattern_list const& patterns_list,
        hpx::id_type const& default_locality)
    {
        environment result;

        for (auto const& p : patterns_list)
        {
            result.define(p.first,
                builtin_function(p.second.creator_, default_locality));
        }

        return result;
    }

    environment default_environment(hpx::id_type const& default_locality)
    {
        return default_environment(get_all_known_patterns(), default_locality);
    }

    ///////////////////////////////////////////////////////////////////////////
    namespace detail
    {
        std::string construct_extended_pattern(std::string pattern,
            std::string const& main_name, std::string const& suffix)
        {
            auto p = pattern.find(main_name);
            if (p != std::string::npos)
            {
                pattern.replace(p, main_name.size(), main_name + suffix);
            }
            return pattern;
        }

        ///////////////////////////////////////////////////////////////////////
        std::pair<std::string, std::string> split_argument(std::string argname)
        {
            using namespace boost::spirit;

            std::pair<std::string, std::string> names;

            auto begin = argname.begin();
            bool result = qi::parse(begin, argname.end(),
                    '_' >> +qi::digit >> -('_' >> +(qi::alnum | qi::char_('_'))),
                names);

            if (!result || begin != argname.end())
            {
                names.second = argname;
            }
            else
            {
                names.first = '_' + names.first;
            }
            return names;
        }

        ///////////////////////////////////////////////////////////////////////
        // parse an __arg(_1, _2) construct
        bool parse_argument_value(expression_pattern_list const& patterns,
            ast::expression const& expr, std::string& argname,
            std::string& value)
        {
            using placeholder_map_type =
                std::multimap<std::string, ast::expression>;

            // find __arg pattern
            auto arg_it = patterns.lower_bound("__arg");
            if (arg_it == patterns.end())
            {
                return false;
            }

            // attempt to match the argument against __arg(_1, _2)
            placeholder_map_type placeholders;
            bool result = ast::match_ast(
                arg_it->second.pattern_ast_, expr,
                ast::detail::on_placeholder_match{placeholders});
            if (!result) return false;

            auto p = placeholders.find("_1");
            if (p == placeholders.end() ||
                !ast::detail::is_identifier(p->second))
            {
                return false;
            }

            auto names = split_argument(ast::detail::identifier_name(p->second));

            p = placeholders.find("_2");
            if (p == placeholders.end())
            {
                return false;
            }

            argname = std::move(names.second);
            value = to_string(p->second, true);

            return true;
        }

        ///////////////////////////////////////////////////////////////////////
        bool extract_arguments(std::string const& name,
            expression_pattern_list const& patterns, ast::expression const& expr,
            std::vector<std::string>& args, std::vector<std::string>& defaults)
        {
            // extract arguments, match primitive invocation
            using placeholder_map_type =
                std::multimap<std::string, ast::expression>;

            std::string match = hpx::util::format("{}(__1_args)", name);
            placeholder_map_type placeholders;
            auto result = ast::match_ast(ast::generate_ast(match)[0], expr,
                ast::detail::on_placeholder_match{placeholders});
            if (!result) return true;       // could be operator

            bool has_ellipses = false;
            bool has_default_value = false;

            auto its = placeholders.equal_range("_1_args");
            for (auto it = its.first; it != its.second; ++it)
            {
                if (ast::detail::is_identifier(it->second))
                {
                    // simple argument name
                    if (ast::detail::is_placeholder_ellipses(it->second))
                    {
                        args.push_back(ast::detail::identifier_name(it->second));
                        has_ellipses = true;        // must be last argument
                    }
                    else
                    {
                        auto names = split_argument(
                            ast::detail::identifier_name(it->second));

                        // kwargs cannot come after varargs
                        HPX_ASSERT(!has_ellipses);

                        // plain argument should always be given
                        args.push_back(std::move(names.second));

                        // all arguments after the first one with with a default
                        // value must fill the defaults array
                        if (has_default_value)
                        {
                            defaults.emplace_back();
                        }
                    }
                }
                else if (ast::detail::is_function_call(it->second))
                {
                    // argument with default value
                    HPX_ASSERT(
                        ast::detail::function_name(it->second) == "__arg");

                    std::string argname;
                    std::string default_value;

                    if (!parse_argument_value(
                            patterns, it->second, argname, default_value))
                    {
                        HPX_ASSERT(false);
                    }

                    // kwargs cannot come after varargs
                    HPX_ASSERT(!has_ellipses);

                    args.push_back(std::move(argname));
                    defaults.push_back(std::move(default_value));

                    has_default_value = true;
                }
                else
                {
                    HPX_ASSERT(false);
                }
            }
            return true;
        }

        ///////////////////////////////////////////////////////////////////////
        std::string reconstruct_pattern(std::string const& name,
            std::vector<std::string> const& args, std::size_t maxcount)
        {
            std::size_t count = 0;
            std::string pattern = name + '(';
            for (auto const& arg : args)
            {
                // generate only as many arguments as required
                if (count == maxcount)
                {
                    break;
                }

                if (count != 0)
                {
                    pattern += ", ";
                }

                if (ast::detail::is_placeholder(arg))
                {
                    pattern += arg;
                }
                else
                {
                    pattern += '_' + std::to_string(count + 1);
                    if (!arg.empty())
                    {
                        pattern += '_' + arg;
                    }
                }
                ++count;
            }
            pattern += ')';
            return pattern;
        }

        ///////////////////////////////////////////////////////////////////////
        void insert_pattern(expression_pattern_list& result,
            std::string pattern, match_pattern_type const& p,
            std::string const& suffix)
        {
            if (!suffix.empty())
            {
                pattern = construct_extended_pattern(
                    std::move(pattern), p.primitive_type_, suffix);
            }

            auto exprs = ast::generate_ast(pattern);
            HPX_ASSERT(exprs.size() == 1);

            std::vector<std::string> args;
            std::vector<std::string> defaults;

            if (ast::detail::is_function_call(exprs[0]))
            {
                // handle named arguments
                if (!extract_arguments(p.primitive_type_ + suffix, result,
                    exprs[0], args, defaults))
                {
                    // something went wrong
                    HPX_ASSERT(false);
                }
            }

            // reconstruct the pattern, if needed (leaving out default values)
            if (defaults.empty())
            {
                result.insert(expression_pattern_list::value_type(
                    p.primitive_type_ + suffix,
                    expression_pattern{std::move(pattern), std::move(exprs[0]),
                        p.create_primitive_, std::move(args),
                        std::move(defaults)}));
            }
            else
            {
                // reconstruct all patterns (with varying number of default
                // arguments)
                for (std::size_t i = defaults.size() + 1; i != 0; --i)
                {
                    std::string resulting_pattern =
                        reconstruct_pattern(p.primitive_type_ + suffix, args,
                            args.size() - (i - 1));
                    exprs = ast::generate_ast(resulting_pattern);

                    result.insert(expression_pattern_list::value_type(
                        p.primitive_type_ + suffix,
                        expression_pattern{std::move(resulting_pattern),
                            std::move(exprs[0]), p.create_primitive_, args,
                            defaults}));
                }
            }
        }

        ///////////////////////////////////////////////////////////////////////
        expression_pattern_list generate_patterns()
        {
            std::string empty_suffix;
            expression_pattern_list result;

            // add internal arg(_1, _2) needed for default arguments
            match_pattern_type match("__arg",
                std::vector<std::string>{"__arg(_1, _2)"}, nullptr, nullptr,
                "Internal");

            insert_pattern(result, match.patterns_[0], match, empty_suffix);

            for (auto const& patterns : get_all_known_patterns())
            {
                auto const& p = patterns.data_;
                for (auto const& pattern : p.patterns_)
                {
                    insert_pattern(result, pattern, p, empty_suffix);

                    if (p.supports_dtype_)
                    {
                        insert_pattern(result, pattern, p, "__bool");
                        insert_pattern(result, pattern, p, "__int");
                        insert_pattern(result, pattern, p, "__float");
                    }
                }
            }

            return result;
        }
    }

    expression_pattern_list const& generate_patterns()
    {
        static expression_pattern_list patterns = detail::generate_patterns();
        return patterns;
    }

    ///////////////////////////////////////////////////////////////////////////
    struct compiler_helper
    {
        compiler_helper(std::string const& name, function_list& snippets,
                environment& env, expression_pattern_list const& patterns,
                hpx::id_type const& default_locality)
          : name_(name)
          , env_(env)
          , snippets_(snippets)
          , patterns_(patterns)
          , default_locality_(default_locality)
        {}

    private:
        using placeholder_map_type =
            std::multimap<std::string, ast::expression>;

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
                    !ast::detail::is_identifier(it->second) &&
                    !ast::detail::is_function_call(it->second))
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
                    !ast::detail::is_identifier(it->second) &&
                    !ast::detail::is_function_call(it->second))
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
        std::pair<std::string, function>
        extract_default_argument_value(ast::expression const& arg,
            hpx::id_type const& locality, ast::tagged const& id) const
        {
            expression_pattern_list::const_iterator cit =
                patterns_.lower_bound("__arg");
            HPX_ASSERT(cit != patterns_.end());

            placeholder_map_type placeholders;
            if (!ast::match_ast(cit->second.pattern_ast_, arg,
                    ast::detail::on_placeholder_match{placeholders}))
            {
                HPX_THROW_EXCEPTION(hpx::bad_parameter,
                    "phylanx::execution_tree::compiler::compile_body",
                    generate_error_message(
                        hpx::util::format(
                            "invalid default argument {}, should be arg() "
                                "construct",
                            ast::to_string(arg)),
                        name_, id));
            }

            placeholder_map_type::iterator p = placeholders.find("_1");
            if (p == placeholders.end() || !ast::detail::is_identifier(p->second))
            {
                HPX_THROW_EXCEPTION(hpx::bad_parameter,
                    "phylanx::execution_tree::compiler::compile_body",
                    generate_error_message(
                        hpx::util::format(
                            "invalid default argument {}, should be arg() "
                                "construct (missing variable name)",
                            ast::to_string(arg)),
                        name_, id));
            }

            std::string arg_name = ast::detail::identifier_name(p->second);

            p = placeholders.find("_2");
            if (p == placeholders.end())
            {
                HPX_THROW_EXCEPTION(hpx::bad_parameter,
                    "phylanx::execution_tree::compiler::compile_body",
                    generate_error_message(
                        hpx::util::format(
                            "invalid default argument {}, should be arg() "
                                "construct (missing default value)",
                            ast::to_string(arg)),
                        name_, id));
            }

            return std::make_pair(
                std::move(arg_name), compile_body(p->second, locality));
        }

        ///////////////////////////////////////////////////////////////////////
        function compile_body(
            ast::expression const& body, hpx::id_type const& locality) const
        {
            environment env(&env_);
            return compile(name_, body, snippets_, env, patterns_, locality);
        }

        function compile_body(
            std::vector<ast::expression> const& args,
            ast::expression const& body, hpx::id_type const& locality) const
        {
            std::shared_ptr<std::string[]> named_args;
            std::size_t base_arg_num = env_.base_arg_num();

            bool has_default_value = false;
            environment env(&env_, args.size());
            for (std::size_t i = 0; i != args.size(); ++i)
            {
                ast::tagged id = ast::detail::tagged_id(args[i]);
                if (ast::detail::is_identifier(args[i]))
                {
                    if (has_default_value)
                    {
                        HPX_THROW_EXCEPTION(hpx::bad_parameter,
                            "phylanx::execution_tree::compiler::compile_body",
                            generate_error_message(
                                hpx::util::format("missing default argument "
                                    "{}, should be args() construct",
                                    ast::to_string(args[i])),
                                name_, id));
                    }

                    env.define(ast::detail::identifier_name(args[i]),
                        access_argument(i + base_arg_num, locality));
                }
                else if (ast::detail::is_function_call(args[i]))
                {
                    if (!named_args)
                    {
                        named_args.reset(new std::string[args.size()]);
                    }

                    if (ast::detail::function_name(args[i]) != "__arg")
                    {
                        HPX_THROW_EXCEPTION(hpx::bad_parameter,
                            "phylanx::execution_tree::compiler::compile_body",
                            generate_error_message(
                                hpx::util::format("invalid default argument "
                                    "{}, should be __arg() construct",
                                    ast::to_string(args[i])),
                                name_, id));
                    }

                    // returns pair<name, value>
                    auto default_value = extract_default_argument_value(
                        args[i], locality, id);

                    named_args[i] = default_value.first;

                    env.define(std::move(default_value.first),
                        access_argument(i + base_arg_num,
                            std::move(default_value.second.arg_),
                            locality));

                    // all subsequent arguments must have a default value
                    has_default_value = true;
                }
                else
                {
                    HPX_THROW_EXCEPTION(hpx::bad_parameter,
                        "phylanx::execution_tree::compiler::compile_body",
                        generate_error_message(
                            hpx::util::format("invalid argument {}, should be "
                                "either variable name or args() construct",
                                ast::to_string(args[i])),
                            name_, id));
                }
            }

            // compose the compiled function representing the default arguments
            // and the body
            function f =
                compile(name_, body, snippets_, env, patterns_, locality);
            if (named_args)
            {
                f.set_named_args(std::move(named_args), args.size());
            }

            return f;
        }

        function compile_lambda(std::vector<ast::expression> const& args,
            ast::expression const& body, ast::tagged const& id,
            hpx::id_type const& locality)
        {
            function& f = snippets_.program_.add_empty(name_);

            static std::string define_lambda_("lambda");

            primitive_name_parts name_parts(define_lambda_,
                snippets_.sequence_numbers_[define_lambda_]++, id.id, id.col,
                snippets_.compile_id_ - 1, get_locality_id(locality));

            std::string lambda_name = compose_primitive_name(name_parts);
            f = function{
                    primitive_argument_type{create_primitive_component(
                        default_locality_, name_parts.primitive,
                        primitive_argument_type{}, lambda_name, name_)},
                    lambda_name};

            function body_f = compile_body(args, body, locality);
            f.set_named_args(
                std::move(body_f.named_args_), body_f.num_named_args_);

            auto p = primitive_operand(f.arg_, lambda_name, name_);
            p.store(hpx::launch::sync, std::move(body_f.arg_), {});

            return f;
        }

        function handle_lambda(placeholder_map_type& placeholders,
            ast::tagged const& lambda_id)
        {
            // we know that 'lambda()' uses '__1' to match arguments
            using iterator = placeholder_map_type::iterator;
            std::pair<iterator, iterator> p = placeholders.equal_range("_1");

            // extract expressions representing the newly defined function
            auto args = extract_lambda_arguments(p, lambda_id);
            auto body = extract_lambda_body(p, lambda_id);

            return compile_lambda(args, body, lambda_id, default_locality_);
        }

        function handle_define(placeholder_map_type& placeholders,
            ast::tagged const& define_id, hpx::id_type const& locality)
        {
            // we know that 'define()' uses '__1' to match arguments
            using iterator = placeholder_map_type::iterator;
            std::pair<iterator, iterator> p = placeholders.equal_range("_1");

            // extract expressions representing the newly defined variable
            // and store new function description for later use
            function& f = snippets_.program_.add_empty(name_);

            ast::expression name_expr = extract_name(p, define_id);
            std::string name = ast::detail::identifier_name(name_expr);

            auto args = extract_define_arguments(p, define_id);
            auto body = extract_define_body(p, define_id);

            ast::tagged id = ast::detail::tagged_id(name_expr);

            // define(x, ...) creates a new variable that stores the value of x
            //
            // The symbol table will hold a compiler-function that returns an
            // object of type 'access-variable' that extracts the current value
            // of the variable it refers to.

            // a define() either sets up a named variable or a named lambda
            primitive_name_parts name_parts;
            if (args.empty())
            {
                // create variable in the current environment
                compiled_function* cf = env_.define_variable(
                    name, access_target(f, "access-variable", default_locality_));

                // Correct type of the access object if this variable refers
                // to a lambda or a block.
                auto body_f = compile_body(body, locality);
                primitive_name_parts body_name_parts;
                if (parse_primitive_name(body_f.name_, body_name_parts) &&
                    (body_name_parts.primitive == "lambda" ||
                        body_name_parts.primitive == "block"))
                {
                    std::string variable_type = "function";
                    name_parts = primitive_name_parts(variable_type,
                        snippets_.sequence_numbers_[variable_type]++, id.id,
                        id.col, snippets_.compile_id_ - 1,
                        get_locality_id(locality));

                    cf->target<access_target>()->target_name_ =
                        "access-function";
                }
                else
                {
                    std::string variable_type = "variable";
                    name_parts = primitive_name_parts(variable_type,
                        snippets_.sequence_numbers_[variable_type]++, id.id,
                        id.col, snippets_.compile_id_ - 1,
                        get_locality_id(locality));
                }
                name_parts.instance = std::move(name);

                // now create the variable-factory object
                std::string variable_name = compose_primitive_name(name_parts);
                f = function{primitive_argument_type{
                        create_primitive_component(
                            default_locality_, "variable-factory",
                            primitive_argument_type{}, variable_name, name_)
                    }, variable_name};
                f.set_named_args(
                    std::move(body_f.named_args_), body_f.num_named_args_);

                auto var = primitive_operand(f.arg_, variable_name, name_);
                var.store(hpx::launch::sync, std::move(body_f.arg_), {});
            }
            else
            {
                std::string variable_type = "function";

                name_parts = primitive_name_parts(variable_type,
                    snippets_.sequence_numbers_[variable_type]++, id.id, id.col,
                    snippets_.compile_id_ - 1, get_locality_id(locality));
                name_parts.instance = std::move(name);

                // create variable in the current environment
                env_.define_variable(name_parts.instance,
                    access_target(f, "access-function", default_locality_));

                std::string variable_name = compose_primitive_name(name_parts);
                f = function{primitive_argument_type{
                        create_primitive_component(
                            default_locality_, "variable-factory",
                            primitive_argument_type{}, variable_name, name_)
                    }, variable_name};

                function body_f = compile_lambda(args, body, id, locality);
                f.set_named_args(
                    std::move(body_f.named_args_), body_f.num_named_args_);

                auto var = primitive_operand(f.arg_, variable_name, name_);
                var.store(hpx::launch::sync, std::move(body_f.arg_), {});
            }

            // the define-variable object is invoked whenever a define() is
            // executed
            std::string define_variable = "define-variable";

            name_parts.sequence_number =
                snippets_.sequence_numbers_[define_variable]++;
            name_parts.primitive = std::move(define_variable);

            function variable_ref = f;      // copy f as we need to move it

            function define_f = define_operation{default_locality_}(
                std::move(variable_ref.arg_), std::move(name_parts), name_);
            define_f.set_named_args(f.named_args_, f.num_named_args_);

            return define_f;
        }

        bool handle_sliced_variable_reference(std::string name,
            ast::expression const& expr, std::list<function>&& elements,
            function& result)
        {
            ast::tagged id = ast::detail::tagged_id(expr);

            primitive_name_parts name_parts(name, 0ull, id.id, id.col,
                snippets_.compile_id_ - 1, get_locality_id(default_locality_));

            if (compiled_function* cf = env_.find(name))
            {
                // make sure the target can handle slicing directly
                auto at = cf->target<access_target>();
                if (at == nullptr || at->target_name_ != "access-variable")
                {
                    if (cf->target<access_argument>() == nullptr)
                    {
                        return false;
                    }
                }

                if (at != nullptr)
                {
                    name_parts.sequence_number =
                        snippets_.sequence_numbers_[at->target_name_]++;
                }

                result = (*cf)(std::move(elements), std::move(name_parts), name_);
                return true;
            }

            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "phylanx::execution_tree::compiler::handle_variable",
                generate_error_message(
                    "couldn't find variable '" + name + "' in symbol table",
                    name_, id));
        }

        // handle slice(), this has to be transformed into an immediate slicing
        // on the first argument
        bool handle_slice(placeholder_map_type& placeholders,
            ast::tagged const& slice_id, function& result)
        {
            //  _1 represents the expression to slice
            using iterator = placeholder_map_type::iterator;
            std::pair<iterator, iterator> p1 = placeholders.equal_range("_1");

            // we don't know how to handle a missing slicing target
            if (p1.first == p1.second ||
                !ast::detail::is_identifier(p1.first->second))
            {
                return false;
            }

            // __2 represents (up to two) slicing arguments
            std::pair<iterator, iterator> pargs =
                placeholders.equal_range("_2");

            // handle only cases with one, two, or three slicing arguments
            std::size_t numargs = std::distance(pargs.first, pargs.second);
            if (numargs == 0 || numargs > PHYLANX_MAX_DIMENSIONS)
            {
                return false;
            }

            // now compile arguments
            std::list<function> args;
            {
                environment env(&env_);
                for (iterator it = pargs.first; it != pargs.second; ++it)
                {
                    args.emplace_back(compile(name_, it->second,
                        snippets_, env, patterns_, default_locality_));
                }
            }

            // compile the target expression, make sure slicing parameters are
            // passed through to the access-variable object
            return handle_sliced_variable_reference(
                ast::detail::identifier_name(p1.first->second),
                p1.first->second, std::move(args), result);
        }

        // handle list() constructs directly in  the compiler
        function handle_list(placeholder_map_type& placeholders,
            ast::tagged const& id)
        {
            //  _1 represents the expressions passed to list
            using iterator = placeholder_map_type::iterator;
            std::pair<iterator, iterator> p = placeholders.equal_range("_1");

            // now compile list elements
            std::list<function> args;

            for (iterator it = p.first; it != p.second; ++it)
            {
                args.emplace_back(compile(name_, it->second,
                    snippets_, env_, patterns_, default_locality_));
            }

            primitive_name_parts name_parts("list", 0ull, id.id, id.col,
                snippets_.compile_id_ - 1, get_locality_id(default_locality_));

            return list_value{default_locality_}(
                std::move(args), std::move(name_parts), name_);
        }

        function handle_variable_reference(std::string name,
            ast::expression const& expr)
        {
            ast::tagged id = ast::detail::tagged_id(expr);

            if (compiled_function* cf = env_.find(name))
            {
                std::size_t seq_num = 0;
                auto at = cf->target<access_target>();
                if (at != nullptr)
                {
                    seq_num = snippets_.sequence_numbers_[at->target_name_]++;
                }
                else
                {
                    seq_num = snippets_.sequence_numbers_[name]++;
                }

                primitive_name_parts name_parts(name, seq_num, id.id, id.col,
                    snippets_.compile_id_ - 1,
                    get_locality_id(default_locality_));

                return (*cf)(std::list<function>{}, std::move(name_parts), name_);
            }

            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "phylanx::execution_tree::compiler::handle_variable",
                generate_error_message(
                    "couldn't find variable '" + name + "' in symbol table",
                    name_, id));
        }

        // handle function call arguments (default values, keyword arguments)
        void handle_function_call_argument(std::string const& function_name,
            primitive_arguments_type& fargs,
            std::vector<ast::expression> const& exprs,
            hpx::id_type const& locality, ast::tagged id)
        {
            // first argument slot to fill by this function
            std::size_t base = fargs.size();
            environment env(&env_);

            // try to find this symbol in primitives table
            auto p = patterns_.equal_range(function_name);
            if (p.first == p.second)
            {
                if (nullptr == env_.find(function_name))
                {
                    HPX_THROW_EXCEPTION(hpx::bad_parameter,
                        "phylanx::execution_tree::compiler::"
                            "handle_function_call_argument",
                        generate_error_message(
                            "couldn't find function '" + function_name +
                            "' in symbol table", name_, id));
                }

                // this seem to be a normal PhySL function, for now handle
                // arguments blindly
                fargs.reserve(base + exprs.size());
                for (auto const& argexpr : exprs)
                {
                    fargs.push_back(compile(name_, argexpr, snippets_, env,
                        patterns_, locality).arg_);
                }
                return;
            }

            // find the first pattern that has a sufficient number of
            // placeholders
            auto it = p.first;
            while (it != p.second && !it->second.expect_variadics() &&
                exprs.size() > it->second.args_.size())
            {
                it = ++p.first;
            }

            if (it == p.second)
            {
                HPX_THROW_EXCEPTION(hpx::bad_parameter,
                    "phylanx::execution_tree::compiler::"
                    "handle_function_call_argument",
                    generate_error_message(
                        hpx::util::format("attempt to call function '{}' "
                            "with too many arguments (expected: {}, "
                            "supplied: {})", function_name,
                            it->second.args_.size(), exprs.size()),
                        name_, id));
            }

            std::size_t num_kwargs = it->second.num_keyword_arguments();
            std::size_t num_defaults = it->second.num_default_value_arguments();

            if (num_defaults == 0 && num_kwargs == 0)
            {
                // no keyword arguments and no default values are defined for
                // this function - no special argument handling is required
                fargs.reserve(base + exprs.size());
                for (auto const& argexpr : exprs)
                {
                    if (ast::detail::is_function_call(argexpr) &&
                        ast::detail::function_name(argexpr) == "__arg")
                    {
                        std::string argname;
                        std::string value;
                        detail::parse_argument_value(
                            patterns_, argexpr, argname, value);

                        HPX_THROW_EXCEPTION(hpx::bad_parameter,
                            "phylanx::execution_tree::compiler::"
                            "handle_function_call_argument",
                            generate_error_message(hpx::util::format(
                                "unexpected keyword argument '{}' for "
                                "function '{}'", argname, function_name),
                                name_, id));
                    }

                    fargs.push_back(compile(name_, argexpr, snippets_, env,
                        patterns_, locality).arg_);
                }
                return;
            }

            // flags telling whether an argument was filled
            std::vector<std::int8_t> args_valid;

            // make sure the argument array is large enough for what the
            // function expects
            fargs.resize(
                base + (std::max)(exprs.size(), it->second.args_.size()));
            args_valid.resize(fargs.size() - base);

            // pre-fill argument array with default values
            if (num_defaults != 0)
            {
                std::size_t size = it->second.args_.size();
                std::size_t default_arg = 0;
                for (std::size_t pos = size - num_defaults; pos != size;
                     ++pos, ++default_arg)
                {
                    if (it->second.defaults_[default_arg].empty())
                    {
                        continue;   // skip arguments that have no default value
                    }

                    ast::expression default_expr =
                        ast::generate_ast(it->second.defaults_[default_arg])[0];

                    fargs[base + pos] = compile(name_, default_expr,
                        snippets_, env, patterns_, locality).arg_;
                    args_valid[pos] = true;
                }
            }

            // make sure arguments are used in correct order
            bool has_seen_keyword = false;

            // default values and keyword arguments
            std::size_t count = base;
            for (auto const& argexpr : exprs)
            {
                if (ast::detail::is_function_call(argexpr) &&
                    ast::detail::function_name(argexpr) == "__arg")
                {
                    // named argument
                    std::string argname;
                    std::string value;

                    if (detail::parse_argument_value(
                            patterns_, argexpr, argname, value))
                    {
                        std::size_t pos = it->second.keyword_position(argname);
                        if (pos == std::size_t(-1))
                        {
                            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                                "phylanx::execution_tree::compiler::"
                                    "handle_function_call_argument",
                                generate_error_message(
                                    "unknown keyword argument '" +
                                        argname + "'", name_, id));
                        }
                        if (fargs.size() <= base + pos)
                        {
                            fargs.resize(base + pos + 1);
                            args_valid.resize(pos + 1);
                        }

                        // place the keyword argument into the argument slot
                        // it belongs
                        fargs[base + pos] = compile(name_,
                            ast::generate_ast(value)[0], snippets_, env,
                            patterns_, locality).arg_;
                        args_valid[pos] = true;

                        count = base + pos + 1;
                    }

                    has_seen_keyword = true;
                }
                else
                {
                    if (has_seen_keyword)
                    {
                        HPX_THROW_EXCEPTION(hpx::bad_parameter,
                            "phylanx::execution_tree::compiler::"
                                "handle_function_call_argument",
                            generate_error_message(
                                "keyword arguments should not be followed "
                                "by non-keyword arguments",
                                name_, id));
                    }

                    if (fargs.size() <= count)
                    {
                        fargs.resize(count + 1);
                        args_valid.resize(count - base + 1);
                    }

                    // normal function argument, just place in array
                    fargs[count] = compile(name_, argexpr, snippets_, env,
                        patterns_, locality).arg_;
                    args_valid[count - base] = true;

                    ++count;
                }
            }

            // make sure all argument slots have been filled
            for (std::size_t i = 0; i != args_valid.size(); ++i)
            {
                if (!args_valid[i])
                {
                    HPX_THROW_EXCEPTION(hpx::bad_parameter,
                        "phylanx::execution_tree::compiler::"
                        "handle_function_call_argument",
                        generate_error_message(
                            hpx::util::format("missing positional argument {} "
                                              "for function '{}'",
                                i, function_name),
                            name_, id));
                }
            }
        }

        function handle_function_call(std::string name,
            ast::expression const& expr)
        {
            ast::tagged id = ast::detail::tagged_id(expr);

            if (compiled_function* cf = env_.find(name))
            {
                // extract and propagate locality
                hpx::id_type locality = default_locality_;

                std::string attr = ast::detail::function_attribute(expr);
                if (!attr.empty())
                {
                    // a function attribute could reference a specific locality
                    parse_locality_attribute(attr, locality);
                }

                // extract and prepare arguments
                std::vector<ast::expression> argexprs =
                    ast::detail::function_arguments(expr);

                static std::string call_function_("call-function");
                primitive_name_parts name_parts(call_function_,
                    snippets_.sequence_numbers_[call_function_]++, id.id,
                    id.col, snippets_.compile_id_ - 1,
                    get_locality_id(locality));
                name_parts.instance = std::move(name);

                primitive_arguments_type fargs;
                fargs.reserve(argexprs.size() + 1);

                fargs.push_back(
                    (*cf)(std::list<function>{}, name_parts, name_).arg_);

                // we represent function calls with empty argument lists as
                // a function call with a single nil argument to be able to
                // distinguish func() from invoke(func)
                if (argexprs.empty())
                {
                    fargs.emplace_back();
                }
                else
                {
                    handle_function_call_argument(
                        name_parts.instance, fargs, argexprs, locality, id);
                }

                // instantiate the function on the target locality
                std::string full_name = compose_primitive_name(name_parts);
                return function{
                    primitive_argument_type{
                        create_primitive_component(
                            locality, name_parts.primitive,
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

        function handle_placeholders(placeholder_map_type& placeholders,
            std::string const& name, ast::tagged id)
        {
            // add sequence number for this primitive component
            std::size_t sequence_number =
                snippets_.sequence_numbers_[name]++;

            // get global name of the component created
            primitive_name_parts name_parts(name, sequence_number, id.id,
                id.col, snippets_.compile_id_ - 1,
                get_locality_id(default_locality_));

            if (compiled_function* cf = env_.find(name))
            {
                std::list<function> args;

                // we represent function calls with empty argument lists as
                // a function call with a single nil argument to be able to
                // distinguish func() from invoke(func)
                if (placeholders.empty())
                {
                    args.emplace_back(
                        ast::nil{}, compose_primitive_name(name_parts));
                }
                else
                {
                    std::vector<ast::expression> argexprs;
                    argexprs.reserve(placeholders.size());
                    for (auto const& placeholder : placeholders)
                    {
                        argexprs.push_back(std::move(placeholder.second));
                    }

                    primitive_arguments_type fargs;
                    handle_function_call_argument(
                        name, fargs, argexprs, default_locality_, id);

                    for (auto&& arg : std::move(fargs))
                    {
                        args.emplace_back(std::move(arg));
                    }
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

        // separate name from possible dtype
        static std::string extract_name_and_dtype(std::string const& fullname)
        {
            std::string::size_type p  = fullname.find("__");
            if (p != std::string::npos)
            {
                return fullname.substr(0, p);
            }
            return fullname;
        }

    public:
        function operator()(ast::expression const& expr)
        {
            ast::tagged id = ast::detail::tagged_id(expr);
            if (ast::detail::is_function_call(expr))
            {
                // handle function calls separately
                std::string const& function_name =
                    ast::detail::function_name(expr);

                expression_pattern_list::const_iterator cit =
                    patterns_.lower_bound(function_name);
                if (cit != patterns_.end())
                {
                    // Handle define(__1)
                    if (function_name == "define")
                    {
                        placeholder_map_type placeholders;
                        if (ast::match_ast(expr, cit->second.pattern_ast_,
                                ast::detail::on_placeholder_match{placeholders}))
                        {
                            // extract and propagate locality
                            hpx::id_type locality = default_locality_;

                            std::string attr =
                                ast::detail::function_attribute(expr);
                            if (!attr.empty())
                            {
                                // a attribute on the define() could reference
                                // a specific locality
                                parse_locality_attribute(attr, locality);
                            }

                            return handle_define(placeholders, id, locality);
                        }
                    }

                    // Handle lambda(__1)
                    if (function_name == "lambda")
                    {
                        placeholder_map_type placeholders;
                        if (ast::match_ast(expr, cit->second.pattern_ast_,
                                ast::detail::on_placeholder_match{placeholders}))
                        {
                            return handle_lambda(placeholders, id);
                        }
                    }

                    // Handle slice(_1, __2)
                    if (function_name == "slice")
                    {
                        placeholder_map_type placeholders;
                        if (ast::match_ast(expr, cit->second.pattern_ast_,
                                ast::detail::on_placeholder_match{placeholders}))
                        {
                            function slice_result;
                            if (handle_slice(placeholders, id, slice_result))
                            {
                                // handle only special case where sliced
                                // expression is a variable
                                return slice_result;
                            }

                            // fall through to normal processing for all other
                            // cases
                        }
                    }

                    // handle list(__1)/make_list(__1)
//                     if (function_name == "list" || function_name == "make_list")
//                     {
//                         placeholder_map_type placeholders;
//                         if (ast::match_ast(expr, cit->second.pattern_ast_,
//                                 ast::detail::on_placeholder_match{placeholders}))
//                         {
//                             return handle_list(placeholders, id);
//                         }
//                     }

                    // handle all non-special functions
                    while (
                        cit != patterns_.end() && (*cit).first == function_name)
                    {
                        placeholder_map_type placeholders;
                        if (!ast::match_ast(expr, cit->second.pattern_ast_,
                                ast::detail::on_placeholder_match{placeholders}))
                        {
                            ++cit;
                            continue;   // no match found for the current pattern
                        }

                        return handle_placeholders(
                            placeholders, (*cit).first, id);
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
                    placeholder_map_type placeholders;
                    if (!ast::match_ast(expr, pattern.second.pattern_ast_,
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
                auto it = get_constants().find(name);
                if (it != get_constants().end())
                {
                    auto arg = it->second;
                    return literal_value(std::move(arg));
                }
                return handle_variable_reference(std::move(name), expr);
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

        std::uint32_t get_locality_id(hpx::id_type const& locality) const
        {
            return hpx::get_initial_num_localities() == 1 ?
                hpx::naming::invalid_locality_id :
                hpx::naming::get_locality_id_from_id(locality);
        }

        static std::map<std::string, primitive_argument_type>
            initialize_constants()
        {
            std::map<std::string, primitive_argument_type> constants =
            {
                // generally useful constants
                {"nil", primitive_argument_type{ast::nil{true}}},
                {"false", primitive_argument_type{false}},
                {"true", primitive_argument_type{true}},
                // number extrema
                {"inf",
                    primitive_argument_type{
                        std::numeric_limits<double>::infinity()}},
                {"ninf",
                    primitive_argument_type{
                        -std::numeric_limits<double>::infinity()}},
                {"nan",
                    primitive_argument_type{
                        std::numeric_limits<double>::quiet_NaN()}},
                {"NZERO", primitive_argument_type{-0.0}},
                {"PZERO", primitive_argument_type{+0.0}},
                // useful math constants
                {"euler",
                    primitive_argument_type{
                        2.7182818284590452353602874713526624977572}},
                {"euler_gamma",
                    primitive_argument_type{
                        0.5772156649015328606065120900824024310421}},
                {"pi",
                    primitive_argument_type{
                        3.1415926535897932384626433832795028841971}},
                // numpy dtype type constants
                {"int", primitive_argument_type{"int64"}},
                {"float", primitive_argument_type{"float64"}},
                {"bool", primitive_argument_type{"bool"}}
            };
            return constants;
        }

        static std::map<std::string, primitive_argument_type> const&
            get_constants()
        {
            static std::map<std::string, primitive_argument_type> constants =
                initialize_constants();
            return constants;
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
        compiler_helper comp{codename, snippets, env, patterns, default_locality};
        return comp(expr);
    }

    ///////////////////////////////////////////////////////////////////////////
    function define_variable(std::string const& codename,
        primitive_name_parts name_parts, function_list& snippets,
        environment& env, primitive_argument_type body,
        hpx::id_type const& default_locality)
    {
        function& f = snippets.program_.add_empty(codename);

        if (name_parts.instance.empty())
        {
            name_parts.instance = std::move(name_parts.primitive);
        }
        name_parts.primitive = "variable";
        name_parts.sequence_number =
            snippets.sequence_numbers_[name_parts.primitive]++;

        // create variable in the given environment
        env.define_variable(name_parts.instance,
            access_target(f, "access-variable", default_locality));

        // now create the variable object
        std::string variable_name = compose_primitive_name(name_parts);
        f = function{primitive_argument_type{
                create_primitive_component(
                    default_locality, "variable-factory",
                    primitive_argument_type{}, variable_name, codename)
            }, variable_name};

        auto var = primitive_operand(f.arg_, variable_name, codename);
        var.store(hpx::launch::sync, std::move(body), {});

        // the define-variable object is invoked whenever a define() is
        // executed
        std::string define_variable = "define-variable";

        name_parts.sequence_number =
            snippets.sequence_numbers_[define_variable]++;
        name_parts.primitive = std::move(define_variable);

        function variable_ref = f;      // copy f as we need to move it
        return define_operation{default_locality}(std::move(variable_ref.arg_),
            std::move(name_parts), codename);
    }
}}}

