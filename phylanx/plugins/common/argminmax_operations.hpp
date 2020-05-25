//  Copyright (c) 2017-2020 Hartmut Kaiser
//  Copyright (c) 2017 Parsa Amini
//  Copyright (c) 2019 Bita Hasheminezhad
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(PHYLANX_COMMON_ARGMINMAX_OPERATIONS_2020_MAY_21_0352PM)
#define PHYLANX_COMMON_ARGMINMAX_OPERATIONS_2020_MAY_21_0352PM

#include <phylanx/config.hpp>
#include <phylanx/plugins/common/export_definitions.hpp>
#include <phylanx/util/detail/numeric_limits_min.hpp>

#include <algorithm>
#include <functional>
#include <limits>
#include <string>

///////////////////////////////////////////////////////////////////////////////
// explicitly instantiate the required functions
namespace phylanx { namespace common {

    ///////////////////////////////////////////////////////////////////////////
    struct argmax_op
    {
        template <typename T>
        static T initial()
        {
            return util::detail::numeric_limits_min<T>();
        }

        template <typename T, typename Comp = std::less<>>
        static bool compare(T lhs, T rhs, Comp comp = Comp{})
        {
            return comp(rhs, lhs);
        }

        template <typename Iter, typename Comp = std::less<>>
        Iter operator()(Iter begin, Iter end, Comp comp = Comp{}) const
        {
            return std::max_element(begin, end, comp);
        }
    };

    ///////////////////////////////////////////////////////////////////////////
    struct argmin_op
    {
        template <typename T>
        static T initial()
        {
            return (std::numeric_limits<T>::max)();
        }

        template <typename T, typename Comp = std::greater<>>
        static bool compare(T lhs, T rhs, Comp comp = Comp{})
        {
            return comp(rhs, lhs);
        }

        template <typename Iter, typename Comp = std::less<>>
        Iter operator()(Iter begin, Iter end, Comp comp = Comp{}) const
        {
            return std::min_element(begin, end, comp);
        }
    };
}}    // namespace phylanx::common

#endif
