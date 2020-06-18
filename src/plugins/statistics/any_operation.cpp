//  Copyright (c) 2018 Shahrzad Shirzad
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>
#include <phylanx/plugins/statistics/any_operation.hpp>
#include <phylanx/plugins/statistics/statistics_base_impl.hpp>

#include <string>
#include <type_traits>
#include <utility>
#include <vector>

///////////////////////////////////////////////////////////////////////////////
namespace phylanx { namespace execution_tree { namespace primitives {

    ///////////////////////////////////////////////////////////////////////////
    match_pattern_type const any_operation::match_data =
    {
        match_pattern_type{
            "any",
            std::vector<std::string>{
                "any(_1)", "any(_1, _2)", "any(_1, _2, _3)"
            },
            &create_any_operation, &create_primitive<any_operation>, R"(
            a, axis, keepdims, initial
            Args:

                arg (matrix or vector of numbers) : the input values

            Returns:

            True if any values in the matrix/vector are nonzero, False
            otherwise.)"
        }
    };

    ///////////////////////////////////////////////////////////////////////////
    any_operation::any_operation(primitive_arguments_type && args,
            std::string const& name, std::string const& codename)
      : base_type(std::move(args), name, codename)
    {
    }
}}}    // namespace phylanx::execution_tree::primitives
