// Copyright (c) 2018 Hartmut Kaiser
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>
#include <phylanx/execution_tree/primitives/base_primitive.hpp>
#include <phylanx/ir/node_data.hpp>
#include <phylanx/ir/ranges.hpp>
#include <phylanx/ir/slice_node_data.hpp>

#include <hpx/util/assert.hpp>

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <vector>

#include <blaze/Math.h>
#include <blaze/math/Elements.h>

namespace phylanx { namespace ir
{
    ///////////////////////////////////////////////////////////////////////////
    // Extracting slice functionality
    template <typename T, typename F>
    node_data<T> slice0d(T data, ir::slicing_indices const& indices, F const& f)
    {
#if defined(_DEBUG)
        if (indices.start() != 0 || indices.span() != 1 || indices.step() != 1)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "phylanx::ir::slicing0d",
                "cannot extract anything but the first element from a scalar");
        }
#endif
        return node_data<T>{f.scalar(data)};
    }

    ///////////////////////////////////////////////////////////////////////////
    namespace detail
    {
        // generate a list of indices to extract from a given vector
        std::vector<std::int64_t> create_list_slice(
            std::int64_t start, std::int64_t stop, std::int64_t step,
            std::size_t array_length)
        {
            if (start < 0)
            {
                start = array_length + start;
            }

            if (stop < 0)
            {
                stop = array_length + stop;
            }

            std::vector<std::int64_t> result;

            if (step > 0)
            {
                result.reserve((std::max)(0ll, stop - start));
                for(std::int64_t i = start; i < stop; i += step)
                {
                    result.push_back(i);
                }
            }
            else if (step < 0)
            {
                result.reserve((std::max)(0ll, start - stop));
                for(std::int64_t i = start; i > stop; i += step)
                {
                    result.push_back(i);
                }
            }

            return result;
        }
    }

    ///////////////////////////////////////////////////////////////////////////
    // return a slice from a 1d node_data
    template <typename T, typename F>
    node_data<T> slice1d(blaze::CustomVector<T, true, true> const& data,
        slicing_indices const& indices, F const& f)
    {
#if defined(_DEBUG)
        std::size_t size = data.size();
        if (indices.start() >= size || indices.span() >= size ||
            indices.stop() > size)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "phylanx::ir::slicing1d",
                "cannot extract anything but the existing elements from a "
                "vector");
        }
#endif

        // handle single argument slicing parameters
        std::int64_t start = indices.start();

        // handle single value slicing result
        if (indices.single_value())
        {
            if (start < 0)
            {
                start = data.size() + start;
            }

            return node_data<T>{f.scalar(data[start])};
        }

        std::int64_t stop = indices.stop();
        std::int64_t step = indices.step();

        // extract a consecutive sub-vector
        if (step == 1)
        {
            HPX_ASSERT(stop > start);
            auto sv = blaze::subvector(data, start, stop - start);
            return node_data<T>{f.vector(sv)};
        }

        // most general case, pick arbitrary elements
        if (step == 0)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "phylanx::ir::slicing1d", "step can not be zero");
        }

        auto sv = blaze::elements(
            data, detail::create_list_slice(start, stop, step, data.size()));

        return node_data<T>{f.vector(sv)};
    }

    ///////////////////////////////////////////////////////////////////////////
    // return a slice from a 2d node_data
    template <typename T, typename F>
    node_data<T> slice2d(blaze::CustomMatrix<T, true, true>&& input_matrix,
        slicing_indices const& rows, slicing_indices const& columns, F const& f)
    {
        std::int64_t row_start = rows.start();
        std::int64_t row_stop = rows.stop();
        std::int64_t row_step = rows.step();

        std::int64_t col_start = columns.start();
        std::int64_t col_stop = columns.stop();
        std::int64_t col_step = columns.step();

        if (row_step == 0 && !rows.single_value())
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "phylanx::ir::slicing2d", "row-step can not be zero");
        }

        if (col_step == 0 && !columns.single_value())
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "phylanx::ir::slicing2d", "column-step can not be zero");
        }

        std::size_t num_matrix_rows = input_matrix.rows();
        std::size_t num_matrix_cols = input_matrix.columns();

        // return a value and not a vector if you are not given a list
        if (rows.single_value())
        {
            if (row_start < 0)
            {
                row_start = num_matrix_rows + row_start;
            }

            auto row = blaze::trans(blaze::row(input_matrix, row_start));

            // handle single value slicing result
            if (columns.single_value())
            {
                if (col_start < 0)
                {
                    col_start = num_matrix_cols + col_start;
                }

                return node_data<T>{f.scalar(row[col_start])}};
            }

            // extract a consecutive sub-vector (sub-row)
            if (col_step == 1)
            {
                HPX_ASSERT(col_stop > col_start);
                auto sv =
                    blaze::subvector(row, col_start, col_stop - col_start);
                return node_data<T>{f.vector(sv)};
            }

            // general case, pick arbitrary elements from selected row
            auto sv = blaze::elements(row, detail::create_list_slice(
                col_start, col_stop, col_step, num_matrix_cols));

            return node_data<T>{f.vector(sv)};
        }
        else if (columns.single_value())
        {
            // handle single column case
            if (col_start < 0)
            {
                col_start = num_matrix_cols + col_start;
            }

            auto col = blaze::column(input_matrix, col_start);

            // extract a consecutive sub-vector (sub-column)
            if (row_step == 1)
            {
                HPX_ASSERT(row_stop > row_start);
                auto sv =
                    blaze::subvector(col, row_start, row_stop - row_start);
                return node_data<T>{f.vector(sv)}};
            }

            // general case, pick arbitrary elements from selected column
            auto sv = blaze::elements(col, detail::create_list_slice(
                row_start, row_stop, row_step, num_matrix_rows));

            return node_data<T>{f.vector(sv)};
        }

        // extract various sub-matrices of the given matrix
        if (col_step == 1)
        {
            HPX_ASSERT(col_stop > col_start);

            if (row_step == 1)
            {
                HPX_ASSERT(row_stop > row_start);
                auto result = blaze::submatrix(input_matrix, row_start,
                    col_start, row_stop - row_start, col_stop - col_start);
                return node_data<T>{f.matrix(result)};
            }

            auto sm = blaze::submatrix(input_matrix, 0ll, col_start,
                num_matrix_rows, col_stop - col_start);

            auto result = blaze::rows(sm,
                detail::create_list_slice(
                    row_start, row_stop, row_step, num_matrix_rows));

            return node_data<T>{f.matrix(result)};
        }
        else if (row_step == 1)
        {
            HPX_ASSERT(row_stop > row_start);

            auto sm = blaze::submatrix(input_matrix, row_start, 0ll,
                row_stop - row_start, num_matrix_cols);

            auto result = blaze::rows(sm,
                detail::create_list_slice(
                    col_start, col_stop, col_step, num_matrix_cols));

            return node_data<T>{f.matrix(result)};
        }

        // general case, pick arbitrary elements from matrix
        auto sm = blaze::rows(input_matrix,
            detail::create_list_slice(
                row_start, row_stop, row_step, num_matrix_rows));

        auto result = blaze::columns(sm,
            detail::create_list_slice(
                col_start, col_stop, col_step, num_matrix_cols));

        return node_data<T>{f.matrix(result)};
    }

    namespace detail
    {
        std::int64_t extract_integer(
            execution_tree::primitive_argument_type const& val,
            std::int64_t default_value)
        {
            if (valid(val))
            {
                auto&& nd =
                    execution_tree::extract_integer_value(val);

                if (nd.size() == 0)
                {
                    HPX_THROW_EXCEPTION(hpx::bad_parameter,
                        "phylanx::ir::detail::extract_integer_value",
                        "slicing arguments cannot be empty");
                }

                return nd[0];
            }
            return default_value;
        }

        slicing_indices extract_slicing(
            execution_tree::primitive_argument_type const& arg,
            std::size_t arg_size)
        {
            slicing_indices indices;

            // Extract the list or the single integer index
            // from second argument (row-> start, stop, step)
            if (execution_tree::is_list_operand_strict(arg))
            {
                auto arg_list =
                    execution_tree::extract_list_value(arg);

                if (arg_list.index() == 0)
                {
                    return arg_list.xrange();
                }

                std::size_t size = arg_list.size();
                if (size > 3)
                {
                    HPX_THROW_EXCEPTION(hpx::bad_parameter,
                        "phylanx::ir::detail::extract_slicing",
                        "too many indicies given");
                }

                auto it = arg_list.begin();

                // if step is negative and start/stop are not given, then start
                // and stop must be swapped
                std::int64_t default_start = 0;
                std::int64_t default_stop = arg_size;
                std::int64_t step = 1;
                if (size == 3)
                {
                    std::advance(it, 2);
                    step = extract_integer(*it, 1ll);
                    if (step < 0)
                    {
                        // create_list_slice above will add list size to these
                        // values
                        default_start = -1;
                        default_stop = -arg_size - 1;
                    }
                }

                // reinit iterator
                it = arg_list.begin();

                // default first index is '0'
                if (size > 0)
                {
                    indices.start(extract_integer(*it, default_start), true);
                }
                else
                {
                    indices.start(0, true);
                }

                // default last index is 'size'
                if (size > 1)
                {
                    indices.stop(extract_integer(*++it, default_stop), false);
                }
                else
                {
                    indices.stop(arg_size, false);
                }

                // default step is '1'
                indices.step(step);
            }
            else if (!valid(arg))
            {
                // no arguments given means return all of the argument
                indices.start(0, false);
                indices.stop(arg_size);
                indices.step(1);
            }
            else
            {
                // allow for the slicing parameters to be a single integer
                indices.start(extract_integer(arg, 0), true);
            }

            return indices;
        }
    }

    ///////////////////////////////////////////////////////////////////////////
    // return a slice of the given node_data instance
    template <typename T>
    node_data<T> slice(node_data<T> const& data, ir::range const& indices)
    {
        switch (data.index())
        {
        case 0:
            return slice0d(data.scalar(), detail::extract_slicing(indices, 1));

        case 1: HPX_FALLTHROUGH;
        case 3:
            {
                auto v = data.vector();
                std::size_t size = v.size();
                return slice1d(
                    std::move(v), detail::extract_slicing(indices, size));
            }

        case 2: HPX_FALLTHROUGH;
        case 4:
            {
                auto m = data.matrix();
                std::size_t rows = m.rows();
                slicing_indices columns{0ll, std::int64_t(m.columns()), 1ll};
                return slice2d(std::move(m),
                    detail::extract_slicing(indices, rows), columns);
            }

        default:
            break;
        }

        HPX_THROW_EXCEPTION(hpx::invalid_status,
            "phylanx::ir::slice",
            "target node_data object holds unsupported data type");
    }

    template <typename T>
    node_data<T> slice(node_data<T> const& data, ir::range const& rows,
        ir::range const& columns)
    {
        switch (data.index())
        {
        case 2: HPX_FALLTHROUGH;
        case 4:
            {
                auto m = data.matrix();
                std::size_t numrows = m.rows();
                std::size_t numcols = m.columns();
                return slice2d(std::move(m),
                    detail::extract_slicing(rows, numrows),
                    detail::extract_slicing(columns, numcols));
            }

        case 0: HPX_FALLTHROUGH;
        case 1: HPX_FALLTHROUGH;
        case 3: HPX_FALLTHROUGH;
        default:
            break;
        }

        HPX_THROW_EXCEPTION(hpx::invalid_status,
            "phylanx::ir::slice",
            "target node_data object holds data type that does not support 2d "
            "slicing");
    }

    ///////////////////////////////////////////////////////////////////////////
    // explicit instantiations of the slice (extract) functionality
    template PHYLANX_EXPORT node_data<std::uint8_t> slice<std::uint8_t>(
        node_data<std::uint8_t> const& data, ir::range const& indices);

    template PHYLANX_EXPORT node_data<double> slice<double>(
        node_data<double> const& data, ir::range const& indices);

    template PHYLANX_EXPORT node_data<std::int64_t> slice<std::int64_t>(
        node_data<std::int64_t> const& data, ir::range const& indices);

    template PHYLANX_EXPORT node_data<std::uint8_t> slice<std::uint8_t>(
        node_data<std::uint8_t> const& data, ir::range const& rows,
        ir::range const& columns);

    template PHYLANX_EXPORT node_data<double> slice<double>(
        node_data<double> const& data, ir::range const& rows,
        ir::range const& columns);

    template PHYLANX_EXPORT node_data<std::int64_t> slice<std::int64_t>(
        node_data<std::int64_t> const& data, ir::range const& rows,
        ir::range const& columns);

    ///////////////////////////////////////////////////////////////////////////
    template <typename T>
    node_data<T> slice0d(
        ir::slicing_indices const& indices, node_data<T>&& value)
    {
#if defined(_DEBUG)
        if (indices.start() != 0 || indices.span() != 1 || indices.step() != 1)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "phylanx::ir::slicing0d",
                "cannot modify anything but the first element of a scalar");
        }
#endif
        return std::move(value);
    }

    template <typename T>
    node_data<T> slice1d(blaze::CustomVector<T, true, true> const& data,
        slicing_indices const& indices, node_data<T>&& value)
    {
        return std::move(value);
    }

    template <typename T>
    node_data<T> slice2d(blaze::CustomMatrix<T, true, true> const& data,
        slicing_indices const& rows, slicing_indices const& columns,
        node_data<T>&& value)
    {
        return std::move(value);
    }

    ///////////////////////////////////////////////////////////////////////////
    // Modifying slice functionality
    template <typename T>
    node_data<T> slice(node_data<T>&& data, slicing_indices const& indices,
        node_data<T>&& value)
    {
        switch (data.index())
        {
        case 0:
            return slice0d(indices, std::move(value));

        case 1: HPX_FALLTHROUGH;
        case 3:
            return slice1d(data.vector(), indices, std::move(value));

        case 2: HPX_FALLTHROUGH;
        case 4:
            {
                auto m = data.matrix();
                slicing_indices columns{0ll, std::int64_t(m.columns()), 1ll};
                return slice2d(
                    std::move(m), indices, columns, std::move(value));
            }

        default:
            break;
        }

        HPX_THROW_EXCEPTION(hpx::invalid_status,
            "phylanx::ir::slice",
            "target node_data object holds unsupported data type");
    }

    template <typename T>
    node_data<T> slice(node_data<T>&& data, slicing_indices const& rows,
        slicing_indices const& columns, node_data<T>&& value)
    {
        switch (data.index())
        {
        case 2: HPX_FALLTHROUGH;
        case 4:
            return slice2d(data.matrix(), rows, columns, std::move(value));

        case 0: HPX_FALLTHROUGH;
        case 1: HPX_FALLTHROUGH;
        case 3: HPX_FALLTHROUGH;
        default:
            break;
        }

        HPX_THROW_EXCEPTION(hpx::invalid_status,
            "phylanx::ir::slice",
            "target node_data object holds data type that does not support 2d "
            "slicing");
    }

    ///////////////////////////////////////////////////////////////////////////
    // explicit instantiations of the slice (modify) functionality
    template PHYLANX_EXPORT node_data<std::uint8_t> slice<std::uint8_t>(
        node_data<std::uint8_t>&& data, ir::range const& indices,
        node_data<std::uint8_t>&& value);

    template PHYLANX_EXPORT node_data<double> slice<double>(
        node_data<double>&& data, ir::range const& indices,
        node_data<double>&& value);

    template PHYLANX_EXPORT node_data<std::int64_t> slice<std::int64_t>(
        node_data<std::int64_t>&& data, ir::range const& indices,
        node_data<std::int64_t>&& value);

    template PHYLANX_EXPORT node_data<std::uint8_t> slice<std::uint8_t>(
        node_data<std::uint8_t>&& data, ir::range const& rows,
        ir::range const& columns, node_data<std::uint8_t>&& value);

    template PHYLANX_EXPORT node_data<double> slice<double>(
        node_data<double>&& data, ir::range const& rows,
        ir::range const& columns, node_data<double>&& value);

    template PHYLANX_EXPORT node_data<std::int64_t> slice<std::int64_t>(
        node_data<std::int64_t>&& data, ir::range const& rows,
        ir::range const& columns, node_data<std::int64_t>&& value);
}}
