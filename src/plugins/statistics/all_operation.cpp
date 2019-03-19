//  Copyright (c) 2018 Shahrzad Shirzad
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>
#include <phylanx/plugins/statistics/all_operation.hpp>
#include <phylanx/plugins/statistics/statistics_base_impl.hpp>
#include <phylanx/util/blaze_traits.hpp>

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <limits>
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
        struct statistics_all_op
        {
            using result_type = std::uint8_t;

            statistics_all_op(std::string const& name,
                std::string const& codename)
            {}

            static constexpr bool initial()
            {
                return true;
            }

            template <typename Scalar>
            typename std::enable_if<traits::is_scalar<Scalar>::value, bool>::type
            operator()(Scalar s, bool initial) const
            {
                return s && initial;
            }

            template <typename Vector>
            typename std::enable_if<!traits::is_scalar<Vector>::value, T>::type
            operator()(Vector& v, bool initial) const
            {
                return initial && std::all_of(v.begin(), v.end(),
                    [](T val) -> bool
                    {
                        return val != 0;
                    });
            }

            static constexpr bool finalize(bool value, std::size_t size)
            {
                return value;
            }
        };
    }

    ///////////////////////////////////////////////////////////////////////////
    match_pattern_type const all_operation::match_data =
    {
        match_pattern_type{
            "all",
            std::vector<std::string>{
                "all(_1)", "all(_1, _2)", "all(_1, _2, _3)"
            },
            &create_all_operation, &create_primitive<all_operation>, R"(
            a, axis, keepdims, initial
            Args:

                arg (matrix or vector of numbers) : the input values

            Returns:

            True if all values in the matrix/vector are nonzero, False
            otherwise.)"
        }
    };

    ///////////////////////////////////////////////////////////////////////////
    all_operation::all_operation(primitive_arguments_type && operands,
            std::string const& name, std::string const& codename)
      : base_type(std::move(operands), name, codename)
    {
    }
}}}
