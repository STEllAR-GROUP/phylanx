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
#include <phylanx/plugins/dist_matrixops/dist_constant.hpp>
#include <phylanx/plugins/dist_matrixops/tile_calculation_helper.hpp>
#include <phylanx/execution_tree/localities_annotation.hpp>
#include <phylanx/util/detail/range_dimension.hpp>

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

        hpx::util::make_tuple("constant_d", std::vector<std::string>{R"(
                constant_d(
                    _1_value,
                    _2_shape,
                    __arg(_3_tile_index, find_here()),
                    __arg(_4_numtiles, num_localities()),
                    __arg(_5_name, ""),
                    __arg(_6_tiling_type, "sym"),
                    __arg(_7_itersection, nil),
                    __arg(_8_dtype, "float64")
                )
            )"},
            &create_dist_constant,
            &execution_tree::create_primitive<dist_constant>, R"(
            value, shape, tile_index, numtiles, name, tiling_type, intersection,
            dtype
            Args:

                value (float): fill value
                shape (int or list of ints): overall shape of the array. It
                    only contains positive integers.
                tile_index (int, optional): the tile index we need to generate
                    the constant array for. A non-negative integer. If not given,
                    it sets to current locality.
                numtiles (int, optional): number of tiles of the returned array.
                    if not given it sets to the number of localities in the
                    application.
                name (string, optional): the array given name. If not given, a
                    globally unique name will be generated.
                tiling_type (string, optional): defaults to `sym` which is a
                    balanced way of tiling among all the numtiles localities.
                    Other options are `page`, `row` or `column` tiling. For a
                    vector all these three tiling_type are the same.
                intersection (int or a tuple of ints, optional): the size of
                    overlapped part on each dimension. If an integer is given,
                    that would be the intersection length on all dimensions
                    that are tiled. The middle parts get to have two
                    intersections, one with the tile before it and one with the
                    tile after it.
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
    execution_tree::primitive_argument_type dist_constant::constant1d_helper(
        execution_tree::primitive_argument_type&& value, std::size_t const& dim,
        std::uint32_t const& tile_idx, std::uint32_t const& numtiles,
        std::string&& given_name, std::size_t intersection) const
    {
        using namespace execution_tree;

        T const_value =
            extract_scalar_data<T>(std::move(value), name_, codename_);

        std::int64_t start;
        std::uint32_t size;
        std::tie(start, size) =
            tile_calculation::tile_calculation_1d(tile_idx, dim, numtiles);

        // adding overlap
        if (intersection != 0)
        {
            std::tie(start, size) =
                tile_calculation::tile_calculation_overlap_1d(
                    start, size, dim, intersection);
        }

        tiling_information_1d tile_info(
            tiling_information_1d::tile1d_type::columns,
            tiling_span(start, start + size));
        locality_information locality_info(tile_idx, numtiles);
        annotation locality_ann = locality_info.as_annotation();

        std::string base_name =
            detail::generate_const_name(std::move(given_name));

        annotation_information ann_info(
            std::move(base_name), 0);    //generation 0

        auto attached_annotation =
            std::make_shared<annotation>(localities_annotation(locality_ann,
                tile_info.as_annotation(name_, codename_), ann_info, name_,
                codename_));

        return primitive_argument_type(
            blaze::DynamicVector<T>(size, const_value), attached_annotation);
    }

    execution_tree::primitive_argument_type dist_constant::constant1d(
        execution_tree::primitive_argument_type&& value,
        operand_type::dimensions_type const& dims,
        std::uint32_t const& tile_idx, std::uint32_t const& numtiles,
        std::string&& given_name, std::size_t intersection,
        execution_tree::node_data_type dtype) const
    {
        using namespace execution_tree;

        switch (dtype)
        {
        case node_data_type_bool:
            return constant1d_helper<std::uint8_t>(std::move(value), dims[0],
                tile_idx, numtiles, std::move(given_name), intersection);

        case node_data_type_int64:
            return constant1d_helper<std::int64_t>(std::move(value), dims[0],
                tile_idx, numtiles, std::move(given_name), intersection);

        case node_data_type_unknown: HPX_FALLTHROUGH;
        case node_data_type_double:
            return constant1d_helper<double>(std::move(value), dims[0],
                tile_idx, numtiles, std::move(given_name), intersection);

        default:
            break;
        }

        HPX_THROW_EXCEPTION(hpx::bad_parameter,
            "dist_matrixops::dist_constant::constant1d",
            util::generate_error_message(
                "the constant primitive requires for all arguments to "
                    "be numeric data types"));
    }

    ///////////////////////////////////////////////////////////////////////////
    template <typename T>
    execution_tree::primitive_argument_type dist_constant::constant2d_helper(
        execution_tree::primitive_argument_type&& value,
        operand_type::dimensions_type const& dims,
        std::uint32_t const& tile_idx, std::uint32_t const& numtiles,
        std::string&& given_name, std::string const& tiling_type,
        std::array<std::size_t, PHYLANX_MAX_DIMENSIONS> const& intersections)
        const
    {
        using namespace execution_tree;

        T const_value =
            extract_scalar_data<T>(std::move(value), name_, codename_);

        std::int64_t row_start, column_start;
        std::size_t row_size, column_size;
        std::uint32_t row_dim = dims[0];
        std::uint32_t column_dim = dims[1];

        std::tie(row_start, column_start, row_size, column_size) =
            tile_calculation::tile_calculation_2d(
                tile_idx, row_dim, column_dim, numtiles, tiling_type);

        // adding overlap
        if (row_size != row_dim &&
            intersections[0] != 0)    // rows overlap
        {
            std::tie(row_start, row_size) =
                tile_calculation::tile_calculation_overlap_1d(
                    row_start, row_size, row_dim, intersections[0]);
        }
        if (column_size != column_dim &&
            intersections[1] != 0)    // columns overlap
        {
            std::tie(column_start, column_size) =
                tile_calculation::tile_calculation_overlap_1d(
                    column_start, column_size, column_dim, intersections[1]);
        }

        tiling_information_2d tile_info(
            tiling_span(row_start, row_start + row_size),
            tiling_span(column_start, column_start + column_size));

        locality_information locality_info(tile_idx, numtiles);
        annotation locality_ann = locality_info.as_annotation();

        std::string base_name =
            detail::generate_const_name(std::move(given_name));

        annotation_information ann_info(
            std::move(base_name), 0);    //generation 0

        auto attached_annotation =
            std::make_shared<annotation>(localities_annotation(locality_ann,
                tile_info.as_annotation(name_, codename_), ann_info, name_,
                codename_));

        return primitive_argument_type(
            blaze::DynamicMatrix<T>(row_size, column_size, const_value),
            attached_annotation);
    }

    execution_tree::primitive_argument_type dist_constant::constant2d(
        execution_tree::primitive_argument_type&& value,
        operand_type::dimensions_type const& dims,
        std::uint32_t const& tile_idx, std::uint32_t const& numtiles,
        std::string&& given_name, std::string const& tiling_type,
        std::array<std::size_t, PHYLANX_MAX_DIMENSIONS> const& intersections,
        execution_tree::node_data_type dtype) const
    {
        using namespace execution_tree;

        switch (dtype)
        {
        case node_data_type_bool:
            return constant2d_helper<std::uint8_t>(std::move(value), dims,
                tile_idx, numtiles, std::move(given_name), tiling_type,
                intersections);

        case node_data_type_int64:
            return constant2d_helper<std::int64_t>(std::move(value), dims,
                tile_idx, numtiles, std::move(given_name), tiling_type,
                intersections);

        case node_data_type_unknown: HPX_FALLTHROUGH;
        case node_data_type_double:
            return constant2d_helper<double>(std::move(value), dims, tile_idx,
                numtiles, std::move(given_name), tiling_type, intersections);

        default:
            break;
        }

        HPX_THROW_EXCEPTION(hpx::bad_parameter,
            "dist_matrixops::dist_constant::constant2d",
            util::generate_error_message(
                "the constant primitive requires for all arguments to "
                    "be numeric data types"));
    }

    ///////////////////////////////////////////////////////////////////////////
    template <typename T>
    execution_tree::primitive_argument_type dist_constant::constant3d_helper(
        execution_tree::primitive_argument_type&& value,
        operand_type::dimensions_type const& dims,
        std::uint32_t const& tile_idx, std::uint32_t const& numtiles,
        std::string&& given_name, std::string const& tiling_type,
        std::array<std::size_t, PHYLANX_MAX_DIMENSIONS> const& intersections)
        const
    {
        using namespace execution_tree;

        T const_value =
            extract_scalar_data<T>(std::move(value), name_, codename_);

        std::int64_t page_start, row_start, column_start;
        std::size_t page_size, row_size, column_size;
        std::uint32_t page_dim = dims[0];
        std::uint32_t row_dim = dims[1];
        std::uint32_t column_dim = dims[2];

        std::tie(page_start, row_start, column_start, page_size, row_size,
            column_size) = tile_calculation::tile_calculation_3d(tile_idx,
            page_dim, row_dim, column_dim, numtiles, tiling_type);

        // adding overlap
        if (page_size != page_dim && intersections[0] != 0)    // pages overlap
        {
            std::tie(page_start, page_size) =
                tile_calculation::tile_calculation_overlap_1d(
                    page_start, page_size, page_dim, intersections[0]);
        }
        if (row_size != row_dim && intersections[1] != 0)    // rows overlap
        {
            std::tie(row_start, row_size) =
                tile_calculation::tile_calculation_overlap_1d(
                    row_start, row_size, row_dim, intersections[1]);
        }
        if (column_size != column_dim &&
            intersections[2] != 0)    // columns overlap
        {
            std::tie(column_start, column_size) =
                tile_calculation::tile_calculation_overlap_1d(
                    column_start, column_size, column_dim, intersections[2]);
        }

        tiling_information_3d tile_info(
            tiling_span(page_start, page_start + page_size),
            tiling_span(row_start, row_start + row_size),
            tiling_span(column_start, column_start + column_size));

        locality_information locality_info(tile_idx, numtiles);
        annotation locality_ann = locality_info.as_annotation();

        std::string base_name =
            detail::generate_const_name(std::move(given_name));

        annotation_information ann_info(
            std::move(base_name), 0);    //generation 0

        auto attached_annotation =
            std::make_shared<annotation>(localities_annotation(locality_ann,
                tile_info.as_annotation(name_, codename_), ann_info, name_,
                codename_));

        return primitive_argument_type(blaze::DynamicTensor<T>(page_size,
                                           row_size, column_size, const_value),
            attached_annotation);
    }

    execution_tree::primitive_argument_type dist_constant::constant3d(
        execution_tree::primitive_argument_type&& value,
        operand_type::dimensions_type const& dims,
        std::uint32_t const& tile_idx, std::uint32_t const& numtiles,
        std::string&& given_name, std::string const& tiling_type,
        std::array<std::size_t, PHYLANX_MAX_DIMENSIONS> const& intersections,
        execution_tree::node_data_type dtype) const
    {
        using namespace execution_tree;

        switch (dtype)
        {
        case node_data_type_bool:
            return constant3d_helper<std::uint8_t>(std::move(value), dims,
                tile_idx, numtiles, std::move(given_name), tiling_type,
                intersections);

        case node_data_type_int64:
            return constant3d_helper<std::int64_t>(std::move(value), dims,
                tile_idx, numtiles, std::move(given_name), tiling_type,
                intersections);

        case node_data_type_unknown: HPX_FALLTHROUGH;
        case node_data_type_double:
            return constant3d_helper<double>(std::move(value), dims, tile_idx,
                numtiles, std::move(given_name), tiling_type, intersections);

        default:
            break;
        }

        HPX_THROW_EXCEPTION(hpx::bad_parameter,
            "dist_matrixops::dist_constant::constant3d",
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
        if (operands.size() < 2 || operands.size() > 8)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "dist_constant::eval",
                generate_error_message(
                    "the constant_d primitive requires "
                        "at least 2 and at most 8 operands"));
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

                        dims = util::detail::extract_positive_range_dimensions(
                            overall_shape, this_->name_, this_->codename_);
                        numdims = util::detail::extract_range_num_dimensions(
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

                    std::uint32_t tile_idx = hpx::get_locality_id();
                    if (valid(args[2]))
                    {
                        extract_scalar_nonneg_integer_value_strict(
                            std::move(args[2]), this_->name_, this_->codename_);
                    }
                    std::uint32_t numtiles =
                        hpx::get_num_localities(hpx::launch::sync);
                    if (valid(args[3]))
                    {
                        extract_scalar_positive_integer_value_strict(
                            std::move(args[3]), this_->name_, this_->codename_);
                    }
                    if (tile_idx >= numtiles)
                    {
                        HPX_THROW_EXCEPTION(hpx::bad_parameter,
                            "dist_constant::eval",
                            this_->generate_error_message(
                                "invalid tile index. Tile indices start from 0 "
                                "and should be smaller than number of tiles"));
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
                        if ((tiling_type != "sym" && tiling_type != "page") &&
                            tiling_type != "row" && tiling_type != "column")
                        {
                            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                                "dist_constant::eval",
                                this_->generate_error_message(
                                    "invalid tiling_type. The tiling_type can "
                                    "be one of these: `sym`, `page`, `row` or "
                                    "`column`"));
                        }
                    }

                    std::array<std::size_t, PHYLANX_MAX_DIMENSIONS>
                        intersections{0};
                    if (valid(args[6]))
                    {
                        if (is_list_operand_strict(args[6]))
                        {
                            ir::range&& intersection_list =
                                extract_list_value_strict(std::move(args[6]),
                                    this_->name_, this_->codename_);

                            if (intersection_list.size() != 1 &&
                                intersection_list.size() != numdims)
                            {
                                HPX_THROW_EXCEPTION(hpx::bad_parameter,
                                    "dist_constant::eval",
                                    this_->generate_error_message(
                                        "intersection should have the same "
                                        "number of dimensions as the array, or "
                                        "be represented with an integer for "
                                        "all dimensions"));
                            }
                            intersections =
                                util::detail::extract_nonneg_range_dimensions(
                                    intersection_list, this_->name_,
                                    this_->codename_);
                        }
                        else if (is_numeric_operand(args[6]))
                        {
                            intersections[0] =
                                extract_scalar_nonneg_integer_value_strict(
                                    std::move(args[6]), this_->name_,
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
                                "dist_constant::eval",
                                this_->generate_error_message(
                                    "intersection can be an integer or a list "
                                    "of integers"));
                        }
                    }

                    node_data_type dtype = node_data_type_unknown;
                    if (valid(args[7]))
                    {
                        dtype =
                            map_dtype(extract_string_value(std::move(args[7]),
                                this_->name_, this_->codename_));
                    }

                    switch (numdims)
                    {
                    case 1:
                        return this_->constant1d(std::move(args[0]), dims,
                            tile_idx, numtiles, std::move(given_name),
                            intersections[0], dtype);

                    case 2:
                        return this_->constant2d(std::move(args[0]), dims,
                            tile_idx, numtiles, std::move(given_name),
                            tiling_type, intersections, dtype);

                    case 3:
                        return this_->constant3d(std::move(args[0]), dims,
                            tile_idx, numtiles, std::move(given_name),
                            tiling_type, intersections, dtype);

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

