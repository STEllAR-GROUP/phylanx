//  Copyright (c) 2017-2018 Hartmut Kaiser
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(PHYLANX_EXECUTION_TREE_COMPILE_HPP)
#define PHYLANX_EXECUTION_TREE_COMPILE_HPP

#include <phylanx/config.hpp>
#include <phylanx/ast/node.hpp>
#include <phylanx/execution_tree/primitives/base_primitive.hpp>
#include <phylanx/execution_tree/compiler/compiler.hpp>

#include <hpx/include/naming.hpp>

#include <vector>

namespace phylanx { namespace execution_tree
{
    ///////////////////////////////////////////////////////////////////////////
    /// Retrieve the full list of known patterns to be used with any of the
    /// \a generate_tree functions below.
    PHYLANX_EXPORT pattern_list const& get_all_known_patterns();

    /// Compile a given expression into a function, which when invoked will
    /// evaluate the expression corresponding to the expression.
    PHYLANX_EXPORT compiler::function compile(ast::expression const& expr,
        compiler::function_list& snippets,
        hpx::id_type const& default_locality = hpx::find_here());

    /// Compile a given expressions into a function, which when invoked will
    /// evaluate the expression corresponding to the expression.
    PHYLANX_EXPORT compiler::function compile(
        std::vector<ast::expression> const& exprs,
        compiler::function_list& snippets,
        hpx::id_type const& default_locality = hpx::find_here());

    /// Compile a given expression into a function, which when invoked will
    /// evaluate the expression corresponding to the expression.
    PHYLANX_EXPORT compiler::function compile(std::string const& expr,
        compiler::function_list& snippets,
        hpx::id_type const& default_locality = hpx::find_here());

    /// Compile a given expression into a function, which when invoked will
    /// evaluate the expression corresponding to the expression. Reuse the
    /// given compilation environment.
    PHYLANX_EXPORT compiler::function compile(ast::expression const& expr,
        compiler::function_list& snippets, compiler::environment& env,
        hpx::id_type const& default_locality = hpx::find_here());

    /// Compile a given expressions into a function, which when invoked will
    /// evaluate the expression corresponding to the expression. Reuse the
    /// given compilation environment.
    PHYLANX_EXPORT compiler::function compile(
        std::vector<ast::expression> const& exprs,
        compiler::function_list& snippets, compiler::environment& env,
        hpx::id_type const& default_locality = hpx::find_here());

    /// Compile a given expression into a function, which when invoked will
    /// evaluate the expression corresponding to the expression. Reuse the
    /// given compilation environment.
    PHYLANX_EXPORT compiler::function compile(std::string const& expr,
        compiler::function_list& snippets, compiler::environment& env,
        hpx::id_type const& default_locality = hpx::find_here());

    ///////////////////////////////////////////////////////////////////////////
    /// Compile a given expression into a function, additionally invoke it to
    /// evaluate the expression corresponding to the expression.
    PHYLANX_EXPORT compiler::function compile_and_run(
        ast::expression const& expr, compiler::function_list& snippets,
        hpx::id_type const& default_locality = hpx::find_here());

    /// Compile a given expression into a function, additionally invoke it to
    /// evaluate the expression corresponding to the expression.
    PHYLANX_EXPORT compiler::function compile_and_run(
        std::vector<ast::expression> const& exprs,
        compiler::function_list& snippets,
        hpx::id_type const& default_locality = hpx::find_here());

    /// Compile a given expression into a function, additionally invoke it to
    /// evaluate the expression corresponding to the expression.
    PHYLANX_EXPORT compiler::function compile_and_run(std::string const& expr,
        compiler::function_list& snippets,
        hpx::id_type const& default_locality = hpx::find_here());

    /// Compile a given expression into a function, additionally invoke it to
    /// evaluate the expression corresponding to the expression. Reuse the
    /// given compilation environment.
    PHYLANX_EXPORT compiler::function compile_and_run(
        ast::expression const& expr, compiler::function_list& snippets,
        compiler::environment& env,
        hpx::id_type const& default_locality = hpx::find_here());

    /// Compile a given expression into a function, additionally invoke it to
    /// evaluate the expression corresponding to the expression. Reuse the
    /// given compilation environment.
    PHYLANX_EXPORT compiler::function compile_and_run(
        std::vector<ast::expression> const& exprs,
        compiler::function_list& snippets, compiler::environment& env,
        hpx::id_type const& default_locality = hpx::find_here());

    /// Compile a given expression into a function, additionally invoke it to
    /// evaluate the expression corresponding to the expression. Reuse the
    /// given compilation environment.
    PHYLANX_EXPORT compiler::function compile_and_run(std::string const& expr,
        compiler::function_list& snippets, compiler::environment& env,
        hpx::id_type const& default_locality = hpx::find_here());
}}

#endif
