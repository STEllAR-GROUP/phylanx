// Copyright (c) 2018-2020 Hartmut Kaiser
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>
#include <phylanx/plugins/statistics/var_operation.hpp>
#include <phylanx/plugins/statistics/statistics_base_impl.hpp>

#include <string>
#include <type_traits>
#include <utility>
#include <vector>

///////////////////////////////////////////////////////////////////////////////
namespace phylanx { namespace execution_tree { namespace primitives {

    ///////////////////////////////////////////////////////////////////////////
    match_pattern_type const var_operation::match_data =
    {
        match_pattern_type{
            "var",
            std::vector<std::string>{
                "var(_1)", "var(_1, _2)", "var(_1, _2, _3)"
            },
            &create_var_operation, &create_primitive<var_operation>, R"(
            v, axis, keepdims
            Args:

                v (vector or matrix) : a vector or matrix
                axis (optional, integer): a axis to sum along
                keepdims (optional, boolean): keep dimension of input

            Returns:

            The statistical variance of all values along the specified axis.)",
            true
        }
    };

    ///////////////////////////////////////////////////////////////////////////
    var_operation::var_operation(primitive_arguments_type&& operands,
            std::string const& name, std::string const& codename)
      : base_type(std::move(operands), name, codename)
    {
    }
}}}    // namespace phylanx::execution_tree::primitives
