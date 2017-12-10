//  Copyright (c) 2017 Hartmut Kaiser
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(PHYLANX_AST_GENERATE_AST_HPP)
#define PHYLANX_AST_GENERATE_AST_HPP

#include <phylanx/config.hpp>
#include <phylanx/ast/node.hpp>

#include <string>
#include <vector>

namespace phylanx { namespace ast
{
    /// Parse the given string and convert it into an instance of an AST
    PHYLANX_EXPORT ast::expression generate_ast(std::string const& input);

    /// Parse the given string and convert it into an instance of an AST,
    /// fill given vector with iterators, one for each tagged_id assigned
    /// to one of the matched attributes.
    PHYLANX_EXPORT ast::expression generate_ast(std::string const& input,
        std::vector<std::string::const_iterator>& iters);

    /// Parse the given string and convert it into a list of AST instances
    PHYLANX_EXPORT std::vector<ast::expression> generate_asts(
        std::string const& input);

    /// Parse the given string and convert it into a list of AST instances,
    /// fill given vector with iterators, one for each tagged_id assigned
    /// to one of the matched attributes.
    PHYLANX_EXPORT std::vector<ast::expression> generate_asts(
        std::string const& input,
        std::vector<std::string::const_iterator>& iters);
}}

#endif
