// Copyright (c) 2018 Hartmut Kaiser
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

#include <hpx/exception.hpp>
#include <hpx/util/assert.hpp>

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
        std::string const& name, std::string const& codename)
    {
        switch (data.num_dimensions())
        {
        case 0:
            return slice1d_extract0d(data, indices, name, codename);

        case 1:
            return slice1d_extract1d(data, indices, name, codename);

        case 2:
            return slice1d_extract2d(data, indices, name, codename);

#if defined(PHYLANX_HAVE_BLAZE_TENSOR)
        case 3:
            return slice1d_extract3d(data, indices, name, codename);
#endif
        default:
            break;
        }

        HPX_THROW_EXCEPTION(hpx::invalid_status,
            "phylanx::execution_tree::slice_extract",
            util::generate_error_message(
                "target ir::node_data object holds unsupported data type", name,
                codename));
    }

    template <typename T>
    ir::node_data<T> slice_extract(ir::node_data<T> const& data,
        execution_tree::primitive_argument_type const& rows,
        execution_tree::primitive_argument_type const& columns,
        std::string const& name, std::string const& codename)
    {
        switch (data.num_dimensions())
        {
        case 1:
            {
                if (valid(rows))
                {
                    HPX_ASSERT(!valid(columns));
                    return slice1d_extract1d(data, rows, name, codename);
                }

                if (valid(columns))
                {
                    HPX_ASSERT(!valid(rows));
                    return slice1d_extract1d(data, columns, name, codename);
                }
            }
            break;

        case 2:
            return slice2d_extract2d(data, rows, columns, name, codename);

#if defined(PHYLANX_HAVE_BLAZE_TENSOR)
        case 3:
            return slice2d_extract3d(data, rows, columns, name, codename);
#endif
        case 0:
        default:
            break;
        }

        HPX_THROW_EXCEPTION(hpx::invalid_status,
            "phylanx::execution_tree::slice_extract",
            util::generate_error_message(
                "target ir::node_data object holds data type that does not "
                "support 2d slicing",
                name, codename));
    }

#if defined(PHYLANX_HAVE_BLAZE_TENSOR)
    template <typename T>
    ir::node_data<T> slice_extract(ir::node_data<T> const& data,
        execution_tree::primitive_argument_type const& pages,
        execution_tree::primitive_argument_type const& rows,
        execution_tree::primitive_argument_type const& columns,
        std::string const& name, std::string const& codename)
    {
        switch (data.num_dimensions())
        {
        case 3:
            return slice3d_extract3d(data, pages, rows, columns, name, codename);

        case 0:
        case 1:
        case 2:
        default:
            break;
        }

        HPX_THROW_EXCEPTION(hpx::invalid_status,
            "phylanx::execution_tree::slice_extract",
            util::generate_error_message(
                "target ir::node_data object holds data type that does not "
                "support 3d slicing",
                name, codename));
    }
#endif

    ///////////////////////////////////////////////////////////////////////////
    // explicit instantiations of the slice (extract) functionality
    template PHYLANX_EXPORT ir::node_data<std::uint8_t>
    slice_extract<std::uint8_t>(ir::node_data<std::uint8_t> const& data,
        execution_tree::primitive_argument_type const& indices,
        std::string const& name, std::string const& codename);

    template PHYLANX_EXPORT ir::node_data<double>
    slice_extract<double>(ir::node_data<double> const& data,
        execution_tree::primitive_argument_type const& indices,
        std::string const& name, std::string const& codename);

    template PHYLANX_EXPORT ir::node_data<std::int64_t>
    slice_extract<std::int64_t>(ir::node_data<std::int64_t> const& data,
        execution_tree::primitive_argument_type const& indices,
        std::string const& name, std::string const& codename);

    template PHYLANX_EXPORT ir::node_data<std::uint8_t>
    slice_extract<std::uint8_t>(ir::node_data<std::uint8_t> const& data,
        execution_tree::primitive_argument_type const& rows,
        execution_tree::primitive_argument_type const& columns,
        std::string const& name, std::string const& codename);

    template PHYLANX_EXPORT ir::node_data<double>
    slice_extract<double>(ir::node_data<double> const& data,
        execution_tree::primitive_argument_type const& rows,
        execution_tree::primitive_argument_type const& columns,
        std::string const& name, std::string const& codename);

    template PHYLANX_EXPORT ir::node_data<std::int64_t>
    slice_extract<std::int64_t>(ir::node_data<std::int64_t> const& data,
        execution_tree::primitive_argument_type const& rows,
        execution_tree::primitive_argument_type const& columns,
        std::string const& name, std::string const& codename);

#if defined(PHYLANX_HAVE_BLAZE_TENSOR)
    template PHYLANX_EXPORT ir::node_data<std::uint8_t>
    slice_extract<std::uint8_t>(ir::node_data<std::uint8_t> const& data,
        execution_tree::primitive_argument_type const& pages,
        execution_tree::primitive_argument_type const& rows,
        execution_tree::primitive_argument_type const& columns,
        std::string const& name, std::string const& codename);

    template PHYLANX_EXPORT ir::node_data<double>
    slice_extract<double>(ir::node_data<double> const& data,
        execution_tree::primitive_argument_type const& pages,
        execution_tree::primitive_argument_type const& rows,
        execution_tree::primitive_argument_type const& columns,
        std::string const& name, std::string const& codename);

    template PHYLANX_EXPORT ir::node_data<std::int64_t>
    slice_extract<std::int64_t>(ir::node_data<std::int64_t> const& data,
        execution_tree::primitive_argument_type const& pages,
        execution_tree::primitive_argument_type const& rows,
        execution_tree::primitive_argument_type const& columns,
        std::string const& name, std::string const& codename);
#endif

    ///////////////////////////////////////////////////////////////////////////
    // Modifying slice functionality
    template <typename T>
    ir::node_data<T> slice_assign(ir::node_data<T>&& data,
        execution_tree::primitive_argument_type const& indices,
        ir::node_data<T>&& value, std::string const& name,
        std::string const& codename)
    {
        switch (data.num_dimensions())
        {
        case 0:
            return slice1d_assign0d(std::move(data), indices, std::move(value),
                name, codename);

        case 1:
            return slice1d_assign1d(std::move(data), indices, std::move(value),
                name, codename);

        case 2:
            return slice1d_assign2d(std::move(data), indices, std::move(value),
                name, codename);

#if defined(PHYLANX_HAVE_BLAZE_TENSOR)
        case 3:
            return slice1d_assign3d(std::move(data), indices, std::move(value),
                name, codename);
#endif
        default:
            break;
        }

        HPX_THROW_EXCEPTION(hpx::invalid_status,
            "phylanx::execution_tree::slice_assign",
            util::generate_error_message(
                "target ir::node_data object holds unsupported data type", name,
                codename));
    }

    template <typename T>
    ir::node_data<T> slice_assign(ir::node_data<T>&& data,
        execution_tree::primitive_argument_type const& rows,
        execution_tree::primitive_argument_type const& columns,
        ir::node_data<T>&& value, std::string const& name,
        std::string const& codename)
    {
        switch (data.num_dimensions())
        {
        case 1:
            return slice2d_assign1d(std::move(data), rows, columns,
                std::move(value), name, codename);

        case 2:
            return slice2d_assign2d(std::move(data), rows, columns,
                std::move(value), name, codename);

#if defined(PHYLANX_HAVE_BLAZE_TENSOR)
        case 3:
            return slice2d_assign3d(std::move(data), rows, columns,
                std::move(value), name, codename);
#endif
        case 0:
        default:
            break;
        }

        HPX_THROW_EXCEPTION(hpx::invalid_status,
            "phylanx::execution_tree::slice_assign",
            util::generate_error_message(
                "target ir::node_data object holds data type that does not "
                "support 2d slicing",
                name, codename));
    }

#if defined(PHYLANX_HAVE_BLAZE_TENSOR)
    template <typename T>
    ir::node_data<T> slice_assign(ir::node_data<T>&& data,
        execution_tree::primitive_argument_type const& pages,
        execution_tree::primitive_argument_type const& rows,
        execution_tree::primitive_argument_type const& columns,
        ir::node_data<T>&& value, std::string const& name,
        std::string const& codename)
    {
        HPX_THROW_EXCEPTION(hpx::invalid_status,
            "phylanx::execution_tree::slice_assign",
            util::generate_error_message(
                "target ir::node_data object holds data type that does not "
                "support 3d slicing",
                name, codename));
    }
#endif

    ///////////////////////////////////////////////////////////////////////////
    // explicit instantiations of the slice (modify) functionality
    template PHYLANX_EXPORT ir::node_data<std::uint8_t>
    slice_assign<std::uint8_t>(ir::node_data<std::uint8_t>&& data,
        execution_tree::primitive_argument_type const& indices,
        ir::node_data<std::uint8_t>&& value, std::string const& name,
        std::string const& codename);

    template PHYLANX_EXPORT ir::node_data<double>
    slice_assign<double>(ir::node_data<double>&& data,
        execution_tree::primitive_argument_type const& indices,
        ir::node_data<double>&& value, std::string const& name,
        std::string const& codename);

    template PHYLANX_EXPORT ir::node_data<std::int64_t>
    slice_assign<std::int64_t>(ir::node_data<std::int64_t>&& data,
        execution_tree::primitive_argument_type const& indices,
        ir::node_data<std::int64_t>&& value, std::string const& name,
        std::string const& codename);

    template PHYLANX_EXPORT ir::node_data<std::uint8_t>
    slice_assign<std::uint8_t>(ir::node_data<std::uint8_t>&& data,
        execution_tree::primitive_argument_type const& rows,
        execution_tree::primitive_argument_type const& columns,
        ir::node_data<std::uint8_t>&& value, std::string const& name,
        std::string const& codename);

    template PHYLANX_EXPORT ir::node_data<double>
    slice_assign<double>(ir::node_data<double>&& data,
        execution_tree::primitive_argument_type const& rows,
        execution_tree::primitive_argument_type const& columns,
        ir::node_data<double>&& value, std::string const& name,
        std::string const& codename);

    template PHYLANX_EXPORT ir::node_data<std::int64_t>
    slice_assign<std::int64_t>(ir::node_data<std::int64_t>&& data,
        execution_tree::primitive_argument_type const& rows,
        execution_tree::primitive_argument_type const& columns,
        ir::node_data<std::int64_t>&& value, std::string const& name,
        std::string const& codename);

#if defined(PHYLANX_HAVE_BLAZE_TENSOR)
    template PHYLANX_EXPORT ir::node_data<std::uint8_t>
    slice_assign<std::uint8_t>(ir::node_data<std::uint8_t>&& data,
        execution_tree::primitive_argument_type const& rows,
        execution_tree::primitive_argument_type const& columns,
        execution_tree::primitive_argument_type const& page,
        ir::node_data<std::uint8_t>&& value, std::string const& name,
        std::string const& codename);

    template PHYLANX_EXPORT ir::node_data<double>
    slice_assign<double>(ir::node_data<double>&& data,
        execution_tree::primitive_argument_type const& rows,
        execution_tree::primitive_argument_type const& columns,
        execution_tree::primitive_argument_type const& page,
        ir::node_data<double>&& value, std::string const& name,
        std::string const& codename);

    template PHYLANX_EXPORT ir::node_data<std::int64_t>
    slice_assign<std::int64_t>(ir::node_data<std::int64_t>&& data,
        execution_tree::primitive_argument_type const& rows,
        execution_tree::primitive_argument_type const& columns,
        execution_tree::primitive_argument_type const& page,
        ir::node_data<std::int64_t>&& value, std::string const& name,
        std::string const& codename);
#endif
}}
