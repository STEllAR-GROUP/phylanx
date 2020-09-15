// Copyright (c) 2018-2020 Hartmut Kaiser
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>
#include <phylanx/execution_tree/primitives/base_primitive.hpp>
#include <phylanx/execution_tree/primitives/detail/advanced_indexes.hpp>
#include <phylanx/execution_tree/primitives/slice_node_data.hpp>
#include <phylanx/execution_tree/primitives/slice_node_data_0d.hpp>
#include <phylanx/execution_tree/primitives/slice_node_data_1d.hpp>
#include <phylanx/execution_tree/primitives/slice_node_data_2d.hpp>
#include <phylanx/execution_tree/primitives/slice_node_data_3d.hpp>
#include <phylanx/ir/node_data.hpp>
#include <phylanx/ir/ranges.hpp>

#include <hpx/assert.hpp>
#include <hpx/errors/exception.hpp>

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

namespace phylanx { namespace execution_tree
{
    ///////////////////////////////////////////////////////////////////////////
    // return a slice of the given ir::node_data instance
    template <typename T>
    ir::node_data<T> slice_extract(ir::node_data<T> const& data,
        execution_tree::primitive_argument_type const& indices,
        std::string const& name, std::string const& codename,
        eval_context const& ctx)
    {
        switch (data.num_dimensions())
        {
        case 0:
            return slice1d_extract0d(data, indices, name, codename, ctx);

        case 1:
            return slice1d_extract1d(data, indices, name, codename, ctx);

        case 2:
            return slice1d_extract2d(data, indices, name, codename, ctx);

        case 3:
            return slice1d_extract3d(data, indices, name, codename, ctx);

        default:
            break;
        }

        HPX_THROW_EXCEPTION(hpx::invalid_status,
            "phylanx::execution_tree::slice_extract",
            util::generate_error_message(
                "target ir::node_data object holds unsupported data type", name,
                codename, ctx.back_trace()));
    }

    template <typename T>
    execution_tree::primitive_argument_type dist_slice_extract(
        ir::node_data<T> const& data,
        execution_tree::primitive_argument_type const& indices,
        execution_tree::localities_information&& arr_localities,
        std::string const& name, std::string const& codename,
        eval_context const& ctx)
    {
        switch (data.num_dimensions())
        {
        case 2:
            return dist_slice1d_extract2d(
                data, indices, std::move(arr_localities), name, codename, ctx);

        default:
            break;
        }

        HPX_THROW_EXCEPTION(hpx::invalid_status,
            "phylanx::execution_tree::dist_slice_extract",
            util::generate_error_message(
                "distributed target ir::node_data object has an unsupported "
                "number of dimensions",
                name, codename, ctx.back_trace()));
    }

    template <typename T>
    ir::node_data<T> slice_extract(ir::node_data<T> const& data,
        execution_tree::primitive_argument_type const& rows,
        execution_tree::primitive_argument_type const& columns,
        std::string const& name, std::string const& codename,
        eval_context const& ctx)
    {
        switch (data.num_dimensions())
        {
        case 1:
            {
                if (valid(rows))
                {
                    HPX_ASSERT(!valid(columns));
                    return slice1d_extract1d(data, rows, name, codename, ctx);
                }

                if (valid(columns))
                {
                    HPX_ASSERT(!valid(rows));
                    return slice1d_extract1d(data, columns, name, codename, ctx);
                }
            }
            break;

        case 2:
            return slice2d_extract2d(data, rows, columns, name, codename, ctx);

        case 3:
            return slice2d_extract3d(data, rows, columns, name, codename, ctx);

        case 0: HPX_FALLTHROUGH;
        default:
            break;
        }

        HPX_THROW_EXCEPTION(hpx::invalid_status,
            "phylanx::execution_tree::slice_extract",
            util::generate_error_message(
                "target ir::node_data object holds data type that does not "
                "support 2d slicing",
                name, codename, ctx.back_trace()));
    }

    template <typename T>
    execution_tree::primitive_argument_type dist_slice_extract(
        ir::node_data<T> const& data,
        execution_tree::primitive_argument_type const& rows,
        execution_tree::primitive_argument_type const& columns,
        execution_tree::localities_information&& arr_localities,
        std::string const& name, std::string const& codename,
        eval_context const& ctx)
    {
        switch (data.num_dimensions())
        {
        case 2:
            return dist_slice2d_extract2d(data, rows, columns,
                std::move(arr_localities), name, codename, ctx);

        default:
            break;
        }

        HPX_THROW_EXCEPTION(hpx::invalid_status,
            "phylanx::execution_tree::dist_slice_extract",
            util::generate_error_message(
                "distributed target ir::node_data object has an unsupported "
                "number of dimensions",
                name, codename, ctx.back_trace()));
    }

    template <typename T>
    ir::node_data<T> slice_extract(ir::node_data<T> const& data,
        execution_tree::primitive_argument_type const& pages,
        execution_tree::primitive_argument_type const& rows,
        execution_tree::primitive_argument_type const& columns,
        std::string const& name, std::string const& codename,
        eval_context const& ctx)
    {
        switch (data.num_dimensions())
        {
        case 3:
            return slice3d_extract3d(
                data, pages, rows, columns, name, codename, ctx);

        case 0: HPX_FALLTHROUGH;
        case 1: HPX_FALLTHROUGH;
        case 2: HPX_FALLTHROUGH;
        default:
            break;
        }

        HPX_THROW_EXCEPTION(hpx::invalid_status,
            "phylanx::execution_tree::slice_extract",
            util::generate_error_message(
                "target ir::node_data object holds data type that does not "
                "support 3d slicing",
                name, codename, ctx.back_trace()));
    }

    ///////////////////////////////////////////////////////////////////////////
    // explicit instantiations of the slice (extract) functionality
    template PHYLANX_EXPORT ir::node_data<std::uint8_t>
    slice_extract<std::uint8_t>(ir::node_data<std::uint8_t> const& data,
        execution_tree::primitive_argument_type const& indices,
        std::string const& name, std::string const& codename,
        eval_context const& ctx);

    template PHYLANX_EXPORT ir::node_data<double>
    slice_extract<double>(ir::node_data<double> const& data,
        execution_tree::primitive_argument_type const& indices,
        std::string const& name, std::string const& codename,
        eval_context const& ctx);

    template PHYLANX_EXPORT ir::node_data<std::int64_t>
    slice_extract<std::int64_t>(ir::node_data<std::int64_t> const& data,
        execution_tree::primitive_argument_type const& indices,
        std::string const& name, std::string const& codename,
        eval_context const& ctx);

    template PHYLANX_EXPORT execution_tree::primitive_argument_type
    dist_slice_extract<std::uint8_t>(ir::node_data<std::uint8_t> const& data,
        execution_tree::primitive_argument_type const& indices,
        execution_tree::localities_information&& arr_localities,
        std::string const& name, std::string const& codename,
        eval_context const& ctx);

    template PHYLANX_EXPORT execution_tree::primitive_argument_type
    dist_slice_extract<double>(ir::node_data<double> const& data,
        execution_tree::primitive_argument_type const& indices,
        execution_tree::localities_information&& arr_localities,
        std::string const& name, std::string const& codename,
        eval_context const& ctx);

    template PHYLANX_EXPORT execution_tree::primitive_argument_type
    dist_slice_extract<std::int64_t>(ir::node_data<std::int64_t> const& data,
        execution_tree::primitive_argument_type const& indices,
        execution_tree::localities_information&& arr_localities,
        std::string const& name, std::string const& codename,
        eval_context const& ctx);

    template PHYLANX_EXPORT ir::node_data<std::uint8_t>
    slice_extract<std::uint8_t>(ir::node_data<std::uint8_t> const& data,
        execution_tree::primitive_argument_type const& rows,
        execution_tree::primitive_argument_type const& columns,
        std::string const& name, std::string const& codename,
        eval_context const& ctx);

    template PHYLANX_EXPORT ir::node_data<double>
    slice_extract<double>(ir::node_data<double> const& data,
        execution_tree::primitive_argument_type const& rows,
        execution_tree::primitive_argument_type const& columns,
        std::string const& name, std::string const& codename,
        eval_context const& ctx);

    template PHYLANX_EXPORT ir::node_data<std::int64_t>
    slice_extract<std::int64_t>(ir::node_data<std::int64_t> const& data,
        execution_tree::primitive_argument_type const& rows,
        execution_tree::primitive_argument_type const& columns,
        std::string const& name, std::string const& codename,
        eval_context const& ctx);

    template PHYLANX_EXPORT execution_tree::primitive_argument_type
    dist_slice_extract<std::uint8_t>(ir::node_data<std::uint8_t> const& data,
        execution_tree::primitive_argument_type const& rows,
        execution_tree::primitive_argument_type const& columns,
        execution_tree::localities_information&& arr_localities,
        std::string const& name, std::string const& codename,
        eval_context const& ctx);

    template PHYLANX_EXPORT execution_tree::primitive_argument_type
    dist_slice_extract<double>(ir::node_data<double> const& data,
        execution_tree::primitive_argument_type const& rows,
        execution_tree::primitive_argument_type const& columns,
        execution_tree::localities_information&& arr_localities,
        std::string const& name, std::string const& codename,
        eval_context const& ctx);

    template PHYLANX_EXPORT execution_tree::primitive_argument_type
    dist_slice_extract<std::int64_t>(ir::node_data<std::int64_t> const& data,
        execution_tree::primitive_argument_type const& rows,
        execution_tree::primitive_argument_type const& columns,
        execution_tree::localities_information&& arr_localities,
        std::string const& name, std::string const& codename,
        eval_context const& ctx);

    template PHYLANX_EXPORT ir::node_data<std::uint8_t>
    slice_extract<std::uint8_t>(ir::node_data<std::uint8_t> const& data,
        execution_tree::primitive_argument_type const& pages,
        execution_tree::primitive_argument_type const& rows,
        execution_tree::primitive_argument_type const& columns,
        std::string const& name, std::string const& codename,
        eval_context const& ctx);

    template PHYLANX_EXPORT ir::node_data<double>
    slice_extract<double>(ir::node_data<double> const& data,
        execution_tree::primitive_argument_type const& pages,
        execution_tree::primitive_argument_type const& rows,
        execution_tree::primitive_argument_type const& columns,
        std::string const& name, std::string const& codename,
        eval_context const& ctx);

    template PHYLANX_EXPORT ir::node_data<std::int64_t>
    slice_extract<std::int64_t>(ir::node_data<std::int64_t> const& data,
        execution_tree::primitive_argument_type const& pages,
        execution_tree::primitive_argument_type const& rows,
        execution_tree::primitive_argument_type const& columns,
        std::string const& name, std::string const& codename,
        eval_context const& ctx);

    ///////////////////////////////////////////////////////////////////////////
    // Modifying slice functionality
    template <typename T>
    ir::node_data<T> slice_assign(ir::node_data<T>&& data,
        execution_tree::primitive_argument_type const& indices,
        ir::node_data<T>&& value, std::string const& name,
        std::string const& codename, eval_context const& ctx)
    {
        switch (data.num_dimensions())
        {
        case 0:
            return slice1d_assign0d(std::move(data), indices, std::move(value),
                name, codename, ctx);

        case 1:
            return slice1d_assign1d(std::move(data), indices, std::move(value),
                name, codename, ctx);

        case 2:
            return slice1d_assign2d(std::move(data), indices, std::move(value),
                name, codename, ctx);

        case 3:
            return slice1d_assign3d(std::move(data), indices, std::move(value),
                name, codename, ctx);

        default:
            break;
        }

        HPX_THROW_EXCEPTION(hpx::invalid_status,
            "phylanx::execution_tree::slice_assign",
            util::generate_error_message(
                "target ir::node_data object holds unsupported data type", name,
                codename, ctx.back_trace()));
    }

    template <typename T>
    execution_tree::primitive_argument_type dist_slice_assign(
        ir::node_data<T>&& data,
        execution_tree::primitive_argument_type const& indices,
        ir::node_data<T>&& value,
        execution_tree::localities_information&& arr_localities,
        execution_tree::localities_information&& val_localities,
        std::string const& name, std::string const& codename,
        eval_context const& ctx)
    {
        switch (data.num_dimensions())
        {
        case 2:
            return dist_slice1d_assign2d(std::move(data), indices, std::move(value),
                std::move(arr_localities), std::move(val_localities), name,
                codename, ctx);

        default:
            break;
        }

        HPX_THROW_EXCEPTION(hpx::invalid_status,
            "phylanx::execution_tree::slice_assign",
            util::generate_error_message(
                "distributed target ir::node_data object has an "
                "unsupported number of dimensions",
                name, codename, ctx.back_trace()));
    }

    template <typename T>
    execution_tree::primitive_argument_type dist_slice_assign(
        ir::node_data<T>&& data,
        execution_tree::primitive_argument_type const& indices,
        ir::node_data<T>&& value,
        execution_tree::localities_information&& arr_localities,
        std::string const& name, std::string const& codename,
        eval_context const& ctx)
    {
        switch (data.num_dimensions())
        {
        case 2:
            return dist_slice1d_assign2d(std::move(data), indices,
                std::move(value), std::move(arr_localities), name, codename,
                ctx);

        default:
            break;
        }

        HPX_THROW_EXCEPTION(hpx::invalid_status,
            "phylanx::execution_tree::slice_assign",
            util::generate_error_message(
                "distributed target ir::node_data object has an "
                "unsupported number of dimensions",
                name, codename, ctx.back_trace()));
    }

    template <typename T>
    ir::node_data<T> slice_assign(ir::node_data<T>&& data,
        execution_tree::primitive_argument_type const& rows,
        execution_tree::primitive_argument_type const& columns,
        ir::node_data<T>&& value, std::string const& name,
        std::string const& codename, eval_context const& ctx)
    {
        switch (data.num_dimensions())
        {
        case 1:
            return slice2d_assign1d(std::move(data), rows, columns,
                std::move(value), name, codename, ctx);

        case 2:
            return slice2d_assign2d(std::move(data), rows, columns,
                std::move(value), name, codename, ctx);

        case 3:
            return slice2d_assign3d(std::move(data), rows, columns,
                std::move(value), name, codename, ctx);

        case 0: HPX_FALLTHROUGH;
        default:
            break;
        }

        HPX_THROW_EXCEPTION(hpx::invalid_status,
            "phylanx::execution_tree::slice_assign",
            util::generate_error_message(
                "target ir::node_data object holds data type that does not "
                "support 2d slicing",
                name, codename, ctx.back_trace()));
    }

    template <typename T>
    ir::node_data<T> slice_assign(ir::node_data<T>&& data,
        execution_tree::primitive_argument_type const& pages,
        execution_tree::primitive_argument_type const& rows,
        execution_tree::primitive_argument_type const& columns,
        ir::node_data<T>&& value, std::string const& name,
        std::string const& codename, eval_context const& ctx)
    {
        HPX_THROW_EXCEPTION(hpx::invalid_status,
            "phylanx::execution_tree::slice_assign",
            util::generate_error_message(
                "target ir::node_data object holds data type that does not "
                "support 3d slicing",
                name, codename, ctx.back_trace()));
    }

    ///////////////////////////////////////////////////////////////////////////
    // explicit instantiations of the slice (modify) functionality
    template PHYLANX_EXPORT ir::node_data<std::uint8_t>
    slice_assign<std::uint8_t>(ir::node_data<std::uint8_t>&& data,
        execution_tree::primitive_argument_type const& indices,
        ir::node_data<std::uint8_t>&& value, std::string const& name,
        std::string const& codename, eval_context const& ctx);

    template PHYLANX_EXPORT ir::node_data<double>
    slice_assign<double>(ir::node_data<double>&& data,
        execution_tree::primitive_argument_type const& indices,
        ir::node_data<double>&& value, std::string const& name,
        std::string const& codename, eval_context const& ctx);

    template PHYLANX_EXPORT ir::node_data<std::int64_t>
    slice_assign<std::int64_t>(ir::node_data<std::int64_t>&& data,
        execution_tree::primitive_argument_type const& indices,
        ir::node_data<std::int64_t>&& value, std::string const& name,
        std::string const& codename, eval_context const& ctx);

    template PHYLANX_EXPORT execution_tree::primitive_argument_type
    dist_slice_assign<std::uint8_t>(ir::node_data<std::uint8_t>&& data,
        execution_tree::primitive_argument_type const& indices,
        ir::node_data<std::uint8_t>&& value,
        execution_tree::localities_information&& arr_localities,
        execution_tree::localities_information&& val_localities,
        std::string const& name, std::string const& codename,
        eval_context const& ctx);

    template PHYLANX_EXPORT execution_tree::primitive_argument_type
    dist_slice_assign<double>(ir::node_data<double>&& data,
        execution_tree::primitive_argument_type const& indices,
        ir::node_data<double>&& value,
        execution_tree::localities_information&& arr_localities,
        execution_tree::localities_information&& val_localities,
        std::string const& name, std::string const& codename,
        eval_context const& ctx);

    template PHYLANX_EXPORT execution_tree::primitive_argument_type
    dist_slice_assign<std::int64_t>(ir::node_data<std::int64_t>&& data,
        execution_tree::primitive_argument_type const& indices,
        ir::node_data<std::int64_t>&& value,
        execution_tree::localities_information&& arr_localities,
        execution_tree::localities_information&& val_localities,
        std::string const& name, std::string const& codename,
        eval_context const& ctx);

    template PHYLANX_EXPORT execution_tree::primitive_argument_type
    dist_slice_assign<std::uint8_t>(ir::node_data<std::uint8_t>&& data,
        execution_tree::primitive_argument_type const& indices,
        ir::node_data<std::uint8_t>&& value,
        execution_tree::localities_information&& arr_localities,
        std::string const& name, std::string const& codename,
        eval_context const& ctx);

    template PHYLANX_EXPORT execution_tree::primitive_argument_type
    dist_slice_assign<double>(ir::node_data<double>&& data,
        execution_tree::primitive_argument_type const& indices,
        ir::node_data<double>&& value,
        execution_tree::localities_information&& arr_localities,
        std::string const& name, std::string const& codename,
        eval_context const& ctx);

    template PHYLANX_EXPORT execution_tree::primitive_argument_type
    dist_slice_assign<std::int64_t>(ir::node_data<std::int64_t>&& data,
        execution_tree::primitive_argument_type const& indices,
        ir::node_data<std::int64_t>&& value,
        execution_tree::localities_information&& arr_localities,
        std::string const& name, std::string const& codename,
        eval_context const& ctx);

    template PHYLANX_EXPORT ir::node_data<std::uint8_t>
    slice_assign<std::uint8_t>(ir::node_data<std::uint8_t>&& data,
        execution_tree::primitive_argument_type const& rows,
        execution_tree::primitive_argument_type const& columns,
        ir::node_data<std::uint8_t>&& value, std::string const& name,
        std::string const& codename, eval_context const& ctx);

    template PHYLANX_EXPORT ir::node_data<double>
    slice_assign<double>(ir::node_data<double>&& data,
        execution_tree::primitive_argument_type const& rows,
        execution_tree::primitive_argument_type const& columns,
        ir::node_data<double>&& value, std::string const& name,
        std::string const& codename, eval_context const& ctx);

    template PHYLANX_EXPORT ir::node_data<std::int64_t>
    slice_assign<std::int64_t>(ir::node_data<std::int64_t>&& data,
        execution_tree::primitive_argument_type const& rows,
        execution_tree::primitive_argument_type const& columns,
        ir::node_data<std::int64_t>&& value, std::string const& name,
        std::string const& codename, eval_context const& ctx);

    template PHYLANX_EXPORT ir::node_data<std::uint8_t>
    slice_assign<std::uint8_t>(ir::node_data<std::uint8_t>&& data,
        execution_tree::primitive_argument_type const& rows,
        execution_tree::primitive_argument_type const& columns,
        execution_tree::primitive_argument_type const& page,
        ir::node_data<std::uint8_t>&& value, std::string const& name,
        std::string const& codename, eval_context const& ctx);

    template PHYLANX_EXPORT ir::node_data<double>
    slice_assign<double>(ir::node_data<double>&& data,
        execution_tree::primitive_argument_type const& rows,
        execution_tree::primitive_argument_type const& columns,
        execution_tree::primitive_argument_type const& page,
        ir::node_data<double>&& value, std::string const& name,
        std::string const& codename, eval_context const& ctx);

    template PHYLANX_EXPORT ir::node_data<std::int64_t>
    slice_assign<std::int64_t>(ir::node_data<std::int64_t>&& data,
        execution_tree::primitive_argument_type const& rows,
        execution_tree::primitive_argument_type const& columns,
        execution_tree::primitive_argument_type const& page,
        ir::node_data<std::int64_t>&& value, std::string const& name,
        std::string const& codename, eval_context const& ctx);
}}
