// Copyright (c) 2018 Parsa Amini
// Copyright (c) 2018-2020 Hartmut Kaiser
// Copyright (c) 2018-2020 Bita Hasheminezhad
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>
#include <phylanx/execution_tree/localities_annotation.hpp>
#include <phylanx/execution_tree/meta_annotation.hpp>
#include <phylanx/execution_tree/tiling_annotations.hpp>
#include <phylanx/ir/node_data.hpp>
#include <phylanx/plugins/matrixops/expand_dims.hpp>
#include <phylanx/util/matrix_iterators.hpp>

#include <hpx/include/lcos.hpp>
#include <hpx/include/naming.hpp>
#include <hpx/include/util.hpp>
#include <hpx/errors/throw_exception.hpp>

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include <blaze/Math.h>
#include <blaze_tensor/Math.h>

///////////////////////////////////////////////////////////////////////////////
namespace phylanx { namespace execution_tree { namespace primitives
{
    ///////////////////////////////////////////////////////////////////////////
    match_pattern_type const expand_dims::match_data =
    {
        match_pattern_type("expand_dims",
            std::vector<std::string>{"expand_dims(_1,_2)"},
            &create_expand_dims, &create_primitive<expand_dims>, R"(
            arg, axis
            Args:

                arg (number or list of numbers): number or list of numbers
                axis (integer): an axis to expand along

            Returns:

            Expand the shape of an array by adding a dimension.
            )")
    };

    ///////////////////////////////////////////////////////////////////////////
    expand_dims::expand_dims(primitive_arguments_type&& operands,
            std::string const& name, std::string const& codename)
      : primitive_component_base(std::move(operands), name, codename)
    {}

    ///////////////////////////////////////////////////////////////////////////
    template <typename T>
    primitive_argument_type expand_dims::expand_dims_0d(
        ir::node_data<T>&& arg) const
    {
        return primitive_argument_type{
            blaze::DynamicVector<T>(1, arg.scalar())};
    }

    primitive_argument_type expand_dims::expand_dims_0d(
        primitive_arguments_type&& args) const
    {
        std::int64_t axis =
            extract_scalar_integer_value_strict(args[1], name_, codename_);

        if (axis != 0 && axis != -1)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "phylanx::execution_tree::primitives::expand_dims::expand_dims_0d",
                generate_error_message(
                    "the expand_dims primitive requires operand axis to be "
                    "either 0 or -1 for scalars."));
        }


        switch (extract_common_type(args[0]))
        {
        case node_data_type_bool:
            return expand_dims_0d(extract_boolean_value_strict(
                std::move(args[0]), name_, codename_));

        case node_data_type_int64:
            return expand_dims_0d(extract_integer_value_strict(
                std::move(args[0]), name_, codename_));

        case node_data_type_double:
            return expand_dims_0d(extract_numeric_value_strict(
                std::move(args[0]), name_, codename_));

        case node_data_type_unknown:
            return expand_dims_0d(extract_numeric_value(
                std::move(args[0]), name_, codename_));

        default:
            break;
        }

        HPX_THROW_EXCEPTION(hpx::bad_parameter,
            "phylanx::execution_tree::primitives::expand_dims::expand_dims_0d",
            generate_error_message(
                "the expand_dims primitive requires for all arguments to "
                    "be numeric data types"));
    }

    ///////////////////////////////////////////////////////////////////////////
    template <typename T>
    primitive_argument_type expand_dims::expand_dims_1d(
        ir::node_data<T>&& arg, std::int64_t axis) const
    {
        auto data = arg.vector();
        if (axis == 0)
        {
            blaze::DynamicMatrix<T> result(1, data.size());
            blaze::row(result, 0) = blaze::trans(data);
            return primitive_argument_type{std::move(result)};
        }

        // axis == 1
        blaze::DynamicMatrix<T> result(data.size(), 1);
        blaze::column(result, 0) = data;
        return primitive_argument_type{std::move(result)};
    }

    template <typename T>
    primitive_argument_type expand_dims::expand_dims_1d(ir::node_data<T>&& arg,
        std::int64_t axis, localities_information&& arr_localities) const
    {
        auto v = arg.vector();
        std::uint32_t const loc_id = arr_localities.locality_.locality_id_;
        std::uint32_t const num_localities =
            arr_localities.locality_.num_localities_;
        tiling_information_1d current_tile_info(
            arr_localities.tiles_[loc_id], name_, codename_);
        std::int64_t start = current_tile_info.span_.start_;
        std::int64_t stop = current_tile_info.span_.stop_;

        arr_localities.annotation_.name_ += "_expanded";
        ++arr_localities.annotation_.generation_;
        auto locality_ann = arr_localities.locality_.as_annotation();

        if (axis == 0)
        {
            blaze::DynamicMatrix<T> result(1, v.size());
            blaze::row(result, 0) = blaze::trans(v);
            tiling_information_2d tile_info = tiling_information_2d(
                tiling_span(0, 1), tiling_span(start, stop));
            auto attached_annotation =
                std::make_shared<annotation>(localities_annotation(locality_ann,
                    tile_info.as_annotation(name_, codename_),
                    arr_localities.annotation_, name_, codename_));
            return primitive_argument_type(std::move(result), attached_annotation);
        }

        // axis == 1
        blaze::DynamicMatrix<T> result(v.size(), 1);
        blaze::column(result, 0) = v;
        tiling_information_2d tile_info =
            tiling_information_2d(tiling_span(start, stop), tiling_span(0, 1));
        auto attached_annotation =
            std::make_shared<annotation>(localities_annotation(locality_ann,
                tile_info.as_annotation(name_, codename_),
                arr_localities.annotation_, name_, codename_));
        return primitive_argument_type(std::move(result), attached_annotation);
    }

    primitive_argument_type expand_dims::expand_dims_1d(
        primitive_arguments_type&& args) const
    {
        std::int64_t axis =
            extract_scalar_integer_value_strict(args[1], name_, codename_);

        if (axis > 1 || axis < -2)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "phylanx::execution_tree::primitives::expand_dims::expand_dims_1d",
                generate_error_message(
                    "the expand_dims primitive requires operand axis "
                    "to be between -2 and 1 for vector values."));
        }

        if (axis < 0)
        {
            axis += 2;
        }

        if (args[0].has_annotation())
        {
            localities_information arr_localities =
                extract_localities_information(args[0], name_, codename_);

            switch (extract_common_type(args[0]))
            {
            case node_data_type_bool:
                return expand_dims_1d(extract_boolean_value_strict(
                                          std::move(args[0]), name_, codename_),
                    axis, std::move(arr_localities));

            case node_data_type_int64:
                return expand_dims_1d(extract_integer_value_strict(
                                          std::move(args[0]), name_, codename_),
                    axis, std::move(arr_localities));

            case node_data_type_double:
                return expand_dims_1d(extract_numeric_value_strict(
                                          std::move(args[0]), name_, codename_),
                    axis, std::move(arr_localities));

            case node_data_type_unknown:
                return expand_dims_1d(
                    extract_numeric_value(std::move(args[0]), name_, codename_),
                    axis, std::move(arr_localities));

            default:
                break;
            }

            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "phylanx::execution_tree::primitives::expand_dims::expand_dims_"
                "1d",
                generate_error_message(
                    "the expand_dims primitive requires for all arguments to "
                    "be numeric data types"));
        }

        switch (extract_common_type(args[0]))
        {
        case node_data_type_bool:
            return expand_dims_1d(extract_boolean_value_strict(
                std::move(args[0]), name_, codename_), axis);

        case node_data_type_int64:
            return expand_dims_1d(extract_integer_value_strict(
                std::move(args[0]), name_, codename_), axis);

        case node_data_type_double:
            return expand_dims_1d(extract_numeric_value_strict(
                std::move(args[0]), name_, codename_), axis);

        case node_data_type_unknown:
            return expand_dims_1d(extract_numeric_value(
                std::move(args[0]), name_, codename_), axis);

        default:
            break;
        }

        HPX_THROW_EXCEPTION(hpx::bad_parameter,
            "phylanx::execution_tree::primitives::expand_dims::expand_dims_1d",
            generate_error_message(
                "the expand_dims primitive requires for all arguments to "
                    "be numeric data types"));
    }

    ///////////////////////////////////////////////////////////////////////////
    template <typename T>
    primitive_argument_type expand_dims::expand_dims_2d(
        ir::node_data<T>&& arg, std::int64_t axis) const
    {
        auto data = arg.matrix();
        if (axis == 0)
        {
            blaze::DynamicTensor<T> result(1, data.rows(), data.columns());
            blaze::pageslice(result, 0) = data;
            return primitive_argument_type{std::move(result)};
        }
        if (axis == 1)
        {
            blaze::DynamicTensor<T> result(data.rows(), 1, data.columns());
            blaze::rowslice(result, 0) = blaze::trans(data);
            return primitive_argument_type{std::move(result)};
        }

        // axis == 2
        blaze::DynamicTensor<T> result(data.rows(), data.columns(), 1);
        blaze::columnslice(result, 0) = data;
        return primitive_argument_type{std::move(result)};
    }

    primitive_argument_type expand_dims::expand_dims_2d(
        primitive_arguments_type&& args) const
    {
        std::int64_t axis =
            extract_scalar_integer_value_strict(args[1], name_, codename_);

        if (axis > 2 || axis < -3)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "phylanx::execution_tree::primitives::expand_dims::expand_dims_2d",
                generate_error_message(
                    "the expand_dims primitive requires operand axis "
                    "to be between -3 and 2 for matrix values."));
        }

        if (axis < 0)
        {
            axis += 3;
        }

        switch (extract_common_type(args[0]))
        {
        case node_data_type_bool:
            return expand_dims_2d(extract_boolean_value_strict(
                std::move(args[0]), name_, codename_), axis);

        case node_data_type_int64:
            return expand_dims_2d(extract_integer_value_strict(
                std::move(args[0]), name_, codename_), axis);

        case node_data_type_double:
            return expand_dims_2d(extract_numeric_value_strict(
                std::move(args[0]), name_, codename_), axis);

        case node_data_type_unknown:
            return expand_dims_2d(extract_numeric_value(
                std::move(args[0]), name_, codename_), axis);

        default:
            break;
        }

        HPX_THROW_EXCEPTION(hpx::bad_parameter,
            "phylanx::execution_tree::primitives::expand_dims::expand_dims_2d",
            generate_error_message(
                "the arange primitive requires for all arguments to "
                    "be numeric data types"));
    }

    ///////////////////////////////////////////////////////////////////////////
    template <typename T>
    primitive_argument_type expand_dims::expand_dims_3d(
        ir::node_data<T>&& arg, std::int64_t axis) const
    {
        auto t = arg.tensor();
        std::size_t pages = t.pages();
        std::size_t rows  = t.rows();
        std::size_t columns = t.columns();

        if (axis == 0)
        {
            blaze::DynamicArray<4UL, T> result(1UL, pages, rows, columns);
            blaze::quatslice(result, 0) = t;
            return primitive_argument_type{std::move(result)};
        }
        if (axis == 1)
        {
            blaze::DynamicArray<4UL, T> result(pages, 1UL, rows, columns);
            for (std::size_t i = 0; i != pages; ++i)
            {
                blaze::quatslice(result, i) =
                    blaze::subtensor(t, i, 0UL, 0UL, 1UL, rows, columns);
            }
            return primitive_argument_type{std::move(result)};
        }

        if (axis == 2)
        {
            blaze::DynamicArray<4UL, T> result(pages, rows, 1UL, columns);
            for (std::size_t i = 0; i != pages; ++i)
            {
                blaze::quatslice(result, i) = blaze::trans(
                    blaze::subtensor(t, i, 0UL, 0UL, 1UL, rows, columns),
                    {1, 0, 2});
            }
            return primitive_argument_type{std::move(result)};
        }

        // axis == 3
        blaze::DynamicArray<4UL, T> result(pages, rows, columns, 1UL);
        for (std::size_t i = 0; i != pages; ++i)
        {
            blaze::quatslice(result, i) = blaze::trans(
                blaze::subtensor(t, i, 0UL, 0UL, 1UL, rows, columns),
                {1, 2, 0});
        }
        return primitive_argument_type{std::move(result)};
    }

    primitive_argument_type expand_dims::expand_dims_3d(
        primitive_arguments_type&& args) const
    {
        std::int64_t axis =
            extract_scalar_integer_value_strict(args[1], name_, codename_);

        if (axis > 3 || axis < -4)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "phylanx::execution_tree::primitives::expand_dims::expand_dims_3d",
                generate_error_message(
                    "the expand_dims primitive requires operand axis "
                    "to be between -4 and 3 for tensor values."));
        }

        if (axis < 0)
        {
            axis += 4;
        }

        switch (extract_common_type(args[0]))
        {
        case node_data_type_bool:
            return expand_dims_3d(extract_boolean_value_strict(
                std::move(args[0]), name_, codename_), axis);

        case node_data_type_int64:
            return expand_dims_3d(extract_integer_value_strict(
                std::move(args[0]), name_, codename_), axis);

        case node_data_type_double:
            return expand_dims_3d(extract_numeric_value_strict(
                std::move(args[0]), name_, codename_), axis);

        case node_data_type_unknown:
            return expand_dims_3d(extract_numeric_value(
                std::move(args[0]), name_, codename_), axis);

        default:
            break;
        }

        HPX_THROW_EXCEPTION(hpx::bad_parameter,
            "phylanx::execution_tree::primitives::expand_dims::expand_dims_3d",
            generate_error_message(
                "the arange primitive requires for all arguments to "
                    "be numeric data types"));
    }

    ///////////////////////////////////////////////////////////////////////////
    hpx::future<primitive_argument_type> expand_dims::eval(
        primitive_arguments_type const& operands,
        primitive_arguments_type const& args, eval_context ctx) const
    {
        if (operands.size() != 2)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "expand_dims::eval",
                generate_error_message(
                    "the expand_dims primitive requires exactly two "
                    "operands"));
        }

        for (auto const& i : operands)
        {
            if (!valid(i))
            {
                HPX_THROW_EXCEPTION(hpx::bad_parameter,
                    "expand_dims::eval",
                    generate_error_message(
                        "the expand_dims primitive requires that the "
                        "arguments given by the operands array are valid"));
            }
        }

        auto this_ = this->shared_from_this();
        return hpx::dataflow(hpx::launch::sync, hpx::util::unwrapping(
            [this_ = std::move(this_)](primitive_arguments_type&& args)
            -> primitive_argument_type
            {
                std::size_t a_dims = extract_numeric_value_dimension(
                    args[0], this_->name_, this_->codename_);

                switch (a_dims)
                {
                case 0:
                    return this_->expand_dims_0d(std::move(args));

                case 1:
                    return this_->expand_dims_1d(std::move(args));

                case 2:
                    return this_->expand_dims_2d(std::move(args));

                case 3:
                    return this_->expand_dims_3d(std::move(args));

                default:
                    HPX_THROW_EXCEPTION(hpx::bad_parameter,
                        "expand_dims::eval",
                        this_->generate_error_message(
                            "operand a has an invalid number of dimensions"));
                }
            }),
            detail::map_operands(
                operands, functional::value_operand{}, args,
                name_, codename_, std::move(ctx)));
    }
}}}
