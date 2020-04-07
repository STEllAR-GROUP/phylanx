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

#include <hpx/assertion.hpp>
#include <hpx/include/lcos.hpp>
#include <hpx/include/naming.hpp>
#include <hpx/include/util.hpp>
#include <hpx/errors/throw_exception.hpp>

#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <string>
#include <tuple>
#include <utility>
#include <vector>

//#include <blaze/Math.h>
//#include <blaze_tensor/Math.h>

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

                a (array): overall shape of the array. It
                    only contains positive integers.
                new_tiling (a list): the tile index we need to generate the
                    random array for. A non-negative integer.

            Returns:

            A part of an array of random numbers on tile_index-th tile out of
            numtiles using the nrmal distribution)")
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

        std::tuple<std::size_t, std::size_t, std::size_t> retile_calculation_1d(
            execution_tree::tiling_span const& loc_span,
            std::int64_t const& des_start, std::int64_t const& cur_start)
        {
            execution_tree::tiling_span pre_span(des_start, cur_start);
            execution_tree::tiling_span intersection;
            if (intersect(pre_span, loc_span, intersection))
            {
                std::int64_t rel_loc_start =
                    intersection.start_ - loc_span.start_;
                HPX_ASSERT(rel_loc_start >= 0);
                return std::make_tuple(intersection.start_ - des_start,
                    intersection.size(), rel_loc_start);
            }
            return std::make_tuple(0, 0, 0);
        }

        std::tuple<std::size_t, std::size_t, std::size_t> retile_calculation_1d(
            execution_tree::tiling_span const& loc_span,
            std::int64_t const& des_stop, std::int64_t const& cur_stop,
            std::int64_t const& des_start)
        {
            execution_tree::tiling_span post_span(cur_stop, des_stop);
            execution_tree::tiling_span intersection;
            if (intersect(post_span, loc_span, intersection))
            {
                std::int64_t rel_loc_start =
                    intersection.start_ - loc_span.start_;
                HPX_ASSERT(rel_loc_start >= 0);
                return std::make_tuple(intersection.start_ - des_start,
                    intersection.size(), rel_loc_start);
            }
            return std::make_tuple(0, 0, 0);
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
        util::distributed_vector<T> v_data(arr_localities.annotation_.name_, v,
            arr_localities.locality_.num_localities_, loc_id);

        std::int64_t rel_start = des_start - cur_start;
        if (rel_start < 0 || des_stop > cur_stop)
        {
            // copying the local part
            if (des_start < cur_stop && des_stop > cur_start)
            {
                std::int64_t local_intersection_size =
                    (std::min)(des_stop, cur_stop) -
                    (std::max)(des_start, cur_start);
                std::int64_t res_intersection_start =
                    rel_start < 0 ? -rel_start : 0;
                std::int64_t v_intersection_start =
                    rel_start > 0 ? rel_start : 0;
                blaze::subvector(result, res_intersection_start,
                    local_intersection_size) = blaze::subvector(v,
                    v_intersection_start, local_intersection_size);
            }

            for (std::uint32_t loc = 0;
                 loc != arr_localities.locality_.num_localities_; ++loc)
            {
                if (loc == loc_id)
                {
                    continue;
                }

                // the array span in locality loc
                tiling_span loc_span =
                    arr_localities.tiles_[loc].spans_[span_index];
                std::size_t res_start, res_size, rel_loc_start;

                if (rel_start < 0)
                {
                    std::tie(res_start, res_size, rel_loc_start) =
                        detail::retile_calculation_1d(loc_span, des_start,
                            (std::min)(cur_start, des_stop));
                    if (res_size > 0)
                    {
                        // loc_span has the part of result that we need
                        blaze::subvector(result, res_start, res_size) =
                            v_data
                                .fetch(loc, rel_loc_start,
                                    rel_loc_start + res_size)
                                .get();
                    }
                }

                if (des_stop > cur_stop)
                {
                    std::tie(res_start, res_size, rel_loc_start) =
                        detail::retile_calculation_1d(loc_span, des_stop,
                            (std::max)(cur_stop, des_start), des_start);
                    if (res_size > 0)
                    {
                        // loc_span has the part of result that we need
                        blaze::subvector(result, res_start, res_size) =
                            v_data
                                .fetch(loc, rel_loc_start,
                                    rel_loc_start + res_size)
                                .get();
                    }
                }
            }
        }
        else // the new array is a subset of the origina array
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
                    auto label =
                        extract_string_value_strict(*new_tiling.begin());
                    if (label != "args" && label != "tile")
                    {
                        HPX_THROW_EXCEPTION(hpx::bad_parameter, "retile::eval",
                            this_->generate_error_message(
                                "the given shape is of an unsupported "
                                "dimensionality"));
                    }


                    switch (numdims)
                    {
                    case 1:
                        return this_->retile1d(std::move(args[0]),
                            std::move(new_tiling));

                        //case 2:
                        //    return this_->retile2d();

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

