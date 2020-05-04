// Copyright (c) 2020 Hartmut Kaiser
// Copyright (c) 2020 Bita Hasheminezhad

//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(PHYLANX_DIST_MATRIXOPS_IDX_TILE_CALCULATION_HELPER)
#define PHYLANX_DIST_MATRIXOPS_IDX_TILE_CALCULATION_HELPER

#include <hpx/assertion.hpp>

#include <cstddef>
#include <cstdint>
#include <string>
#include <tuple>

#include <blaze/Math.h>


namespace phylanx { namespace dist_matrixops { namespace calculation_detail
{
    struct indices_pack
    {
        std::int64_t intersection_size_;
        std::int64_t projected_start_;
        std::int64_t local_start_;
    };

    inline indices_pack index_calculation_1d(std::int64_t des_start,
        std::int64_t des_stop, std::int64_t cur_start,
        std::int64_t cur_stop)
    {
        std::int64_t local_intersection_size =
            (std::min)(des_stop, cur_stop) -
            (std::max)(des_start, cur_start);
        std::int64_t rel_start = des_start - cur_start;
        return indices_pack{local_intersection_size,
            rel_start < 0 ? -rel_start : 0, rel_start > 0 ? rel_start : 0};
    }

    inline indices_pack retile_calculation_1d(
        execution_tree::tiling_span const& loc_span,
        std::int64_t des_start, std::int64_t des_stop)
    {
        execution_tree::tiling_span span(des_start, des_stop);
        execution_tree::tiling_span intersection;
        if (intersect(span, loc_span, intersection))
        {
            std::int64_t rel_loc_start =
                intersection.start_ - loc_span.start_;
            HPX_ASSERT(rel_loc_start >= 0);
            return indices_pack{intersection.size(),
                intersection.start_ - des_start, rel_loc_start};
        }
        return indices_pack{0, 0, 0};
    }
}}}

#endif

