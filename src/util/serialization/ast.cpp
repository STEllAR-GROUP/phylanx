//  Copyright (c) 2017 Hartmut Kaiser
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>
#include <phylanx/ast/node.hpp>
#include <phylanx/util/serialization/ast.hpp>

#include <hpx/include/serialization.hpp>

#include <cstddef>
#include <cstdint>
#include <sstream>
#include <vector>

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

    std::vector<char> serialize(ir::node_data<std::uint8_t> const& ast)
    {
        return detail::serialize(ast);
    }

    std::vector<char> serialize(ir::node_data<double> const& ast)
    {
        return detail::serialize(ast);
    }

    std::vector<char> serialize(ast::nil ast)
    {
        return std::vector<char>{};
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

    std::vector<char> serialize(ast::literal_value_type const& ast)
    {
        return detail::serialize(ast);
    }

    std::vector<char> serialize(std::vector<ast::expression> const& ast)
    {
        return detail::serialize(ast);
    }

    ///////////////////////////////////////////////////////////////////////////
    namespace detail
    {
        template <typename Ast>
        inline void unserialize_helper(std::vector<char> const& input, Ast& ast)
        {
            hpx::serialization::input_archive archive(input, input.size());
            archive >> ast;
        }

        void unserialize(
            std::vector<char> const& input, ir::node_data<double>& ast)
        {
            detail::unserialize_helper(input, ast);
        }

        void unserialize(
            std::vector<char> const& input, ir::node_data<std::uint8_t>& ast)
        {
            detail::unserialize_helper(input, ast);
        }

        void unserialize(std::vector<char> const& input, ast::nil& ast)
        {
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

        void unserialize(
            std::vector<char> const& input, ast::literal_value_type& ast)
        {
            detail::unserialize_helper(input, ast);
        }

        void unserialize(
            std::vector<char> const& input, std::vector<ast::expression>& ast)
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
}}


