//  Copyright (c) 2017-2018 Hartmut Kaiser
//
//  Distributed under the Boost Software License}, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>
#include <phylanx/execution_tree/primitives.hpp>
#include <phylanx/execution_tree/compile.hpp>

#include <string>
#include <vector>

namespace phylanx { namespace execution_tree
{
    ///////////////////////////////////////////////////////////////////////////
    namespace detail
    {
#define PHYLANX_MATCH_DATA(type)                                               \
    hpx::util::make_tuple(hpx::util::get<0>(primitives::type::match_data),     \
        primitives::type::match_data)                                          \
/**/
#define PHYLANX_MATCH_DATA_VERBATIM(type)                                      \
    hpx::util::make_tuple(                                                     \
        hpx::util::get<0>(primitives::type), primitives::type)                 \
/**/

        pattern_list get_all_known_patterns()
        {
            pattern_list patterns =
            {
                // variadic functions
                PHYLANX_MATCH_DATA(block_operation),
                PHYLANX_MATCH_DATA(column_set_operation),
                PHYLANX_MATCH_DATA(column_slicing_operation),
                PHYLANX_MATCH_DATA(console_output),
                PHYLANX_MATCH_DATA(debug_output),
                PHYLANX_MATCH_DATA(hstack_operation),
                PHYLANX_MATCH_DATA(make_list),
                PHYLANX_MATCH_DATA(map_operation),
                PHYLANX_MATCH_DATA(parallel_block_operation),
                PHYLANX_MATCH_DATA(row_set_operation),
                PHYLANX_MATCH_DATA(row_slicing_operation),
                PHYLANX_MATCH_DATA(set_operation),
                PHYLANX_MATCH_DATA(slicing_operation),
                PHYLANX_MATCH_DATA(string_output),
                PHYLANX_MATCH_DATA(vstack_operation),
                // n-nary functions
                PHYLANX_MATCH_DATA(if_conditional),
                PHYLANX_MATCH_DATA(fold_left_operation),
                PHYLANX_MATCH_DATA(fold_right_operation),
                PHYLANX_MATCH_DATA(for_operation),
                PHYLANX_MATCH_DATA(linearmatrix),
                PHYLANX_MATCH_DATA(linspace),
                // binary functions
                PHYLANX_MATCH_DATA(apply),
                PHYLANX_MATCH_DATA(cross_operation),
                PHYLANX_MATCH_DATA(diag_operation),
                PHYLANX_MATCH_DATA(dot_operation),
                PHYLANX_MATCH_DATA(file_read),
                PHYLANX_MATCH_DATA(file_write),
                PHYLANX_MATCH_DATA(file_read_csv),
                PHYLANX_MATCH_DATA(file_write_csv),
                PHYLANX_MATCH_DATA(filter_operation),
                PHYLANX_MATCH_DATA(while_operation),
#if defined(PHYLANX_HAVE_HIGHFIVE)
                PHYLANX_MATCH_DATA(file_read_hdf5),
                PHYLANX_MATCH_DATA(file_write_hdf5),
#endif

                // unary functions
                PHYLANX_MATCH_DATA(all_operation),
                PHYLANX_MATCH_DATA(any_operation),
                PHYLANX_MATCH_DATA(argmin),
                PHYLANX_MATCH_DATA(argmax),
                PHYLANX_MATCH_DATA(constant),
                PHYLANX_MATCH_DATA(determinant),
                PHYLANX_MATCH_DATA(enable_tracing),
                PHYLANX_MATCH_DATA(exponential_operation),
                PHYLANX_MATCH_DATA(extract_shape),
                PHYLANX_MATCH_DATA(gradient_operation),
                PHYLANX_MATCH_DATA(identity),
                PHYLANX_MATCH_DATA(inverse_operation),
                PHYLANX_MATCH_DATA(mean_operation),
                PHYLANX_MATCH_DATA(power_operation),
                PHYLANX_MATCH_DATA(random),
                PHYLANX_MATCH_DATA(shuffle_operation),
                PHYLANX_MATCH_DATA(square_root_operation),
                PHYLANX_MATCH_DATA(sum_operation),
                PHYLANX_MATCH_DATA(transpose_operation),
                // variadic operations
                PHYLANX_MATCH_DATA(add_operation),
                PHYLANX_MATCH_DATA(and_operation),
                PHYLANX_MATCH_DATA(div_operation),
                PHYLANX_MATCH_DATA(add_dimension),
                PHYLANX_MATCH_DATA(mul_operation),
                PHYLANX_MATCH_DATA(or_operation),
                PHYLANX_MATCH_DATA(sub_operation),
                // binary operations
                PHYLANX_MATCH_DATA(equal),
                PHYLANX_MATCH_DATA(greater),
                PHYLANX_MATCH_DATA(greater_equal),
                PHYLANX_MATCH_DATA(less),
                PHYLANX_MATCH_DATA(less_equal),
                PHYLANX_MATCH_DATA(not_equal),
                PHYLANX_MATCH_DATA(store_operation),
                // unary operations
                PHYLANX_MATCH_DATA(unary_minus_operation),
                PHYLANX_MATCH_DATA(unary_not_operation),
                //
                // compiler-specific (internal) primitives
                //
                PHYLANX_MATCH_DATA(access_argument),
                PHYLANX_MATCH_DATA(function_reference),
                PHYLANX_MATCH_DATA(wrapped_function),
                PHYLANX_MATCH_DATA(define_function),
                PHYLANX_MATCH_DATA_VERBATIM(define_function::match_data_lambda),

                PHYLANX_MATCH_DATA(variable),
                PHYLANX_MATCH_DATA(wrapped_variable),
                PHYLANX_MATCH_DATA(define_variable),
                PHYLANX_MATCH_DATA_VERBATIM(define_variable::match_data_define)
            };

            std::string car_cdr_name("car_cdr");
            for (auto const& pattern : primitives::car_cdr_operation::match_data)
            {
                patterns.push_back(hpx::util::make_tuple(car_cdr_name, pattern));
            }

            // generic functions
            patterns.push_back(hpx::util::make_tuple(
                "get_seed_action", primitives::get_seed_match_data));
            patterns.push_back(hpx::util::make_tuple(
                "set_seed_action", primitives::set_seed_match_data));

            return patterns;
        }

#undef PHYLANX_MATCH_DATA_VERBATIM
#undef PHYLANX_MATCH_DATA
    }

    pattern_list const& get_all_known_patterns()
    {
        static pattern_list patterns = detail::get_all_known_patterns();
        return patterns;
    }
}}
