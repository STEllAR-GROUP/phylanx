//  Copyright (c) 2017 Hartmut Kaiser
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>
#include <phylanx/ast/generate_ast.hpp>
#include <phylanx/ast/node.hpp>
#include <phylanx/ast/parser/expression.hpp>
#include <phylanx/ast/parser/skipper.hpp>

#include <hpx/throw_exception.hpp>

#include <boost/spirit/include/qi.hpp>

#include <sstream>
#include <string>

namespace phylanx { namespace ast
{
    ast::expression generate_ast(std::string const& input)
    {
        using iterator = std::string::const_iterator;

        iterator first = input.begin();
        iterator last = input.end();

        std::stringstream strm;
        ast::parser::error_handler<iterator> error_handler(first, last, strm);

        ast::parser::expression<iterator> expr(error_handler);
        ast::parser::skipper<iterator> skipper;

        ast::expression ast;

        if (!boost::spirit::qi::phrase_parse(first, last, expr, skipper, ast))
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "phylanx::ast::generate_ast", strm.str());
        }

        if (first != last)
        {
            error_handler("Error! ", "Incomplete parse:", first);

            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "phylanx::ast::generate_ast", strm.str());
        }

        return ast;
    }

    std::vector<ast::expression> generate_asts(std::string const& input)
    {
        using iterator = std::string::const_iterator;

        iterator first = input.begin();
        iterator last = input.end();

        std::stringstream strm;
        ast::parser::error_handler<iterator> error_handler(first, last, strm);

        ast::parser::expression<iterator> expr(error_handler);
        ast::parser::skipper<iterator> skipper;

        std::vector<ast::expression> asts;

        if (!boost::spirit::qi::phrase_parse(first, last, +expr, skipper, asts))
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "phylanx::ast::generate_asts", strm.str());
        }

        if (first != last)
        {
            error_handler("Error! ", "Incomplete parse:", first);

            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "phylanx::ast::generate_asts", strm.str());
        }

        return asts;
    }
}}


