//  Copyright (c) 2018 Shahrzad Shirzad
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>
#include <phylanx/plugins/statistics/any_operation.hpp>
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
#include <blaze_tensor/Math.h>

///////////////////////////////////////////////////////////////////////////////
namespace phylanx { namespace execution_tree { namespace primitives
{
    ///////////////////////////////////////////////////////////////////////////
    namespace detail
    {
        template <typename T>
        struct statistics_any_op
        {
            using result_type = std::uint8_t;

            statistics_any_op(std::string const& name,
                std::string const& codename)
            {}

            static constexpr bool initial()
            {
                return false;
            }

            template <typename Scalar>
            typename std::enable_if<traits::is_scalar<Scalar>::value, bool>::type
            operator()(Scalar s, bool initial) const
            {
                return s || initial;
            }

            template <typename Vector>
            typename std::enable_if<!traits::is_scalar<Vector>::value, T>::type
            operator()(Vector& v, bool initial) const
            {
                return initial || std::any_of(v.begin(), v.end(),
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
}}}
