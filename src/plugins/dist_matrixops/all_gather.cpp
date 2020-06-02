// Copyright (c) 2020 Hartmut Kaiser
// Copyright (c) 2020 Nanmiao Wu
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
#include <phylanx/plugins/dist_matrixops/tile_calculation_helper.hpp>
#include <phylanx/plugins/matrixops/concatenate.hpp>
#include <phylanx/util/distributed_matrix.hpp>
#include <phylanx/util/distributed_vector.hpp>
#include <phylanx/util/generate_error_message.hpp>
#include <phylanx/util/index_calculation_helper.hpp>


#include <hpx/assertion.hpp>
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
namespace phylanx { namespace dist_matrixops { namespace primitives
{
    ///////////////////////////////////////////////////////////////////////////
    match_pattern_type const all_gather::match_data =
    {
        hpx::util::make_tuple("all_gather", std::vector<std::string>{R"(
                all_gather_d(
                    _1_local_result,
                    __arg(_2_numtiles, num_localities()),
                    __arg(_3_this_tile, find_here())
                )
            )"},
            &create_all_gather,
            &execution_tree::create_primitive<all_gather>, R"(
            local_result, numtiles, this_tile
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
            execution_tree::primitive_arguments_type&& operands,
            std::string const& name, std::string const& codename)
      : primitive_component_base(std::move(operands), name, codename)
    {}

    ///////////////////////////////////////////////////////////////////////////
    template <typename T>
    execution_tree::primitive_argument_type all_gather::all_gather2d(
        ir::node_data<T>&& local_result, std::size_t numtiles,
        std::size_t this_tile,
        execution_tree::localities_information&& locs) const
    {
        using namespace execution_tree;
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

        // check the tiling type is column-tiling or row-tiling
        tiling_information_2d tile_info(
            locs.tiles_[loc_id], name_, codename_);

        std::int64_t cur_row_size = tile_info.spans_[0].size();
        std::int64_t cur_col_size = tile_info.spans_[1].size();

        std::int64_t axis; // along with the array will be joined
        if (cur_row_size == rows_dim)
        {
            // column-tiling
            axis = 1;
        }
        else if (cur_col_size == cols_dim)
        {
            // row-tiling
            axis = 0;
        }
        else
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                    "all_gather::all_gather2d",
                    util::generate_error_message(
                        "invalid tiling_type. The tiling_type can"
                        "be `row` or `column`"));
        }

        //concatenate the vector of values according to the tiling-type
        return concatenate::concatenate2d_helper(primitive_argument_type
            {ir::node_data<T>{std::move(local_result)}}, std::move(axis));
    }
    ///////////////////////////////////////////////////////////////////////////
    execution_tree::primitive_argument_type all_gather::all_gather2d(
        execution_tree::primitive_arguments_type&& local_result,
        std::size_t numtiles, std::size_t this_tile) const
    {
        using namespace execution_tree;

        execution_tree::localities_information locs =
            extract_localities_information(local_result, name_, codename_);

        std::size_t ndim = locs.num_dimensions();
        if (!(ndim == 2 || ndim == 1))
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "all_gather::all_gather2d",
                util::generate_error_message(
                    "the operand has incompatible dimensionalities"));
        }

        switch (extract_common_type(local_result)
        {
        case node_data_type_bool:
            return all_gather2d(
                extract_boolean_value_strict(std::move(local_result), name_,
                codename_), numtiles, this_tile, std::move(arr_localities));

        case node_data_type_int64:
            return all_gather2d(
                extract_integer_value_strict(std::move(local_result), name_,
                codename_), numtiles, this_tile, std::move(arr_localities));

        case node_data_type_unknown:
            return all_gather2d(
                extract_numeric_value(std::move(local_result), name_,
                codename_), numtiles, this_tile, std::move(arr_localities));

        case node_data_type_double:
            return all_gather2d(
                extract_numeric_value_strict(std::move(local_result), name_,
                codename_), numtiles, this_tile, std::move(arr_localities));

        default:
            break;
        }

        HPX_THROW_EXCEPTION(hpx::bad_parameter,
            "phylanx::execution_tree::primitives::"
            "all_gather::all_gather2d",
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
                            numtiles =
                                extract_scalar_positive_integer_value_strict(
                                std::move(args[1]), this_->name_,
                                this_->codename_);
                        }

                        std::size_t this_tile = hpx::get_locality_id();
                        if (valid(args[2]))
                        {
                            this_tile =
                                extract_scalar_nonneg_integer_value_strict(
                                std::move(args[2]), this_->name_,
                                this_->codename_);
                        }

                        if (this_tile >= numtiles)
                        {
                            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                                "all_gather::eval",
                                this_->generate_error_message(
                                    "invalid tile index. Tile indices start"
                                    "from 0 and should be smaller than number"
                                    "of tiles"));
                        }

                        switch (extract_numeric_value_dimension(
                            std::move(args[0]), this_->name_, this_->codename_))
                        {

                            case 1:
                                HPX_FALLTHROUGH;

                            case 2:
                                return this_->all_gather2d(std::move(args[0]),
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
}}}    // namespace phylanx::dist_matrixops::primitives

