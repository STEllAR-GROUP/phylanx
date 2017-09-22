//  Copyright (c) 2017 Hartmut Kaiser
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>
#include <phylanx/ast/node.hpp>
#include <phylanx/util/serialization/ast.hpp>

#include <hpx/include/serialization.hpp>

#include <cstddef>
#include <vector>
#include <sstream>

namespace phylanx { namespace util
{
    ///////////////////////////////////////////////////////////////////////////
    namespace detail
    {
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
    namespace detail
    {
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

        void unserialize(std::vector<char> const& input, ast::function_call& ast)
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

    void append_operation(ast::expression& ast,ast::operation const& o) {
      ast.rest.push_back(o);
    }

    std::string stringify_expression(ast::expression const & ast) {
      std::ostringstream txt;
      txt << "(";
      txt << stringify_operand(ast.first);
      for(auto r = ast.rest.begin(); r != ast.rest.end(); ++r) {
        txt << ' ';
        txt << stringify_operation(*r);
      }
      txt << ")";
      return txt.str();
    }
    std::string stringify_identifier(ast::identifier const & ast) {
      std::ostringstream m;
      m << "indent(" << ast.name << ")";
      return m.str();
    }
    std::string stringify_operand(ast::operand const & ast) {
      switch(ast.index()) {
        case 0:
          return "";
        case 1:
          {
            auto v = phylanx::util::get<1>(ast.get());
            return stringify_primary_expr(v.get());
          }
        case 2:
          {
            auto v = phylanx::util::get<2>(ast.get());
            return stringify_unary_expr(v.get());
          }
      };
      return "";
    }
    std::string stringify_operation(ast::operation const & ast) {
      std::string out;
      out += stringify_optoken(ast.operator_);
      out += ' ';
      out += stringify_operand(ast.operand_);
      return out;
    }
    std::string stringify_optoken(ast::optoken st) {
      switch(st) {
        case ast::optoken::op_plus:
          return "+";
        case ast::optoken::op_negative:
        case ast::optoken::op_minus:
          return "-";
        case ast::optoken::op_times:
          return "*";
      }
      std::ostringstream m;
      m  << "optoken(" <<  st << ")";
      return m.str();
    }
    std::string stringify_primary_expr(ast::primary_expr const & ast) {
      switch(ast.index()) {
        case 0:
          return "";
        case 1:
          return phylanx::util::get<1>(ast.get()) ? "T" : "F";
        case 2:
          {
            std::ostringstream m;

            phylanx::ir::node_data<double> d = phylanx::util::get<2>(ast.get());
            m << d;
            return m.str();
          }
        case 3:
          return stringify_identifier(phylanx::util::get<3>(ast.get()));
        case 4:
          return stringify_expression(phylanx::util::get<4>(ast.get()).get());
      };
      return "";
    }
    std::string stringify_unary_expr(ast::unary_expr const & ast) {
      std::string out;
      out += stringify_optoken(ast.operator_);
      out += stringify_operand(ast.operand_);
      return out;
    }
}}
