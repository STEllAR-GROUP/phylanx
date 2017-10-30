//  Copyright (c) 2017 Hartmut Kaiser
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(PHYLANX_EXECUTION_TREE_TRANSFORM_AST_HPP)
#define PHYLANX_EXECUTION_TREE_TRANSFORM_AST_HPP

#include <phylanx/config.hpp>
#include <phylanx/ast/node.hpp>

#include <vector>
#include <utility>

namespace phylanx { namespace ast
{
    using transform_rule = std::pair<expression, expression>;
    using weighted_transform_rule = std::pair< std::pair<expression, expression>, double >;

    /// Traverse the given AST expression and replace nodes in the AST based
    /// on the given transformation rules.
    PHYLANX_EXPORT expression transform_ast(
        expression const& in, transform_rule const& rule);

    /// Traverse the given AST expression and replace nodes in the AST based
    /// on the given transformation rules.
    PHYLANX_EXPORT expression transform_ast(
        expression const& in, std::vector<transform_rule> const& rules);
}}

#endif
