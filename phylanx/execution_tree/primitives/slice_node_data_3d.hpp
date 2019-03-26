// Copyright (c) 2018-2019 Hartmut Kaiser
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(PHYLANX_IR_NODE_SLICE_NODE_DATA_3d_SEP_18_2018_0125PM)
#define PHYLANX_IR_NODE_SLICE_NODE_DATA_3d_SEP_18_2018_0125PM

#include <phylanx/config.hpp>

#if defined(PHYLANX_HAVE_BLAZE_TENSOR)
#include <phylanx/execution_tree/primitives/base_primitive.hpp>
#include <phylanx/execution_tree/primitives/detail/slice_assign.hpp>
#include <phylanx/execution_tree/primitives/detail/slice_identity.hpp>
#include <phylanx/execution_tree/primitives/node_data_helpers.hpp>
#include <phylanx/ir/node_data.hpp>
#include <phylanx/ir/ranges.hpp>
#include <phylanx/util/slicing_helpers.hpp>

#include <hpx/throw_exception.hpp>
#include <hpx/util/assert.hpp>

#include <cstddef>
#include <cstdint>
#include <string>
#include <utility>

#include <blaze/Math.h>
#include <blaze/math/Elements.h>

// Slicing functionality for 3d data
namespace phylanx { namespace execution_tree
{
    ///////////////////////////////////////////////////////////////////////////
    // return a slice from a 3d ir::node_data
    template <typename T, typename Data, typename F>
    ir::node_data<T> slice3d_basic_basic_basic(Data&& t,
        ir::slicing_indices const& pages, ir::slicing_indices const& rows,
        ir::slicing_indices const& columns, F const& f,
        std::string const& name, std::string const& codename)
    {
        std::size_t numpages = t.pages();
        if (pages.start() >= std::int64_t(numpages) ||
            pages.span() > std::int64_t(numpages))
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "phylanx::execution_tree::slice3d_basic_basic_basic",
                util::generate_error_message(
                    "cannot extract the requested tensor element(s)",
                    name, codename));
        }

        std::int64_t page_start = pages.start();
        std::int64_t page_stop = pages.stop();
        std::int64_t page_step = pages.step();

        if (page_step == 0 && !pages.single_value())
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "phylanx::execution_tree::slice3d_basic_basic_basic",
                util::generate_error_message(
                    "page-step can not be zero", name, codename));
        }

        std::size_t numrows = t.rows();
        if (rows.start() >= std::int64_t(numrows) ||
            rows.span() > std::int64_t(numrows))
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "phylanx::execution_tree::slice3d_basic_basic_basic",
                util::generate_error_message(
                    "cannot extract the requested tensor element(s)",
                    name, codename));
        }

        std::int64_t row_start = rows.start();
        std::int64_t row_stop = rows.stop();
        std::int64_t row_step = rows.step();

        if (row_step == 0 && !rows.single_value())
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "phylanx::execution_tree::slice3d_basic_basic_basic",
                util::generate_error_message(
                    "row-step can not be zero", name, codename));
        }

        std::size_t numcols = t.columns();
        if (columns.start() >= std::int64_t(numcols) ||
            columns.span() > std::int64_t(numcols))
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "phylanx::execution_tree::slice3d_basic_basic_basic",
                util::generate_error_message(
                    "cannot extract the requested tensor element(s)",
                    name, codename));
        }

        std::int64_t col_start = columns.start();
        std::int64_t col_stop = columns.stop();
        std::int64_t col_step = columns.step();

        if (col_step == 0 && !columns.single_value())
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "phylanx::execution_tree::slice3d_basic_basic_basic",
                util::generate_error_message(
                    "column-step can not be zero", name, codename));
        }

        std::size_t num_tensor_pages = t.pages();
        std::size_t num_tensor_rows = t.rows();
        std::size_t num_tensor_cols = t.columns();

        // handle special cases
        if (pages.single_value())
        {
            auto pageslice = blaze::pageslice(t, page_start);

            if (rows.single_value())
            {
                auto row = blaze::row(pageslice, row_start);

                // handle single value slicing result
                if (columns.single_value())
                {
                    return f.scalar(t, row[col_start]);
                }

                // extract a consecutive sub-vector (sub-row)
                if (col_step == 1)
                {
                    HPX_ASSERT(col_stop > col_start);
                    auto sv =
                        blaze::subvector(row, col_start, col_stop - col_start);
                    return f.trans_vector(t, std::move(sv));
                }

                // general case, pick arbitrary elements from selected row
                auto indices = util::slicing_helpers::create_list_slice(
                    col_start, col_stop, col_step);
                auto sv = blaze::elements(row, indices);

                return f.trans_vector(t, std::move(sv));
            }
            else if (columns.single_value())
            {
                // handle single column case
                auto col = blaze::column(pageslice, col_start);

                // extract a consecutive sub-vector (sub-column)
                if (row_step == 1)
                {
                    HPX_ASSERT(row_stop > row_start);
                    auto sv =
                        blaze::subvector(col, row_start, row_stop - row_start);
                    return f.vector(t, std::move(sv));
                }

                // general case, pick arbitrary elements from selected column
                auto indices = util::slicing_helpers::create_list_slice(
                    row_start, row_stop, row_step);
                auto sv = blaze::elements(col, indices);

                return f.vector(t, std::move(sv));
            }
        }
        else if (rows.single_value())
        {
            auto rowslice = blaze::rowslice(t, row_start);

            if (columns.single_value())
            {
                // the row of a rowslice is a vector along the pages of the tensor
                auto row = blaze::row(rowslice, col_start);

                // extract a consecutive sub-vector (sub-'column')
                if (page_step == 1)
                {
                    HPX_ASSERT(page_stop > page_start);
                    auto sv = blaze::subvector(
                        row, page_start, page_stop - page_start);
                    return f.vector(t, std::move(sv));
                }

                // general case, pick arbitrary elements from selected 'column'
                auto indices = util::slicing_helpers::create_list_slice(
                    page_start, page_stop, page_step);
                auto sv = blaze::elements(row, indices);

                return f.vector(t, std::move(sv));
            }

            // general case, pick arbitrary elements from matrix (rowslice)
            auto row_indices = util::slicing_helpers::create_list_slice(
                page_start, page_stop, page_step);
            auto sm = blaze::columns(rowslice, row_indices);

            auto column_indices = util::slicing_helpers::create_list_slice(
                col_start, col_stop, col_step);
            auto result = blaze::rows(sm, column_indices);

            return f.matrix(t, std::move(result));
        }
        else if (columns.single_value())
        {
            auto columnslice = blaze::columnslice(t, col_start);

            // general case, pick arbitrary elements from matrix (columnslice)
            auto row_indices = util::slicing_helpers::create_list_slice(
                page_start, page_stop, page_step);
            auto sm = blaze::rows(columnslice, row_indices);

            auto column_indices = util::slicing_helpers::create_list_slice(
                row_start, row_stop, row_step);
            auto result = blaze::columns(sm, column_indices);

            return f.matrix(t, std::move(result));
        }

        // extract various sub-tensors of the given tensor
        if (page_step == 1)
        {
            HPX_ASSERT(page_stop > page_start);

            if (col_step == 1)
            {
                HPX_ASSERT(col_stop > col_start);

                if (row_step == 1)
                {
                    HPX_ASSERT(row_stop > row_start);
                    auto result = blaze::subtensor(t, page_start, row_start,
                        col_start, page_stop - page_start, row_stop - row_start,
                        col_stop - col_start);
                    return f.tensor(t, std::move(result));
                }

                // row_step != 1: fall through as not-implemented
            }
            else if (row_step == 1)
            {
                // column_step != 1: fall through as not-implemented
            }
        }

        HPX_THROW_EXCEPTION(hpx::bad_parameter,
            "phylanx::execution_tree::slice3d_basic_basic_basic",
            util::generate_error_message(
                "unsupported indexing type", name, codename));
    }

    ///////////////////////////////////////////////////////////////////////////
    template <typename T, typename Data, typename F>
    ir::node_data<T> slice3d_integer_integer_integer(Data&& t,
        ir::node_data<std::int64_t> && pages,
        ir::node_data<std::int64_t> && rows,
        ir::node_data<std::int64_t> && columns, F const& f,
        std::string const& name, std::string const& codename)
    {
        std::size_t numpages = t.pages();
        std::size_t pages_index_size = pages.size();
        for (std::size_t i = 0; i != pages_index_size; ++i)
        {
            pages[i] = detail::check_index(pages[i], numpages, name, codename);
        }

        std::size_t numrows = t.rows();
        std::size_t rows_index_size = rows.size();
        for (std::size_t i = 0; i != rows_index_size; ++i)
        {
            rows[i] = detail::check_index(rows[i], numrows, name, codename);
        }

        std::size_t numcols = t.columns();
        std::size_t columns_index_size = columns.size();
        for (std::size_t i = 0; i != columns_index_size; ++i)
        {
            columns[i] =
                detail::check_index(columns[i], numcols, name, codename);
        }

        // broadcast all index arrays, use result to index elements in tensor
        std::size_t largest_dimensionality =
            extract_largest_dimension(name, codename, pages, rows);
        if (largest_dimensionality > 1)
        {
            HPX_THROW_EXCEPTION(hpx::not_implemented,
                "phylanx::execution_tree::slice3d_integer_integer_integer",
                util::generate_error_message(
                    "this operation is not supported (yet)", name, codename));
        }

        if (largest_dimensionality == 0)
        {
            auto row = blaze::row(blaze::pageslice(t, pages[0]), rows[0]);
            return f.scalar(t, t(pages[0], rows[0], columns[0]));
        }

        auto sizes = extract_largest_dimensions(name, codename, pages, rows);
        auto page_indices =
            extract_value_vector<T>(std::move(pages), sizes[0], name, codename);
        auto row_indices =
            extract_value_vector<T>(std::move(rows), sizes[0], name, codename);
        auto column_indices =
            extract_value_vector<T>(std::move(columns), sizes[0], name, codename);

        typename ir::node_data<T>::storage1d_type result(page_indices.size());

        for (std::size_t i = 0; i != page_indices.size(); ++i)
        {
            result[i] = t(page_indices[i], row_indices[i], column_indices[i]);
        }

        return f.vector(t, std::move(result));
    }

    ///////////////////////////////////////////////////////////////////////////
    template <typename T, typename Data, typename F>
    ir::node_data<T> slice3d_integer_integer(Data&& t,
        ir::node_data<std::int64_t> && pages,
        ir::node_data<std::int64_t> && rows, F const& f,
        std::string const& name, std::string const& codename)
    {
        std::size_t numpages = t.pages();
        std::size_t pages_index_size = pages.size();
        for (std::size_t i = 0; i != pages_index_size; ++i)
        {
            pages[i] = detail::check_index(pages[i], numpages, name, codename);
        }

        std::size_t numrows = t.rows();
        std::size_t rows_index_size = rows.size();
        for (std::size_t i = 0; i != rows_index_size; ++i)
        {
            rows[i] = detail::check_index(rows[i], numrows, name, codename);
        }

        // broadcast both index arrays, use result to index elements in matrix
        std::size_t largest_dimensionality =
            extract_largest_dimension(name, codename, pages, rows);
        if (largest_dimensionality > 1)
        {
            HPX_THROW_EXCEPTION(hpx::not_implemented,
                "phylanx::execution_tree::slice3d_integer_integer",
                util::generate_error_message(
                    "this operation is not supported (yet)", name, codename));
        }

        if (largest_dimensionality == 0)
        {
            auto row = blaze::row(blaze::pageslice(t, pages[0]), rows[0]);
            return f.vector(t, row);
        }

        auto sizes = extract_largest_dimensions(name, codename, pages, rows);
        auto page_indices =
            extract_value_vector<T>(std::move(pages), sizes[0], name, codename);
        auto row_indices =
            extract_value_vector<T>(std::move(rows), sizes[0], name, codename);

        typename ir::node_data<T>::storage2d_type result(
            page_indices.size(), t.columns());

        for (std::size_t i = 0; i != page_indices.size(); ++i)
        {
            blaze::row(result, i) = blaze::row(
                blaze::pageslice(t, page_indices[i]), row_indices[i]);
        }

        return f.matrix(t, std::move(result));
    }

    ///////////////////////////////////////////////////////////////////////////
    template <typename T, typename Data, typename F>
    ir::node_data<T> slice3d_integer_0d(Data&& t,
        ir::node_data<std::int64_t> && pages, F const& f,
        std::string const& name, std::string const& codename)
    {
        // handle single value page-slicing result
        auto pageslice = blaze::pageslice(
            t, detail::check_index(pages[0], pages.size(), name, codename));

        return f.matrix(t, std::move(pageslice));
    }

#if defined(PHYLANX_HAVE_BLAZE_TENSOR)
    template <typename T, typename Data, typename F>
    ir::node_data<T> slice3d_integer_1d(Data&& t,
        ir::node_data<std::int64_t> && pages, F const& f,
        std::string const& name, std::string const& codename)
    {
        std::size_t numpages = t.pages();
        std::size_t pages_index_size = pages.size();

        typename ir::node_data<T>::storage3d_type result(
            pages_index_size, t.rows(), t.columns());

        for (std::size_t i = 0; i != pages_index_size; ++i)
        {
            blaze::pageslice(result, i) = blaze::pageslice(
                t, detail::check_index(pages[i], numpages, name, codename));
        }

        return f.tensor(t, std::move(result));
    }
#endif

    template <typename T, typename Data, typename F>
    ir::node_data<T> slice3d_integer(Data&& t,
        ir::node_data<std::int64_t> && pages, F const& f,
        std::string const& name, std::string const& codename)
    {
        switch (pages.num_dimensions())
        {
        case 0:
            return slice3d_integer_0d<T>(std::forward<Data>(t),
                std::move(pages), f, name, codename);

#if defined(PHYLANX_HAVE_BLAZE_TENSOR)
        case 1:
            return slice3d_integer_1d<T>(std::forward<Data>(t),
                std::move(pages), f, name, codename);
#endif
        default:
            break;
        }

        HPX_THROW_EXCEPTION(hpx::not_implemented,
            "phylanx::execution_tree::slice3d_integer",
            util::generate_error_message(
                "this operation is not supported (yet)", name, codename));
    }

    ///////////////////////////////////////////////////////////////////////////
    template <typename T, typename Data, typename F>
    ir::node_data<T> slice3d_boolean_boolean_boolean(Data&& t,
        ir::node_data<std::uint8_t> && pages,
        ir::node_data<std::uint8_t> && rows,
        ir::node_data<std::uint8_t> && columns, F const& f,
        std::string const& name, std::string const& codename)
    {
        HPX_THROW_EXCEPTION(hpx::bad_parameter,
            "phylanx::execution_tree::slice3d_boolean_boolean_boolean",
            util::generate_error_message(
                "unsupported indexing type", name, codename));
    }

//         auto t = data.tensor();
//         ir::slicing_indices rows{0ll, std::int64_t(t.rows()), 1ll};
//         ir::slicing_indices columns{0ll, std::int64_t(t.columns()), 1ll};
//         return slice3d_basic_basic_basic<T>(t,
//             util::slicing_helpers::extract_slicing(
//                 indices, t.pages(), name, codename),
//             rows, columns, detail::slice_identity<T>{}, name, codename);

    ///////////////////////////////////////////////////////////////////////////
    // This is the main entry point for 3d slicing
    template <typename T, typename Data, typename F>
    ir::node_data<T> slice3d(Data&& t, primitive_argument_type const& pages,
        primitive_argument_type const& rows,
        primitive_argument_type const& columns, F const& f,
        std::string const& name, std::string const& codename)
    {
        if (is_list_operand_strict(rows) && is_list_operand_strict(columns) &&
            is_list_operand_strict(pages))
        {
            auto column_type =
                detail::extract_slicing_index_type(rows, name, codename);
            auto row_type =
                detail::extract_slicing_index_type(columns, name, codename);
            auto page_type =
                detail::extract_slicing_index_type(pages, name, codename);

            if (column_type == detail::slicing_index_basic &&
                row_type == detail::slicing_index_basic &&
                page_type == detail::slicing_index_basic)
            {
                auto page_indices = util::slicing_helpers::extract_slicing(
                    pages, t.pages(), name, codename);
                auto row_indices = util::slicing_helpers::extract_slicing(
                    rows, t.rows(), name, codename);
                auto col_indices = util::slicing_helpers::extract_slicing(
                    columns, t.columns(), name, codename);

                return slice3d_basic_basic_basic<T>(std::forward<Data>(t),
                    page_indices, row_indices, col_indices, f, name, codename);
            }
            else if (column_type == detail::slicing_index_advanced_integer &&
                row_type == detail::slicing_index_advanced_integer &&
                page_type == detail::slicing_index_advanced_integer)
            {
                auto page_indices = extract_integer_value_strict(
                    detail::extract_advanced_integer_index(
                        pages, name, codename),
                    name, codename);
                auto row_indices = extract_integer_value_strict(
                    detail::extract_advanced_integer_index(
                        rows, name, codename),
                    name, codename);
                auto col_indices = extract_integer_value_strict(
                    detail::extract_advanced_integer_index(
                        columns, name, codename),
                    name, codename);

                return slice3d_integer_integer_integer<T>(std::forward<Data>(t),
                    std::move(page_indices), std::move(row_indices),
                    std::move(col_indices), f, name, codename);
            }
            else if (column_type == detail::slicing_index_advanced_boolean &&
                row_type == detail::slicing_index_advanced_boolean &&
                page_type == detail::slicing_index_advanced_boolean)
            {
                auto page_indices = extract_boolean_value_strict(
                    detail::extract_advanced_boolean_index(
                        pages, name, codename),
                    name, codename);
                auto row_indices = extract_boolean_value_strict(
                    detail::extract_advanced_boolean_index(
                        rows, name, codename),
                    name, codename);
                auto col_indices = extract_boolean_value_strict(
                    detail::extract_advanced_boolean_index(
                        columns, name, codename),
                    name, codename);

                return slice3d_boolean_boolean_boolean<T>(std::forward<Data>(t),
                    std::move(page_indices), std::move(row_indices),
                    std::move(col_indices), f, name, codename);
            }
        }
        else if (is_boolean_operand_strict(rows) &&
            is_boolean_operand_strict(columns) &&
            is_boolean_operand_strict(pages))
        {
            return slice3d_boolean_boolean_boolean<T>(std::forward<Data>(t),
                extract_boolean_value_strict(pages, name, codename),
                extract_boolean_value_strict(rows, name, codename),
                extract_boolean_value_strict(columns, name, codename), f, name,
                codename);
        }
        else if (is_integer_operand(pages))
        {
            if (is_integer_operand(rows))
            {
                if (is_integer_operand(columns))
                {
                    // 'normal' advanced integer indexing
                    return slice3d_integer_integer_integer<T>(
                        std::forward<Data>(t),
                        extract_integer_value(pages, name, codename),
                        extract_integer_value(rows, name, codename),
                        extract_integer_value(columns, name, codename), f, name,
                        codename);
                }
                else if (!valid(columns))
                {
                    // advanced integer indexing with valid page and column
                    // index
                    HPX_ASSERT(!is_explicit_nil(columns));

                    return slice3d_integer_integer<T>(std::forward<Data>(t),
                        extract_integer_value(pages, name, codename),
                        extract_integer_value(rows, name, codename), f, name,
                        codename);
                }
            }
            else if (!valid(rows) && !valid(columns))
            {
                // advanced integer indexing with valid page index
                HPX_ASSERT(!is_explicit_nil(rows) && !is_explicit_nil(columns));

                return slice3d_integer<T>(std::forward<Data>(t),
                    extract_integer_value(pages, name, codename), f, name,
                    codename);
            }
        }

        HPX_THROW_EXCEPTION(hpx::bad_parameter,
            "phylanx::execution_tree::slice3d",
            util::generate_error_message(
                "unsupported indexing type", name, codename));
    }

    ///////////////////////////////////////////////////////////////////////////
    // Slice 3D data with three (page, row, and column) indices
    template <typename T>
    ir::node_data<T> slice3d_extract3d(ir::node_data<T> const& data,
        execution_tree::primitive_argument_type const& pages,
        execution_tree::primitive_argument_type const& rows,
        execution_tree::primitive_argument_type const& columns,
        std::string const& name, std::string const& codename)
    {
        return slice3d<T>(data.tensor(), pages, rows, columns,
            detail::slice_identity<T>{}, name, codename);
    }

    ///////////////////////////////////////////////////////////////////////////
    // Slice 3D data with two (page and row) indices, extracts full rows
    template <typename T>
    ir::node_data<T> slice2d_extract3d(ir::node_data<T> const& data,
        execution_tree::primitive_argument_type const& rows,
        execution_tree::primitive_argument_type const& columns,
        std::string const& name, std::string const& codename)
    {
        return slice3d<T>(data.tensor(), rows, columns,
            primitive_argument_type{}, detail::slice_identity<T>{}, name,
            codename);
    }

    ///////////////////////////////////////////////////////////////////////////
    // Slice 3D data with one (page) index, extracts full matrices
    template <typename T>
    ir::node_data<T> slice1d_extract3d(ir::node_data<T> const& data,
        execution_tree::primitive_argument_type const& indices,
        std::string const& name, std::string const& codename)
    {
        return slice3d<T>(data.tensor(), indices, primitive_argument_type{},
            primitive_argument_type{}, detail::slice_identity<T>{}, name,
            codename);
    }

    ///////////////////////////////////////////////////////////////////////////
    // Slice 3D data with one (page) index, extracts full matrices
    template <typename T>
    ir::node_data<T> slice1d_assign3d(ir::node_data<T>&& data,
        execution_tree::primitive_argument_type const& indices,
        ir::node_data<T>&& value, std::string const& name,
        std::string const& codename)
    {
        HPX_THROW_EXCEPTION(hpx::bad_parameter,
            "phylanx::execution_tree::slice1d_assign3d",
            util::generate_error_message(
                "unsupported indexing type", name, codename));
    }

    ///////////////////////////////////////////////////////////////////////////
    // Slice 3d data with two indices
    template <typename T>
    ir::node_data<T> slice2d_assign3d(ir::node_data<T>&& data,
        execution_tree::primitive_argument_type const& rows,
        execution_tree::primitive_argument_type const& columns,
        ir::node_data<T>&& value, std::string const& name,
        std::string const& codename)
    {
        HPX_THROW_EXCEPTION(hpx::bad_parameter,
            "phylanx::execution_tree::slice2d_assign3d",
            util::generate_error_message(
                "unsupported indexing type", name, codename));
    }

    ///////////////////////////////////////////////////////////////////////////
    // Slice 3d data with three indices
    template <typename T>
    ir::node_data<T> slice3d_assign3d(ir::node_data<T>&& data,
        execution_tree::primitive_argument_type const& pages,
        execution_tree::primitive_argument_type const& rows,
        execution_tree::primitive_argument_type const& columns,
        ir::node_data<T>&& value, std::string const& name,
        std::string const& codename)
    {
        HPX_THROW_EXCEPTION(hpx::bad_parameter,
            "phylanx::execution_tree::slice3d",
            util::generate_error_message(
                "unsupported indexing type", name, codename));
    }
}}

#endif
#endif
