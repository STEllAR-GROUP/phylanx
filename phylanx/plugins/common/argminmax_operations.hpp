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
#include <cstddef>
#include <cstdint>
#include <functional>
#include <limits>
#include <string>
#include <utility>

///////////////////////////////////////////////////////////////////////////////
// explicitly instantiate the required functions
namespace phylanx { namespace common {

    ///////////////////////////////////////////////////////////////////////////
    struct argmax_op
    {
        template <typename T>
        static constexpr T initial()
        {
            return util::detail::numeric_limits_min<T>();
        }

        static constexpr std::int64_t index_initial()
        {
            return (std::numeric_limits<std::int64_t>::max)();
        }

        template <typename T, typename Comp = std::less<>>
        static bool compare(T lhs, T rhs, Comp comp = Comp{})
        {
            return comp(rhs, lhs);
        }

        template <typename T, typename Comp = std::less<>>
        static bool index_compare(std::pair<T, std::size_t> const& lhs,
            std::pair<T, std::size_t> const& rhs, Comp comp = Comp{})
        {
            if (comp(rhs.first, lhs.first))
            {
                return true;
            }
            if (rhs.first == lhs.first)
            {
                // we seek for the smallest index that has the maximum value
                return comp(lhs.second, rhs.second);
            }
            return false;
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
        static constexpr T initial()
        {
            return (std::numeric_limits<T>::max)();
        }

        static constexpr std::int64_t index_initial()
        {
            return (std::numeric_limits<std::int64_t>::max)();
        }

        template <typename T, typename Comp = std::less<>>
        static bool compare(T lhs, T rhs, Comp comp = Comp{})
        {
            return comp(lhs, rhs);
        }

        template <typename T, typename Comp = std::less<>>
        static bool index_compare(std::pair<T, std::size_t> const& lhs,
            std::pair<T, std::size_t> const& rhs, Comp comp = Comp{})
        {
            if (comp(lhs.first, rhs.first))
            {
                return true;
            }
            if (lhs.first == rhs.first)
            {
                // we seek for the smallest index that has the minimum value
                return comp(lhs.second, rhs.second);
            }
            return false;
        }

        template <typename Iter, typename Comp = std::less<>>
        Iter operator()(Iter begin, Iter end, Comp comp = Comp{}) const
        {
            return std::min_element(begin, end, comp);
        }
    };
}}    // namespace phylanx::common

#endif
