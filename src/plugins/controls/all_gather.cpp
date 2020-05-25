// Copyright (c) 2020 Hartmut Kaiser
// Copyright (c) 2020 Nanmiao Wu
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>
#include <phylanx/execution_tree/localities_annotation.hpp>
#include <phylanx/plugins/controls/all_gather.hpp>

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
                "all_gather(_1, _2, _3)", "all_gather(_1, _2, _3, _4)"
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
                root_site (int, optional): The site that is responsible for
                    creating the all_gather support object. This value is 
                    optional and defaults to '0' (zero).

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
    primitive_argument_type all_gather::all_gather_helper(
        ir::node_data<T>&& arr, std::size_t numtiles,
        std::size_t this_tile, std::size_t root_site = 0,
        execution_tree::localities_information&& arr_localities) const
    {
        // use hpx::all_gather to get the whole vector of values
        auto p = hpx::all_to_all(
            ("all_gather_" + arr_localities.annotation_.name_).c_str(),
            std::move(arr), arr_localities.locality_.num_localities_,
            arr_localities.annotation_.generation_,
            arr_localities.locality_.locality_id_)
                .get();

        std::size_t row_size, column_size;
        row_size = arr_localities.rows();
        column_size = arr_localities.columns();
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
    execution_tree::primitive_argument_type all_gather::all_gather(
        execution_tree::primitive_arguments_type&& args) const
    {
        execution_tree::localities_information locs =
            extract_localities_information(args[0], name_, codename_);

        switch (extract_common_type(arr))
        {
        case node_data_type_bool:
            return all_gather_helper(
                extract_boolean_value_strict(std::move(arr), name_, codename_),
                numtiles, this_tile, root_site, std::move(arr_localities));

        case node_data_type_int64:
            return all_gather_helper(
                extract_integer_value_strict(std::move(arr), name_, codename_),
                numtiles, this_tile, root_site, std::move(arr_localities));

        case node_data_type_unknown:
            return all_gather_helper(
                extract_numeric_value(std::move(arr), name_, codename_),
                numtiles, this_tile, root_site, std::move(arr_localities));

        case node_data_type_double:
            return all_gather_helper(
                extract_numeric_value_strict(std::move(arr), name_, codename_),
                numtiles, this_tile, root_site, std::move(arr_localities));

        default:
            break;
        }

        HPX_THROW_EXCEPTION(hpx::bad_parameter,
            "phylanx::execution_tree::primitives::"
                "all_gather::all_gather_array",
            generate_error_message(
                "the primitive requires for all arguments to "
                "be numeric data types"));
    }
    ///////////////////////////////////////////////////////////////////////////

    hpx::future<execution_tree::primitive_argument_type> all_gather::eval(
        primitive_arguments_type const& operands,
        primitive_arguments_type const& args, eval_context ctx) const
    {
        if (operands.size() < 1)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "all_gather::eval",
                generate_error_message(
                    "the all_gather primitive requires at "
                        "least one operand"));
        }

        for (auto const& i : operands)
        {
            if (!valid(i))
            {
                HPX_THROW_EXCEPTION(hpx::bad_parameter,
                    "all_gather::eval",
                    generate_error_message(
                        "the all_gather primitive requires that the "
                        "arguments given by the operands array are valid"));
            }
        }


        auto this_ = this->shared_from_this();
        return hpx::dataflow(hpx::launch::sync,
            hpx::util::unwrapping(
                [this_ = std::move(this_)](
                    execution_tree::primitive_arguments_type&& args)
                    -> execution_tree::primitive_argument_type {
                    std::size_t a_dims =
                        execution_tree::extract_numeric_value_dimension(
                            args[0], this_->name_, this_->codename_);
                    switch (a_dims)
                    {
                    case 0:
                        return this_->all_gather0d(std::move(args));

                    case 1:
                        return this_->all_gather1d(std::move(args));

                    case 2:
                        return this_->all_gather2d(std::move(args));

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
