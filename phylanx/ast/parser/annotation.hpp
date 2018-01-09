//  Copyright (c) 2001-2011 Joel de Guzman
//  Copyright (c) 2001-2017 Hartmut Kaiser
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(PHYLANX_AST_PARSER_ANNOTATION_HPP)
#define PHYLANX_AST_PARSER_ANNOTATION_HPP

#include <phylanx/config.hpp>
#include <phylanx/ast/detail/tagged_id.hpp>
#include <phylanx/ast/parser/ast.hpp>
#include <phylanx/util/variant.hpp>

#include <cstddef>
#include <cstdint>
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
        std::vector<Iterator>& iters;

        annotation(std::vector<Iterator>& iters)
          : iters(iters)
        {
        }

        struct set_id
        {
            std::int64_t id;

            set_id(std::size_t id)
              : id(static_cast<std::int64_t>(id))
            {
            }

            bool operator()(ast::function_call& x) const
            {
                if (x.function_name.id <= 0)
                {
                    x.function_name.id = id;
                    return true;
                }
                return false;
            }

            bool operator()(ast::identifier& x) const
            {
                x.id = id;
                return false;
            }

            bool operator()(ast::primary_expr& x) const
            {
                if (ast::detail::tagged_id(x) <= 0)
                {
                    x.id = id;
                    return true;
                }
                return false;
            }

            bool operator()(ast::unary_expr& x) const
            {
                if (ast::detail::tagged_id(x) <= 0)
                {
                    x.id = id;
                    return true;
                }
                return false;
            }

            template <typename T>
            bool operator()(T& x) const
            {
                return false;   // no-op
            }

            template <typename T>
            bool operator()(util::recursive_wrapper<T>& x) const
            {
                return (*this)(x.get());
            }
        };

        void operator()(ast::operand& ast, Iterator pos) const
        {
            std::size_t id = iters.size();
            if (visit(set_id(id), ast))
            {
                iters.push_back(pos);
            }
        }

//         void operator()(ast::variable_declaration& ast, Iterator pos) const
//         {
//             std::size_t id = iters.size();
//             iters.push_back(pos);
//             ast.lhs.id = id;
//         }
//
//         void operator()(ast::assignment& ast, Iterator pos) const
//         {
//             std::size_t id = iters.size();
//             iters.push_back(pos);
//             ast.lhs.id = id;
//         }
//
//         void operator()(ast::return_statement& ast, Iterator pos) const
//         {
//             std::size_t id = iters.size();
//             iters.push_back(pos);
//             ast.id = id;
//         }
//
//         void operator()(ast::identifier& ast, Iterator pos) const
//         {
//             std::size_t id = iters.size();
//             iters.push_back(pos);
//             ast.id = id;
//         }

        void operator()(ast::function_call& ast, Iterator pos) const
        {
            if (ast.function_name.id <= 0)
            {
                std::size_t id = iters.size();
                iters.push_back(pos);
                ast.function_name.id = static_cast<std::int64_t>(id);
            }
        }

        void operator()(ast::identifier& ast, Iterator pos) const
        {
            std::size_t id = iters.size();
            iters.push_back(pos);
            ast.id = static_cast<std::int64_t>(id);
        }
    };
}}}

#endif

