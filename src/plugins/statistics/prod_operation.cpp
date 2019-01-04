// Copyright (c) 2018 Shahrzad Shirzad
// Copyright (c) 2018 Parsa Amini
// Copyright (c) 2018 Hartmut Kaiser
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>
#include <phylanx/plugins/statistics/prod_operation.hpp>
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
        struct statistics_prod_op
        {
            statistics_prod_op(std::string const& name,
                std::string const& codename)
            {}

            static constexpr T initial()
            {
                return T(1);
            }

            template <typename Scalar>
            typename std::enable_if<traits::is_scalar<Scalar>::value, T>::type
            operator()(Scalar s, T initial) const
            {
                return s * initial;
            }

            template <typename Vector>
            typename std::enable_if<!traits::is_scalar<Vector>::value, T>::type
            operator()(Vector const& v, T initial) const
            {
                return blaze::prod(v) * initial;
            }

            static T finalize(T value, std::size_t size)
            {
                return value;
            }
        };
    }

    ///////////////////////////////////////////////////////////////////////////
    match_pattern_type const prod_operation::match_data =
    {
        match_pattern_type{
            "prod",
            std::vector<std::string>{
                "prod(_1)", "prod(_1, _2)", "prod(_1, _2, _3)",
                "prod(_1, _2, _3, _4)"
            },
            &create_prod_operation, &create_primitive<prod_operation>, R"(
            v, axis, keepdims
            Args:

                v (vector or matrix) : a vector or matrix
                axis (optional, integer): a axis to sum along
                keepdims (optional, boolean): keep dimension of input

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
