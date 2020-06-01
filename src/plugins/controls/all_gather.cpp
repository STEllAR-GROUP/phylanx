// Copyright (c) 2020 Hartmut Kaiser
// Copyright (c) 2020 Nanmiao Wu
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>
#include <phylanx/execution_tree/localities_annotation.hpp>
#include <phylanx/plugins/controls/all_gather.hpp>
#include <phylanx/plugins/dist_matrixops/tile_calculation_helper.hpp>

#include <hpx/include/lcos.hpp>
#include <hpx/include/naming.hpp>
#include <hpx/include/util.hpp>
#include <hpx/errors/throw_exception.hpp>


#include <algorithm>
#include <cstddef>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include <blaze/Math.h>

///////////////////////////////////////////////////////////////////////////////
namespace phylanx { namespace execution_tree { namespace primitives
{
    ///////////////////////////////////////////////////////////////////////////
    match_pattern_type const all_gather::match_data =
    {
        hpx::util::make_tuple("all_gather",
            std::vector<std::string>{
                "all_gather(_1)", "all_gather(_1, _2)",
                "all_gather(_1, _2, _3)"
            },
            &create_all_gather,
            &create_primitive<all_gather>,
            R"(local_result, numtiles, this_tile, root_site
            Args:

                local_result (array) : a distributed array. A scalar, vector,
                    or matrix.
                numtiles (int, optional): number of tiles of the returned array.
                    If not given it sets to the number of localities in the
                    application.
                this_tile (int, optional): the tile index we need to generate
                    the return array for. A non-negative integer. If not
                    given, it sets to current locality.

            Returns:

                A future holding a 1-D array or 2-D array with all values send
                    by all participating localities.)"
            )
    };

    ///////////////////////////////////////////////////////////////////////////
    all_gather::all_gather(
            primitive_arguments_type&& operands,
            std::string const& name, std::string const& codename)
      : primitive_component_base(std::move(operands), name, codename)
    {}

    ///////////////////////////////////////////////////////////////////////////
//    template <typename T>
//    hpx::future<primitive_argument_type> all_gather::all_gather(
//        ir::node_data<T>&& arr, std::uint32_t tile_idx, std::uint32_t numtiles,
//        execution_tree::localities_information&& arr_localities) const
//    {
//        using namespace execution_tree;
//
//        // current annotation information
//        std::uint32_t const loc_id = arr_localities.locality_.locality_id_;
//        std::uint32_t const num_localities =
//            arr_localities.locality_.num_localities_;
//
//        // row/column dimension of the whole array
//        std::size_t row_size = arr_localities.rows();
//        std::size_t column_size = arr_localities.columns();
//
//        tiling_information_2d tile_info(
//            arr_localities.tiles_[loc_id], name_, codename_);
//
//        std::int64_t cur_row_start = tile_info.spans_[0].start_;
//        std::int64_t cur_row_stop = tile_info.spans_[0].stop_;
//        std::int64_t cur_col_start = tile_info.spans_[1].start_;
//        std::int64_t cur_col_stop = tile_info.spans_[1].stop_;
//
//        std::int64_t cur_row_size = cur_row_stop - cur_row_start;
//        std::int64_t cur_col_size = cur_col_stop - cur_col_start;
//
//        // updating the annotation_ part of localities annotation
//        arr_localities.annotation_.name_ += "_all_gather";
//        ++arr_localities.annotation_.generation_;
//
//        blaze::DynamicMatrix<T> result(row_size, column_size, T(0));
//
//        // desired annotation information
//        std::int64_t des_row_start, des_row_stop, des_col_start, des_col_stop,
//            des_row_size, des_col_size;
//
//        // updating the array
//        auto m = arr.matrix();
//        util::distributed_matrix<T> m_data(
//            arr_localities.annotation_.name_, m, num_localities, loc_id);
//
//        // copying the local part
//        blaze::submatrix(result, cur_row_start, cur_col_start,
//            cur_row_size, cur_col_size) = m;
//
//        // collecting other parts from remote localities
//        for (std::uint32_t loc = 0; loc != num_localities; ++loc)
//        {
//            if (loc == loc_id)
//            {
//                continue;
//            }
//            // the array span in locality loc
//            std::int64_t loc_row_start = 
//                arr_localities.tiles_[loc].spans_[0].start_;
//            std::int64_t loc_row_stop = 
//                arr_localities.tiles_[loc].spans_[0].stop_;
//            std::int64_t loc_col_start = 
//                arr_localities.tiles_[loc].spans_[1].start_;
//            std::int64_t loc_col_stop = 
//                arr_localities.tiles_[loc].spans_[1].stop_;
//
//            std::int64_t loc_row_size = loc_row_stop - loc_row_start;
//            std::int64_t loc_col_size = loc_col_stop - loc_col_start;
//
//            blaze::submatrix(result, loc_row_start, loc_col_start,
//                loc_row_size, loc_col_size) = m_data
//                    .fetch(loc, loc_row_start, loc_col_start,
//                        loc_row_size, loc_col_size)
//                    .get();
//            
//        }
//    }

    ///////////////////////////////////////////////////////////////////////////
    template <typename T>
    execution_tree::primitive_argument_type all_gather::all_gather1d(
        ir::node_data<T>&& local_result, std::size_t numtiles,
        std::size_t this_tile,
        execution_tree::localities_information&& locs) const
    {
        // use hpx::all_gather to get the whole vector of values
        auto p = hpx::all_to_all(
            ("all_gather_" + locs.annotation_.name_).c_str(),
            std::move(local_result), locs.locality_.num_localities_,
            locs.annotation_.generation_,
            locs.locality_.locality_id_)
                .get();

        // is it a row vector or a column vector?
        std::size_t span_index = 0;
        if (!locs.has_span(0))
        {
            HPX_ASSERT(locs.has_span(1));
            span_index = 1;
        }

        std::size_t dim = locs.size();
        blaze::DynamicMatrix<T> result(row_size, column_size, T(0));

        // combine the vector of values as a matrix
        for (std::vector<int>::iterator it = p.begin(); it != p.end(); ++i)
        {
            auto m = (*it).matrix();
            result = blaze::submatrix(
                m, rel_row_start, rel_col_start, des_row_size, des_col_size);


        }

        return primitive_argument_type



    }
    ///////////////////////////////////////////////////////////////////////////
    template <typename T>
    execution_tree::primitive_argument_type all_gather::all_gather1d2d(
        ir::node_data<T>&& local_result, std::size_t numtiles,
        std::size_t this_tile,
        execution_tree::localities_information&& locs) const
    {
        // use hpx::all_gather to get the whole vector of values
        auto overall_result = hpx::all_to_all(
            ("all_gather_" + locs.annotation_.name_).c_str(),
            std::move(local_result), locs.locality_.num_localities_,
            locs.annotation_.generation_,
            locs.locality_.locality_id_)
                .get();

        // row and column dimensions of the whole array 
        std::size_t rows_dim, cols_dim;
        rows_dim = locs.rows();
        cols_dim = locs.columns();

        // check the tiling type is column-tiling (0) or row-tiling (1)
        tiling_information_2d tile_info(
            locs.tiles_[loc_id], name_, codename_);

        std::int64_t cur_row_size = tile_info.spans_[0].size();
        std::int64_t cur_col_size = tile_info.spans_[1].size();

        std::size_t span_index;

        std::int64_t rel_row_start, rel_col_start;
        std::size_t row_size, column_size;

        if (cur_row_size == rows_dim)
        {
            // column-tiling
            span_index = 0;
            rel_row_start = 0;
            std::tie(rel_col_start, column_size) =
                tile_calculation::tile_calculation_1d(this_tile,
                cols_dim, numtiles);
        }
        else if (cur_col_size == cols_dim)
        {
            // row-tiling
            span_index = 1;
            rel_col_start = 0;
            std::tie(rel_row_start, row_size) =
                tile_calculation::tile_calculation_1d(this_tile,
                rows_dim, numtiles);
        }
        else
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                    "all_gather::all_gather1d2d",
                    generate_error_message(
                        "invalid tiling_type. The tiling_type can"
                        "be `row` or `column`"));
        }

        //combine the vector of values as a matrix according to the tiling-type
        blaze::DynamicMatrix<T> result(rows_dim, cols_dim, T(0));

        for (std::size_t j = 0; j != overall_result.size(); ++j)
        {

            auto m = overall_result[j].matrix();
            blaze::submatrix(
                result, rel_row_start, rel_col_start,
                cur_row_size, cur_col_size) = m;
        }

        return execution_tree::primitive_argument_type{std::move(result)};

    }
    ///////////////////////////////////////////////////////////////////////////
    execution_tree::primitive_argument_type all_gather::all_gather1d2d(
        execution_tree::primitive_arguments_type&& local_result,
        std::size_t numtiles, std::size_t this_tile) const
    {
        using namespace execution_tree;

        execution_tree::localities_information locs =
            extract_localities_information(local_result, name_, codename_);

        std::size_t ndim = locs.num_dimensions();
        if (!(ndim == 1 || ndim == 2))
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "all_gather::all_gather1d",
                generate_error_message(
                    "the operand has incompatible dimensionalities"));
        }

        switch (extract_common_type(local_result)
        {
        case node_data_type_bool:
            return all_gather1d2d(
                extract_boolean_value_strict(std::move(local_result), name_,
                codename_), numtiles, this_tile, std::move(arr_localities));

        case node_data_type_int64:
            return all_gather1d2d(
                extract_integer_value_strict(std::move(local_result), name_,
                codename_), numtiles, this_tile, std::move(arr_localities));

        case node_data_type_unknown:
            return all_gather1d2d(
                extract_numeric_value(std::move(local_result), name_,
                codename_), numtiles, this_tile, std::move(arr_localities));

        case node_data_type_double:
            return all_gather1d2d(
                extract_numeric_value_strict(std::move(local_result), name_,
                codename_), numtiles, this_tile, std::move(arr_localities));

        default:
            break;
        }

        HPX_THROW_EXCEPTION(hpx::bad_parameter,
            "phylanx::execution_tree::primitives::"
            "all_gather::all_gather1d2d",
            util::generate_error_message(
                "the primitive requires for all arguments to "
                "be numeric data types"));
    }
    ///////////////////////////////////////////////////////////////////////////

    hpx::future<execution_tree::primitive_argument_type> all_gather::eval(
        execution_tree::primitive_arguments_type const& operands,
        execution_tree::primitive_arguments_type const& args,
        execution_tree::eval_context ctx) const
    {
        if (operands.empty() || operands.size() > 3)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "all_gather::eval",
                generate_error_message(
                    "the all_gather primitive requires"
                    "at least 1 and at most 3 operands"));
        }

        if (!valid(operands[0]))
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter, "all_gather::eval",
                generate_error_message(
                    "the all_gather primitive requires the first argument"
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

                        std::size_t numtiles =
                            hpx::get_num_localities(hpx::launch::sync);
                        if (valid(args[1]))
                        {
                            numtiles = extract_scalar_positive_integer_value_strict(
                                std::move(args[1]), this_->name_, this_->codename_);
                        }

                        std::size_t this_tile = hpx::get_locality_id();
                        if (valid(args[2]))
                        {
                            this_tile = extract_scalar_nonneg_integer_value_strict(
                                std::move(args[2]), this_->name_, this_->codename_);
                        }

                        if (this_tile >= numtiles)
                        {
                            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                                "all_gather::eval",
                                this_->generate_error_message(
                                    "invalid tile index. Tile indices start from 0 "
                                    "and should be smaller than number of tiles"));
                        }

                        switch (extract_numeric_value_dimension(
                            std::move(args[0]), this_->name_, this_->codename_))
                        {
                            case 0:
                                return this_->all_gather0d(std::move(args[0]),
                                numtiles, this_tile);

                            case 1:
                                return this_->all_gather1d2d(std::move(args[0]),
                                numtiles, this_tile);

                            case 2:
                                return this_->all_gather1d2d(std::move(args[0]),
                                numtiles, this_tile);

                            default:
                                HPX_THROW_EXCEPTION(hpx::bad_parameter,
                                    "all_gather::eval",
                                    this_->generate_error_message(
                                        "operand a has an invalid number of "
                                        "dimensions"));
                        }
                    }),
        execution_tree::primitives::detail::map_operands(operands,
            execution_tree::functional::value_operand{}, args, name_,
            codename_, std::move(ctx)));
    }
}}}
