// Copyright (c) 2020 Nanmiao Wu
// Copyright (c) 2020 Bita Hasheminezhad
// Copyright (c) 2017-2020 Hartmut Kaiser
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>
#include <phylanx/execution_tree/annotation.hpp>
#include <phylanx/execution_tree/localities_annotation.hpp>
#include <phylanx/execution_tree/locality_annotation.hpp>
#include <phylanx/execution_tree/meta_annotation.hpp>
#include <phylanx/execution_tree/primitives/node_data_helpers.hpp>
#include <phylanx/execution_tree/tiling_annotations.hpp>
#include <phylanx/ir/node_data.hpp>
#include <phylanx/plugins/dist_matrixops/dist_identity.hpp>
#include <phylanx/plugins/dist_matrixops/tile_calculation_helper.hpp>

#include <hpx/errors/throw_exception.hpp>
#include <hpx/include/lcos.hpp>
#include <hpx/include/naming.hpp>
#include <hpx/include/util.hpp>

#include <atomic>
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
namespace phylanx { namespace dist_matrixops { namespace primitives {
    ///////////////////////////////////////////////////////////////////////////
    execution_tree::match_pattern_type const dist_identity::match_data = {

        hpx::make_tuple("identity_d", std::vector<std::string>{R"(
                identity_d(
                    _1_sz,
                    __arg(_2_tile_index, find_here()),
                    __arg(_3_numtiles, num_localities()),
                    __arg(_4_name, ""),
                    __arg(_5_tiling_type, "sym"),
                    __arg(_6_dtype, nil)

                )
            )"},
            &create_dist_identity,
            &execution_tree::create_primitive<dist_identity>, R"(
            sz, dtype, tile_index, numtiles, name, tiling_type,
            Args:
                sz (int): the size of a created (n x n) matrix.
                tile_index (int, optional): the tile index we need to generate
                    the identity array for. A non-negative integer. If not
                    given, it sets to current locality.
                numtiles (int, optional): number of tiles of the returned array.
                    If not given it sets to the number of localities in the
                    application.
                name (string, optional): the array given name. If not given, a
                    globally unique name will be generated.
                tiling_type (string, optional): defaults to `sym` which is a
                    balanced way of tiling among all the numtiles localities.
                    Other options are `row` or `column` tiling.
                dtype (string, optional): the data-type of the returned array,
                    defaults to 'float'.

            Returns:

            A part of an array of size 'size', which has the tile index of
             'tile_index'.)")};

    ///////////////////////////////////////////////////////////////////////////
    dist_identity::dist_identity(
        execution_tree::primitive_arguments_type&& operands,
        std::string const& name, std::string const& codename)
      : primitive_component_base(std::move(operands), name, codename)
    {
    }

    ///////////////////////////////////////////////////////////////////////////
    namespace detail {
        static std::atomic<std::size_t> identity_count(0);
        std::string generate_identity_name(std::string&& given_name)
        {
            if (given_name.empty())
            {
                return "identity_array_" + std::to_string(++identity_count);
            }

            return std::move(given_name);
        }
    }    // namespace detail

    ///////////////////////////////////////////////////////////////////////////
    template <typename T>
    execution_tree::primitive_argument_type dist_identity::dist_identity_helper(
        std::int64_t&& sz, std::uint32_t const& tile_idx,
        std::uint32_t const& numtiles, std::string&& given_name,
        std::string const& tiling_type) const
    {
        using namespace execution_tree;

        std::size_t size = static_cast<std::size_t>(sz);

        std::int64_t row_start, column_start;
        std::size_t row_size, column_size;

        std::tie(row_start, column_start, row_size, column_size) =
            tile_calculation::tile_calculation_2d(
                tile_idx, size, size ,numtiles, tiling_type);

        tiling_information_2d tile_info(
            tiling_span(row_start, row_start + row_size),
            tiling_span(column_start, column_start + column_size));

        locality_information locality_info(tile_idx, numtiles);
        annotation locality_ann = locality_info.as_annotation();

        std::string base_name =
            detail::generate_identity_name(std::move(given_name));

        annotation_information ann_info(
            std::move(base_name), 0);    //generation 0

        auto attached_annotation =
            std::make_shared<annotation>(localities_annotation(locality_ann,
                tile_info.as_annotation(name_, codename_), ann_info, name_,
                codename_));

        blaze::DynamicMatrix<T> m(row_size, column_size, T(0));
        if (tiling_type == "row")
        {
            blaze::band(m, row_start) = T(1);
        }
        else if (tiling_type == "column")
        {
            blaze::band(m, -column_start) = T(1);
        }
        else if (tiling_type == "sym")
        {
            std::int64_t num_band = row_start - column_start;
            std::int64_t upper_band = column_size - 1;
            std::int64_t lower_band = 1 - row_size;

            if (num_band <= (std::max)(int64_t(0), upper_band) &&
                num_band >= (std::min)(int64_t(0), lower_band))
            {
                blaze::band(m, num_band) = T(1);
            }
        }
        else
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "dist_identity::dist_identity_helper",
                generate_error_message(
                    "wrong numtiles input when tiling_type is sym"));
        }

        return primitive_argument_type(std::move(m), attached_annotation);
    }

    execution_tree::primitive_argument_type dist_identity::dist_identity_nd(
        std::int64_t&& sz, std::uint32_t const& tile_idx,
        std::uint32_t const& numtiles, std::string&& given_name,
        std::string const& tiling_type,
        execution_tree::node_data_type dtype) const
    {
        using namespace execution_tree;

        switch (dtype)
        {
        case node_data_type_bool:
            return dist_identity_helper<std::uint8_t>(std::move(sz), tile_idx,
                numtiles, std::move(given_name), tiling_type);

        case node_data_type_int64:
            return dist_identity_helper<std::int64_t>(std::move(sz), tile_idx,
                numtiles, std::move(given_name), tiling_type);

        case node_data_type_unknown:
            HPX_FALLTHROUGH;
        case node_data_type_double:
            return dist_identity_helper<double>(std::move(sz), tile_idx,
                numtiles, std::move(given_name), tiling_type);

        default:
            break;
        }

        HPX_THROW_EXCEPTION(hpx::bad_parameter,
            "dist_matrixops::dist_identity::dist_identity_nd",
            util::generate_error_message(
                "the constant primitive requires for all arguments to "
                "be numeric data types"));
    }

    ///////////////////////////////////////////////////////////////////////////
    hpx::future<execution_tree::primitive_argument_type> dist_identity::eval(
        execution_tree::primitive_arguments_type const& operands,
        execution_tree::primitive_arguments_type const& args,
        execution_tree::eval_context ctx) const
    {
        // verify arguments
        if (operands.size() < 1 || operands.size() > 6)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter, "dist_identity::eval",
                generate_error_message("the identity_d primitive requires "
                                       "at least 1 and at most 6 operands"));
        }

        if (!valid(operands[0]))
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter, "dist_identity::eval",
                generate_error_message(
                    "the identity_d primitive requires the first argument"
                    "given by the operands array is valid"));
        }

        auto this_ = this->shared_from_this();
        return hpx::dataflow(hpx::launch::sync,
            hpx::util::unwrapping(
                [this_ = std::move(this_)](
                    execution_tree::primitive_arguments_type&& args)
                    -> execution_tree::primitive_argument_type {
                    using namespace execution_tree;

                    std::int64_t sz = extract_scalar_integer_value_strict(
                        std::move(args[0]), this_->name_, this_->codename_);

                    std::uint32_t tile_idx = hpx::get_locality_id();
                    if (valid(args[1]))
                    {
                        tile_idx = extract_scalar_nonneg_integer_value_strict(
                            std::move(args[1]), this_->name_, this_->codename_);
                    }
                    std::uint32_t numtiles =
                        hpx::get_num_localities(hpx::launch::sync);
                    if (valid(args[2]))
                    {
                        numtiles = extract_scalar_positive_integer_value_strict(
                            std::move(args[2]), this_->name_, this_->codename_);
                    }
                    if (tile_idx >= numtiles)
                    {
                        HPX_THROW_EXCEPTION(hpx::bad_parameter,
                            "dist_identity::eval",
                            this_->generate_error_message(
                                "invalid tile index. Tile indices start from 0 "
                                "and should be smaller than number of tiles"));
                    }

                    std::string given_name = "";
                    if (valid(args[3]))
                    {
                        given_name = extract_string_value(
                            std::move(args[3]), this_->name_, this_->codename_);
                    }

                    // using balanced symmetric tiles
                    std::string tiling_type = "sym";
                    if (valid(args[4]))
                    {
                        tiling_type = extract_string_value(
                            std::move(args[4]), this_->name_, this_->codename_);
                        if ((tiling_type != "sym" && tiling_type != "row") &&
                            tiling_type != "column")
                        {
                            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                                "dist_identity::eval",
                                this_->generate_error_message(
                                    "invalid tling_type. the tiling_type cane "
                                    "one of these: `sym`, `row` or `column`"));
                        }
                    }

                    node_data_type dtype = node_data_type_unknown;
                    if (valid(args[5]))
                    {
                        dtype =
                            map_dtype(extract_string_value(std::move(args[5]),
                                this_->name_, this_->codename_));
                    }

                    return this_->dist_identity_nd(std::move(sz), tile_idx,
                        numtiles, std::move(given_name), tiling_type, dtype);
                }),
            execution_tree::primitives::detail::map_operands(operands,
                execution_tree::functional::value_operand{}, args, name_,
                codename_, std::move(ctx)));
    }
}}}    // namespace phylanx::dist_matrixops::primitives
