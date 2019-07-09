// Copyright (c) 2018 Hartmut Kaiser
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(PHYLANX_IR_NODE_SLICE_NODE_DATA_1D_SEP_18_2018_1220PM)
#define PHYLANX_IR_NODE_SLICE_NODE_DATA_1D_SEP_18_2018_1220PM

#include <phylanx/config.hpp>
#include <phylanx/execution_tree/primitives/base_primitive.hpp>
#include <phylanx/execution_tree/primitives/detail/advanced_indexes.hpp>
#include <phylanx/execution_tree/primitives/detail/slice_assign.hpp>
#include <phylanx/execution_tree/primitives/detail/slice_identity.hpp>
#include <phylanx/execution_tree/primitives/node_data_helpers.hpp>
#include <phylanx/ir/node_data.hpp>
#include <phylanx/ir/ranges.hpp>
#include <phylanx/util/slicing_helpers.hpp>

#include <hpx/throw_exception.hpp>
#include <hpx/assertion.hpp>
#include <hpx/util/format.hpp>

#include <cstddef>
#include <cstdint>
#include <string>
#include <utility>

#include <blaze/Math.h>
#include <blaze/math/Elements.h>

// Slicing functionality for 1d data
namespace phylanx { namespace execution_tree
{
    ///////////////////////////////////////////////////////////////////////////
    // return a slice from a 1d ir::node_data
    template <typename T, typename Data, typename F>
    ir::node_data<T> slice1d_basic(Data&& data,
        ir::slicing_indices const& indices, F const& f, std::string const& name,
        std::string const& codename)
    {
        std::size_t size = data.size();
        if (indices.start() >= std::int64_t(size) ||
            indices.span() > std::int64_t(size))
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "phylanx::execution_tree::slicing1d",
                util::generate_error_message(
                    "cannot extract anything but the existing elements from a "
                    "vector",
                    name, codename));
        }

        // handle single argument slicing parameters
        std::int64_t start = indices.start();

        // handle single value slicing result
        if (indices.single_value())
        {
            return f.scalar(
                data, data[detail::check_index(start, size, name, codename)]);
        }

        std::int64_t stop = indices.stop();
        std::int64_t step = indices.step();

        // extract a consecutive sub-vector
        if (step == 1)
        {
            HPX_ASSERT(stop > start);
            auto sv = blaze::subvector(data, start, stop - start);
            return f.vector(data, std::move(sv));
        }

        // most general case, pick arbitrary elements
        if (step == 0)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "phylanx::execution_tree::slicing1d",
                util::generate_error_message(
                    "step can not be zero", name, codename));
        }

        auto element_indices =
            util::slicing_helpers::create_list_slice(start, stop, step);
        auto sv = blaze::elements(
            data, element_indices.data(), element_indices.size());

        return f.vector(data, std::move(sv));
    }

    ///////////////////////////////////////////////////////////////////////////
    template <typename T, typename Data, typename F>
    ir::node_data<T> slice_integer_1d0d_scalar(Data&& data,
        ir::node_data<std::int64_t>&& indices, F const& f,
        std::string const& name, std::string const& codename)
    {
        std::size_t size = data.size();
        return f.scalar(data,
            data[detail::check_index(indices.scalar(), size, name, codename)]);
    }

    template <typename T, typename Data, typename F>
    ir::node_data<T> slice_integer_1d0d(Data&& data,
        ir::node_data<std::int64_t>&& indices, F const& f,
        std::string const& name, std::string const& codename)
    {
        std::size_t size = data.size();
        std::size_t index_size = indices.size();

        auto index_list = indices.vector();
        for (std::size_t i = 0; i != index_size; ++i)
        {
            index_list[i] =
                detail::check_index(index_list[i], size, name, codename);
        }

        auto sv = blaze::elements(data, index_list.data(), index_list.size());
        return f.vector(data, std::move(sv));
    }

    template <typename T, typename Data, typename F>
    ir::node_data<T> slice_integer_1d1d(Data&& data,
        ir::node_data<std::int64_t>&& indices, F const& f,
        std::string const& name, std::string const& codename)
    {
        std::size_t size = data.size();
        std::size_t index_size = indices.size();

        auto index_list = indices.vector();
        for (std::size_t i = 0; i != index_size; ++i)
        {
            index_list[i] =
                detail::check_index(index_list[i], size, name, codename);
        }

        auto sv = blaze::elements(data, index_list.data(), index_list.size());
        return f.vector(data, std::move(sv));
    }

    template <typename T, typename Data, typename F>
    ir::node_data<T> slice_integer_1d2d(Data&& data,
        ir::node_data<std::int64_t>&& indices, F const& f,
        std::string const& name, std::string const& codename)
    {
        std::size_t size = data.size();

        // general case, pick arbitrary elements from matrix
        auto index_matrix = indices.matrix();

        typename ir::node_data<T>::storage2d_type result(
            index_matrix.rows(), index_matrix.columns());

        std::size_t index_rows = index_matrix.rows();
        for (std::size_t row = 0; row != index_rows; ++row)
        {
            auto index_row = blaze::row(index_matrix, row);
            std::size_t index_size = index_row.size();

            for (std::size_t i = 0; i != index_size; ++i)
            {
                index_row[i] =
                    detail::check_index(index_row[i], size, name, codename);
            }

            blaze::row(result, row) = blaze::trans(
                blaze::elements(data, index_row.data(), index_row.size()));
        }

        return f.matrix(result, result);
    }

    template <typename T, typename Data, typename F>
    ir::node_data<T> slice1d_integer(Data&& data,
        ir::node_data<std::int64_t>&& indices, F const& f,
        std::string const& name, std::string const& codename)
    {
        std::size_t size = data.size();

        switch (indices.num_dimensions())
        {
        case 0:         // 0d index
            return slice_integer_1d0d_scalar<T>(std::forward<Data>(data),
                std::move(indices), f, name, codename);

        case 1:         // 1d indexes
            return slice_integer_1d1d<T>(std::forward<Data>(data),
                std::move(indices), f, name, codename);

        case 2:         // 2d indexes
            return slice_integer_1d2d<T>(std::forward<Data>(data),
                std::move(indices), f, name, codename);

        default:
            break;
        }

        HPX_THROW_EXCEPTION(hpx::bad_parameter,
            "phylanx::execution_tree::slice1d_integer",
            util::generate_error_message(
                "unexpected type for indices used for advanced integer array "
                "indexing", name, codename));
    }

    ///////////////////////////////////////////////////////////////////////////
    template <typename T, typename Data, typename F>
    ir::node_data<T> slice_boolean_1d0d(Data&& data,
        ir::node_data<std::uint8_t>&& indices, F const& f,
        std::string const& name, std::string const& codename)
    {
        if (indices.size() == 0)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "phylanx::execution_tree::slice_boolean1d0d",
                util::generate_error_message(
                    "boolean arrays used as indices must be non-empty", name,
                    codename));
        }

        return slice_integer_1d0d<T>(std::forward<Data>(data),
            util::slicing_helpers::create_list_slice(indices[0], data.size()),
            f, name, codename);
    }

    template <typename T, typename Data, typename F>
    ir::node_data<T> slice_boolean_1d1d(Data&& data,
        ir::node_data<std::uint8_t>&& indices, F const& f,
        std::string const& name, std::string const& codename)
    {
        if (data.size() != indices.size())
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "phylanx::execution_tree::slice_boolean1d1d",
                util::generate_error_message(
                    hpx::util::format(
                        "boolean index did not match indexed array along "
                        "dimension 0; dimension is {} but corresponding "
                        "boolean dimension is {}",
                        data.size(), indices.size()),
                    name, codename));
        }

        return slice_integer_1d1d<T>(std::forward<Data>(data),
            util::slicing_helpers::create_list_slice(indices), f,
            name, codename);
    }

    template <typename T, typename Data, typename F>
    ir::node_data<T> slice_boolean_1d2d(Data&& data,
        ir::node_data<std::uint8_t>&& indices, F const& f,
        std::string const& name, std::string const& codename)
    {
        // we are not allowed to use a 2d boolean index array for indexing a
        // vector
        HPX_THROW_EXCEPTION(hpx::bad_parameter,
            "phylanx::execution_tree::slice_boolean1d2d",
            util::generate_error_message(
                "too many indices for array", name, codename));
    }

    template <typename T, typename Data, typename F>
    ir::node_data<T> slice1d_boolean(Data&& data,
        ir::node_data<std::uint8_t>&& indices, F const& f,
        std::string const& name, std::string const& codename)
    {
        switch (indices.num_dimensions())
        {
        case 0:         // 0d index
            return slice_boolean_1d0d<T>(std::forward<Data>(data),
                std::move(indices), f, name, codename);

        case 1:         // 1d indexes
            return slice_boolean_1d1d<T>(std::forward<Data>(data),
                std::move(indices), f, name, codename);

        case 2:         // 2d indexes
            return slice_boolean_1d2d<T>(std::forward<Data>(data),
                std::move(indices), f, name, codename);

        default:
            break;
        }

        HPX_THROW_EXCEPTION(hpx::bad_parameter,
            "phylanx::execution_tree::slice1d_boolean",
            util::generate_error_message(
                "unexpected type for indices used for advanced boolean array "
                "indexing", name, codename));
    }

    ///////////////////////////////////////////////////////////////////////////
    template <typename T, typename Data, typename F>
    ir::node_data<T> slice1d(Data&& data,
        primitive_argument_type const& indices, F const& f,
        std::string const& name, std::string const& codename)
    {
        if (is_list_operand_strict(indices))
        {
            detail::slicing_index_type t =
                detail::extract_slicing_index_type(indices, name, codename);

            if (t == detail::slicing_index_basic)
            {
                // basic slicing and indexing
                std::size_t size = data.size();
                return slice1d_basic<T>(std::forward<Data>(data),
                    util::slicing_helpers::extract_slicing(
                        indices, size, name, codename),
                    f, name, codename);
            }

            if (t == detail::slicing_index_advanced_integer)
            {
                // advanced indexing (integer array indexing)
                auto integer_index = detail::extract_advanced_integer_index(
                    indices, name, codename);

                return slice1d_integer<T>(std::forward<Data>(data),
                    extract_integer_value_strict(
                        std::move(integer_index), name, codename),
                    f, name, codename);
            }

            if (t == detail::slicing_index_advanced_boolean)
            {
                // advanced indexing (Boolean array indexing)
                auto boolean_index = detail::extract_advanced_boolean_index(
                    indices, name, codename);

                return slice1d_boolean<T>(std::forward<Data>(data),
                    extract_boolean_value_strict(
                        std::move(boolean_index), name, codename),
                    f, name, codename);
            }
        }
        else if (valid(indices))
        {
            if (is_boolean_operand_strict(indices))
            {
                // advanced indexing (Boolean array indexing)
                return slice1d_boolean<T>(std::forward<Data>(data),
                    extract_boolean_value_strict(indices, name, codename), f,
                    name, codename);
            }

            if (is_integer_operand(indices))
            {
                // advanced indexing (integer array indexing)
                return slice1d_integer<T>(std::forward<Data>(data),
                    extract_integer_value(indices, name, codename), f,
                    name, codename);
            }
        }
        else
        {
            std::size_t size = data.size();
            return slice1d_basic<T>(std::forward<Data>(data),
                util::slicing_helpers::extract_slicing(
                    indices, size, name, codename),
                f, name, codename);
        }

        HPX_THROW_EXCEPTION(hpx::bad_parameter,
            "phylanx::execution_tree::slice1d",
            util::generate_error_message(
                "unsupported indexing type", name, codename));
    }

    ///////////////////////////////////////////////////////////////////////////
    template <typename T>
    ir::node_data<T> slice1d_extract1d(ir::node_data<T> const& data,
        execution_tree::primitive_argument_type const& indices,
        std::string const& name, std::string const& codename)
    {
        return slice1d<T>(data.vector(), indices,
            detail::slice_identity<T>{}, name, codename);
    }

    ///////////////////////////////////////////////////////////////////////////
    template <typename T>
    ir::node_data<T> slice1d_assign1d(ir::node_data<T>&& data,
        execution_tree::primitive_argument_type const& indices,
        ir::node_data<T>&& value, std::string const& name,
        std::string const& codename)
    {
        switch (value.num_dimensions())
        {
        case 0: HPX_FALLTHROUGH;
        case 1:
            {
                auto v = data.vector();

                std::size_t result_size =
                    detail::slicing_size(indices, v.size(), name, codename);

                typename ir::node_data<T>::storage1d_type result;
                extract_value_vector(result, std::move(value),
                    result_size, name, codename);

                ir::node_data<T> rhs(std::move(result));
                if (data.is_ref())
                {
                    return slice1d<T>(std::move(v), indices,
                        detail::slice_assign_vector<T>{rhs}, name, codename);
                }

                return slice1d<T>(std::move(data.vector_non_ref()), indices,
                    detail::slice_assign_vector<T>{rhs}, name, codename);
            }

        case 2: HPX_FALLTHROUGH;
        default:
            break;
        }

        HPX_THROW_EXCEPTION(hpx::invalid_status,
            "phylanx::execution_tree::slice1d_assign1d",
            util::generate_error_message(
                "source ir::node_data object holds unsupported data type", name,
                codename));
    }

    ///////////////////////////////////////////////////////////////////////////
    template <typename T>
    ir::node_data<T> slice2d_assign1d(ir::node_data<T>&& data,
        execution_tree::primitive_argument_type const& rows,
        execution_tree::primitive_argument_type const& columns,
        ir::node_data<T>&& value, std::string const& name,
        std::string const& codename)
    {
        switch (value.num_dimensions())
        {
        case 0: HPX_FALLTHROUGH;
        case 1:
            {
                if (valid(rows))
                {
                    HPX_ASSERT(!valid(columns));
                    return slice1d_assign1d(std::move(data), rows,
                        std::move(value), name, codename);
                }

                if (valid(columns))
                {
                    HPX_ASSERT(!valid(rows));
                    return slice1d_assign1d(std::move(data), columns,
                        std::move(value), name, codename);
                }
            }
            break;

        case 2: HPX_FALLTHROUGH;
        default:
            break;
        }

        HPX_THROW_EXCEPTION(hpx::invalid_status,
            "phylanx::execution_tree::slice2d_assign1d",
            util::generate_error_message(
                "source ir::node_data object holds unsupported data type", name,
                codename));
    }
}}

#endif
