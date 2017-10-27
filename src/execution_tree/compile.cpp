//  Copyright (c) 2017 Hartmut Kaiser
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>
#include <phylanx/ast/generate_ast.hpp>
#include <phylanx/ast/node.hpp>
#include <phylanx/execution_tree/compile.hpp>
#include <phylanx/execution_tree/compiler/compiler.hpp>
#include <phylanx/execution_tree/primitives/base_primitive.hpp>

#include <hpx/include/naming.hpp>

#include <algorithm>
#include <cstddef>

namespace phylanx { namespace execution_tree
{
    ///////////////////////////////////////////////////////////////////////////
    compiler::function compile(ast::expression const& expr,
        compiler::function_list& snippets, compiler::environment& env,
        hpx::id_type const& default_locality)
    {
        std::size_t snippet_count = snippets.size();

        pattern_list const& patterns = get_all_known_patterns();
        compiler::function f = compiler::compile(expr, snippets, env,
            compiler::generate_patterns(patterns), default_locality);

        if (valid(f.arg_) || snippet_count == snippets.size())
            return f;

        compiler::function_list::iterator it = snippets.begin();
        std::advance(it, snippet_count);
        return *it;
    }

    compiler::function compile(std::vector<ast::expression> const& exprs,
        compiler::function_list& snippets, compiler::environment& env,
        hpx::id_type const& default_locality)
    {
        std::size_t snippet_count = snippets.size();

        pattern_list const& patterns = get_all_known_patterns();
        compiler::function f = compiler::compile(exprs, snippets, env,
            compiler::generate_patterns(patterns), default_locality);

        if (valid(f.arg_) || snippet_count == snippets.size())
            return f;

        compiler::function_list::iterator it = snippets.begin();
        std::advance(it, snippet_count);
        return *it;
    }

    compiler::function compile(std::string const& expr,
        compiler::function_list& snippets, compiler::environment& env,
        hpx::id_type const& default_locality)
    {
        return compile(
            ast::generate_asts(expr), snippets, env, default_locality);
    }

    ///////////////////////////////////////////////////////////////////////////
    compiler::function compile(ast::expression const& expr,
        compiler::function_list& snippets, hpx::id_type const& default_locality)
    {
        std::size_t snippet_count = snippets.size();

        pattern_list const& patterns = get_all_known_patterns();
        compiler::environment env =
            compiler::default_environment(default_locality);
        compiler::function f = compiler::compile(expr, snippets, env,
            compiler::generate_patterns(patterns), default_locality);

        if (valid(f.arg_) || snippet_count == snippets.size())
            return f;

        compiler::function_list::iterator it = snippets.begin();
        std::advance(it, snippet_count);
        return *it;
    }

    compiler::function compile(std::vector<ast::expression> const& exprs,
        compiler::function_list& snippets, hpx::id_type const& default_locality)
    {
        std::size_t snippet_count = snippets.size();

        pattern_list const& patterns = get_all_known_patterns();
        compiler::environment env =
            compiler::default_environment(default_locality);
        compiler::function f = compiler::compile(exprs, snippets, env,
            compiler::generate_patterns(patterns), default_locality);

        if (valid(f.arg_) || snippet_count == snippets.size())
            return f;

        compiler::function_list::iterator it = snippets.begin();
        std::advance(it, snippet_count);
        return *it;
    }

    compiler::function compile(std::string const& expr,
        compiler::function_list& snippets, hpx::id_type const& default_locality)
    {
        return compile(ast::generate_asts(expr), snippets, default_locality);
    }
}}

