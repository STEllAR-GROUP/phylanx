//  Copyright (c) 2017 Hartmut Kaiser
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>
#include <phylanx/ast/node.hpp>
#include <phylanx/util/serialization/ast.hpp>

#include <hpx/include/serialization.hpp>

#include <cstddef>
#include <sstream>
#include <vector>

namespace phylanx {
namespace util {
    ///////////////////////////////////////////////////////////////////////////
    namespace detail {
        template <typename Ast>
        std::vector<char> serialize(Ast const& input)
        {
            std::vector<char> data;
            std::size_t archive_size = 0;

            {
                hpx::serialization::output_archive archive(data);
                archive << input;
                archive_size = archive.bytes_written();
            }

            data.resize(archive_size);
            return data;
        }
    }

    std::vector<char> serialize(ir::node_data<double> const& ast)
    {
        return detail::serialize(ast);
    }

    std::vector<char> serialize(ast::optoken ast)
    {
        return detail::serialize(ast);
    }

    std::vector<char> serialize(ast::identifier const& ast)
    {
        return detail::serialize(ast);
    }

    std::vector<char> serialize(ast::primary_expr const& ast)
    {
        return detail::serialize(ast);
    }

    std::vector<char> serialize(ast::operand const& ast)
    {
        return detail::serialize(ast);
    }

    std::vector<char> serialize(ast::unary_expr const& ast)
    {
        return detail::serialize(ast);
    }

    std::vector<char> serialize(ast::operation const& ast)
    {
        return detail::serialize(ast);
    }

    std::vector<char> serialize(ast::expression const& ast)
    {
        return detail::serialize(ast);
    }

    std::vector<char> serialize(ast::function_call const& ast)
    {
        return detail::serialize(ast);
    }

    ///////////////////////////////////////////////////////////////////////////
    namespace detail {
        template <typename Ast>
        void unserialize_helper(std::vector<char> const& input, Ast& ast)
        {
            hpx::serialization::input_archive archive(input, input.size());
            archive >> ast;
        }

        void unserialize(
            std::vector<char> const& input, ir::node_data<double>& ast)
        {
            detail::unserialize_helper(input, ast);
        }

        void unserialize(std::vector<char> const& input, ast::optoken& ast)
        {
            detail::unserialize_helper(input, ast);
        }

        void unserialize(std::vector<char> const& input, ast::identifier& ast)
        {
            detail::unserialize_helper(input, ast);
        }

        void unserialize(std::vector<char> const& input, ast::primary_expr& ast)
        {
            detail::unserialize_helper(input, ast);
        }

        void unserialize(std::vector<char> const& input, ast::operand& ast)
        {
            detail::unserialize_helper(input, ast);
        }

        void unserialize(std::vector<char> const& input, ast::unary_expr& ast)
        {
            detail::unserialize_helper(input, ast);
        }

        void unserialize(std::vector<char> const& input, ast::operation& ast)
        {
            detail::unserialize_helper(input, ast);
        }

        void unserialize(std::vector<char> const& input, ast::expression& ast)
        {
            detail::unserialize_helper(input, ast);
        }

        void unserialize(
            std::vector<char> const& input, ast::function_call& ast)
        {
            detail::unserialize_helper(input, ast);
        }
    }

    ast::expression unserialize(std::vector<char> const& input)
    {
        ast::expression expr;
        detail::unserialize_helper(input, expr);
        return expr;
    }

    // You cannot add to expression.rest unless you
    // use this function. Note sure why.
    void append_operation(ast::expression& ast, ast::operation const& o)
    {
        ast.rest.push_back(o);
    }

    namespace detail {
        template <typename Ast>
        PHYLANX_EXPORT std::string stringify(Ast const& ast);

        struct ast_stringify_visitor
        {
            mutable std::string out;

            template <typename Ast>
            PHYLANX_EXPORT bool operator()(Ast const& ast) const
            {
                out = stringify<Ast>(ast);
                return true;
            }
        };
    }

    std::string stringify_expression(ast::expression const& ast)
    {
        std::ostringstream txt;
        txt << "(";
        txt << stringify_operand(ast.first);
        for (auto r = ast.rest.begin(); r != ast.rest.end(); ++r)
        {
            txt << ' ';
            txt << stringify_operation(*r);
        }
        txt << ")";
        return txt.str();
    }
    std::string stringify_identifier(ast::identifier const& ast)
    {
        std::ostringstream m;
        m << "indent(" << ast.name << ")";
        return m.str();
    }
    std::string stringify_operand(ast::operand const& ast)
    {
        detail::ast_stringify_visitor p;
        visit(p, ast.get());
        return p.out;
    }
    std::string stringify_operation(ast::operation const& ast)
    {
        std::string out;
        out += stringify_optoken(ast.operator_);
        out += ' ';
        out += stringify_operand(ast.operand_);
        return out;
    }
    std::string stringify_optoken(ast::optoken st)
    {
        switch (st)
        {
        case ast::optoken::op_plus:
            return "+";
        case ast::optoken::op_negative:
        case ast::optoken::op_minus:
            return "-";
        case ast::optoken::op_times:
            return "*";
        }
        std::ostringstream m;
        m << "optoken(" << st << ")";
        return m.str();
    }

    std::string stringify_primary_expr(ast::primary_expr const& ast)
    {
        detail::ast_stringify_visitor p;
        visit(p, ast.get());
        return p.out;
    }
    std::string stringify_unary_expr(ast::unary_expr const& ast)
    {
        std::string out;
        out += stringify_optoken(ast.operator_);
        out += stringify_operand(ast.operand_);
        return out;
    }

    namespace detail {
        template <>
        PHYLANX_EXPORT std::string stringify(bool const& b)
        {
            return b ? "T" : "F";
        }

        template <>
        PHYLANX_EXPORT std::string stringify(long const& lg)
        {
            std::ostringstream m;
            m << lg;
            return m.str();
        }

        template <>
        PHYLANX_EXPORT std::string stringify(std::string const& s)
        {
            return s;
        }

        template <>
        PHYLANX_EXPORT std::string stringify(ast::nil const& n)
        {
            return "";
        }

        template <>
        PHYLANX_EXPORT std::string stringify(phylanx::ir::node_data<double> const& d)
        {
            std::ostringstream m;
            m << "(" << d << ")";
            return m.str();
        }

        template <>
        PHYLANX_EXPORT std::string stringify(ast::identifier const& ident)
        {
            return stringify_identifier(ident);
        }

        template <>
        PHYLANX_EXPORT std::string stringify(
            phylanx::util::recursive_wrapper<phylanx::ast::expression> const&
                expr)
        {
            return stringify_expression(expr.get());
        }

        template <>
        PHYLANX_EXPORT std::string stringify(
            phylanx::util::recursive_wrapper<phylanx::ast::function_call> const&
                func)
        {
            return "func";
        }

        template <>
        PHYLANX_EXPORT std::string stringify(
            phylanx::util::recursive_wrapper<phylanx::ast::unary_expr> const&
                un)
        {
            return stringify_unary_expr(un.get());
        }

        template <>
        PHYLANX_EXPORT std::string stringify(
            phylanx::util::recursive_wrapper<phylanx::ast::primary_expr> const&
                pr)
        {
            return stringify_primary_expr(pr.get());
        }
    }
}
}
