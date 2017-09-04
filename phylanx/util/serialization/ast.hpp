//  Copyright (c) 2017 Hartmut Kaiser
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(PHYLANX_SERIALIZE_HPP)
#define PHYLANX_SERIALIZE_HPP

#include <phylanx/config.hpp>
#include <phylanx/ast/node.hpp>
#include <phylanx/ir/node_data.hpp>

#include <vector>

namespace phylanx { namespace util
{
    ///////////////////////////////////////////////////////////////////////////
    PHYLANX_EXPORT std::vector<char> serialize(ir::node_data<double> const&);

    namespace detail
    {
        PHYLANX_EXPORT void unserialize(
            std::vector<char> const&, ir::node_data<double>&);
    }

    ///////////////////////////////////////////////////////////////////////////
    PHYLANX_EXPORT std::vector<char> serialize(ast::optoken);
    PHYLANX_EXPORT std::vector<char> serialize(ast::identifier const&);
    PHYLANX_EXPORT std::vector<char> serialize(ast::primary_expr const&);
    PHYLANX_EXPORT std::vector<char> serialize(ast::operand const&);
    PHYLANX_EXPORT std::vector<char> serialize(ast::unary_expr const&);
    PHYLANX_EXPORT std::vector<char> serialize(ast::operation const&);
    PHYLANX_EXPORT std::vector<char> serialize(ast::expression const&);

    namespace detail
    {
        PHYLANX_EXPORT void unserialize(
            std::vector<char> const&, ast::optoken&);
        PHYLANX_EXPORT void unserialize(
            std::vector<char> const&, ast::identifier&);
        PHYLANX_EXPORT void unserialize(
            std::vector<char> const&, ast::primary_expr&);
        PHYLANX_EXPORT void unserialize(
            std::vector<char> const&, ast::operand&);
        PHYLANX_EXPORT void unserialize(
            std::vector<char> const&, ast::unary_expr&);
        PHYLANX_EXPORT void unserialize(
            std::vector<char> const&, ast::operation&);
        PHYLANX_EXPORT void unserialize(
            std::vector<char> const&, ast::expression&);
    }

    PHYLANX_EXPORT ast::expression unserialize(std::vector<char> const&);
}}

#endif

