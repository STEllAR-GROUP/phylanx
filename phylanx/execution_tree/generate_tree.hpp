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
    using variables = std::map<std::string, primitive>;

    PHYLANX_EXPORT primitive generate_tree(std::string const& exprstr,
        pattern_list const& patterns, variables const& variables);
    PHYLANX_EXPORT primitive generate_tree(ast::expression const& expr,
        pattern_list const& patterns, variables const& variables);
}}

#endif
