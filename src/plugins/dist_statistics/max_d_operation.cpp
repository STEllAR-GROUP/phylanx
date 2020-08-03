// Copyright (c) 2018 Bita Hasheminezhad
// Copyright (c) 2018-2020 Hartmut Kaiser
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>
#include <phylanx/plugins/dist_statistics/dist_statistics_base_impl.hpp>
#include <phylanx/plugins/dist_statistics/max_d_operation.hpp>

#include <string>
#include <type_traits>
#include <utility>
#include <vector>

///////////////////////////////////////////////////////////////////////////////
namespace phylanx { namespace execution_tree { namespace primitives {

    ///////////////////////////////////////////////////////////////////////////
    match_pattern_type const max_d_operation::match_data = {
        match_pattern_type{"amax_d",
            std::vector<std::string>{"amax_d(_1)", "amax_d(_1, _2)",
                "amax_d(_1, _2, _3)", "amax_d(_1, _2, _3, _4)"},
            &create_amax_d_operation, &create_primitive<max_d_operation>, R"(
            a, axis, keepdims, initial
            Args:

                a (vector or matrix): a scalar, a vector or a matrix
                axis (optional, integer): an axis to max along. By default, "
                   flattened input is used.
                keepdims (optional, bool): If this is set to True, the axes which "
                   are reduced are left in the result as dimensions with size "
                   one. False by default
                initial (optional, scalar): The minimum value of an output
                   element.

            Returns:

            Returns the maximum of an array or maximum along an axis.)",
            true}};

    ///////////////////////////////////////////////////////////////////////////
    max_d_operation::max_d_operation(primitive_arguments_type&& operands,
        std::string const& name, std::string const& codename)
      : base_type(std::move(operands), name, codename)
    {
    }
}}}    // namespace phylanx::execution_tree::primitives
