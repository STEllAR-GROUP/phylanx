//  Copyright (c) 2017 Hartmut Kaiser
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(PHYLANX_AST_GENERATE_TRANSFORM_RULE_HPP)
#define PHYLANX_AST_GENERATE_TRANSFORM_RULE_HPP

#include <phylanx/config.hpp>
#include <phylanx/ast/transform_ast.hpp>

#include <string>
#include <vector>

namespace phylanx { namespace ast
{
    /// Parse the given string and convert it into a list of transform rules
    /// instances
    PHYLANX_EXPORT std::vector<ast::transform_rule> generate_transform_rules(
        std::string const& input);
}}

#endif
