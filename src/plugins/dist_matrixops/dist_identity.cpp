// Copyright (c) 2017-2020 Hartmut Kaiser
// Copyright (c) 2020 Nanmiao Wu
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>
#include <phylanx/execution_tree/annotation.hpp>
#include <phylanx/execution_tree/locality_annotation.hpp>
#include <phylanx/execution_tree/meta_annotation.hpp>
#include <phylanx/execution_tree/primitives/node_data_helpers.hpp>
#include <phylanx/execution_tree/tiling_annotations.hpp>
#include <phylanx/ir/node_data.hpp>
#include <phylanx/plugins/common/constant_nd.hpp>
#include <phylanx/plugins/dist_matrixops/dist_constant.hpp>
#include <phylanx/plugins/dist_matrixops/tile_calculation_helper.hpp>
#include <phylanx/execution_tree/localities_annotation.hpp>

#include <hpx/include/lcos.hpp>
#include <hpx/include/naming.hpp>
#include <hpx/include/util.hpp>
#include <hpx/errors/throw_exception.hpp>

#include <array>
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
namespace phylanx { namespace dist_matrixops { namespace primitives
{
    ///////////////////////////////////////////////////////////////////////////
    execution_tree::match_pattern_type const dist_constant::match_data =
    {

        hpx::util::make_tuple("identity_d", std::vector<std::string>{R"(
                identity_d(
                    _1_size, 
                    __arg(_2_tile_index, nil),
                    __arg(_3_numtiles, nil),
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
                tile_index (int): the tile index we need to generate the
                    constant array for. A non-negative integer.
                numtiles (int): number of tiles of the returned array
                name (string, optional): the array given name. If not given, a
                    globally unique name will be generated.
                tiling_type (string, optional): defaults to `sym` which is a
                    balanced way of tiling among all the numtiles localities.
                    Other options are `row` or `column` tiling. 
                dtype (string, optional): the data-type of the returned array,
                    defaults to 'float'.
                

            Returns:

            A part of an array of size 'size', which has the tile index of
             'tile_index'.)")
    };

    ///////////////////////////////////////////////////////////////////////////
    dist_constant::dist_identity(
            execution_tree::primitive_arguments_type&& operands,
            std::string const& name, std::string const& codename)
      : primitive_component_base(std::move(operands), name, codename)
    {}

    ///////////////////////////////////////////////////////////////////////////
    namespace detail
    {
        static std::atomic<std::size_t> const_count(0);
        std::string generate_const_name(std::string&& given_name)
        {
            if (given_name.empty())
            {
                return "full_array_" + std::to_string(++const_count);
            }

            return std::move(given_name);
        }
    }

   
    ///////////////////////////////////////////////////////////////////////////
    template <typename T>
    execution_tree::primitive_argument_type dist_identity::identity_helper(
        std::int64_t&& sz,
        std::uint32_t const& tile_idx, std::uint32_t const& numtiles,
        std::string&& given_name, std::string const& tiling_type,
        std::string const& name, std::string const& codename) const
    {
        using namespace execution_tree;
        if (sz < 0)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "dist_identity::identity_helper",
                generate_error_message("input should be greater than zero"));
        }

        std::size_t size = static_cast<std::size_t>(sz);
        
        T const_value =
            extract_scalar_data<T>(std::move(value), name, codename);

        std::int64_t row_start, column_start;
        std::size_t row_size, column_size;
        std::uint32_t row_dim = size;
        std::uint32_t column_dim = size;

        std::tie(row_start, column_start, row_size, column_size) =
            tile_calculation::tile_calculation_2d(
                tile_idx, row_dim, column_dim, numtiles, tiling_type);

        tiling_information_2d tile_info(
            tiling_span(row_start, row_start + row_size),
            tiling_span(column_start, column_start + column_size));

        locality_information locality_info(tile_idx, numtiles);
        annotation locality_ann = locality_info.as_annotation();

        std::string base_name =
            detail::generate_const_name(std::move(given_name));

        annotation_information ann_info(base_name, 0);    //generation 0

        auto attached_annotation =
            std::make_shared<annotation>(localities_annotation(
                locality_ann, tile_info.as_annotation(name, codename), ann_info,
                name, codename));

        primitive_argument_type res(
            blaze::DynamicMatrix<T>(row_size, column_size, const_value),
            attached_annotation);

        return std::move(res);
    }

    execution_tree::primitive_argument_type dist_identity::identity_nd(
        std::int64_t&& sz,
        std::uint32_t const& tile_idx, std::uint32_t const& numtiles,
        std::string&& given_name, std::string const& tiling_type,
        execution_tree::node_data_type dtype, std::string const& name_,
        std::string const& codename_) const
    {
        using namespace execution_tree;

        switch (dtype)
        {
        case node_data_type_bool:
            return identity_helper<std::uint8_t>(std::move(sz), tile_idx, 
            numtiles, std::move(given_name), tiling_type, name_, codename_);

        case node_data_type_int64:
            return identity_helper<std::int64_t>(std::move(sz), tile_idx, 
            numtiles, std::move(given_name), tiling_type, name_, codename_);

        case node_data_type_unknown: HPX_FALLTHROUGH;
        case node_data_type_double:
            return identity_helper<double>(std::move(sz), tile_idx, 
            numtiles, std::move(given_name), tiling_type, name_, codename_);

        default:
            break;
        }

        HPX_THROW_EXCEPTION(hpx::bad_parameter,
            "dist_matrixops::dist_identity::identity_nd",
            util::generate_error_message(
                "the constant primitive requires for all arguments to "
                    "be numeric data types"));
    }

    ///////////////////////////////////////////////////////////////////////////
    hpx::future<execution_tree::primitive_argument_type> dist_constant::eval(
        execution_tree::primitive_arguments_type const& operands,
        execution_tree::primitive_arguments_type const& args,
        execution_tree::eval_context ctx) const
    {
        // verify arguments
        if (operands.size() < 2 || operands.size() > 7)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "dist_constant::eval",
                generate_error_message(
                    "the constant_d primitive requires "
                        "at least 2 and at most 7 operands"));
        }

        if (!valid(operands[0]) || !valid(operands[1]))
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter, "dist_constant::eval",
                generate_error_message(
                    "the constant_d primitive requires the first two arguments "
                    "given by the operands array are valid"));
        }

        auto this_ = this->shared_from_this();
        return hpx::dataflow(hpx::launch::sync, hpx::util::unwrapping(
                [this_ = std::move(this_)](
                    execution_tree::primitive_arguments_type&& args)
                -> execution_tree::primitive_argument_type
                {
                    using namespace execution_tree;

                    if (valid(args[0]) &&
                        extract_numeric_value_dimension(args[0]) != 0)
                    {
                        HPX_THROW_EXCEPTION(hpx::bad_parameter,
                            "dist_constant::eval",
                            this_->generate_error_message(
                                "the first argument must be a literal "
                                "scalar value"));
                    }

                    std::array<std::size_t, PHYLANX_MAX_DIMENSIONS> dims{0};
                    std::size_t numdims = 0;
                    if (is_list_operand_strict(args[1]))
                    {
                        ir::range&& overall_shape = extract_list_value_strict(
                            std::move(args[1]), this_->name_, this_->codename_);

                        if (overall_shape.size() > PHYLANX_MAX_DIMENSIONS)
                        {
                            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                                "dist_constant::eval",
                                this_->generate_error_message(
                                    "the given shape has a number of "
                                    "dimensions that is not supported"));
                        }

                        dims =
                            common::extract_dimensions(overall_shape);
                        numdims = common::extract_num_dimensions(
                            overall_shape);
                    }
                    else if (is_numeric_operand(args[1]))
                    {
                        // support constant_d(42, 3, 0, 3), to annotate the
                        // first tile of [42, 42, 42]
                        numdims = 1;
                        dims[0] = extract_scalar_positive_integer_value_strict(
                            std::move(args[1]), this_->name_, this_->codename_);
                    }

                    std::string given_name = "";
                    if (valid(args[4]))
                    {
                        given_name = extract_string_value(std::move(args[4]),
                            this_->name_, this_->codename_);
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
                                "dist_constant::eval",
                                this_->generate_error_message(
                                    "invalid tling_type. the tiling_type cane "
                                    "one of these: `sym`, `row` or `column`"));
                        }
                    }

                    node_data_type dtype = node_data_type_unknown;
                    if (valid(args[6]))
                    {
                        dtype =
                            map_dtype(extract_string_value(std::move(args[6]),
                                this_->name_, this_->codename_));
                    }

                    if (valid(args[2]) && valid(args[3]))
                    {
                        std::uint32_t tile_idx =
                            extract_scalar_nonneg_integer_value_strict(
                                std::move(args[2]), this_->name_, this_->codename_);
                        std::uint32_t numtiles =
                            extract_scalar_positive_integer_value_strict(
                                std::move(args[3]), this_->name_, this_->codename_);
                        if (tile_idx >= numtiles)
                        {
                            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                                "dist_constant::eval",
                                this_->generate_error_message(
                                    "invalid tile index. Tile indices start from 0 "
                                    "and should be smaller than number of tiles"));
                        }

                        switch (numdims)
                        {
                        case 1:
                            return this_->constant1d(std::move(args[0]), dims,
                                tile_idx, numtiles, std::move(given_name),
                                dtype, this_->name_, this_->codename_);

                        case 2:
                            return this_->constant2d(std::move(args[0]), dims,
                                tile_idx, numtiles, std::move(given_name),
                                tiling_type, dtype, this_->name_, this_->codename_);

                        default:
                            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                                "dist_constant::eval",
                                util::generate_error_message(
                                    "the given shape is of an unsupported "
                                    "dimensionality",
                                    this_->name_, this_->codename_));
                        }
                    }
                    else if (!valid(args[2]) && !valid(args[3]))
                    {
                        switch (numdims)
                        {
                        case 0:
                            return common::constant0d(std::move(args[0]), dtype,
                                false, this_->name_, this_->codename_);

                        case 1:
                            return common::constant1d(std::move(args[0]),
                                dims[0], dtype, false, this_->name_,
                                this_->codename_);

                        case 2:
                            return common::constant2d(std::move(args[0]), dims,
                                dtype, false, this_->name_, this_->codename_);

                        case 3:
                            return common::constant3d(std::move(args[0]), dims,
                                dtype, false, this_->name_, this_->codename_);

                        case 4:
                            return common::constant4d(std::move(args[0]), dims,
                                dtype, false, this_->name_, this_->codename_);

                        default:
                            break;
                        }
                        HPX_THROW_EXCEPTION(hpx::bad_parameter,
                            "dist_constant::eval",
                            util ::generate_error_message(
                                "left hand side operand has unsupported "
                                "number of dimensions"));
                    }
                    else
                    {
                        HPX_THROW_EXCEPTION(hpx::bad_parameter,
                            "dist_constant::eval",
                            util::generate_error_message(
                                "both tile_idx and numtiles should be given to "
                                "generate a distributed constant",
                                this_->name_, this_->codename_));
                    }

                }),
            execution_tree::primitives::detail::map_operands(operands,
                execution_tree::functional::value_operand{}, args, name_,
                codename_, std::move(ctx)));
    }
}}}

