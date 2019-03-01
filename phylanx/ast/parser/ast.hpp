//  Copyright (c) 2001-2011 Joel de Guzman
//  Copyright (c) 2001-2018 Hartmut Kaiser
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(PHYLANX_AST_PARSER_AST_HPP)
#define PHYLANX_AST_PARSER_AST_HPP

#include <phylanx/ast/node.hpp>
#include <phylanx/ast/parser/extended_variant.hpp>
#include <phylanx/ast/parser/variant_traits.hpp>

#include <boost/fusion/include/sequence.hpp>
#include <boost/fusion/include/adapt_struct.hpp>
#include <boost/spirit/include/support_attributes.hpp>

#include <cstdint>
#include <list>
#include <string>
#include <vector>

BOOST_FUSION_ADAPT_STRUCT(
    phylanx::ast::identifier,
    (std::string, name)
    (std::int64_t, id)
    (std::int64_t, col)
)

BOOST_FUSION_ADAPT_STRUCT(
    phylanx::ast::unary_expr,
    (phylanx::ast::optoken, operator_)
    (phylanx::ast::operand, operand_)
)

BOOST_FUSION_ADAPT_STRUCT(
    phylanx::ast::operation,
    (phylanx::ast::optoken, operator_)
    (phylanx::ast::operand, operand_)
)

BOOST_FUSION_ADAPT_STRUCT(
    phylanx::ast::expression,
    (phylanx::ast::operand, first)
    (std::vector<phylanx::ast::operation>, rest)
)

BOOST_FUSION_ADAPT_STRUCT(
    phylanx::ast::function_call,
    (phylanx::ast::identifier, function_name)
    (std::string, attribute)
    (std::vector<phylanx::ast::expression>, args)
)

#endif


