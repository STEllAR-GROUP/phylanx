// Copyright (c) 2018 Bita Hasheminezhad
// Copyright (c) 2018-2020 Hartmut Kaiser
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>
#include <phylanx/plugins/statistics/max_operation.hpp>
#include <phylanx/plugins/statistics/statistics_base_impl.hpp>

#include <string>
#include <type_traits>
#include <utility>
#include <vector>

///////////////////////////////////////////////////////////////////////////////
namespace phylanx { namespace execution_tree { namespace primitives {

    ///////////////////////////////////////////////////////////////////////////
    match_pattern_type const max_operation::match_data =
    {
        match_pattern_type{
            "amax",
            std::vector<std::string>{
                "amax(_1)", "amax(_1, _2)", "amax(_1, _2, _3)",
                "amax(_1, _2, _3, _4)"
            },
            &create_amax_operation, &create_primitive<max_operation>, R"(
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
            true
        }
    };

    ///////////////////////////////////////////////////////////////////////////
    max_operation::max_operation(primitive_arguments_type&& operands,
            std::string const& name, std::string const& codename)
      : base_type(std::move(operands), name, codename)
    {
    }
}}}
