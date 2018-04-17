//  Copyright (c) 2017-2018 Hartmut Kaiser
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>
#include <phylanx/ast/generate_ast.hpp>
#include <phylanx/ast/node.hpp>
#include <phylanx/ast/parser/expression.hpp>
#include <phylanx/ast/parser/skipper.hpp>
#include <phylanx/ast/transform_ast.hpp>
#include <phylanx/ast/generate_transform_rules.hpp>

#include <hpx/throw_exception.hpp>

#include <boost/spirit/include/qi.hpp>
#include <boost/fusion/include/std_pair.hpp>

#include <algorithm>
#include <cstddef>
#include <sstream>
#include <string>
#include <vector>

namespace phylanx { namespace ast
{
    ///////////////////////////////////////////////////////////////////////////
    std::vector<ast::transform_rule> generate_transform_rules(
        std::string const& input)
    {
        ir::reset_enable_counts_on_exit on_exit;

        using iterator = std::string::const_iterator;

        iterator first = input.begin();
        iterator last = input.end();

        std::vector<std::string::const_iterator> iters;
        std::stringstream strm;
        ast::parser::error_handler<iterator> error_handler(
            first, last, strm, iters);

        ast::parser::expression<iterator> expr(error_handler);
        ast::parser::skipper<iterator> skipper;

        std::vector<ast::transform_rule> rules;

        if (!boost::spirit::qi::phrase_parse(first, last,
                *(expr >> ':' >> expr), skipper, rules))
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "phylanx::ast::generate_transform_rule", strm.str());
        }

        if (first != last)
        {
            error_handler("Error! ", "Incomplete parse:", first);

            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "phylanx::ast::generate_transform_rule", strm.str());
        }

        // replace compile-tags with line/column information
        for (auto& rule : rules)
        {
            detail::replace_compile_ids(rule.first, iters, input);
            detail::replace_compile_ids(rule.second, iters, input);
        }

        return rules;
    }
}}


