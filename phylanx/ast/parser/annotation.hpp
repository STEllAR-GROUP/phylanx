//  Copyright (c) 2001-2011 Joel de Guzman
//  Copyright (c) 2001-2017 Hartmut Kaiser
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(PHYLANX_AST_PARSER_ANNOTATION_HPP)
#define PHYLANX_AST_PARSER_ANNOTATION_HPP

#include <phylanx/config.hpp>
#include <phylanx/ast/parser/ast.hpp>
#include <phylanx/util/variant.hpp>

#include <cstddef>
#include <map>
#include <type_traits>
#include <vector>

namespace phylanx { namespace ast { namespace parser
{
    ///////////////////////////////////////////////////////////////////////////////
    //  The annotation handler links the AST to a map of iterator positions
    //  for the purpose of subsequent semantic error handling when the
    //  program is being compiled.
    template <typename Iterator>
    struct annotation
    {
        template <typename, typename>
        struct result
        {
            typedef void type;
        };

        std::vector<Iterator>& iters;

        annotation(std::vector<Iterator>& iters)
          : iters(iters)
        {
        }

        struct set_id
        {
            using result_type = void;

            std::size_t id;

            set_id(std::size_t id)
              : id(id)
            {
            }

//             void operator()(ast::function_call& x) const
//             {
//                 x.function_name.id = id;
//             }

            void operator()(ast::identifier& x) const
            {
                x.id = id;
            }

            template <typename T>
            void operator()(T& x) const
            {
                // no-op
            }
        };

        void operator()(ast::operand& ast, Iterator pos) const
        {
            std::size_t id = iters.size();
            iters.push_back(pos);
//             ast.apply_visitor(set_id(id));
            visit(set_id(id), ast);
        }

//         void operator()(ast::variable_declaration& ast, Iterator pos) const
//         {
//             int id = iters.size();
//             iters.push_back(pos);
//             ast.lhs.id = id;
//         }
//
//         void operator()(ast::assignment& ast, Iterator pos) const
//         {
//             int id = iters.size();
//             iters.push_back(pos);
//             ast.lhs.id = id;
//         }
//
//         void operator()(ast::return_statement& ast, Iterator pos) const
//         {
//             int id = iters.size();
//             iters.push_back(pos);
//             ast.id = id;
//         }

        void operator()(ast::identifier& ast, Iterator pos) const
        {
            int id = iters.size();
            iters.push_back(pos);
            ast.id = id;
        }
    };
}}}

#endif

