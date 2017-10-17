//  Copyright (c) 2017 Hartmut Kaiser
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(PHYLANX_EXECUTION_TREE_GENERATE_TREE_HPP)
#define PHYLANX_EXECUTION_TREE_GENERATE_TREE_HPP

#include <phylanx/config.hpp>
#include <phylanx/ast/node.hpp>
#include <phylanx/execution_tree/primitives/base_primitive.hpp>

#include <hpx/runtime/find_here.hpp>
#include <hpx/util/tuple.hpp>

#include <string>
#include <vector>

namespace phylanx { namespace execution_tree
{
    ///////////////////////////////////////////////////////////////////////////
    /// Retrieve the full list of known patterns to be used with any of the
    /// \a generate_tree functions below.
    PHYLANX_EXPORT pattern_list const& get_all_known_patterns();

    namespace detail
    {
        ///////////////////////////////////////////////////////////////////////
        using expression_pattern =
            hpx::util::tuple<
                std::string, std::string, ast::expression, factory_function_type
            >;
        using expression_pattern_list = std::vector<expression_pattern>;

        PHYLANX_EXPORT expression_pattern_list generate_patterns(
            pattern_list const& patterns_list);

        PHYLANX_EXPORT primitive_argument_type generate_tree(
            ast::expression const& expr,
            expression_pattern_list const& patterns,
            phylanx::execution_tree::variables& variables,
            phylanx::execution_tree::functions& functions,
            hpx::id_type const& default_locality);
    }

    /// Generate an expression tree corresponding to the given textual
    /// expression. All variables used by the expression will be implicitly
    /// created.
    PHYLANX_EXPORT primitive_argument_type generate_tree(
        std::string const& exprstr,
        hpx::id_type default_locality = hpx::find_here());

    /// Generate an expression tree corresponding to the given textual
    /// expression. The given symbol-table is used to populate variables used
    /// in the expression. Unknown variables will be implicitly created.
    PHYLANX_EXPORT primitive_argument_type generate_tree(
        std::string const& exprstr, variables variables,
        hpx::id_type default_locality = hpx::find_here());

    PHYLANX_EXPORT primitive_argument_type generate_tree(
        std::string const& exprstr, pattern_list const& patterns,
        variables variables, hpx::id_type default_locality = hpx::find_here());

    PHYLANX_EXPORT primitive_argument_type generate_tree(
        ast::expression const& expr, pattern_list const& patterns,
        variables variables, hpx::id_type default_locality = hpx::find_here());

    /// Generate an expression tree corresponding to the given textual
    /// expression. The given symbol-table is used to populate variables used
    /// in the expression. Unknown variables will be implicitly created.
    /// The given function table is used to expand invocations of unknown
    /// functions.
    PHYLANX_EXPORT primitive_argument_type generate_tree(
        std::string const& exprstr, variables variables, functions funcs,
        hpx::id_type default_locality = hpx::find_here());

    PHYLANX_EXPORT primitive_argument_type generate_tree(
        std::string const& exprstr, pattern_list const& patterns,
        variables variables, functions funcs,
        hpx::id_type default_locality = hpx::find_here());

    PHYLANX_EXPORT primitive_argument_type generate_tree(
        ast::expression const& expr, pattern_list const& patterns,
        variables variables, functions funcs,
        hpx::id_type default_locality = hpx::find_here());
}}

#endif
