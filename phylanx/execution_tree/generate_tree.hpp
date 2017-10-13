//  Copyright (c) 2017 Hartmut Kaiser
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(PHYLANX_EXECUTION_TREE_GENERATE_TREE_HPP)
#define PHYLANX_EXECUTION_TREE_GENERATE_TREE_HPP

#include <phylanx/config.hpp>
#include <phylanx/ast/node.hpp>
#include <phylanx/execution_tree/primitives/base_primitive.hpp>

#include <map>
#include <string>

namespace phylanx { namespace execution_tree
{
    using variables = std::map<std::string, primitive_argument_type>;

    /// Retrieve the full list of known patterns to be used with any of the
    /// \a generate_tree functions below.
    PHYLANX_EXPORT pattern_list const& get_all_known_patterns();

    /// Generate an expression tree corresponding to the given textual
    /// expression. All variables used by the expression will be implicitly
    /// created.
    PHYLANX_EXPORT primitive_argument_type generate_tree(
        std::string const& exprstr);

    /// Generate an expression tree corresponding to the given textual
    /// expression. The given symbol-table is used to populate variables used
    /// in the expression. Unknown variables will be implicitly created.
    PHYLANX_EXPORT primitive_argument_type generate_tree(
        std::string const& exprstr,
        variables const& variables);

    PHYLANX_EXPORT primitive_argument_type generate_tree(
        std::string const& exprstr, pattern_list const& patterns,
        variables const& variables);

    PHYLANX_EXPORT primitive_argument_type generate_tree(
        ast::expression const& expr, pattern_list const& patterns,
        variables const& variables);
}}

#endif
