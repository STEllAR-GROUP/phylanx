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
#include <phylanx/util/distributed_vector.hpp>
#include <phylanx/util/distributed_matrix.hpp>

#include <hpx/assertion.hpp>
#include <hpx/include/lcos.hpp>
#include <hpx/include/naming.hpp>
#include <hpx/include/util.hpp>
#include <hpx/errors/throw_exception.hpp>

#include <cstddef>
#include <cstdint>
#include <memory>
#include <string>
#include <tuple>
#include <utility>
#include <vector>

#include <blaze/Math.h>
#include <blaze_tensor/Math.h>

///////////////////////////////////////////////////////////////////////////////
namespace phylanx { namespace dist_matrixops { namespace primitives
{
    ///////////////////////////////////////////////////////////////////////////
    execution_tree::match_pattern_type const retile_annotations::match_data =
    {
        hpx::util::make_tuple("retile_d", std::vector<std::string>{R"(
                retile_d(
                    _1_a,
                    _2_new_tiling
                )
            )"},
            &create_retile_annotations,
            &execution_tree::create_primitive<retile_annotations>, R"(
            a, new_tiling
            Args:

                a (array): a part of an array with its attached annotation that
                    locates on the current locality
                new_tiling (list): a new tiling specification for the current
                    locality, e.g. for a matrix we can have
                    list("tile", list("columns", 0, 2), list("rows", 0, 2)) or
                    list("args", list("locality", 0, 4),
                       list("tile", list("columns", 0, 2), list("rows", 0, 2)))

            Returns:

            The part of array on the current locality according to the
            new_tiling)")
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

            // label is "tile"
            return tile_extraction_1d_helper(std::move(it));
        }

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
    }    // namespace detail

    ///////////////////////////////////////////////////////////////////////////
    template <typename T>
    execution_tree::primitive_argument_type retile_annotations::retile1d(
        ir::node_data<T>&& arr, ir::range&& new_tiling,
        execution_tree::localities_information&& arr_localities) const
    {
        using namespace execution_tree;

        // current annotation information
        std::uint32_t const loc_id = arr_localities.locality_.locality_id_;
        std::uint32_t const num_localities =
            arr_localities.locality_.num_localities_;
        tiling_information_1d tile_info(
            arr_localities.tiles_[loc_id], name_, codename_);

        std::int64_t cur_start = tile_info.span_.start_;
        std::int64_t cur_stop = tile_info.span_.stop_;

        // updating the annotation_ part of localities annotation
        arr_localities.annotation_.name_ += "_retiled";
        ++arr_localities.annotation_.generation_;

        // desired annotation information
        std::int64_t des_start, des_stop;
        bool type_;
        std::tie(type_, des_start, des_stop) =
            detail::tile_extraction_1d(std::move(new_tiling));
        std::int64_t des_size = des_stop - des_start;
        if (des_size <= 0)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "dist_matrixops::primitives::retile_annotations::retile1d",
                generate_error_message(
                    "the given start point of the new_tiling should be less "
                    "than its stop point"));
        }

        std::size_t span_index = 0;
        if (!arr_localities.has_span(0))
        {
            HPX_ASSERT(arr_localities.has_span(1));
            span_index = 1;
        }

        // updating the array
        auto v = arr.vector();
        blaze::DynamicVector<T> result(des_size);
        util::distributed_vector<T> v_data(
            arr_localities.annotation_.name_, v, num_localities, loc_id);

        std::int64_t rel_start = des_start - cur_start;
        if (rel_start < 0 || des_stop > cur_stop)
        {
            // copying the local part
            if (des_start < cur_stop && des_stop > cur_start)
            {
                auto indices = detail::index_calculation_1d(
                    des_start, des_stop, cur_start, cur_stop);
                blaze::subvector(result, indices.projected_start_,
                    indices.intersection_size_) = blaze::subvector(v,
                    indices.local_start_, indices.intersection_size_);
            }

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
        ir::range&& new_tiling) const
    {
        using namespace execution_tree;
        execution_tree::localities_information arr_localities =
            extract_localities_information(arr, name_, codename_);

        switch (extract_common_type(arr))
        {
        case node_data_type_bool:
            return retile1d(
                extract_boolean_value_strict(std::move(arr), name_, codename_),
                std::move(new_tiling), std::move(arr_localities));

        case node_data_type_int64:
            return retile1d(
                extract_integer_value_strict(std::move(arr), name_, codename_),
                std::move(new_tiling), std::move(arr_localities));

        case node_data_type_double:
            return retile1d(
                extract_numeric_value_strict(std::move(arr), name_, codename_),
                std::move(new_tiling), std::move(arr_localities));

        case node_data_type_unknown:
            return retile1d(
                extract_numeric_value(std::move(arr), name_, codename_),
                std::move(new_tiling), std::move(arr_localities));

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
        ir::node_data<T>&& arr, ir::range&& new_tiling,
        execution_tree::localities_information&& arr_localities) const
    {
        using namespace execution_tree;

        // current annotation information
        std::uint32_t const loc_id = arr_localities.locality_.locality_id_;
        std::uint32_t const num_localities =
            arr_localities.locality_.num_localities_;
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
        std::int64_t des_row_start, des_row_stop, des_col_start,
            des_col_stop;
        std::tie(des_row_start, des_col_start, des_row_stop,
            des_col_stop) =
            detail::tile_extraction_2d(std::move(new_tiling));

        std::int64_t des_row_size = des_row_stop - des_row_start;
        std::int64_t des_col_size = des_col_stop - des_col_start;
        if (des_row_size <= 0 || des_col_size <= 0)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "dist_matrixops::primitives::retile_annotations::retile2d",
                generate_error_message(
                    "the given start point of the new_tiling should be less "
                    "than its stop point on each dimension"));
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
        ir::range&& new_tiling) const
    {
        using namespace execution_tree;
        execution_tree::localities_information arr_localities =
            extract_localities_information(arr, name_, codename_);

        switch (extract_common_type(arr))
        {
        case node_data_type_bool:
            return retile2d(
                extract_boolean_value_strict(std::move(arr), name_, codename_),
                std::move(new_tiling), std::move(arr_localities));

        case node_data_type_int64:
            return retile2d(
                extract_integer_value_strict(std::move(arr), name_, codename_),
                std::move(new_tiling), std::move(arr_localities));

        case node_data_type_double:
            return retile2d(
                extract_numeric_value_strict(std::move(arr), name_, codename_),
                std::move(new_tiling), std::move(arr_localities));

        case node_data_type_unknown:
            return retile2d(
                extract_numeric_value(std::move(arr), name_, codename_),
                std::move(new_tiling), std::move(arr_localities));

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
        if (operands.size() != 2)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "retile::eval",
                generate_error_message(
                    "the retile_d primitive requires two operands"));
        }

        if (!valid(operands[0]) || !valid(operands[1]))
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "retile::eval",
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

                    if (!is_list_operand_strict(args[1]))
                    {
                        HPX_THROW_EXCEPTION(hpx::bad_parameter,
                            "retile_d::eval",
                            this_->generate_error_message(
                                "the new tiling should be a list containing "
                                "`tile` or `args`"));
                    }
                    ir::range&& new_tiling = extract_list_value_strict(
                        std::move(args[1]), this_->name_, this_->codename_);
                    auto label = extract_string_value_strict(*new_tiling.begin());
                    if (label != "args" && label != "tile")
                    {
                        HPX_THROW_EXCEPTION(hpx::bad_parameter, "retile::eval",
                            this_->generate_error_message(
                                "the new_tiling should start with either of "
                                "`tile` or `args` tags"));
                    }

                    switch (numdims)
                    {
                    case 1:
                        return this_->retile1d(std::move(args[0]),
                            std::move(new_tiling));

                    case 2:
                        return this_->retile2d(
                            std::move(args[0]), std::move(new_tiling));

                    default:
                        HPX_THROW_EXCEPTION(hpx::bad_parameter,
                            "retile::eval",
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

