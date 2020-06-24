// Copyright (c) 2020 Bita Hasheminezhad
// Copyright (c) 2020 Hartmut Kaiser
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(PHYLANX_DIST_MATRIXOPS_RETILING_CALCULATION_HELPER)
#define PHYLANX_DIST_MATRIXOPS_RETILING_CALCULATION_HELPER

#include <phylanx/execution_tree/localities_annotation.hpp>
#include <phylanx/execution_tree/meta_annotation.hpp>
#include <phylanx/ir/node_data.hpp>
#include <phylanx/util/distributed_tensor.hpp>
#include <phylanx/util/index_calculation_helper.hpp>


#include <cstddef>
#include <cstdint>
#include <string>

#include <blaze/Math.h>

namespace retiling_calculation
{
    ///////////////////////////////////////////////////////////////////////////
    template <typename T>
    inline blaze::DynamicTensor<T> retile3d_calculation(
        phylanx::ir::node_data<T>&& arr,
        phylanx::execution_tree::localities_information&& arr_localities,
        std::int64_t des_page_start, std::int64_t des_page_stop,
        std::int64_t des_row_start, std::int64_t des_row_stop,
        std::int64_t des_col_start, std::int64_t des_col_stop,
        std::string const& name, std::string const& codename)
    {
        using namespace phylanx::execution_tree;

        std::uint32_t const loc_id = arr_localities.locality_.locality_id_;
        std::uint32_t const num_localities =
            arr_localities.locality_.num_localities_;
        tiling_information_3d tile_info(
            arr_localities.tiles_[loc_id], name, codename);

        std::int64_t cur_page_start = tile_info.spans_[0].start_;
        std::int64_t cur_page_stop = tile_info.spans_[0].stop_;
        std::int64_t cur_row_start = tile_info.spans_[1].start_;
        std::int64_t cur_row_stop = tile_info.spans_[1].stop_;
        std::int64_t cur_col_start = tile_info.spans_[2].start_;
        std::int64_t cur_col_stop = tile_info.spans_[2].stop_;

        std::size_t des_page_size = des_page_stop - des_page_start;
        std::size_t des_row_size = des_row_stop - des_row_start;
        std::size_t des_col_size = des_col_stop - des_col_start;

        // updating the array
        auto t = arr.tensor();
        blaze::DynamicTensor<T> result(
            des_page_size, des_row_size, des_col_size);
        phylanx::util::distributed_tensor<T> t_data(
            arr_localities.annotation_.name_, t, num_localities, loc_id);

        // relative starts
        std::int64_t rel_page_start = des_page_start - cur_page_start;
        std::int64_t rel_row_start = des_row_start - cur_row_start;
        std::int64_t rel_col_start = des_col_start - cur_col_start;
        if ((rel_page_start < 0 || des_page_stop > cur_page_stop) ||
            (rel_row_start < 0 || des_row_stop > cur_row_stop) ||
            (rel_col_start < 0 || des_col_stop > cur_col_stop))
        {
            // copying the local part
            if ((des_page_start < cur_page_stop &&
                    des_page_stop > cur_page_start) &&
                (des_row_start < cur_row_stop &&
                    des_row_stop > cur_row_start) &&
                (des_col_start < cur_col_stop && des_col_stop > cur_col_start))
            {
                auto page_indices =
                    phylanx::util::index_calculation_1d(des_page_start,
                        des_page_stop, cur_page_start, cur_page_stop);
                auto row_indices = phylanx::util::index_calculation_1d(
                    des_row_start, des_row_stop, cur_row_start, cur_row_stop);
                auto col_indices = phylanx::util::index_calculation_1d(
                    des_col_start, des_col_stop, cur_col_start, cur_col_stop);

                blaze::subtensor(result, page_indices.projected_start_,
                    row_indices.projected_start_, col_indices.projected_start_,
                    page_indices.intersection_size_,
                    row_indices.intersection_size_,
                    col_indices.intersection_size_) = blaze::subtensor(t,
                    page_indices.local_start_, row_indices.local_start_,
                    col_indices.local_start_, page_indices.intersection_size_,
                    row_indices.intersection_size_,
                    col_indices.intersection_size_);
            }

            for (std::uint32_t loc = 0; loc != num_localities; ++loc)
            {
                if (loc == loc_id)
                {
                    continue;
                }

                // the array span in locality loc
                tiling_span loc_page_span =
                    arr_localities.tiles_[loc].spans_[0];
                tiling_span loc_row_span = arr_localities.tiles_[loc].spans_[1];
                tiling_span loc_col_span = arr_localities.tiles_[loc].spans_[2];

                auto page_indices = phylanx::util::retile_calculation_1d(
                    loc_page_span, des_page_start, des_page_stop);
                auto row_indices = phylanx::util::retile_calculation_1d(
                    loc_row_span, des_row_start, des_row_stop);
                auto col_indices = phylanx::util::retile_calculation_1d(
                    loc_col_span, des_col_start, des_col_stop);
                if (page_indices.intersection_size_ > 0 &&
                    row_indices.intersection_size_ > 0 &&
                    col_indices.intersection_size_ > 0)
                {
                    // loc_span has the block of result that we need
                    blaze::subtensor(result, page_indices.projected_start_,
                        row_indices.projected_start_,
                        col_indices.projected_start_,
                        page_indices.intersection_size_,
                        row_indices.intersection_size_,
                        col_indices.intersection_size_) =
                        t_data
                            .fetch(loc, page_indices.local_start_,
                                row_indices.local_start_,
                                col_indices.local_start_,
                                page_indices.local_start_ +
                                    page_indices.intersection_size_,
                                row_indices.local_start_ +
                                    row_indices.intersection_size_,
                                col_indices.local_start_ +
                                    col_indices.intersection_size_)
                            .get();
                }
            }
        }
        else    // the new array is a subset of the original array
        {
            result = blaze::subtensor(t, rel_page_start, rel_row_start,
                rel_col_start, des_page_size, des_row_size, des_col_size);
        }

        return std::move(result);
    }
}
#endif
