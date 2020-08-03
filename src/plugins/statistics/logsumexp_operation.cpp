// Copyright (c) 2019 Bita Hasheminezhad
// Copyright (c) 2019 Hartmut Kaiser
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>
#include <phylanx/plugins/statistics/logsumexp_operation.hpp>
#include <phylanx/plugins/statistics/statistics_base_impl.hpp>

#include <string>
#include <type_traits>
#include <utility>
#include <vector>

///////////////////////////////////////////////////////////////////////////////
namespace phylanx { namespace execution_tree { namespace primitives {

    ///////////////////////////////////////////////////////////////////////////
    match_pattern_type const logsumexp_operation::match_data =
    {
        match_pattern_type{
            "logsumexp",
            std::vector<std::string>{
                "logsumexp(_1)", "logsumexp(_1, _2)", "logsumexp(_1, _2, _3)"
            },
            &create_logsumexp_operation, &create_primitive<logsumexp_operation>,
            R"(
            a, axis, keepdims
            Args:

                a (array) : a scalar, a vector, a matrix or a tensor
                axis (optional, integer): Axis over which the sum is taken.
                    By default axis is None, and all elements are summed.
                keepdims (optional, boolean): keep dimension of input

            Returns:

            The log of the sum of exponentials of input elements.)",
            true
        }
    };

    ///////////////////////////////////////////////////////////////////////////
    logsumexp_operation::logsumexp_operation(
        primitive_arguments_type&& operands, std::string const& name,
        std::string const& codename)
      : base_type(std::move(operands), name, codename)
    {
    }
}}}    // namespace phylanx::execution_tree::primitives
