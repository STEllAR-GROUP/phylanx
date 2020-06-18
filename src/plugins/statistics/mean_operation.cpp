// Copyright (c) 2018 Monil, Mohammad Alaul Haque
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>
#include <phylanx/plugins/statistics/mean_operation.hpp>
#include <phylanx/plugins/statistics/statistics_base_impl.hpp>

#include <string>
#include <type_traits>
#include <utility>
#include <vector>

///////////////////////////////////////////////////////////////////////////////
namespace phylanx { namespace execution_tree { namespace primitives {

    ///////////////////////////////////////////////////////////////////////////
    match_pattern_type const mean_operation::match_data =
    {
        match_pattern_type{
            "mean",
            std::vector<std::string>{
                "mean(_1, _2, _3)", "mean(_1, _2)", "mean(_1)"
            },
            &create_mean_operation,
            &create_primitive<mean_operation>, R"(
            ar, axis, keepdims
            Args:

                ar (array) : an array of values
                axis (optional, int) : the axis along which to calculate the mean
                keepdims (optional, boolean): keep dimension of input

            Returns:

            The mean of the array. If an axis is specified, the result is the
            vector created when the mean is taken along the specified axis.)",
            true
        }
    };

    ///////////////////////////////////////////////////////////////////////////
    mean_operation::mean_operation(primitive_arguments_type&& operands,
        std::string const& name, std::string const& codename)
      : base_type(std::move(operands), name, codename)
    {
    }

}}}    // namespace phylanx::execution_tree::primitives
