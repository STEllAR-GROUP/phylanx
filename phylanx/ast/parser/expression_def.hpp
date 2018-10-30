//  Copyright (c) 2001-2011 Joel de Guzman
//  Copyright (c) 2001-2019 Hartmut Kaiser
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

#include <cstdint>
#include <string>

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
        qi::lit_type lit;
        qi::real_parser<double, qi::strict_real_policies<double> > strict_double;
        qi::real_parser<double> double_;
        qi::_val_type _val;
        qi::raw_type raw;
        qi::lexeme_type lexeme;
        qi::alpha_type alpha;
        qi::alnum_type alnum;
        qi::bool_type bool_;
        qi::int_parser<std::int64_t> long_long;
        qi::attr_type attr;
        qi::uint_parser<unsigned char, 16, 1, 2> hex;

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
            ("%", ast::optoken::op_mod)
            ;

        unary_op.add
            ("+", ast::optoken::op_positive)
            ("-", ast::optoken::op_negative)
            ("!", ast::optoken::op_not)
            ;

//         keywords.add
//             ("true")
//             ("false")
//             ("nil")
//             ;

        unesc_char.add
            ("\\a", '\a')   // \a is alert
            ("\\b", '\b')   // \b is backspace
            ("\\f", '\f')   // \f is form feed
            ("\\n", '\n')   // \n is linefeed
            ("\\r", '\r')   // \r is carriage return
            ("\\t", '\t')   // \t is tab
            ("\\v", '\v')   // \v is vertical tab
            ("\\\\", '\\')  // \\ is backslash
            ("\\\'", '\'')  // \' is '
            ("\\\"", '"')   // \" is "
            ;

        ///////////////////////////////////////////////////////////////////////
        // Main expression grammar
        expr %=
                unary_expr
            >> *(binary_op > unary_expr)
            ;

        unary_expr %=
                primary_expr
            |   as<ast::unary_expr>()[unary_op > unary_expr]
            ;

        primary_expr %=
                strict_double
            |   function_call
            |   list
            |   identifier
            |   bool_
            |   long_long
            |   string
#if defined(PHYLANX_HAVE_BLAZE_TENSOR)
            |   int64_tensor
#endif
            |   int64_matrix
            |   int64_vector
#if defined(PHYLANX_HAVE_BLAZE_TENSOR)
            |   double_tensor
#endif
            |   double_matrix
            |   double_vector
            |   '(' > expr > ')'
            ;

#if defined(PHYLANX_HAVE_BLAZE_TENSOR)
        int64_tensor %= '[' >> (int64_matrix % ',') >> ']';
#endif
        int64_matrix %= '[' >> (int64_vector % ',') >> ']';

        int64_vector %= '[' >> (long_long % ',') >> ']';

#if defined(PHYLANX_HAVE_BLAZE_TENSOR)
        double_tensor %= '[' >> (double_matrix % ',') > ']';
#endif
        double_matrix %= '[' >> (double_vector % ',') > ']';

        double_vector %= '[' > (double_ % ',') > ']';

        function_call %=
                identifier
            >>  attribute
            >>  ('(' >  argument_list > ')')
            ;

        list %=
                (lit('\'') >> '(')
            >   argument_list
            >   ')'
            ;

        argument_list %= -(expr % ',');

        attribute %=
                '{' > identifier_name > '}'
            |   attr(std::string{})
            ;

        identifier %=
                identifier_name
            >>  (('$' > long_long) | attr(std::int64_t(-1)))
            >>  (('$' > long_long) | attr(std::int64_t(-1)))
            ;

        identifier_name %=
               !lexeme[keywords >> !(alnum | '_')]
            >>  raw[lexeme[(alpha | '_') >> *(alnum | '_')]]
            ;

        string %=
                lexeme['"' > *(unesc_char | "\\x" >> hex | (char_ - '"')) > '"']
            ;

        ///////////////////////////////////////////////////////////////////////
        // Debugging and error handling and reporting support.
#if defined(PHYLANX_HAVE_BLAZE_TENSOR)
        BOOST_SPIRIT_DEBUG_NODES(
            (int64_tensor)
            (double_tensor)
        );
#endif
        BOOST_SPIRIT_DEBUG_NODES(
            (expr)
            (unary_expr)
            (primary_expr)
            (list)
            (int64_matrix)
            (int64_vector)
            (double_matrix)
            (double_vector)
            (function_call)
            (argument_list)
            (identifier)
            (identifier_name)
            (string)
        );

        ///////////////////////////////////////////////////////////////////////
        // Error handling: on error in expr, call error_handler.
        static constexpr char const* const error_msg = "Error! Expecting ";

        on_error<fail>(
            expr, error_handler_function(error_handler)(error_msg, _4, _3));

        ///////////////////////////////////////////////////////////////////////
        // On success in identifier/function_call/unary_expr, call annotation.
        on_success(unary_expr,
            annotation_function(error_handler.iters)(_val, _1));
        on_success(function_call,
            annotation_function(error_handler.iters)(_val, _1));
        on_success(identifier,
            annotation_function(error_handler.iters)(_val, _1));
    }
}}}

#endif

