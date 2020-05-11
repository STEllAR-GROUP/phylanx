// Copyright (c) 2020 Nanmiao Wu
// Copyright (c) 2020 Bita Hasheminezhad
// Copyright (c) 2020 Hartmut Kaiser
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(PHYLANX_DIST_DIAG_IMPL)
#define PHYLANX_DIST_DIAG_IMPL


#include <phylanx/config.hpp>
#include <phylanx/execution_tree/annotation.hpp>
#include <phylanx/execution_tree/localities_annotation.hpp>
#include <phylanx/execution_tree/locality_annotation.hpp>
#include <phylanx/execution_tree/meta_annotation.hpp>
#include <phylanx/execution_tree/primitives/annotate_primitive.hpp>
#include <phylanx/execution_tree/primitives/node_data_helpers.hpp>
#include <phylanx/execution_tree/tiling_annotations.hpp>
#include <phylanx/ir/node_data.hpp>
#include <phylanx/plugins/dist_matrixops/dist_diag.hpp>
#include <phylanx/plugins/dist_matrixops/idx_tile_calculation_helper.hpp>
#include <phylanx/plugins/dist_matrixops/tile_calculation_helper.hpp>
#include <phylanx/util/distributed_matrix.hpp>
#include <phylanx/util/distributed_vector.hpp>

#include <hpx/assertion.hpp>
#include <hpx/errors/throw_exception.hpp>
#include <hpx/include/lcos.hpp>
#include <hpx/include/naming.hpp>
#include <hpx/include/util.hpp>

#include <cstddef>
#include <cstdint>
#include <memory>
#include <string>
#include <tuple>
#include <utility>
#include <vector>

#include <blaze/Math.h>

namespace phylanx { namespace dist_matrixops { namespace primitives {
    ///////////////////////////////////////////////////////////////////////////
    template <typename T>
    execution_tree::primitive_argument_type dist_diag::diag_1d_helper(
       ir::node_data<T>&& arr, std::int64_t k,
       execution_tree::localities_information&& arr_localities,
       std::int64_t row_start, std::int64_t column_start,
       std::size_t row_size, std::size_t column_size,
       std::int64_t des_start, std::int64_t des_size,
       std::int64_t num_band) const
   {
        using namespace execution_tree;

        std::uint32_t const loc_id = arr_localities.locality_.locality_id_;
        std::uint32_t const num_localities =
            arr_localities.locality_.num_localities_;

        tiling_information_1d tile_info_input(
            arr_localities.tiles_[loc_id], name_, codename_);

        std::int64_t cur_start = tile_info_input.span_.start_;
        std::int64_t cur_stop = tile_info_input.span_.stop_;

        arr_localities.annotation_.name_ += "_diag";
        ++arr_localities.annotation_.generation_;

        std::size_t span_index = 0;
        if (!arr_localities.has_span(0))
        {
            HPX_ASSERT(arr_localities.has_span(1));
            span_index = 1;
        }

        blaze::DynamicVector<T> res_arr(des_size);
        blaze::DynamicMatrix<T> result(row_size, column_size, T(0));

        std::int64_t des_stop = des_size + des_start;
        std::int64_t rel_start = des_start - cur_start;

        auto v = arr.vector();
        util::distributed_vector<T> v_data(
            name_, v, num_localities, loc_id);

        if (des_start < cur_start || des_stop > cur_stop)
        {
            // there is a need to fetch some par
            // copying the local part
            if (des_start < cur_stop && des_stop > cur_start)
            {
                auto indices = calculation_detail::index_calculation_1d(
                    des_start, des_stop, cur_start, cur_stop);
                blaze::subvector(res_arr, indices.projected_start_,
                    indices.intersection_size_) = blaze::subvector(v,
                        indices.local_start_,
                        indices.intersection_size_);
            // collecting other parts from remote localities
            for (std::uint32_t loc = 0; loc != num_localities; ++loc)
            {
                if (loc == loc_id)
                {
                    continue;
                // the array span in for loc-th locality
                tiling_span loc_span =
                    arr_localities.tiles_[loc].spans_[span_index];
                auto indices = calculation_detail::retile_calculation_1d(
                    loc_span, des_start, des_stop);
                if (indices.intersection_size_ > 0)
                {
                    // loc_span has the part of result that we need
                    blaze::subvector(res_arr, indices.projected_start_,
                        indices.intersection_size_) =
                        v_data
                        .fetch(loc, indices.local_start_,
                            indices.local_start_ +
                            indices.intersection_size_)
                        .get();
                }
            blaze::band(result, num_band) = res_arr;
        }
        else // the new array is a subset of the original array
        {
            blaze::band(result, num_band) =
                blaze::subvector(v, rel_start, des_size);
        }

        tiling_information_2d tile_info(
             tiling_span(row_start, row_start + row_size),
             tiling_span(column_start, column_start + column_size));
 
        auto locality_ann = arr_localities.locality_.as_annotation();
        auto attached_annotation =
            std::make_shared<annotation>(localities_annotation(locality_ann,
                tile_info.as_annotation(name_, codename_),
                arr_localities.annotation_, name_, codename_));

        return primitive_argument_type(result, attached_annotation);
    }
    ///////////////////////////////////////////////////////////////////////////
    template <typename T>
    execution_tree::primitive_argument_type dist_diag::dist_diag1d(
        ir::node_data<T>&& arr, std::int64_t k, std::string const& tiling_type,
        std::uint32_t tile_idx, std::uint32_t numtiles,
        execution_tree::localities_information&& arr_localities) const
    {
        using namespace execution_tree;

        std::size_t dim = arr_localities.size();    // size of the whole array

        std::size_t size = dim + std::abs(k);

        std::int64_t row_start, column_start;
        std::size_t row_size, column_size;

        std::tie(row_start, column_start, row_size, column_size) =
            tile_calculation::tile_calculation_2d(
                tile_idx, size, size, numtiles, tiling_type);

        blaze::DynamicMatrix<T> result(row_size, column_size, T(0));

        std::int64_t num_band;
        std::int64_t upper_band = column_size - 1;
        std::int64_t lower_band = 1 - row_size;

        std::int64_t des_start, des_stop, des_size;

        if (tiling_type == "row")
        {
            num_band = row_start + k;
            if (num_band <= (std::max)(int64_t(0), upper_band) &&
                    num_band >= (std::min)(int64_t(0), lower_band))
            {
                des_start = k < 0 ?
                    (std::max)(int64_t(0), num_band) : row_start;

                des_size = num_band < 0 ?
                    (std::min)(column_size, row_size + num_band) :
                    (std::min)(row_size, column_size - num_band);

                return diag_1d_helper(
                    std::move(arr), k, std::move(arr_localities),
                    row_start, column_start, row_size, column_size,
                    des_start, des_size, num_band);

            }
        }
        else if (tiling_type == "column")
        {
            num_band = k - column_start;
            if (num_band <= (std::max)(int64_t(0), upper_band) &&
                    num_band >= (std::min)(int64_t(0), lower_band))
            {
                des_start = k < 0 ?
                    column_start : (std::max)(int64_t(0), - num_band);

                des_size = num_band < 0 ?
                    (std::min)(column_size, row_size + num_band) :
                    (std::min)(row_size, column_size - num_band);

                return diag_1d_helper(
                    std::move(arr), k, std::move(arr_localities),
                    row_start, column_start, row_size, column_size,
                    des_start, des_size, num_band);

            }
        }
        else if (tiling_type == "sym")
        {
            num_band = k - (column_start - row_start);
            if (num_band <= (std::max)(int64_t(0), upper_band) &&
                    num_band >= (std::min)(int64_t(0), lower_band))
            {
                des_start = k < 0 ?
                    (std::max)(column_start, column_start + num_band) :
                    (std::max)(row_start, row_start - num_band);

                des_size = num_band < 0 ?
                    (std::min)(column_size, row_size + num_band) :
                    (std::min)(row_size, column_size - num_band);

                return diag_1d_helper(
                    std::move(arr), k, std::move(arr_localities),
                    row_start, column_start, row_size, column_size,
                    des_start, des_size, num_band);
            }
        }
        else
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "dist_diag::dist_diag1d",
                generate_error_message(
                    "wrong numtiles input when tiling_type is column"));
        }

        tiling_information_2d tile_info(
            tiling_span(row_start, row_start + row_size),
            tiling_span(column_start, column_start + column_size));

        auto locality_ann = arr_localities.locality_.as_annotation();
        auto attached_annotation =
            std::make_shared<annotation>(localities_annotation(locality_ann,
                tile_info.as_annotation(name_, codename_),
                arr_localities.annotation_, name_, codename_));

        return primitive_argument_type(result, attached_annotation);
    }

}}}    // namespace phylanx::dist_matrixops::primitives

#endif