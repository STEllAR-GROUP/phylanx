//  Copyright (c) 2001-2011 Joel de Guzman
//  Copyright (c) 2001-2017 Hartmut Kaiser
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(PHYLANX_AST_PARSER_EXPRESSION_DEF_HPP)
#define PHYLANX_AST_PARSER_EXPRESSION_DEF_HPP

#include <phylanx/config.hpp>
#include <phylanx/ast/parser/ast.hpp>
#include <phylanx/ast/parser/annotation.hpp>
#include <phylanx/ast/parser/error_handler.hpp>
#include <phylanx/ast/parser/expression.hpp>

#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/phoenix_function.hpp>

namespace phylanx { namespace ast { namespace parser
{
    template <typename Iterator>
    expression<Iterator>::expression(error_handler<Iterator>& error_handler)
      : expression_base<Iterator>(error_handler),
        expression::base_type(expression_base<Iterator>::expr)
    {}

    template <typename Iterator>
    expression_base<Iterator>::expression_base(
        error_handler<Iterator>& error_handler)
    {
        qi::_1_type _1;
        qi::_3_type _3;
        qi::_4_type _4;

        qi::char_type char_;
        qi::double_type double_;
        qi::_val_type _val;
        qi::raw_type raw;
        qi::lexeme_type lexeme;
        qi::alpha_type alpha;
        qi::alnum_type alnum;
        qi::bool_type bool_;

        using qi::on_error;
        using qi::on_success;
        using qi::fail;
        using qi::as;

        using error_handler_function =
            boost::phoenix::function<ast::parser::error_handler<Iterator>>;
        using annotation_function =
            boost::phoenix::function<ast::parser::annotation<Iterator>>;

        ///////////////////////////////////////////////////////////////////////
        // Tokens
        binary_op.add
            ("||", ast::optoken::op_logical_or)
            ("&&", ast::optoken::op_logical_and)
            ("==", ast::optoken::op_equal)
            ("!=", ast::optoken::op_not_equal)
            ("<", ast::optoken::op_less)
            ("<=", ast::optoken::op_less_equal)
            (">", ast::optoken::op_greater)
            (">=", ast::optoken::op_greater_equal)
            ("+", ast::optoken::op_plus)
            ("-", ast::optoken::op_minus)
            ("*", ast::optoken::op_times)
            ("/", ast::optoken::op_divide)
            ;

        unary_op.add
            ("+", ast::optoken::op_positive)
            ("-", ast::optoken::op_negative)
            ("!", ast::optoken::op_not)
            ;

        keywords.add
            ("true")
            ("false")
            ("if")
            ("else")
            ("while")
            ("var")
            ("void")
            ("return")
            ;

        ///////////////////////////////////////////////////////////////////////
        // Main expression grammar
        expr =
                unary_expr
            >> *(binary_op > unary_expr)
            ;

        unary_expr =
                primary_expr
            |   as<ast::unary_expr>()[unary_op > unary_expr]
            ;

        primary_expr =
                double_
//             |   function_call
            |   identifier
            |   bool_
            |   '(' > expr > ')'
            ;

//         function_call =
//                 (identifier >> '(')
//             >   argument_list
//             >   ')'
//             ;

//         argument_list = -(expr % ',');

        identifier =
                !lexeme[keywords >> !(alnum | '_')]
            >>  raw[lexeme[(alpha | '_') >> *(alnum | '_')]]
            ;

        ///////////////////////////////////////////////////////////////////////
        // Debugging and error handling and reporting support.
        BOOST_SPIRIT_DEBUG_NODES(
            (expr)
            (unary_expr)
            (primary_expr)
//             (function_call)
//             (argument_list)
            (identifier)
        );

        ///////////////////////////////////////////////////////////////////////
        // Error handling: on error in expr, call error_handler.
        static constexpr char const* const error_msg = "Error! Expecting ";

        on_error<fail>(
            expr, error_handler_function(error_handler)(error_msg, _4, _3));

        ///////////////////////////////////////////////////////////////////////
        // Annotation: on success in primary_expr, call annotation.
        on_success(primary_expr,
            annotation_function(error_handler.iters)(_val, _1));
    }
}}}

#endif

