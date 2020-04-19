// Copyright (c) 2020 Bita Hasheminezhad
// Copyright (c) 2020 Hartmut Kaiser
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>
#include <phylanx/execution_tree/annotation.hpp>
#include <phylanx/execution_tree/locality_annotation.hpp>
#include <phylanx/execution_tree/localities_annotation.hpp>
#include <phylanx/execution_tree/meta_annotation.hpp>
#include <phylanx/execution_tree/primitives/annotate_primitive.hpp>
#include <phylanx/execution_tree/primitives/node_data_helpers.hpp>
#include <phylanx/execution_tree/tiling_annotations.hpp>
#include <phylanx/ir/node_data.hpp>
#include <phylanx/plugins/dist_matrixops/retile_annotations.hpp>
#include <phylanx/plugins/dist_matrixops/tile_calculation_helper.hpp>
#include <phylanx/util/distributed_vector.hpp>
#include <phylanx/util/distributed_matrix.hpp>

#include <hpx/assertion.hpp>
#include <hpx/include/lcos.hpp>
#include <hpx/include/naming.hpp>
#include <hpx/include/util.hpp>
#include <hpx/errors/throw_exception.hpp>

#include <array>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <string>
#include <tuple>
#include <utility>
#include <vector>

#include <blaze/Math.h>

///////////////////////////////////////////////////////////////////////////////
namespace phylanx { namespace dist_matrixops { namespace primitives
{
    ///////////////////////////////////////////////////////////////////////////
    execution_tree::match_pattern_type const retile_annotations::match_data =
    {
        hpx::util::make_tuple("retile_d", std::vector<std::string>{R"(
                retile_d(
                    _1_a,
                    __arg(_2_tiling_type, "sym"),
                    __arg(_3_intersection, nil),
                    __arg(_4_numtiles, num_localities()),
                    __arg(_5_new_tiling, nil)
                )
            )"},
            &create_retile_annotations,
            &execution_tree::create_primitive<retile_annotations>, R"(
            a, tiling_type, intersection, numtiles, new_tiling
            Args:

                a (array): a distributed array. A vector or a matrix
                tiling_type (string, optional): defaults to `sym` which is a
                    balanced way of tiling among all localities. Other options
                    are `row`, `column` or `user` tiling types. If an
                    intersection is given, retile produces overlapped parts of
                    arrays. In the `user` mode, using new_tiling, tiles are
                    specified with their spans.
                intersection (int or list of ints, optional): the size of
                    overlapped part on each dimension. If an integer is given,
                    that would be the intersection length on all dimensions
                    that are tiled. If the given intersection is odd, the extra
                    overlapped vector will be at the end of the part.
                    In the `user` mode, intersection cannot be used.
                numtiles (int, optional): number of tiles of the returned array
                    If not given it sets to the number of localities in the
                    application.
                new_tiling (list, optional): a new tiling specification for the
                    current locality that is specified only for the `user` mode
                    E.g. for a matrix we can have:
                    list("tile", list("columns", 0, 2), list("rows", 0, 2)) or
                    list("args", list("locality", 0, 4),
                       list("tile", list("columns", 0, 2), list("rows", 0, 2)))

            Returns:

            A retiled array according to the tiling type on numtiles localities
            The returned array can contain overlapped parts)")
    };

    ///////////////////////////////////////////////////////////////////////////
    retile_annotations::retile_annotations(
        execution_tree::primitive_arguments_type&& operands,
        std::string const& name, std::string const& codename)
      : primitive_component_base(std::move(operands), name, codename)
    {}

    ///////////////////////////////////////////////////////////////////////////
    namespace detail
    {
        std::tuple<bool, std::int64_t, std::int64_t> tile_extraction_1d_helper(
            ir::range_iterator&& it)
        {
            ir::range tile_data = extract_list_value_strict(*++it);
            ir::range_iterator inner_it = tile_data.begin();

            // type_: columns = 0, rows = 1
            bool type_ =
                extract_string_value_strict(*inner_it) == "rows" ? 1 : 0;
            std::int64_t start =
                extract_scalar_nonneg_integer_value_strict(*++inner_it);
            std::int64_t stop =
                extract_scalar_nonneg_integer_value_strict(*++inner_it);

            return std::move(std::make_tuple(type_, start, stop));
        }

        std::tuple<bool, std::int64_t, std::int64_t> tile_extraction_1d(
            ir::range&& new_tiling)
        {
            ir::range_iterator it = new_tiling.begin();
            std::string label = extract_string_value_strict(*it);
            if (label == "args")
            {
                // extracting the "tile" tag
                ++it; // passing the "locality" tag
                return tile_extraction_1d_helper(
                    extract_list_value_strict(*++it).begin());
            }
            else if (label == "tile")
            {
                return tile_extraction_1d_helper(std::move(it));
            }
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "retile_annotations::tile_extraction_1d",
                util::generate_error_message(
                    "the new_tiling should start with either of "
                    "`tile` or `args` tags"));
        }

        ///////////////////////////////////////////////////////////////////////
        std::tuple<std::int64_t, std::int64_t, std::int64_t, std::int64_t>
        tile_extraction_2d_helper(ir::range_iterator&& it)
        {
            ir::range tile_data1 = extract_list_value_strict(*++it);
            ir::range tile_data2 = extract_list_value_strict(*++it);
            ir::range_iterator inner_it1 = tile_data1.begin();
            ir::range_iterator inner_it2 = tile_data2.begin();


            if (extract_string_value_strict(*inner_it1) == "rows" &&
                extract_string_value_strict(*inner_it2) == "columns")
            {
            }
            else if (extract_string_value_strict(*inner_it2) == "rows" &&
                extract_string_value_strict(*inner_it1) == "columns")
            {
                std::swap(inner_it1, inner_it2);
            }
            else
            {
                HPX_THROW_EXCEPTION(hpx::bad_parameter,
                    "dist_matrixops::primitives::retile_annotations::detail::"
                    "tile_extraction_2d_helper",
                    util::generate_error_message(
                        "the new_tiling for a matrix should have two tile "
                        "tags: `rows` and `columns`"));
            }

            std::int64_t row_start =
                extract_scalar_nonneg_integer_value_strict(*++inner_it1);
            std::int64_t row_stop =
                extract_scalar_nonneg_integer_value_strict(*++inner_it1);
            std::int64_t col_start =
                extract_scalar_nonneg_integer_value_strict(*++inner_it2);
            std::int64_t col_stop =
                extract_scalar_nonneg_integer_value_strict(*++inner_it2);

            return std::move(std::make_tuple(
                row_start, col_start, row_stop, col_stop));
        }

        std::tuple<std::int64_t, std::int64_t, std::int64_t, std::int64_t>
        tile_extraction_2d(ir::range&& new_tiling)
        {
            ir::range_iterator it = new_tiling.begin();
            std::string label = extract_string_value_strict(*it);
            if (label == "args")
            {
                // extracting the "tile" tag
                ++it; // passing the "locality" tag
                return tile_extraction_2d_helper(
                    extract_list_value_strict(*++it).begin());
            }

            // label is "tile"
            return tile_extraction_2d_helper(std::move(it));
        }

        ///////////////////////////////////////////////////////////////////////
        struct indices_pack
        {
            std::int64_t intersection_size_;
            std::int64_t projected_start_;
            std::int64_t local_start_;
        };

        indices_pack index_calculation_1d(std::int64_t des_start,
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

        indices_pack retile_calculation_1d(
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
    execution_tree::primitive_argument_type retile_annotations::retile1d(
        ir::node_data<T>&& arr, std::string const& tiling_type,
        std::size_t intersection, std::uint32_t numtiles,
        ir::range&& new_tiling,
        execution_tree::localities_information&& arr_localities) const
    {
        using namespace execution_tree;

        // current annotation information
        std::uint32_t const loc_id = arr_localities.locality_.locality_id_;
        std::uint32_t const num_localities =
            arr_localities.locality_.num_localities_;
        std::size_t dim = arr_localities.size();    // size of the whole array
        tiling_information_1d tile_info(
            arr_localities.tiles_[loc_id], name_, codename_);

        std::int64_t cur_start = tile_info.span_.start_;
        std::int64_t cur_stop = tile_info.span_.stop_;

        // updating the annotation_ part of localities annotation
        arr_localities.annotation_.name_ += "_retiled";
        ++arr_localities.annotation_.generation_;

        // is it a rwo vector or a column vector?
        std::size_t span_index = 0;
        if (!arr_localities.has_span(0))
        {
            HPX_ASSERT(arr_localities.has_span(1));
            span_index = 1;
        }

        // desired annotation information
        std::int64_t des_start, des_stop, des_size;
        bool type_ = span_index;    // initialized by the current type

        if (tiling_type == "user")
        {

            std::tie(type_, des_start, des_stop) =
                detail::tile_extraction_1d(std::move(new_tiling));
            des_size = des_stop - des_start;
            if (des_size <= 0)
            {
                HPX_THROW_EXCEPTION(hpx::bad_parameter,
                    "dist_matrixops::primitives::retile_annotations::retile1d",
                    generate_error_message(
                        "the given start point of the new_tiling should be less "
                        "than its stop point"));
            }
        }
        else    // tiling_type is one of "sym", "row" or "column"
        {
            if (tiling_type == "row")
            {
                type_ = true;
            }
            else if (tiling_type == "column")
            {
                type_ = false;
            }
            std::tie(des_start, des_size) =
                tile_calculation::tile_calculation_1d(loc_id, dim, numtiles);

            if (intersection != 0) // there should be some overlapped sections
            {
                std::tie(des_start, des_size) =
                    tile_calculation::tile_calculation_overlap_1d(
                        des_start, des_size, dim, intersection);
            }
            des_stop = des_start + des_size;
        }


        // creating the updated array as result
        auto v = arr.vector();
        blaze::DynamicVector<T> result(des_size);
        util::distributed_vector<T> v_data(
            arr_localities.annotation_.name_, v, num_localities, loc_id);

        // relative start
        std::int64_t rel_start = des_start - cur_start;
        if (rel_start < 0 || des_stop > cur_stop)
        {
            // there is a need to fetch some part

            // copying the local part
            if (des_start < cur_stop && des_stop > cur_start)
            {
                auto indices = detail::index_calculation_1d(
                    des_start, des_stop, cur_start, cur_stop);
                blaze::subvector(result, indices.projected_start_,
                    indices.intersection_size_) = blaze::subvector(v,
                        indices.local_start_, indices.intersection_size_);
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

                auto indices = detail::retile_calculation_1d(
                    loc_span, des_start, des_stop);
                if (indices.intersection_size_ > 0)
                {
                    // loc_span has the part of result that we need
                    blaze::subvector(result, indices.projected_start_,
                        indices.intersection_size_) =
                        v_data
                        .fetch(loc, indices.local_start_,
                            indices.local_start_ +
                            indices.intersection_size_)
                        .get();
                }
            }
        }
        else // the new array is a subset of the original array
        {
            result = blaze::subvector(v, rel_start, des_size);
        }

        // updating the tile information
        if (type_)    // rows
        {
            tile_info =
                tiling_information_1d(tiling_information_1d::tile1d_type::rows,
                    tiling_span(des_start, des_stop));
        }
        else    // columns
        {
            tile_info = tiling_information_1d(
                tiling_information_1d::tile1d_type::columns,
                tiling_span(des_start, des_stop));
        }

        auto locality_ann = arr_localities.locality_.as_annotation();
        auto attached_annotation =
            std::make_shared<annotation>(localities_annotation(locality_ann,
                tile_info.as_annotation(name_, codename_),
                arr_localities.annotation_, name_, codename_));

        return primitive_argument_type(result, attached_annotation);
    }

    execution_tree::primitive_argument_type retile_annotations::retile1d(
        execution_tree::primitive_argument_type&& arr,
        std::string const& tiling_type, std::size_t intersection,
        std::uint32_t numtiles, ir::range&& new_tiling) const
    {
        using namespace execution_tree;
        localities_information arr_localities =
            extract_localities_information(arr, name_, codename_);

        switch (extract_common_type(arr))
        {
        case node_data_type_bool:
            return retile1d(
                extract_boolean_value_strict(std::move(arr), name_, codename_),
                tiling_type, intersection, numtiles, std::move(new_tiling),
                std::move(arr_localities));

        case node_data_type_int64:
            return retile1d(
                extract_integer_value_strict(std::move(arr), name_, codename_),
                tiling_type, intersection, numtiles, std::move(new_tiling),
                std::move(arr_localities));

        case node_data_type_double:
            return retile1d(
                extract_numeric_value_strict(std::move(arr), name_, codename_),
                tiling_type, intersection, numtiles, std::move(new_tiling),
                std::move(arr_localities));

        case node_data_type_unknown:
            return retile1d(
                extract_numeric_value(std::move(arr), name_, codename_),
                tiling_type, intersection, numtiles, std::move(new_tiling),
                std::move(arr_localities));

        default:
            break;
        }

        HPX_THROW_EXCEPTION(hpx::bad_parameter,
            "dist_matrixops::primitives::retile_annotations::retile1d",
            generate_error_message(
                "the retile_d primitive requires for all arguments to "
                "be numeric data types"));
    }

    ///////////////////////////////////////////////////////////////////////////
    template <typename T>
    execution_tree::primitive_argument_type retile_annotations::retile2d(
        ir::node_data<T>&& arr, std::string const& tiling_type,
        std::array<std::size_t, PHYLANX_MAX_DIMENSIONS> const& intersections,
        std::uint32_t numtiles, ir::range&& new_tiling,
        execution_tree::localities_information&& arr_localities) const
    {
        using namespace execution_tree;

        // current annotation information
        std::uint32_t const loc_id = arr_localities.locality_.locality_id_;
        std::uint32_t const num_localities =
            arr_localities.locality_.num_localities_;
        std::size_t rows_dim = arr_localities.rows();
        std::size_t cols_dim = arr_localities.columns();
        tiling_information_2d tile_info(
            arr_localities.tiles_[loc_id], name_, codename_);

        std::int64_t cur_row_start = tile_info.spans_[0].start_;
        std::int64_t cur_row_stop = tile_info.spans_[0].stop_;
        std::int64_t cur_col_start = tile_info.spans_[1].start_;
        std::int64_t cur_col_stop = tile_info.spans_[1].stop_;

        // updating the annotation_ part of localities annotation
        arr_localities.annotation_.name_ += "_retiled";
        ++arr_localities.annotation_.generation_;

        // desired annotation information
        std::int64_t des_row_start, des_row_stop, des_col_start, des_col_stop,
            des_row_size, des_col_size;

        if (tiling_type == "user")
        {
            std::tie(des_row_start, des_col_start, des_row_stop, des_col_stop) =
                detail::tile_extraction_2d(std::move(new_tiling));

            des_row_size = des_row_stop - des_row_start;
            des_col_size = des_col_stop - des_col_start;

            if (des_row_size <= 0 || des_col_size <= 0)
            {
                HPX_THROW_EXCEPTION(hpx::bad_parameter,
                    "dist_matrixops::primitives::retile_annotations::retile2d",
                    generate_error_message(
                        "the given start point of the new_tiling should be "
                        "smaller than its stop point on each dimension"));
            }
        }
        else    // tiling_type is one of "sym", "row" or "column"
        {
            std::tie(des_row_start, des_col_start, des_row_size, des_col_size) =
                tile_calculation::tile_calculation_2d(
                    loc_id, rows_dim, cols_dim, numtiles, tiling_type);

            if (des_row_size != rows_dim &&
                intersections[0] != 0)    // rows overlap
            {
                std::tie(des_row_start, des_row_size) =
                    tile_calculation::tile_calculation_overlap_1d(des_row_start,
                        des_row_size, rows_dim, intersections[0]);
            }
            if (des_col_size != cols_dim &&
                intersections[1] != 0)    // columns overlap
            {
                std::tie(des_col_start, des_col_size) =
                    tile_calculation::tile_calculation_overlap_1d(des_col_start,
                        des_col_size, cols_dim, intersections[1]);
            }

            des_row_stop = des_row_start + des_row_size;
            des_col_stop = des_col_start + des_col_size;
        }


        // updating the array
        auto m = arr.matrix();
        blaze::DynamicMatrix<T> result(des_row_size, des_col_size);
        util::distributed_matrix<T> m_data(
            arr_localities.annotation_.name_, m, num_localities, loc_id);

        // relative starts
        std::int64_t rel_row_start = des_row_start - cur_row_start;
        std::int64_t rel_col_start = des_col_start - cur_col_start;
        if ((rel_row_start < 0 || des_row_stop > cur_row_stop) ||
            (rel_col_start < 0 || des_col_stop > cur_col_stop))
        {
            // copying the local part
            if ((des_row_start < cur_row_stop &&
                    des_row_stop > cur_row_start) &&
                (des_col_start < cur_col_stop && des_col_stop > cur_col_start))
            {
                auto row_indices = detail::index_calculation_1d(
                    des_row_start, des_row_stop, cur_row_start, cur_row_stop);
                auto col_indices = detail::index_calculation_1d(
                    des_col_start, des_col_stop, cur_col_start, cur_col_stop);

                blaze::submatrix(result, row_indices.projected_start_,
                    col_indices.projected_start_,
                    row_indices.intersection_size_,
                    col_indices.intersection_size_) = blaze::submatrix(m,
                    row_indices.local_start_, col_indices.local_start_,
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
                tiling_span loc_row_span = arr_localities.tiles_[loc].spans_[0];
                tiling_span loc_col_span = arr_localities.tiles_[loc].spans_[1];

                auto row_indices = detail::retile_calculation_1d(
                    loc_row_span, des_row_start, des_row_stop);
                auto col_indices = detail::retile_calculation_1d(
                    loc_col_span, des_col_start, des_col_stop);
                if (row_indices.intersection_size_ > 0 &&
                    col_indices.intersection_size_ > 0)
                {
                    // loc_span has the block of result that we need
                    blaze::submatrix(result, row_indices.projected_start_,
                        col_indices.projected_start_,
                        row_indices.intersection_size_,
                        col_indices.intersection_size_) =
                        m_data
                            .fetch(loc, row_indices.local_start_,
                                row_indices.local_start_ +
                                    row_indices.intersection_size_,
                                col_indices.local_start_,
                                col_indices.local_start_ +
                                    col_indices.intersection_size_)
                            .get();
                }
            }
        }
        else // the new array is a subset of the original array
        {
            result = blaze::submatrix(
                m, rel_row_start, rel_col_start, des_row_size, des_col_size);
        }

        // updating the tile information
        tile_info =
            tiling_information_2d(tiling_span(des_row_start, des_row_stop),
                tiling_span(des_col_start, des_col_stop));

        auto locality_ann = arr_localities.locality_.as_annotation();
        auto attached_annotation =
            std::make_shared<annotation>(localities_annotation(locality_ann,
                tile_info.as_annotation(name_, codename_),
                arr_localities.annotation_, name_, codename_));

        return primitive_argument_type(result, attached_annotation);
    }

    execution_tree::primitive_argument_type retile_annotations::retile2d(
        execution_tree::primitive_argument_type&& arr,
        std::string const& tiling_type,
        std::array<std::size_t, PHYLANX_MAX_DIMENSIONS> const& intersection,
        std::uint32_t numtiles, ir::range&& new_tiling) const
    {
        using namespace execution_tree;
        execution_tree::localities_information arr_localities =
            extract_localities_information(arr, name_, codename_);

        switch (extract_common_type(arr))
        {
        case node_data_type_bool:
            return retile2d(
                extract_boolean_value_strict(std::move(arr), name_, codename_),
                tiling_type, intersection, numtiles, std::move(new_tiling),
                std::move(arr_localities));

        case node_data_type_int64:
            return retile2d(
                extract_integer_value_strict(std::move(arr), name_, codename_),
                tiling_type, intersection, numtiles, std::move(new_tiling),
                std::move(arr_localities));

        case node_data_type_double:
            return retile2d(
                extract_numeric_value_strict(std::move(arr), name_, codename_),
                tiling_type, intersection, numtiles, std::move(new_tiling),
                std::move(arr_localities));

        case node_data_type_unknown:
            return retile2d(
                extract_numeric_value(std::move(arr), name_, codename_),
                tiling_type, intersection, numtiles, std::move(new_tiling),
                std::move(arr_localities));

        default:
            break;
        }

        HPX_THROW_EXCEPTION(hpx::bad_parameter,
            "dist_matrixops::primitives::retile_annotations::retile2d",
            generate_error_message(
                "the retile_d primitive requires for all arguments to "
                "be numeric data types"));
    }

    ///////////////////////////////////////////////////////////////////////////
    hpx::future<execution_tree::primitive_argument_type>
    retile_annotations::eval(
        execution_tree::primitive_arguments_type const& operands,
        execution_tree::primitive_arguments_type const& args,
        execution_tree::eval_context ctx) const
    {
        if (operands.empty() || operands.size() > 5)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter, "retile_annotations::eval",
                generate_error_message("the retile_d primitive requires at "
                                       "least one and at most 5 operands"));
        }

        if (!valid(operands[0]))
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "retile_annotations::eval",
                generate_error_message(
                    "the retile_d primitive requires that the arguments "
                        "given by the operands array are valid"));
        }

        auto this_ = this->shared_from_this();
        return hpx::dataflow(hpx::launch::sync, hpx::util::unwrapping(
                [this_ = std::move(this_)](
                        execution_tree::primitive_arguments_type&& args)
                ->  execution_tree::primitive_argument_type
                {
                    using namespace execution_tree;

                    std::size_t numdims = extract_numeric_value_dimension(
                        args[0], this_->name_, this_->codename_);

                    std::string tiling_type = "sym";
                    if (valid(args[1]))
                    {
                        tiling_type = extract_string_value(
                            std::move(args[1]), this_->name_, this_->codename_);
                        if ((tiling_type != "sym" && tiling_type != "row") &&
                            (tiling_type != "column" && tiling_type != "user"))
                        {
                            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                                "retile_annotations::eval",
                                this_->generate_error_message(
                                    "invalid tiling_type. The tiling_type can "
                                    "be one of these: `sym`, `row`, `column` "
                                    "or `user`"));
                        }
                    }

                    std::uint32_t numtiles =
                        hpx::get_num_localities(hpx::launch::sync);
                    if (valid(args[3]))
                    {
                        extract_scalar_positive_integer_value_strict(
                            std::move(args[3]), this_->name_, this_->codename_);
                    }

                    std::array<std::size_t, PHYLANX_MAX_DIMENSIONS>
                        intersections{0};
                    if (valid(args[2]))
                    {
                        if (tiling_type == "user")
                        {
                            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                                "retile_annotations::eval",
                                this_->generate_error_message(
                                    "for the `user` tiling type, the new "
                                    "tiling should indicate the tile spans and "
                                    "intersection cannot be used"));
                        }

                        if (is_list_operand_strict(args[2]))
                        {
                            ir::range&& intersection_list =
                                extract_list_value_strict(std::move(args[2]),
                                    this_->name_, this_->codename_);

                            if (intersection_list.size() != 1 &&
                                intersection_list.size() != numdims)
                            {
                                HPX_THROW_EXCEPTION(hpx::bad_parameter,
                                    "retile_annotations::eval",
                                    this_->generate_error_message(
                                        "intersection should have the same "
                                        "number of dimensions as the array, or "
                                        "be represented with an integer for "
                                        "all dimensions"));
                            }
                            intersections =
                                tile_calculation::extract_nonneg_dimensions(
                                    intersection_list);
                        }
                        else if (is_numeric_operand(args[2]))
                        {
                            intersections[0] =
                                extract_scalar_nonneg_integer_value_strict(
                                    std::move(args[2]), this_->name_,
                                    this_->codename_);

                            // we assume all dimensions have the same
                            // intersection length which is the given one
                            for (std::size_t i = 1; i != PHYLANX_MAX_DIMENSIONS;
                                 ++i)
                            {
                                if (i == numdims)
                                    break;
                                intersections[i] = intersections[0];
                            }
                        }
                        else
                        {
                            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                                "retile_annotations::eval",
                                this_->generate_error_message(
                                    "intersection can be an integer or a list "
                                    "of integers"));
                        }
                    }

                    ir::range new_tiling(0);    // empty range
                    if (valid(args[4]))
                    {
                        if (tiling_type != "user")
                        {
                            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                                "retile_annotations::eval",
                                this_->generate_error_message(
                                    "new tiling is only a parameter of `user` "
                                    "tiling type"));
                        }

                        if (!is_list_operand_strict(args[4]))
                        {
                            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                                "retile_annotations::eval",
                                this_->generate_error_message(
                                    "the new tiling should be a list "
                                    "containing `tile` or `args`"));
                        }
                        new_tiling = extract_list_value_strict(
                            std::move(args[4]), this_->name_, this_->codename_);
                    }

                    switch (numdims)
                    {
                    case 1:
                        return this_->retile1d(std::move(args[0]), tiling_type,
                            intersections[0], numtiles, std::move(new_tiling));

                    case 2:
                        return this_->retile2d(std::move(args[0]), tiling_type,
                            intersections, numtiles, std::move(new_tiling));

                    default:
                        HPX_THROW_EXCEPTION(hpx::bad_parameter,
                            "retile_annotations::eval",
                            this_->generate_error_message(
                                "the given shape is of an unsupported "
                                "dimensionality"));
                    }
                }),
            execution_tree::primitives::detail::map_operands(operands,
                execution_tree::functional::value_operand{}, args, name_,
                codename_, std::move(ctx)));
    }
}}}

