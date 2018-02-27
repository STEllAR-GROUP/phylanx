//  Copyright (c) 2017-2018 Hartmut Kaiser
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>
#include <phylanx/execution_tree/primitives.hpp>
#include <phylanx/execution_tree/compile.hpp>

#include <string>
#include <vector>

namespace phylanx { namespace execution_tree
{
    ///////////////////////////////////////////////////////////////////////////
    pattern_list const& get_all_known_patterns()
    {
        static pattern_list patterns =
        {
            // variadic functions
            primitives::block_operation::match_data,
            primitives::column_set_operation::match_data,
            primitives::column_slicing_operation::match_data,
            primitives::console_output::match_data,
            primitives::debug_output::match_data,
            primitives::define_variable::match_data_define,
            primitives::hstack_operation::match_data,
            primitives::make_list::match_data,
            primitives::make_vector::match_data,
            primitives::parallel_block_operation::match_data,
            primitives::row_set_operation::match_data,
            primitives::row_slicing_operation::match_data,
            primitives::set_operation::match_data,
            primitives::slicing_operation::match_data,
            primitives::string_output::match_data,
            primitives::vstack_operation::match_data,
            // n-nary functions
            primitives::if_conditional::match_data,
            primitives::for_operation::match_data,
            primitives::linearmatrix::match_data,
            primitives::linspace::match_data,
            // binary functions
            primitives::apply::match_data,
            primitives::cross_operation::match_data,
            primitives::diag_operation::match_data,
            primitives::dot_operation::match_data,
            primitives::file_read::match_data,
            primitives::file_write::match_data,
            primitives::file_read_csv::match_data,
            primitives::file_write_csv::match_data,
            primitives::while_operation::match_data,
#if defined(PHYLANX_HAVE_HIGHFIVE)
            primitives::file_read_hdf5::match_data,
            primitives::file_write_hdf5::match_data,
#endif

            // unary functions
            primitives::constant::match_data,
            primitives::determinant::match_data,
            primitives::enable_tracing::match_data,
            primitives::exponential_operation::match_data,
            primitives::extract_shape::match_data,
            primitives::identity::match_data,
            primitives::inverse_operation::match_data,
            primitives::power_operation::match_data,
            primitives::random::match_data,
            primitives::square_root_operation::match_data,
            primitives::transpose_operation::match_data,
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
            primitives::unary_not_operation::match_data,
            //
            // compiler-specific (internal) primitives
            //
            primitives::access_argument::match_data,
            primitives::function_reference::match_data,
            primitives::wrapped_function::match_data,
            primitives::define_function::match_data,

            primitives::variable::match_data,
            primitives::wrapped_variable::match_data,
            primitives::define_variable::match_data
        };

        return patterns;
    }
}}
