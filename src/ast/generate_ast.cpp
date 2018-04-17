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
#include <cstdint>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

namespace phylanx { namespace ast
{
    namespace detail
    {
        ///////////////////////////////////////////////////////////////////////
        std::pair<std::size_t, std::size_t> get_pos(
            std::string const& code, std::string::const_iterator pos)
        {
            // pos is the offset into the code
            std::size_t line = 1;
            std::size_t column = 1;

            for (auto it = code.begin(); it != pos && it != code.end(); ++it)
            {
                if (*it == '\r' || *it == '\n')    // CR/LF
                {
                    ++line;
                    column = 1;
                }
                else
                {
                    ++column;
                }
            }

            return std::make_pair(line, column);
        }

        ///////////////////////////////////////////////////////////////////////
        expression& replace_compile_ids(expression& ast,
            std::vector<std::string::const_iterator> const& iters,
            std::string const& code);

        template <typename Ast>
        void replace_compile_ids(util::recursive_wrapper<Ast>& ast,
            std::vector<std::string::const_iterator> const& iters,
            std::string const& code);

        inline void replace_compile_ids(operand& op,
            std::vector<std::string::const_iterator> const& iters,
            std::string const& code);
        inline void replace_compile_ids(identifier& id,
            std::vector<std::string::const_iterator> const& iters,
            std::string const& code);
        inline void replace_compile_ids(unary_expr& ue,
            std::vector<std::string::const_iterator> const& iters,
            std::string const& code);
        inline void replace_compile_ids(primary_expr& pe,
            std::vector<std::string::const_iterator> const& iters,
            std::string const& code);
        inline void replace_compile_ids(operation& op,
            std::vector<std::string::const_iterator> const& iters,
            std::string const& code);
        inline void replace_compile_ids(function_call& fc,
            std::vector<std::string::const_iterator> const& iters,
            std::string const& code);

        ///////////////////////////////////////////////////////////////////////
        template <typename Ast>
        void replace_compile_ids(Ast const&,
            std::vector<std::ptrdiff_t> const&,
            std::string const&)
        {
        }

        inline void replace_compile_ids(identifier& id,
            std::vector<std::string::const_iterator> const& iters,
            std::string const& code)
        {
            if (id.id >= 0 && id.col == -1 && std::size_t(id.id) < iters.size())
            {
                auto const& p = get_pos(code, iters[id.id]);
                id.id = p.first;
                id.col = p.second;
            }
        }

        inline void replace_compile_ids(unary_expr& ue,
            std::vector<std::string::const_iterator> const& iters,
            std::string const& code)
        {
            if (ue.id >= 0 && ue.col == -1 && std::size_t(ue.id) < iters.size())
            {
                auto const& p = get_pos(code, iters[ue.id]);
                ue.id = p.first;
                ue.col = p.second;
            }
            replace_compile_ids(ue.operand_, iters, code);
        }

        inline void replace_compile_ids(primary_expr& pe,
            std::vector<std::string::const_iterator> const& iters,
            std::string const& code)
        {
            if (pe.id >= 0 && pe.col == -1 && std::size_t(pe.id) < iters.size())
            {
                auto const& p = get_pos(code, iters[pe.id]);
                pe.id = p.first;
                pe.col = p.second;
            }

            switch (pe.index())
            {
            case 3:     // identifier
                replace_compile_ids(util::get<3>(pe.get()), iters, code);
                break;

            case 6:     // phylanx::util::recursive_wrapper<expression>
                replace_compile_ids(util::get<6>(pe.get()).get(), iters, code);
                break;

            case 7:     // phylanx::util::recursive_wrapper<function_call>
                replace_compile_ids(util::get<7>(pe.get()).get(), iters, code);
                break;

            case 8:     // phylanx::util::recursive_wrapper<std::vector<ast::expression>>
                for (auto& ast : util::get<8>(pe.get()).get())
                {
                    replace_compile_ids(ast, iters, code);
                }
                break;

            default:
                break;
            }
        }

        inline void replace_compile_ids(operand& op,
            std::vector<std::string::const_iterator> const& iters,
            std::string const& code)
        {
            switch (op.index())
            {
            case 1:     // phylanx::util::recursive_wrapper<primary_expr>
                replace_compile_ids(util::get<1>(op.get()).get(), iters, code);
                break;

            case 2:     // phylanx::util::recursive_wrapper<unary_expr>
                replace_compile_ids(util::get<2>(op.get()).get(), iters, code);
                break;

            default:
                break;
            }
        }

        inline void replace_compile_ids(operation& op,
            std::vector<std::string::const_iterator> const& iters,
            std::string const& code)
        {
            replace_compile_ids(op.operand_, iters, code);
        }

        inline void replace_compile_ids(function_call& fc,
            std::vector<std::string::const_iterator> const& iters,
            std::string const& code)
        {
            replace_compile_ids(fc.function_name, iters, code);
            for (auto& arg : fc.args)
            {
                replace_compile_ids(arg, iters, code);
            }
        }

        template <typename Ast>
        void replace_compile_ids(util::recursive_wrapper<Ast>& ast,
            std::vector<std::string::const_iterator> const& iters,
            std::string const& code)
        {
            replace_compile_ids(ast.get(), iters, code);
        }

        ///////////////////////////////////////////////////////////////////////
        expression& replace_compile_ids(expression& ast,
            std::vector<std::string::const_iterator> const& iters,
            std::string const& code)
        {
            for (auto& op : ast.rest)
            {
                replace_compile_ids(op, iters, code);
            }
            replace_compile_ids(ast.first, iters, code);

            return ast;
        }
    }

    ///////////////////////////////////////////////////////////////////////////
    std::vector<ast::expression> generate_ast(std::string const& input)
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

        // replace compile-tags with line/column information
        for (auto& ast : asts)
        {
            detail::replace_compile_ids(ast, iters, input);
        }

        return asts;
    }
}}


