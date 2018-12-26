// Copyright (c) 2018 Shahrzad Shirzad
// Copyright (c) 2018 Parsa Amini
// Copyright (c) 2018 Hartmut Kaiser
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>
#include <phylanx/plugins/arithmetics/prod_operation.hpp>
#include <phylanx/plugins/arithmetics/statistics_impl.hpp>

#include <algorithm>
#include <functional>
#include <string>
#include <utility>
#include <vector>

///////////////////////////////////////////////////////////////////////////////
namespace phylanx { namespace execution_tree { namespace primitives
{
    ///////////////////////////////////////////////////////////////////////////
    namespace detail
    {
        struct statistics_prod_op
        {
            template <typename T>
            static constexpr T initial()
            {
                return T(1);
            }

            template <typename Iter, typename T>
            T operator()(Iter b, Iter e, T initial) const
            {
                return std::accumulate(b, e, initial, std::multiplies<T>());
            }
        };
    }

    ///////////////////////////////////////////////////////////////////////////
    match_pattern_type const prod_operation::match_data =
    {
        match_pattern_type{
            "prod",
            std::vector<std::string>{
                "prod(_1)", "prod(_1, _2)", "prod(_1, _2, _3)"},
            &create_prod_operation, &create_primitive<prod_operation>, R"(
            v, axis, keep_dim
            Args:

                v (vector or matrix) : a vector or matrix
                axis (optional, integer): a axis to sum along
                keep_dim (optional, boolean): keep dimension of input

            Returns:

            The product of all values along the specified axis.)",
            true
        }
    };

    ///////////////////////////////////////////////////////////////////////////
    prod_operation::prod_operation(primitive_arguments_type&& operands,
            std::string const& name, std::string const& codename)
      : base_type(std::move(operands), name, codename)
    {
    }
}}}
