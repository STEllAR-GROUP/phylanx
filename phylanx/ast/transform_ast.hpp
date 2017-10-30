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
    using transform_rule = std::pair<expression, expression>;
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
