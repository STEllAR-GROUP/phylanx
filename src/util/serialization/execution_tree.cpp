//  Copyright (c) 2017 Hartmut Kaiser
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>
#include <phylanx/execution_tree/primitives/base_primitive.hpp>
#include <phylanx/util/serialization/execution_tree.hpp>

#include <hpx/include/serialization.hpp>

#include <cstddef>
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

    std::vector<char> serialize(
        execution_tree::primitive_argument_type const& ast)
    {
        return detail::serialize(ast);
    }

    std::vector<char> serialize(
        execution_tree::primitive_result_type const& ast)
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
    }

    void unserialize(std::vector<char> const& input,
        execution_tree::primitive_argument_type& ast)
    {
        detail::unserialize_helper(input, ast);
    }

    void unserialize(std::vector<char> const& input,
        execution_tree::primitive_result_type& ast)
    {
        detail::unserialize_helper(input, ast);
    }
}}


