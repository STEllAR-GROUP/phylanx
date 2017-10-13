//  Copyright (c) 2001-2011 Joel de Guzman
//  Copyright (c) 2001-2017 Hartmut Kaiser
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(PHYLANX_AST_PARSER_SKIPPER_HPP)
#define PHYLANX_AST_PARSER_SKIPPER_HPP

#include <phylanx/config.hpp>

#include <boost/spirit/include/qi.hpp>

namespace phylanx { namespace ast { namespace parser
{
    namespace qi = boost::spirit::qi;
    namespace ascii = boost::spirit::ascii;

    ///////////////////////////////////////////////////////////////////////////
    //  The skipper grammar
    template <typename Iterator>
    struct skipper : qi::grammar<Iterator>
    {
        skipper() : skipper::base_type(start)
        {
            qi::char_type char_;
            ascii::space_type space;
            qi::eol_type eol;

            start =
                    space                               // tab/space/cr/lf
                |   "/*" >> *(char_ - "*/") >> "*/"     // C-style comments
                |   "//" >> *(char_ - eol) >> eol       // C++-style comments
                ;
        }

        qi::rule<Iterator> start;
    };
}}}

#endif


