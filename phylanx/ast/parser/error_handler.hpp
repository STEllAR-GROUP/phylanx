//  Copyright (c) 2001-2011 Joel de Guzman
//  Copyright (c) 2001-2017 Hartmut Kaiser
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(PHYLANX_AST_PARSER_ERROR_HANDLER_HPP)
#define PHYLANX_AST_PARSER_ERROR_HANDLER_HPP

#include <phylanx/config.hpp>

#include <iosfwd>
#include <string>
#include <vector>

namespace phylanx { namespace ast { namespace parser
{
    ///////////////////////////////////////////////////////////////////////////
    //  The error handler
    template <typename Iterator>
    struct error_handler
    {
        template <typename, typename, typename>
        struct result
        {
            typedef void type;
        };

        error_handler(Iterator first, Iterator last, std::ostream& ostr)
          : first(first)
          , last(last)
          , out(ostr)
        {
        }

        template <typename Message, typename What>
        void operator()(Message const& message,
            What const& what,
            Iterator err_pos) const
        {
            int line;
            Iterator line_start = get_pos(err_pos, line);
            if (err_pos != last)
            {
                out << message << what << " line " << line << ':' << std::endl;
                out << get_line(line_start) << std::endl;
                for (; line_start != err_pos; ++line_start)
                    out << '~';
                out << '^' << std::endl;
            }
            else
            {
                out << "Unexpected end of file. ";
                out << message << what << " line " << line << std::endl;
            }
        }

        Iterator get_pos(Iterator err_pos, int& line) const
        {
            line = 1;
            Iterator i = first;
            Iterator line_start = first;
            while (i != err_pos)
            {
                bool eol = false;
                if (i != err_pos && *i == '\r')    // CR
                {
                    eol = true;
                    line_start = ++i;
                }
                if (i != err_pos && *i == '\n')    // LF
                {
                    eol = true;
                    line_start = ++i;
                }
                if (eol)
                    ++line;
                else
                    ++i;
            }
            return line_start;
        }

        std::string get_line(Iterator err_pos) const
        {
            Iterator i = err_pos;
            // position i to the next EOL
            while (i != last && (*i != '\r' && *i != '\n'))
                ++i;
            return std::string(err_pos, i);
        }

        Iterator first;
        Iterator last;
        std::ostream& out;
        std::vector<Iterator> iters;
    };
}}}

#endif
