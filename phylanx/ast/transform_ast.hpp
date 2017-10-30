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

namespace phylanx { namespace ast
{
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

       composable_grammar( qi::grammar & input ) {
          // TODO ?
       }

       composable_grammar operator+( qi::grammar & other ) {
          // TODO ?
       }

       composable_grammar operator+=( qi::grammar & other ) {
          // TODO ?
       }
    };

    template <typename Iterator>
    struct transform_expression 
       : expression<Iterator> {

       transform_expression( expression & const expr )
           : cgrammar(expr)
       {
       }

       composable_grammar cgrammar;
    };

    using transform_rule = std::pair<transform_expression, transform_expression>;
    using weighted_transform_rule = std::pair< transform_rule, double >;

    double get_weight(weighted_transform_rule & rule) {
        return std::get<1>(rule);
    }

    transform_rule get_rule(weighted_transform_rule & rule) {
        return std::get<0>(rule);
    }


    template<node_type=transform_rule>
    struct treetransducer_t {

        using value_type = node_type;

        using equal_t = std::equal_to<node_type, node_type, bool>;
        using notequal_t = std::not_equal_to<node_type, node_type, bool>;
        using less_t = std::less<node_type, node_type, bool>;
        using lessequal_t = std::less_equal<node_type, node_type, bool>;
        using greater_t = std::greater<node_type, node_type, bool>;
        using greaterequal_t = std::greater_equal<node_type, node_type, bool>;

        treetransducer_t( std::set<transform_rule> input_rules ) {
          // TODO: overload operator+ to do a 'reduction' 
          // over the input_rules to build the finite state 
          // tree
          //
        }
 
        treetransducer_t( std::vector<transform_rule> rules ) {
          // TODO: overload operator+ to do a 'reduction' 
          // over the input_rules to build the finite state 
          // tree
          //
        }

        // initializer_list
        //treetransducer_t( std::vector<transform_rule> rules ) {
        //}

        template<Comapare>
        bool apply_operator(treetransducer_t & other, Compare &cmp) {
            if(cmp(node, other.node)) {
                for(auto child : children) {
                    if( !(child.apply_operator<Compare>(other, cmp)) ) {
                        return false;
                    }
                }
                return true;
            }
            return false;
        }

        bool operator==(treetransducer_t & other) {
            equal_t cmp;
            return apply_operator<equal_t>(other, cmp);
        }

        bool operator!=(treetransducer_t & other) {
            notequal_t cmp;
            return apply_operator<equal_t>(other, cmp);
        }

        bool operator<(treetransducer_t & other) {
            less_t cmp;
            return apply_operator<equal_t>(other, cmp);
        }

        bool operator<=(treetransducer_t & other) {
            lessequal_t cmp;
            return apply_operator<equal_t>(other, cmp);
        }

        bool operator>(treetransducer_t & other) {
            greater_t cmp;
            return apply_operator<equal_t>(other, cmp);
        }

        bool operator>=(treetransducer_t & other) {
            greaterequal_t cmp;
            apply_operator<equal_t>(other, cmp);
        }

        // need to figure out how to compose
        // the expression trees for a rule
        // into a finite state automaton
        //
        // keep transform_rule a tuple but
        // compose the match and transform
        // rules into 1 transform_rule
        //
        void operator+(transform_rule & rule) {
        }

        node_type node;
        std::vector<node_type> children; 
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
