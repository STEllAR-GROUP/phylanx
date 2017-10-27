//  Copyright (c) 2017 Hartmut Kaiser
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>
#include <phylanx/execution_tree/primitives.hpp>
#include <phylanx/execution_tree/compile.hpp>

namespace phylanx { namespace execution_tree
{
    ///////////////////////////////////////////////////////////////////////////
    pattern_list const& get_all_known_patterns()
    {
        static pattern_list patterns =
        {
            // variadic functions
            primitives::block_operation::match_data,
            primitives::parallel_block_operation::match_data,
            primitives::define_::match_data,
            // n-nary functions
            primitives::if_conditional::match_data,
            primitives::for_operation::match_data,
            // binary functions
            primitives::dot_operation::match_data,
            primitives::file_read::match_data,
            primitives::file_write::match_data,
            primitives::while_operation::match_data,
            // unary functions
            primitives::constant::match_data,
            primitives::determinant::match_data,
            primitives::exponential_operation::match_data,
            primitives::extract_shape::match_data,
            primitives::inverse_operation::match_data,
            primitives::transpose_operation::match_data,
            primitives::random::match_data,
            // variadic operations
            primitives::add_operation::match_data,
            primitives::and_operation::match_data,
            primitives::div_operation::match_data,
            primitives::mul_operation::match_data,
            primitives::or_operation::match_data,
            primitives::sub_operation::match_data,
            // binary operations
            primitives::equal::match_data,
            primitives::greater::match_data,
            primitives::greater_equal::match_data,
            primitives::less::match_data,
            primitives::less_equal::match_data,
            primitives::not_equal::match_data,
            primitives::store_operation::match_data,
            // unary operations
            primitives::unary_minus_operation::match_data,
            primitives::unary_not_operation::match_data
        };

        return patterns;
    }
}}
