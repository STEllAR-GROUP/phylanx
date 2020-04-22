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
#include <phylanx/plugins/dist_matrixops/dist_diag.hpp>
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

///////////////////////////////////////////////////////////////////////////////
namespace phylanx { namespace dist_matrixops { namespace primitives {
    ///////////////////////////////////////////////////////////////////////////
    execution_tree::match_pattern_type const dist_diag::match_data = {

        hpx::util::make_tuple("diag_d", std::vector<std::string>{R"(
                diag_d(
                    _1_arr,
                    __arg(_2_k, 0),
                    __arg(_3_tile_index, find_here()),
                    __arg(_4_numtiles, num_localities()),
                    __arg(_5_name, ""),
                    __arg(_6_tiling_type, "sym"),
                    __arg(_7_dtype, nil)

                )
            )"},
            &create_dist_diag,
            &execution_tree::create_primitive<dist_diag>, R"(
            arr, k, tile_index, numtiles, name, tiling_type, dtype
            Args:
                arr (array): a scalar, vector or matrix.
                k (int, optional): The default is 0. Denote the diagonal above
                    the main diagonal when k > 0 and the diagonal below the main
                    diagonal when k < 0.
                tile_index (int, optional): the tile index we need to generate
                    the diag array for. A non-negative integer. If not
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

            A 1-D array of its k-th diagonal when a is a 2-D array; a 2-D array
            with a on thek-th diagonal.)")};

    ///////////////////////////////////////////////////////////////////////////
    dist_diag::dist_diag(
        execution_tree::primitive_arguments_type&& operands,
        std::string const& name, std::string const& codename)
      : primitive_component_base(std::move(operands), name, codename)
    {
    }

    ///////////////////////////////////////////////////////////////////////////
    namespace detail {
        static std::atomic<std::size_t> diag_count(0);
        std::string generate_diag_name(std::string&& given_name)
        {
            if (given_name.empty())
            {
                return "diag_array_" + std::to_string(++diag_count);
            }

            return std::move(given_name);
        }
    }    // namespace detail

    ///////////////////////////////////////////////////////////////////////////

    template <typename T>
    execution_tree::primitive_argument_type dist_diag::dist_diag1d_helper(
        ir::node_data<T>&& arr, std::int64_t k,
        std::uint32_t const& tile_idx, std::uint32_t const& numtiles,
        std::string&& given_name, std::string const& tiling_type) const
    {
        using namespace execution_tree;

        std::size_t size = arr.dimension(0) + std::abs(k);

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
            detail::generate_diag_name(std::move(given_name));

        annotation_information ann_info(
            std::move(base_name), 0);    //generation 0

        auto attached_annotation =
            std::make_shared<annotation>(localities_annotation(locality_ann,
                tile_info.as_annotation(name_, codename_), ann_info, name_,
                codename_));

        blaze::DynamicMatrix<T> result(row_size, column_size, T(0));
        std::int64_t num_band;
        std::int64_t upper_band = column_size - 1;
        std::int64_t lower_band = 1 - row_size;

        if (tiling_type == "row")
        {
            num_band = row_start + k;
            if (num_band <= (std::max)(int64_t(0), upper_band) &&
                    num_band >= (std::min)(int64_t(0), lower_band))
            {
                std::int64_t des_start = k < 0 ?
                    (std::max)(int64_t(0), num_band) : row_start;

                std::int64_t des_size = num_band < 0 ?
                    (std::min)(column_size, row_size + num_band) :
                    (std::min)(row_size, column_size - num_band);

                blaze::band(result, num_band) =
                    blaze::subvector(arr.vector(), des_start, des_size);
            }
        }
        else if (tiling_type == "column")
        {
            num_band = k - column_start;
            if (num_band <= (std::max)(int64_t(0), upper_band) &&
                    num_band >= (std::min)(int64_t(0), lower_band))
            {
                std::int64_t des_start = k < 0 ?
                    column_start : (std::max)(int64_t(0), - num_band);

                std::int64_t des_size = num_band < 0 ?
                    (std::min)(column_size, row_size + num_band) :
                    (std::min)(row_size, column_size - num_band);

                blaze::band(result, num_band) =
                    blaze::subvector(arr.vector(), des_start, des_size);
            }
        }
        else if (tiling_type == "sym")
        {
            num_band = k - (column_start - row_start);
            if (num_band <= (std::max)(int64_t(0), upper_band) &&
                    num_band >= (std::min)(int64_t(0), lower_band))
            {
                std::int64_t des_start = k < 0 ?
                    (std::max)(column_start, column_start + num_band) :
                    (std::max)(row_start, row_start - num_band);

                std::int64_t des_size = num_band < 0 ?
                    (std::min)(column_size, row_size + num_band) :
                    (std::min)(row_size, column_size - num_band);

                blaze::band(result, num_band) =
                    blaze::subvector(arr.vector(), des_start, des_size);
            }
        }
        else
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "dist_diag::dist_diag1d_helper",
                generate_error_message(
                    "wrong numtiles input when tiling_type is sym"));
        }

        return primitive_argument_type(ir::node_data<T>{std::move(result)},
            attached_annotation);
    }

    execution_tree::primitive_argument_type dist_diag::dist_diag1d(
        execution_tree::primitive_argument_type&& arr, std::int64_t k,
        std::uint32_t const& tile_idx, std::uint32_t const& numtiles,
        std::string&& given_name, std::string const& tiling_type,
        execution_tree::node_data_type dtype) const
    {
        using namespace execution_tree;

        switch (dtype)
        {
        case node_data_type_bool:
            return dist_diag1d_helper<std::uint8_t>(
                extract_boolean_value_strict(std::move(arr), name_, codename_),
                k, tile_idx, numtiles, std::move(given_name), tiling_type);

        case node_data_type_int64:
            return dist_diag1d_helper<std::int64_t>(
                extract_integer_value_strict(std::move(arr), name_, codename_),
                k, tile_idx, numtiles, std::move(given_name), tiling_type);

        case node_data_type_unknown:
            HPX_FALLTHROUGH;

        case node_data_type_double:
            return dist_diag1d_helper<double>(
                extract_numeric_value(std::move(arr), name_, codename_),
                k, tile_idx, numtiles, std::move(given_name), tiling_type);

        default:
            break;
        }

        HPX_THROW_EXCEPTION(hpx::bad_parameter,
            "dist_matrixops::dist_diag::dist_diag1d",
            util::generate_error_message(
                "the constant primitive requires for all arguments to "
                "be numeric data types"));
    }

    template <typename T>
    execution_tree::primitive_argument_type dist_diag::dist_diag2d_helper(
        ir::node_data<T>&& arr, std::int64_t k,
        std::uint32_t const& tile_idx, std::uint32_t const& numtiles,
        std::string&& given_name, std::string const& tiling_type) const
    {
        using namespace execution_tree;

        std::size_t row_dim = arr.dimension(0);
        std::size_t column_dim = arr.dimension(1);


        std::int64_t row_start, column_start;
        std::size_t row_size, column_size;

        std::tie(row_start, column_start, row_size, column_size) =
            tile_calculation::tile_calculation_2d(
                tile_idx, row_dim, column_dim ,numtiles, tiling_type);

        tiling_information_2d tile_info(
            tiling_span(row_start, row_start + row_size),
            tiling_span(column_start, column_start + column_size));

        locality_information locality_info(tile_idx, numtiles);
        annotation locality_ann = locality_info.as_annotation();

        std::string base_name =
            detail::generate_diag_name(std::move(given_name));

        annotation_information ann_info(
            std::move(base_name), 0);    //generation 0

        auto attached_annotation =
            std::make_shared<annotation>(localities_annotation(locality_ann,
                tile_info.as_annotation(name_, codename_), ann_info, name_,
                codename_));

        //create an empty array
        blaze::DynamicVector<T> result;
        auto arr_Vec = blaze::band(arr.matrix(), k);
        std::int64_t num_band;
        std::int64_t upper_band = column_size - 1;
        std::int64_t lower_band = 1 - row_size;

        if (tiling_type == "row")
        {
            num_band = row_start + k;

            if (num_band <= (std::max)(int64_t(0), upper_band) &&
                num_band >= (std::min)(int64_t(0), lower_band))
            {
                std::int64_t result_sz = num_band < 0 ?
                    (std::min)(column_size, row_size + num_band) :
                    (std::min)(row_size, column_size - num_band);

                result.resize(result_sz);

                std::int64_t des_start = k < 0 ?
                    (std::max)(int64_t(0), num_band) : row_start;

                result = blaze::subvector(arr_Vec, des_start, result_sz);
            }
        }
        else if (tiling_type == "column")
        {
            num_band = k - column_start;
            if (num_band <= (std::max)(int64_t(0), upper_band) &&
                    num_band >= (std::min)(int64_t(0), lower_band))
            {
                std::int64_t result_sz = num_band < 0 ?
                    (std::min)(column_size, row_size + num_band) :
                    (std::min)(row_size, column_size - num_band);

                result.resize(result_sz);

                std::int64_t des_start = k < 0 ?
                    column_start : (std::max)(int64_t(0), column_start - k);

                result = blaze::subvector(arr_Vec, des_start, result_sz);
            }
        }
        else if (tiling_type == "sym")
        {
            num_band = k - (column_start - row_start);
            if (num_band <= (std::max)(int64_t(0), upper_band) &&
                    num_band >= (std::min)(int64_t(0), lower_band))
            {
                std::int64_t result_sz = num_band < 0 ?
                (std::min)(column_size, row_size + num_band) :
                (std::min)(row_size, column_size - num_band);

                result.resize(result_sz);

                std::int64_t des_start = k < 0 ?
                    (std::max)(column_start, row_start + k) :
                    (std::max)(row_start, column_start - k);

                result = blaze::subvector(arr_Vec, des_start, result_sz);
            }
        }
        else
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "dist_diag::dist_diag2d_helper",
                generate_error_message(
                    "wrong numtiles input when tiling_type is sym"));
        }

        return primitive_argument_type(ir::node_data<T>{std::move(result)},
            attached_annotation);
    }

    execution_tree::primitive_argument_type dist_diag::dist_diag2d(
        execution_tree::primitive_argument_type&& arr, std::int64_t k,
        std::uint32_t const& tile_idx, std::uint32_t const& numtiles,
        std::string&& given_name, std::string const& tiling_type,
        execution_tree::node_data_type dtype) const
    {
        using namespace execution_tree;

        switch (dtype)
        {
        case node_data_type_bool:
            return dist_diag2d_helper<std::uint8_t>(
                extract_boolean_value_strict(std::move(arr), name_, codename_),
                k, tile_idx, numtiles, std::move(given_name), tiling_type);

        case node_data_type_int64:
            return dist_diag2d_helper<std::int64_t>(
                extract_integer_value_strict(std::move(arr), name_, codename_),
                k, tile_idx, numtiles, std::move(given_name), tiling_type);

        case node_data_type_unknown:
            HPX_FALLTHROUGH;

        case node_data_type_double:
            return dist_diag2d_helper<double>(
                extract_numeric_value(std::move(arr), name_, codename_),
                k, tile_idx, numtiles, std::move(given_name), tiling_type);
        }

        HPX_THROW_EXCEPTION(hpx::bad_parameter,
            "dist_matrixops::dist_diag::dist_diag2d",
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
        if (operands.empty() || operands.size() > 7)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter, "dist_diag::eval",
                generate_error_message("the diag_d primitive requires "
                                       "at least 1 and at most 7 operands"));
        }

        if (!valid(operands[0]) || !valid(operands[1]))
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter, "dist_diag::eval",
                generate_error_message(
                    "the diag_d primitive requires the first two arguments"
                    "given by the operands array are valid"));
        }

        auto this_ = this->shared_from_this();
        return hpx::dataflow(hpx::launch::sync,
            hpx::util::unwrapping(
                [this_ = std::move(this_)](
                        execution_tree::primitive_arguments_type&& args)
                -> execution_tree::primitive_argument_type
                {
                    using namespace execution_tree;

                    std::int64_t k = extract_scalar_integer_value_strict(
                            std::move(args[1]), this_->name_, this_->codename_);

                    std::uint32_t tile_idx = hpx::get_locality_id();
                    if (valid(args[2]))
                    {
                        tile_idx = extract_scalar_nonneg_integer_value_strict(
                            std::move(args[2]), this_->name_, this_->codename_);
                    }
                    std::uint32_t numtiles =
                        hpx::get_num_localities(hpx::launch::sync);
                    if (valid(args[3]))
                    {
                        numtiles = extract_scalar_positive_integer_value_strict(
                            std::move(args[3]), this_->name_, this_->codename_);
                    }
                    if (tile_idx >= numtiles)
                    {
                        HPX_THROW_EXCEPTION(hpx::bad_parameter,
                            "dist_diag::eval",
                            this_->generate_error_message(
                                "invalid tile index. Tile indices start from 0 "
                                "and should be smaller than number of tiles"));
                    }

                    std::string given_name = "";
                    if (valid(args[4]))
                    {
                        given_name = extract_string_value(
                            std::move(args[4]), this_->name_, this_->codename_);
                    }

                    // using balanced symmetric tiles
                    std::string tiling_type = "sym";
                    if (valid(args[5]))
                    {
                        tiling_type = extract_string_value(
                            std::move(args[5]), this_->name_, this_->codename_);
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

                    node_data_type dtype =
                        extract_common_type(std::move(args[0]),
                        this_->name_, this_->codename_));

                    if (valid(args[6]))
                    {
                        dtype =
                            map_dtype(extract_string_value(std::move(args[6]),
                                this_->name_, this_->codename_));
                    }

                    switch (extract_numeric_value_dimension(
                        args[0], this_->name_, this_->codename_))
                    {
                    case 1:
                        return this_->dist_diag1d(std::move(args[0]),
                            k, tile_idx, numtiles, std::move(given_name),
                            tiling_type, dtype);

                    case 2:
                        return this_->dist_diag2d(std::move(args[0]),
                            k, tile_idx, numtiles, std::move(given_name),
                            tiling_type, dtype);

                    default:
                        HPX_THROW_EXCEPTION(hpx::bad_parameter,
                            "dist_diag::eval",
                            this_->generate_error_message(
                                "the given shape is of an unsupported "
                                "dimensionality"));
                    }
                }),
            execution_tree::primitives::detail::map_operands(operands,
                execution_tree::functional::value_operand{}, args, name_,
                codename_, std::move(ctx)));
    }
}}}    // namespace phylanx::dist_matrixops::primitives
