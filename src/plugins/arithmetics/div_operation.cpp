// Copyright (c) 2017-2018 Hartmut Kaiser
// Copyright (c) 2018 Parsa Amini
// Copyright (c) 2018 Shahrzad Shirzad
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>
#include <phylanx/plugins/arithmetics/div_operation.hpp>
#include <phylanx/plugins/arithmetics/numeric_impl.hpp>
#include <phylanx/util/detail/div_simd.hpp>
#include <phylanx/util/blaze_traits.hpp>

#include <string>
#include <type_traits>
#include <utility>
#include <vector>

#include <blaze/Math.h>

///////////////////////////////////////////////////////////////////////////////
namespace phylanx { namespace execution_tree { namespace primitives
{
    ///////////////////////////////////////////////////////////////////////////
    namespace detail
    {
        // 't1 / t2' is defined for scalars and vectors, but not for matrices
        struct div_op
        {
            ///////////////////////////////////////////////////////////////////
            template <typename T1, typename T2>
            typename std::enable_if<
                !traits::is_matrix<T1>::value || !traits::is_matrix<T2>::value,
                decltype(std::declval<T1>() / std::declval<T2>())
            >::type
            operator()(T1 const& t1, T2 const& t2) const
            {
                return t1 / t2;
            }

            template <typename T1, typename T2>
            typename std::enable_if<
                traits::is_matrix<T1>::value && traits::is_matrix<T2>::value,
                decltype(blaze::map(std::declval<T1>(), std::declval<T2>(),
                    util::detail::divndnd_simd{}))
            >::type
            operator()(T1 const& t1, T2 const& t2) const
            {
                return blaze::map(t1, t2, util::detail::divndnd_simd{});
            }

            ///////////////////////////////////////////////////////////////////
            template <typename T1, typename T2>
            typename std::enable_if<
                !traits::is_matrix<T1>::value || !traits::is_matrix<T2>::value
            >::type
            op_assign(T1& t1, T2 const& t2) const
            {
                t1 /= t2;
            }

            template <typename T1, typename T2>
            typename std::enable_if<
                traits::is_matrix<T1>::value && traits::is_matrix<T2>::value
            >::type
            op_assign(T1& t1, T2 const& t2) const
            {
                t1 = blaze::map(t1, t2, util::detail::divndnd_simd{});
            }
        };
    }

    ///////////////////////////////////////////////////////////////////////////
    match_pattern_type const div_operation::match_data =
    {
        match_pattern_type{"__div",
            std::vector<std::string>{"_1 / __2", "__div(_1, __2)"},
            &create_div_operation, &create_primitive<div_operation>, R"(
            num, den
            Args:

                 x0 (number): The numerator value\n"
                *x1 (number list): A list of denominator values.\n"

            Returns:

            The result of dividing all arguments.)",
            true
        }
    };

    ///////////////////////////////////////////////////////////////////////////
    div_operation::div_operation(primitive_arguments_type&& operands,
            std::string const& name, std::string const& codename)
      : base_type(std::move(operands), name, codename)
    {}
}}}
