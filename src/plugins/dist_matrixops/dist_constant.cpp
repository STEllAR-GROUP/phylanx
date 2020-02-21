// Copyright (c) 2017-2020 Hartmut Kaiser
// Copyright (c) 2020 Bita Hasheminezhad
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
#include <phylanx/execution_tree/localities_annotation.hpp>

#include <hpx/include/lcos.hpp>
#include <hpx/include/naming.hpp>
#include <hpx/include/util.hpp>
#include <hpx/errors/throw_exception.hpp>

#include <cmath>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include <blaze/Math.h>
#include <blaze_tensor/Math.h>

///////////////////////////////////////////////////////////////////////////////
namespace phylanx { namespace dist_matrixops { namespace primitives
{
    ///////////////////////////////////////////////////////////////////////////
    namespace detail
    {
        std::size_t extract_num_dimensions(ir::range const& shape)
        {
            return shape.size();
        }

        std::array<std::size_t, PHYLANX_MAX_DIMENSIONS> extract_dimensions(
            ir::range const& shape)
        {
            std::array<std::size_t, PHYLANX_MAX_DIMENSIONS> result = {0, 0};
            if (!shape.empty())
            {
                if (shape.size() == 1)
                {
                    result[0] = extract_scalar_nonneg_integer_value_strict(
                        *shape.begin());
                }
                else if (shape.size() == 2)
                {
                    auto elem_1 = shape.begin();
                    result[0] =
                        extract_scalar_nonneg_integer_value_strict(*elem_1);
                    result[1] =
                        extract_scalar_nonneg_integer_value_strict(*++elem_1);
                }
                else if (shape.size() == 3)
                {
                    auto elem_1 = shape.begin();
                    result[0] =
                        extract_scalar_nonneg_integer_value_strict(*elem_1);
                    result[1] =
                        extract_scalar_nonneg_integer_value_strict(*++elem_1);
                    result[2] =
                        extract_scalar_nonneg_integer_value_strict(*++elem_1);
                }
                else if (shape.size() == 4)
                {
                    auto elem_1 = shape.begin();
                    result[0] =
                        extract_scalar_nonneg_integer_value_strict(*elem_1);
                    result[1] =
                        extract_scalar_nonneg_integer_value_strict(*++elem_1);
                    result[2] =
                        extract_scalar_nonneg_integer_value_strict(*++elem_1);
                    result[3] =
                        extract_scalar_nonneg_integer_value_strict(*++elem_1);
                }
            }
            return result;
        }
    }

    ///////////////////////////////////////////////////////////////////////////
    execution_tree::match_pattern_type const dist_constant::match_data =
    {

        hpx::util::make_tuple("constant_d", std::vector<std::string>{R"(
                constant_d(
                    _1_value,
                    _2_shape,
                    _3_tile_index,
                    _4_numtiles,
                    __arg(_5_tiling_type, "sym"),
                    __arg(_3_dtype, "float64")
                )
            )"},
            &create_dist_constant,
            &execution_tree::create_primitive<dist_constant>, R"(
            value, shape, tile_index, numtiles, tiling_type, dtype
            Args:

                value (float): fill value
                shape (int or list of ints): overall shape of the array. It
                    only contains positive integers.
                tile_index (int): the tile index we need to generate the
                    constant array for. A non-negative integer.
                numtiles (int): number of tiles of the returned array
                tiling_type (string, optional): defaults to `sym`
                dtype (string, optional): the data-type of the returned array,
                    defaults to 'float'.

            Returns:

            A part of an array of size 'shape' with each element equal to
            'value', which has the tile index of 'tile_index'.)")
    };

    ///////////////////////////////////////////////////////////////////////////
    dist_constant::dist_constant(
            execution_tree::primitive_arguments_type&& operands,
            std::string const& name, std::string const& codename)
      : primitive_component_base(std::move(operands), name, codename)
    {}

    ///////////////////////////////////////////////////////////////////////////
    template <typename T>
    execution_tree::primitive_argument_type dist_constant::constant1d_helper(
        execution_tree::primitive_argument_type&& value, std::size_t dim,
        std::size_t const& tile_idx, std::size_t const& numtiles,
        std::string const& tiling_type, std::string const& name,
        std::string const& codename) const
    {
        using namespace execution_tree;

        T const_value =
            extract_scalar_data<T>(std::move(value), name, codename);
        std::size_t size = static_cast<std::size_t>(dim / numtiles);
        std::int64_t start = 0;
        std::size_t remainder = dim % numtiles;
        if (tile_idx < remainder)
        {
            size++;
        }

        if (remainder == 0)
        {
            start = size * tile_idx;
        }
        else if (remainder != 0 && tile_idx >= remainder)
        {
            start = (size + 1) * remainder + size * (tile_idx - remainder);
        }

        tiling_information_1d tile_info(
            tiling_information_1d::tile1d_type::columns,
            tiling_span(start, start + size));
        locality_information locality_info(tile_idx, numtiles);
        annotation locality_ann = locality_info.as_annotation();
        std::string base_name =
            "constant_" + std::to_string(numtiles) + "_" + std::to_string(dim);
        annotation_information ann_info(base_name, 0);    //generation 0

        auto attached_annotation =
            std::make_shared<execution_tree::annotation>(localities_annotation(
                locality_ann, tile_info.as_annotation(name, codename), ann_info,
                name, codename));

        execution_tree::primitive_argument_type res(
            blaze::DynamicVector<T>(size, const_value), attached_annotation);

        return std::move(res);
    }

    execution_tree::primitive_argument_type dist_constant::constant1d(
        execution_tree::primitive_argument_type&& value,
        operand_type::dimensions_type const& dims, std::size_t const& tile_idx,
        std::size_t const& numtiles, std::string const& tiling_type,
        execution_tree::node_data_type dtype, std::string const& name_,
        std::string const& codename_) const
    {
        using namespace execution_tree;

        switch (dtype)
        {
        case node_data_type_bool:
            return constant1d_helper<std::uint8_t>(std::move(value), dims[0],
                tile_idx, numtiles, tiling_type, name_, codename_);

        case node_data_type_int64:
            return constant1d_helper<std::int64_t>(std::move(value), dims[0],
                tile_idx, numtiles, tiling_type, name_, codename_);

        case node_data_type_unknown: HPX_FALLTHROUGH;
        case node_data_type_double:
            return constant1d_helper<double>(std::move(value), dims[0],
                tile_idx, numtiles, tiling_type, name_, codename_);

        default:
            break;
        }

        HPX_THROW_EXCEPTION(hpx::bad_parameter,
            "dist_matrixops::dist_constant::constant1d"
                "constant1d",
            util::generate_error_message(
                "the constant primitive requires for all arguments to "
                    "be numeric data types"));
    }
    /////////////////////////////////////////////////////////////////////////////

    ///////////////////////////////////////////////////////////////////////////
    hpx::future<execution_tree::primitive_argument_type> dist_constant::eval(
        execution_tree::primitive_arguments_type const& operands,
        execution_tree::primitive_arguments_type const& args,
        execution_tree::eval_context ctx) const
    {
        // verify arguments
        if (operands.size() < 4 || operands.size() > 6)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "dist_constant::eval",
                generate_error_message(
                    "the constant_d primitive requires "
                        "at least 4 and at most 6 operands"));
        }


        for (auto const& i : operands)
        {
            if (!valid(i))
            {
                HPX_THROW_EXCEPTION(hpx::bad_parameter,
                    "dist_constant::eval",
                    generate_error_message(
                        "the constant_d primitive requires that the "
                        "arguments given by the operands array are valid"));
            }
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

                        dims = detail::extract_dimensions(overall_shape);
                        numdims = detail::extract_num_dimensions(overall_shape);
                    }
                    else if (is_numeric_operand(args[1]))
                    {
                        // support constant_d(42, 3, 0, 3), to annotate the
                        // first tile of [42, 42, 42]
                        numdims = 1;
                        dims[0] = extract_scalar_positive_integer_value_strict(
                            std::move(args[0]), this_->name_, this_->codename_);
                    }

                    std::size_t tile_idx =
                        extract_scalar_nonneg_integer_value_strict(
                            std::move(args[2]), this_->name_, this_->codename_);
                    std::size_t numtiles =
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

                    //using balanced symmetric tiles
                    std::string tiling_type = "sym";

                    node_data_type dtype = node_data_type_unknown;
                    if (valid(args[5]))
                    {
                        dtype =
                            map_dtype(extract_string_value(std::move(args[5]),
                                this_->name_, this_->codename_));
                    }

                    switch (numdims)
                    {
                    case 1:
                        return this_->constant1d(std::move(args[0]), dims,
                            tile_idx, numtiles, tiling_type, dtype,
                            this_->name_, this_->codename_);
                    case 2:
                    case 3:
                    default:
                        HPX_THROW_EXCEPTION(hpx::bad_parameter,
                            "dist_constant::eval",
                            util::generate_error_message(
                                "the given shape is of an unsupported "
                                "dimensionality",
                                this_->name_, this_->codename_));
                    }

                }),
            execution_tree::primitives::detail::map_operands(operands,
                execution_tree::functional::value_operand{}, args, name_,
                codename_, std::move(ctx)));
    }
}}}

