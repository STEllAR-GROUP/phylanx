// Copyright (c) 2020 Hartmut Kaiser
// Copyright (c) 2020 Bita Hasheminezhad
// Copyright (c) 2020 Nanmiao Wu
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

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

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
namespace phylanx { namespace dist_matrixops { namespace primitives {
    ///////////////////////////////////////////////////////////////////////////
    execution_tree::match_pattern_type const dist_diag::match_data = 
    {
        hpx::util::make_tuple("diag_d", std::vector<std::string>{R"(
                diag_d(
                    _1_arr,
                    __arg(_2_k, 0),
                    __arg(_3_tiling_type, "sym"),
                    __arg(_4_tile_index, find_here()),
                    __arg(_5_numtiles, num_localities())
                )
            )"},
            &create_dist_diag,
            &execution_tree::create_primitive<dist_diag>, R"(
            arr, k, tiling_type, tile_index, numtiles
            Args:
                arr (array): a distributed array. A vector or a matrix.
                k (int, optional): The default is 0. Denote the diagonal above
                    the main diagonal when k > 0 and the diagonal below the main
                    diagonal when k < 0.
                tiling_type (string, optional): defaults to `sym`. Other
                    options are `row` or `column` tiling.
                tile_index (int, optional): the tile index we need to generate
                    the diag array for. A non-negative integer. If not
                    given, it sets to current locality.
                numtiles (int, optional): number of tiles of the returned array.
                    If not given it sets to the number of localities in the
                    application.
            Returns:

            A 1-D array of its k-th diagonal when a is a 2-D array; a 2-D array
            with a on the k-th diagonal.)")
    };

    ///////////////////////////////////////////////////////////////////////////
    dist_diag::dist_diag(
        execution_tree::primitive_arguments_type&& operands,
        std::string const& name, std::string const& codename)
      : primitive_component_base(std::move(operands), name, codename)
<<<<<<< HEAD
    {}

    ///////////////////////////////////////////////////////////////////////////

    namespace detail
    {
        //static std::atomic<std::size_t> diag_count(0);
        //std::string generate_diag_name(std::string&& given_name)
        //{
        //    if (given_name.empty())
        //    {
        //        return "diag_array_" + std::to_string(++diag_count);
        //    }
//
        //    return std::move(given_name);
        //}
//
        struct indices_pack
        {
            std::int64_t intersection_size_;
            std::int64_t projected_start_;
            std::int64_t local_start_;
        };

        indices_pack diag_index_calculation_1d(std::int64_t des_start,
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

        indices_pack diag_retile_calculation_1d(
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
    }    // namespace detail



    ///////////////////////////////////////////////////////////////////////////
    template <typename T>
    execution_tree::primitive_argument_type dist_diag::dist_diag1d(
        ir::node_data<T>&& arr, std::int64_t k, std::string const& tiling_type,
        std::uint32_t const& tile_idx, std::uint32_t const& numtiles,
        execution_tree::localities_information&& arr_localities) const
    {
        using namespace execution_tree;

        // current annotation information
        std::uint32_t const loc_id = arr_localities.locality_.locality_id_;
        std::uint32_t const num_localities =
            arr_localities.locality_.num_localities_;

        // size of the whole array
        std::size_t dim = arr_localities.size();

        tiling_information_1d tile_info_input(
            arr_localities.tiles_[loc_id], name_, codename_);

        std::int64_t cur_start = tile_info_input.span_.start_;
        std::int64_t cur_stop = tile_info_input.span_.stop_;


        // updating the annotation_ part of localities annotation
        arr_localities.annotation_.name_ += "_diag";
        ++arr_localities.annotation_.generation_;


        // is it a row vector or a column vector?
        std::size_t span_index = 0;
        if (!arr_localities.has_span(0))
        {
            HPX_ASSERT(arr_localities.has_span(1));
            span_index = 1;
        }

        std::size_t size = arr.dimension(0) + std::abs(k);

        std::int64_t row_start, column_start;
        std::tie(row_start, column_start, row_size, column_size) =
            tile_calculation::tile_calculation_2d(
                tile_idx, size, size, numtiles, tiling_type);

        blaze::DynamicMatrix<T> result(row_size, column_size, T(0));

        auto v = arr.vector();

        util::distributed_vector<T> v_data(
            arr_localities.annotation_.name_, v, num_localities, loc_id);

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

                blaze::DynamicVector<T> res_arr(des_size);

                des_stop = des_size + des_start;

                if (des_start < cur_start || des_stop > cur_stop)
                {
                    // there is a need to fetch some part

                    // copying the local part
                    if (des_start < cur_stop && des_stop > cur_start)
                    {
                        auto indices = detail::diag_index_calculation_1d(
                            des_start, des_stop, cur_start, cur_stop);
                        blaze::subvector(res_arr, indices.projected_start_,
                            indices.intersection_size_) = blaze::subvector(v,
                                indices.local_start_,
                                indices.intersection_size_);
                    }

                    // collecting other parts from remote localities
                    for (std::uint32_t loc = 0; loc != num_localities; ++loc)
                    {
                        if (loc == loc_id)
                        {
                            continue;
                        }

                        // the array span in for loc-th locality
                        tiling_span loc_span =
                            arr_localities.tiles_[loc].spans_[span_index];

                        auto indices = detail::diag_retile_calculation_1d(
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
                    }

                    blaze::band(result, num_band) = res_arr;
                }

                else // the new array is a subset of the original array
                {
                    blaze::band(result, num_band) =
                        blaze::subvector(v, des_start, des_size);
                }
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

                blaze::DynamicVector<T> res_arr(des_size);

                des_stop = des_size + des_start;

                std::int64_t rel_start = des_start - cur_start;

                if (des_start < cur_start || des_stop > cur_stop)
                {
                    // there is a need to fetch some part

                    // copying the local part
                    if (des_start < cur_stop && des_stop > cur_start)
                    {
                        auto indices = detail::diag_index_calculation_1d(
                            des_start, des_stop, cur_start, cur_stop);
                        blaze::subvector(res_arr, indices.projected_start_,
                            indices.intersection_size_) = blaze::subvector(v,
                                indices.local_start_,
                                indices.intersection_size_);
                    }

                    // collecting other parts from remote localities
                    for (std::uint32_t loc = 0; loc != num_localities; ++loc)
                    {
                        if (loc == loc_id)
                        {
                            continue;
                        }

                        // the array span in for loc-th locality
                        tiling_span loc_span =
                            arr_localities.tiles_[loc].spans_[span_index];

                        auto indices = detail::diag_retile_calculation_1d(
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
                    }

                    blaze::band(result, num_band) = res_arr;
                }

                else // the new array is a subset of the original array
                {
                    blaze::band(result, num_band) =
                        blaze::subvector(v, rel_start, des_size);
                }
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

                blaze::DynamicVector<T> res_arr(des_size);

                des_stop = des_size + des_start;

                std::int64_t rel_start = des_start - cur_start;

                if (des_start < cur_start || des_stop > cur_stop)
                {
                    // there is a need to fetch some part

                    // copying the local part
                    if (des_start < cur_stop && des_stop > cur_start)
                    {
                        auto indices = detail::diag_index_calculation_1d(
                            des_start, des_stop, cur_start, cur_stop);
                        blaze::subvector(res_arr, indices.projected_start_,
                            indices.intersection_size_) = blaze::subvector(v,
                                indices.local_start_,
                                indices.intersection_size_);
                    }

                    // collecting other parts from remote localities
                    for (std::uint32_t loc = 0; loc != num_localities; ++loc)
                    {
                        if (loc == loc_id)
                        {
                            continue;
                        }

                        // the array span in for loc-th locality
                        tiling_span loc_span =
                            arr_localities.tiles_[loc].spans_[span_index];

                        auto indices = detail::diag_retile_calculation_1d(
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
                    }

                    blaze::band(result, num_band) = res_arr;
                }

                else // the new array is a subset of the original array
                {
                    blaze::band(result, num_band) =
                        blaze::subvector(v, rel_start, des_size);
                }
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
    ///////////////////////////////////////////////////////////////////////////
//    template <typename T>
//    execution_tree::primitive_argument_type dist_diag::dist_diag1d(
//        ir::node_data<T>&& arr, std::int64_t k, std::string const& tiling_type,
//        std::uint32_t tile_idx, std::uint32_t numtiles,
//        execution_tree::localities_information&& arr_localities) const
//    {
//        using namespace execution_tree;
//
////        // current annotation information
////        std::uint32_t const loc_id = arr_localities.locality_.locality_id_;
////        std::uint32_t const num_localities =
////            arr_localities.locality_.num_localities_;
////
////        tiling_information_1d tile_info_input(
////            arr_localities.tiles_[loc_id], name_, codename_);
////
////        std::int64_t cur_start = tile_info_input.span_.start_;
////        std::int64_t cur_stop = tile_info_input.span_.stop_;
//
//        // updating the annotation_ part of localities annotation
////        arr_localities.annotation_.name_ += "_diag";
////        ++arr_localities.annotation_.generation_;
//
//
//        // is it a row vector or a column vector?
//        //std::size_t span_index = 0;
//        //if (!arr_localities.has_span(0))
//        //{
//        //    HPX_ASSERT(arr_localities.has_span(1));
//        //    span_index = 1;
//        //}
//
//    //    auto v = arr.vector();
////
//    //    util::distributed_vector<T> v_data(
//    //        arr_localities.annotation_.name_, v, num_localities, loc_id);
//
//        std::size_t dim = arr_localities.size();    // size of the whole array
//
//        std::size_t size = dim + std::abs(k);
//
//        std::int64_t row_start, column_start;
//        std::size_t row_size, column_size;
//
//        std::tie(row_start, column_start, row_size, column_size) =
//            tile_calculation::tile_calculation_2d(
//                tile_idx, size, size, numtiles, tiling_type);
//
//        blaze::DynamicMatrix<T> result(row_size, column_size, T(0));
//
//        std::int64_t num_band;
//        std::int64_t upper_band = column_size - 1;
//        std::int64_t lower_band = 1 - row_size;
//
//        std::int64_t des_start, des_stop, des_size;
//
//        if (tiling_type == "row")
//        {
//            num_band = row_start + k;
//            if (num_band <= (std::max)(int64_t(0), upper_band) &&
//                    num_band >= (std::min)(int64_t(0), lower_band))
//            {
//                des_start = k < 0 ?
//                    (std::max)(int64_t(0), num_band) : row_start;
//
//                des_size = num_band < 0 ?
//                    (std::min)(column_size, row_size + num_band) :
//                    (std::min)(row_size, column_size - num_band);
//
//                return diag_1d_helper(
//                    std::move(arr), k, std::move(arr_localities),
//                    row_start, column_start, row_size, column_size,
//                    des_start, des_size, num_band);
//
//                //blaze::DynamicVector<T> res_arr(des_size);
////
//                //des_stop = des_size + des_start;
////
//                //std::int64_t rel_start = des_start - cur_start;
////
//                //if (des_start < cur_start || des_stop > cur_stop)
//                //{
//                //    // there is a need to fetch some part
////
//                //    // copying the local part
//                //    if (des_start < cur_stop && des_stop > cur_start)
//                //    {
//                //        auto indices = calculation_detail::index_calculation_1d(
//                //            des_start, des_stop, cur_start, cur_stop);
//                //        blaze::subvector(res_arr, indices.projected_start_,
//                //            indices.intersection_size_) = blaze::subvector(v,
//                //                indices.local_start_,
//                //                indices.intersection_size_);
//                //    }
////
//                //    // collecting other parts from remote localities
//                //    for (std::uint32_t loc = 0; loc != num_localities; ++loc)
//                //    {
//                //        if (loc == loc_id)
//                //        {
//                //            continue;
//                //        }
////
//                //        // the array span in for loc-th locality
//                //        tiling_span loc_span =
//                //            arr_localities.tiles_[loc].spans_[span_index];
////
//                //        auto indices = calculation_detail::retile_calculation_1d(
//                //            loc_span, des_start, des_stop);
//                //        if (indices.intersection_size_ > 0)
//                //        {
//                //            // loc_span has the part of result that we need
//                //            blaze::subvector(res_arr, indices.projected_start_,
//                //                indices.intersection_size_) =
//                //                v_data
//                //                .fetch(loc, indices.local_start_,
//                //                    indices.local_start_ +
//                //                    indices.intersection_size_)
//                //                .get();
//                //        }
//                //    }
////
//                //    blaze::band(result, num_band) = res_arr;
//                //}
////
//                //else // the new array is a subset of the original array
//                //{
//                //    blaze::band(result, num_band) =
//                //        blaze::subvector(v, rel_start, des_size);
//                //}
//            }
//        }
//        else if (tiling_type == "column")
//        {
//            num_band = k - column_start;
//            if (num_band <= (std::max)(int64_t(0), upper_band) &&
//                    num_band >= (std::min)(int64_t(0), lower_band))
//            {
//                des_start = k < 0 ?
//                    column_start : (std::max)(int64_t(0), - num_band);
//
//                des_size = num_band < 0 ?
//                    (std::min)(column_size, row_size + num_band) :
//                    (std::min)(row_size, column_size - num_band);
//
//                return diag_1d_helper(
//                    std::move(arr), k, std::move(arr_localities),
//                    row_start, column_start, row_size, column_size,
//                    des_start, des_size, num_band);
//
//                //blaze::DynamicVector<T> res_arr(des_size);
////
//                //des_stop = des_size + des_start;
////
//                //std::int64_t rel_start = des_start - cur_start;
////
//                //if (des_start < cur_start || des_stop > cur_stop)
//                //{
//                //    // there is a need to fetch some part
////
//                //    // copying the local part
//                //    if (des_start < cur_stop && des_stop > cur_start)
//                //    {
//                //        auto indices = calculation_detail::index_calculation_1d(
//                //            des_start, des_stop, cur_start, cur_stop);
//                //        blaze::subvector(res_arr, indices.projected_start_,
//                //            indices.intersection_size_) = blaze::subvector(v,
//                //                indices.local_start_,
//                //                indices.intersection_size_);
//                //    }
////
//                //    // collecting other parts from remote localities
//                //    for (std::uint32_t loc = 0; loc != num_localities; ++loc)
//                //    {
//                //        if (loc == loc_id)
//                //        {
//                //            continue;
//                //        }
////
//                //        // the array span in for loc-th locality
//                //        tiling_span loc_span =
//                //            arr_localities.tiles_[loc].spans_[span_index];
////
//                //        auto indices = calculation_detail::retile_calculation_1d(
//                //            loc_span, des_start, des_stop);
//                //        if (indices.intersection_size_ > 0)
//                //        {
//                //            // loc_span has the part of result that we need
//                //            blaze::subvector(res_arr, indices.projected_start_,
//                //                indices.intersection_size_) =
//                //                v_data
//                //                .fetch(loc, indices.local_start_,
//                //                    indices.local_start_ +
//                //                    indices.intersection_size_)
//                //                .get();
//                //        }
//                //    }
////
//                //    blaze::band(result, num_band) = res_arr;
//                //}
////
//                //else // the new array is a subset of the original array
//                //{
//                //    blaze::band(result, num_band) =
//                //        blaze::subvector(v, rel_start, des_size);
//                //}
//            }
//        }
//        else if (tiling_type == "sym")
//        {
//            num_band = k - (column_start - row_start);
//            if (num_band <= (std::max)(int64_t(0), upper_band) &&
//                    num_band >= (std::min)(int64_t(0), lower_band))
//            {
//                des_start = k < 0 ?
//                    (std::max)(column_start, column_start + num_band) :
//                    (std::max)(row_start, row_start - num_band);
//
//                des_size = num_band < 0 ?
//                    (std::min)(column_size, row_size + num_band) :
//                    (std::min)(row_size, column_size - num_band);
//
//                return diag_1d_helper(
//                    std::move(arr), k, std::move(arr_localities),
//                    row_start, column_start, row_size, column_size,
//                    des_start, des_size, num_band);
//
//                //blaze::DynamicVector<T> res_arr(des_size);
////
//                //des_stop = des_size + des_start;
////
//                //std::int64_t rel_start = des_start - cur_start;
////
//                //if (des_start < cur_start || des_stop > cur_stop)
//                //{
//                //    // there is a need to fetch some part
////
//                //    // copying the local part
//                //    if (des_start < cur_stop && des_stop > cur_start)
//                //    {
//                //        auto indices = calculation_detail::index_calculation_1d(
//                //            des_start, des_stop, cur_start, cur_stop);
//                //        blaze::subvector(res_arr, indices.projected_start_,
//                //            indices.intersection_size_) = blaze::subvector(v,
//                //                indices.local_start_,
//                //                indices.intersection_size_);
//                //    }
////
//                //    // collecting other parts from remote localities
//                //    for (std::uint32_t loc = 0; loc != num_localities; ++loc)
//                //    {
//                //        if (loc == loc_id)
//                //        {
//                //            continue;
//                //        }
////
//                //        // the array span in for loc-th locality
//                //        tiling_span loc_span =
//                //            arr_localities.tiles_[loc].spans_[span_index];
////
//                //        auto indices = calculation_detail::retile_calculation_1d(
//                //            loc_span, des_start, des_stop);
//                //        if (indices.intersection_size_ > 0)
//                //        {
//                //            // loc_span has the part of result that we need
//                //            blaze::subvector(res_arr, indices.projected_start_,
//                //                indices.intersection_size_) =
//                //                v_data
//                //                .fetch(loc, indices.local_start_,
//                //                    indices.local_start_ +
//                //                    indices.intersection_size_)
//                //                .get();
//                //        }
//                //    }
////
//                //    blaze::band(result, num_band) = res_arr;
//                //}
////
//                //else // the new array is a subset of the original array
//                //{
//                //    blaze::band(result, num_band) =
//                //        blaze::subvector(v, rel_start, des_size);
//                //}
//            }
//        }
//        else
//        {
//            HPX_THROW_EXCEPTION(hpx::bad_parameter,
//                "dist_diag::dist_diag1d",
//                generate_error_message(
//                    "wrong numtiles input when tiling_type is column"));
//        }
//
//        tiling_information_2d tile_info(
//            tiling_span(row_start, row_start + row_size),
//            tiling_span(column_start, column_start + column_size));
//
//        auto locality_ann = arr_localities.locality_.as_annotation();
//        auto attached_annotation =
//            std::make_shared<annotation>(localities_annotation(locality_ann,
//                tile_info.as_annotation(name_, codename_),
//                arr_localities.annotation_, name_, codename_));
//
//        return primitive_argument_type(result, attached_annotation);
//    }

    execution_tree::primitive_argument_type dist_diag::dist_diag1d(
        execution_tree::primitive_argument_type&& arr, std::int64_t k,
        std::string const& tiling_type, std::uint32_t const& tile_idx,
        std::uint32_t const& numtiles) const
    {
        using namespace execution_tree;
        execution_tree::localities_information arr_localities =
            extract_localities_information(arr, name_, codename_);

        switch (extract_common_type(arr))
        {
        case node_data_type_bool:
            return dist_diag1d(
                extract_boolean_value(std::move(arr), name_, codename_),
                k, tiling_type, tile_idx, numtiles,
                std::move(arr_localities));

        case node_data_type_int64:
            return dist_diag1d(
                extract_integer_value(std::move(arr), name_, codename_),
                k, tiling_type, tile_idx, numtiles,
                std::move(arr_localities));

        case node_data_type_unknown:
            HPX_FALLTHROUGH;

        case node_data_type_double:
            return dist_diag1d(
                extract_numeric_value(std::move(arr), name_, codename_),
                k, tiling_type, tile_idx, numtiles,
                std::move(arr_localities));

        default:
            break;
        }

        HPX_THROW_EXCEPTION(hpx::bad_parameter,
            "dist_matrixops::dist_diag::dist_diag1d",
            util::generate_error_message(
                "the constant primitive requires for all arguments to "
                "be numeric data types"));
    }

    ///////////////////////////////////////////////////////////////////////////
    hpx::future<execution_tree::primitive_argument_type> dist_diag::eval(
        execution_tree::primitive_arguments_type const& operands,
        execution_tree::primitive_arguments_type const& args,
        execution_tree::eval_context ctx) const
    {
        // verify arguments
        if (operands.empty() || operands.size() > 5)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter, "dist_diag::eval",
                generate_error_message("the diag_d primitive requires "
                                       "at least 1 and at most 5 operands"));
        }

        if (!valid(operands[0]))
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter, "dist_diag::eval",
                generate_error_message(
                    "the diag_d primitive requires the first argument"
                    "given by the operands array is valid"));
        }

        auto this_ = this->shared_from_this();
        return hpx::dataflow(hpx::launch::sync,
            hpx::util::unwrapping(
                [this_ = std::move(this_)](
                        execution_tree::primitive_arguments_type&& args)
                -> execution_tree::primitive_argument_type
                {
                    using namespace execution_tree;

                    std::int64_t k = 0;
                    
                    if (valid(args[1]))
                    {
                        k = extract_scalar_integer_value_strict(
                            std::move(args[1]), this_->name_, this_->codename_);
                    }

                    std::string tiling_type = "sym";
                    if (valid(args[2]))
                    {
                        tiling_type = extract_string_value(
                            std::move(args[2]), this_->name_, this_->codename_);
                        if ((tiling_type != "sym" && tiling_type != "row") &&
                            tiling_type != "column")
                        {
                            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                                "dist_diag::eval",
                                this_->generate_error_message(
                                    "invalid tiling_type. the tiling_type cane "
                                    "one of these: `sym`, `row` or `column`"));
                        }
                    }

                    std::uint32_t tile_idx = hpx::get_locality_id();
                    if (valid(args[3]))
                    {
                        tile_idx = extract_scalar_nonneg_integer_value_strict(
                            std::move(args[3]), this_->name_, this_->codename_);
                    }
                    std::uint32_t numtiles =
                        hpx::get_num_localities(hpx::launch::sync);
                    if (valid(args[4]))
                    {
                        numtiles = extract_scalar_positive_integer_value_strict(
                            std::move(args[4]), this_->name_, this_->codename_);
                    }
                    if (tile_idx >= numtiles)
                    {
                        HPX_THROW_EXCEPTION(hpx::bad_parameter,
                            "dist_diag::eval",
                            this_->generate_error_message(
                                "invalid tile index. Tile indices start from 0 "
                                "and should be smaller than number of tiles"));
                    }

<<<<<<< HEAD

                    switch (extract_numeric_value_dimension(
                        args[0], this_->name_, this_->codename_))
                    {
                    case 1:
                        return this_->dist_diag1d(std::move(args[0]),
=======
                    return this_->dist_diag1d(std::move(args[0]),
>>>>>>> Separate long functions in dist_diag_1d that return reults based on tiling type; linking errors
                            k, tiling_type, tile_idx, numtiles);
                }),
            execution_tree::primitives::detail::map_operands(operands,
                execution_tree::functional::value_operand{}, args, name_,
                codename_, std::move(ctx)));
    }
}}}    // namespace phylanx::dist_matrixops::primitives