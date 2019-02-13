// Copyright (c) 2018 Parsa Amini
// Copyright (c) 2018 Hartmut Kaiser
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>
#include <phylanx/plugins/statistics/sum_operation.hpp>
#include <phylanx/plugins/statistics/statistics_base_impl.hpp>
#include <phylanx/util/blaze_traits.hpp>

#include <cstddef>
#include <functional>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

#include <blaze/Math.h>
#if defined(PHYLANX_HAVE_BLAZE_TENSOR)
#include <blaze_tensor/Math.h>
#endif

///////////////////////////////////////////////////////////////////////////////
namespace phylanx { namespace execution_tree { namespace primitives
{
    ///////////////////////////////////////////////////////////////////////////
    namespace detail
    {
        template <typename T>
        struct statistics_sum_op
        {
            statistics_sum_op(std::string const& name,
                std::string const& codename)
            {}

            static constexpr T initial()
            {
                return T(0);
            }

            template <typename Scalar>
            typename std::enable_if<traits::is_scalar<Scalar>::value, T>::type
            operator()(Scalar s, T initial) const
            {
                return s + initial;
            }

            template <typename Vector>
            typename std::enable_if<!traits::is_scalar<Vector>::value, T>::type
            operator()(Vector const& v, T initial) const
            {
                return blaze::sum(v) + initial;
            }

            static T finalize(T value, std::size_t size)
            {
                return value;
            }
        };
    }

    ///////////////////////////////////////////////////////////////////////////
    match_pattern_type const sum_operation::match_data =
    {
        match_pattern_type{
            "sum",
            std::vector<std::string>{
                "sum(_1)", "sum(_1, _2)", "sum(_1, _2, _3)",
                "sum(_1, _2, _3, _4)"
            },
            &create_sum_operation, &create_primitive<sum_operation>, R"(
            v, axis, keepdims, initial
            Args:

                v (vector or matrix) : a vector or matrix
                axis (optional, integer): a axis to sum along
                keepdims (optional, boolean): keep dimension of input
                initial (optional, scalar): The starting value for the sum

            Returns:

            The sum of all values along the specified axis.)",
            true
        }
    };

    ///////////////////////////////////////////////////////////////////////////
    sum_operation::sum_operation(primitive_arguments_type&& operands,
            std::string const& name, std::string const& codename)
      : base_type(std::move(operands), name, codename)
    {
    }
}}}
