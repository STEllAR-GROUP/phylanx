// Copyright (c) 2018 Parsa Amini
// Copyright (c) 2018-2020 Hartmut Kaiser
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>
#include <phylanx/plugins/statistics/sum_operation.hpp>
#include <phylanx/plugins/statistics/statistics_base_impl.hpp>

#include <string>
#include <type_traits>
#include <utility>
#include <vector>

///////////////////////////////////////////////////////////////////////////////
namespace phylanx { namespace execution_tree { namespace primitives {

    ///////////////////////////////////////////////////////////////////////////
    match_pattern_type const sum_operation::match_data =
    {
        match_pattern_type{
            "sum",
            std::vector<std::string>{
                "sum(_1, __arg(_2_axis, nil), __arg(_3_keepdims, nil), "
                    "__arg(_4_initial, nil), __arg(_5_dtype, nil))"
            },
            &create_sum_operation, &create_primitive<sum_operation>, R"(
            v, axis, keepdims, initial, dtype
            Args:

                v (vector or matrix) : a vector or matrix
                axis (optional, integer): a axis to sum along
                keepdims (optional, boolean): keep dimension of input
                initial (optional, scalar): The starting value for the sum
                dtype (optional, string) : the data-type of the returned array,
                  defaults to dtype of input array.

            Returns:

            The sum of all values along the specified axis.)"
        }
    };

    ///////////////////////////////////////////////////////////////////////////
    sum_operation::sum_operation(primitive_arguments_type&& operands,
            std::string const& name, std::string const& codename)
      : base_type(std::move(operands), name, codename)
    {
    }
}}}    // namespace phylanx::execution_tree::primitives
