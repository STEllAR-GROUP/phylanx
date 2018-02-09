//  Copyright (c) 2017-2018 Hartmut Kaiser
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

#include <algorithm>
#include <cstddef>
#include <sstream>
#include <string>
#include <vector>

namespace phylanx { namespace ast
{
    namespace detail
    {
        ///////////////////////////////////////////////////////////////////////
        expression& replace_compile_ids(expression& ast,
            std::vector<std::string::const_iterator> const& iters,
            std::string::const_iterator begin);

        template <typename Ast>
        void replace_compile_ids(util::recursive_wrapper<Ast>& ast,
            std::vector<std::string::const_iterator> const& iters,
            std::string::const_iterator begin);

        inline void replace_compile_ids(operand& op,
            std::vector<std::string::const_iterator> const& iters,
            std::string::const_iterator begin);
        inline void replace_compile_ids(identifier& id,
            std::vector<std::string::const_iterator> const& iters,
            std::string::const_iterator begin);
        inline void replace_compile_ids(unary_expr& ue,
            std::vector<std::string::const_iterator> const& iters,
            std::string::const_iterator begin);
        inline void replace_compile_ids(primary_expr& pe,
            std::vector<std::string::const_iterator> const& iters,
            std::string::const_iterator begin);
        inline void replace_compile_ids(operation& op,
            std::vector<std::string::const_iterator> const& iters,
            std::string::const_iterator begin);
        inline void replace_compile_ids(function_call& fc,
            std::vector<std::string::const_iterator> const& iters,
            std::string::const_iterator begin);

        ///////////////////////////////////////////////////////////////////////
        template <typename Ast>
        void replace_compile_ids(Ast const&,
            std::vector<std::ptrdiff_t> const&,
            std::string::const_iterator)
        {
        }

        inline void replace_compile_ids(identifier& id,
            std::vector<std::string::const_iterator> const& iters,
            std::string::const_iterator begin)
        {
            if (id.id >= 0 && id.col == -1 && std::size_t(id.id) < iters.size())
            {
                id.id = std::distance(begin, iters[id.id]);
            }
        }

        inline void replace_compile_ids(unary_expr& ue,
            std::vector<std::string::const_iterator> const& iters,
            std::string::const_iterator begin)
        {
            if (ue.id >= 0 && ue.col == -1 && std::size_t(ue.id) < iters.size())
            {
                ue.id = std::distance(begin, iters[ue.id]);
            }
            replace_compile_ids(ue.operand_, iters, begin);
        }

        inline void replace_compile_ids(primary_expr& pe,
            std::vector<std::string::const_iterator> const& iters,
            std::string::const_iterator begin)
        {
            if (pe.id >= 0 && pe.col == -1 && std::size_t(pe.id) < iters.size())
            {
                pe.id = std::distance(begin, iters[pe.id]);
            }

            switch (pe.index())
            {
            case 3:     // identifier
                replace_compile_ids(util::get<3>(pe.get()), iters, begin);
                break;

            case 6:     // phylanx::util::recursive_wrapper<expression>
                replace_compile_ids(util::get<6>(pe.get()).get(), iters, begin);
                break;

            case 7:     // phylanx::util::recursive_wrapper<function_call>
                replace_compile_ids(util::get<7>(pe.get()).get(), iters, begin);
                break;

            case 8:     // phylanx::util::recursive_wrapper<std::vector<ast::expression>>
                for (auto& ast : util::get<8>(pe.get()).get())
                {
                    replace_compile_ids(ast, iters, begin);
                }
                break;

            default:
                break;
            }
        }

        inline void replace_compile_ids(operand& op,
            std::vector<std::string::const_iterator> const& iters,
            std::string::const_iterator begin)
        {
            switch (op.index())
            {
            case 1:     // phylanx::util::recursive_wrapper<primary_expr>
                replace_compile_ids(util::get<1>(op.get()).get(), iters, begin);
                break;

            case 2:     // phylanx::util::recursive_wrapper<unary_expr>
                replace_compile_ids(util::get<2>(op.get()).get(), iters, begin);
                break;

            default:
                break;
            }
        }

        inline void replace_compile_ids(operation& op,
            std::vector<std::string::const_iterator> const& iters,
            std::string::const_iterator begin)
        {
            replace_compile_ids(op.operand_, iters, begin);
        }

        inline void replace_compile_ids(function_call& fc,
            std::vector<std::string::const_iterator> const& iters,
            std::string::const_iterator begin)
        {
            replace_compile_ids(fc.function_name, iters, begin);
            for (auto& arg : fc.args)
            {
                replace_compile_ids(arg, iters, begin);
            }
        }

        template <typename Ast>
        void replace_compile_ids(util::recursive_wrapper<Ast>& ast,
            std::vector<std::string::const_iterator> const& iters,
            std::string::const_iterator begin)
        {
            replace_compile_ids(ast.get(), iters, begin);
        }

        ///////////////////////////////////////////////////////////////////////
        expression& replace_compile_ids(expression& ast,
            std::vector<std::string::const_iterator> const& iters,
            std::string::const_iterator begin)
        {
            for (auto& op : ast.rest)
            {
                replace_compile_ids(op, iters, begin);
            }
            replace_compile_ids(ast.first, iters, begin);

            return ast;
        }
    }

    ///////////////////////////////////////////////////////////////////////////
    std::vector<ast::expression> generate_ast(std::string const& input)
    {
        using iterator = std::string::const_iterator;

        iterator first = input.begin();
        iterator last = input.end();

        std::vector<std::string::const_iterator> iters;
        std::stringstream strm;
        ast::parser::error_handler<iterator> error_handler(
            first, last, strm, iters);

        ast::parser::expression<iterator> expr(error_handler);
        ast::parser::skipper<iterator> skipper;

        std::vector<ast::expression> asts;

        if (!boost::spirit::qi::phrase_parse(first, last, *expr, skipper, asts))
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

        // replace compile-tags with offsets against begin of input
        for (auto& ast : asts)
        {
            detail::replace_compile_ids(ast, iters, input.begin());
        }

        return asts;
    }
}}


