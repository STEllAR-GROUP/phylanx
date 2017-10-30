//  Copyright (c) 2017 Hartmut Kaiser
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(PHYLANX_EXECUTION_TREE_TRANSFORM_AST_HPP)
#define PHYLANX_EXECUTION_TREE_TRANSFORM_AST_HPP

#include <phylanx/config.hpp>
#include <phylanx/ast/node.hpp>

#include <vector>
#include <utility>
#include <functional>
#include <iterator>

namespace phylanx { namespace ast
{
    using transform_rule = std::pair<transform_expression, transform_expression>;

    // need a struct to represent a
    // transform_expression has to
    // be able to compose multiple
    // expression trees into a k-ary
    // tree
    //
    // this would require being able to 
    // support an operation like this:
    //
    // add(qi::grammar a, qi::grammar b)
    //
    struct composable_grammar {

       composable_grammar() {}

       composable_grammar( qi::grammar & input ) {
          composed_grammars.push_back(input);
       }

       composable_grammar( std::vector<qi::grammar> & input ) {
          std::copy(std::begin(input), std::end(intput), std::back_inserter(composed_grammars));
       }


       composable_grammar operator+=( qi::grammar & other ) {
         composed_grammars.push_back(other);
         return (*this);
       }

       std::vector<phylanx::ast::transform_rule> composed_grammars;

    };

    struct treetransducer_t {

        treetransducer_t() {}

        treetransducer_t(qi::grammar & input)
            : cgrammar(input) {
        }
 
        treetransducer_t(std::vector<qi::grammar> & input)
            : cgrammar(input) {
        }
        
        // initializer_list
        //treetransducer_t() {}

        phylanx::ast::expression result operator()(char * const match) {

            phylanx::ast::expression expr;
            const size_t cgrammar_count = cgrammar.composed_grammars.size();

            if(cgrammar_count > 0) {
                expr = phylanx::ast::transform_ast(match, cgrammar.composed_grammars[0]);
                for(size_t i = 1; i < cgrammar_count; ++i) {
                    expr = phylanx::ast::transform_ast(match, expr);
                }
            }

            return expr;
        }

        composable_grammar cgrammar;
    };


    /// Traverse the given AST expression and replace nodes in the AST based
    /// on the given transformation rules.
    PHYLANX_EXPORT expression transform_ast(
        expression const& in, transform_rule const& rule);

    /// Traverse the given AST expression and replace nodes in the AST based
    /// on the given transformation rules.
    PHYLANX_EXPORT expression transform_ast(
        expression const& in, std::vector<transform_rule> const& rules);
}}

#endif
