//  Copyright (c) 2017 Hartmut Kaiser
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(PHYLANX_AST_GENERATE_AST_HPP)
#define PHYLANX_AST_GENERATE_AST_HPP

#include <phylanx/config.hpp>
#include <phylanx/ast/node.hpp>

#include <string>

namespace phylanx { namespace ast
{
    PHYLANX_EXPORT ast::expression generate_ast(std::string const& input);
}}

#endif
