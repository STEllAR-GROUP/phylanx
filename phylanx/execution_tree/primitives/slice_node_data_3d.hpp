// Copyright (c) 2018 Hartmut Kaiser
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
        ir::slicing_indices const& rows, ir::slicing_indices const& columns,
        ir::slicing_indices const& pages, F const& f, std::string const& name,
        std::string const& codename)
    {
        std::size_t numrows = t.rows();
        if (rows.start() >= std::int64_t(numrows) ||
            rows.span() > std::int64_t(numrows))
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "phylanx::execution_tree::slice3d_basic_basic_basic",
                util::generate_error_message(
                    "cannot extract the requested tensor elements",
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
                    "cannot extract the requested tensor elements",
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

        std::size_t numpages = t.pages();
        if (pages.start() >= std::int64_t(numpages) ||
            pages.span() > std::int64_t(numpages))
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "phylanx::execution_tree::slice3d_basic_basic_basic",
                util::generate_error_message(
                    "cannot extract the requested tensor elements",
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

        HPX_THROW_EXCEPTION(hpx::bad_parameter,
            "phylanx::execution_tree::slice3d_basic_basic_basic",
            util::generate_error_message(
                "unsupported indexing type", name, codename));
    }

    ///////////////////////////////////////////////////////////////////////////
    template <typename T, typename Data, typename F>
    ir::node_data<T> slice3d_integer_integer_integer(Data&& t,
        ir::node_data<std::int64_t> && rows,
        ir::node_data<std::int64_t> && columns,
        ir::node_data<std::int64_t> && pages, F const& f,
        std::string const& name, std::string const& codename)
    {
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

        std::size_t numpages = t.pages();
        std::size_t pages_index_size = pages.size();
        for (std::size_t i = 0; i != pages_index_size; ++i)
        {
            pages[i] = detail::check_index(pages[i], numpages, name, codename);
        }

        // return a value and not a vector if you are not given a list
        if (pages.size() == 1)
        {
            // handle single value slicing result
            auto pageslice = blaze::pageslice(t, pages[0]);
            if (rows.size() == 1)
            {
                auto row = blaze::row(pageslice, rows[0]);

                if (columns.size() == 1)
                {
                    return f.scalar(t, row[columns[0]]);
                }

                // general case, pick arbitrary elements from selected row
                auto sv = blaze::elements(
                    row, columns.vector().data(), columns.size());

                return f.trans_vector(t, std::move(sv));
            }
            else if (columns.size() == 1)
            {
                auto col = blaze::column(pageslice, columns[0]);

                // general case, pick arbitrary elements from selected column
                auto sv = blaze::elements(col, rows.vector().data(), rows.size());

                return f.vector(t, std::move(sv));
            }

            // general case, pick arbitrary elements from matrix
            auto sm = blaze::rows(pageslice, rows.vector().data(), rows.size());
            auto result =
                blaze::columns(sm, columns.vector().data(), columns.size());

            return f.matrix(t, std::move(result));
        }

        HPX_THROW_EXCEPTION(hpx::bad_parameter,
            "phylanx::execution_tree::slice3d_integer_integer_integer",
            util::generate_error_message(
                "unsupported indexing type", name, codename));
    }

    ///////////////////////////////////////////////////////////////////////////
    template <typename T>
    ir::node_data<T> slice1d_extract3d(ir::node_data<T> const& data,
        execution_tree::primitive_argument_type const& indices,
        std::string const& name, std::string const& codename)
    {
        auto t = data.tensor();
        ir::slicing_indices pages{0ll, std::int64_t(t.pages()), 1ll};
        ir::slicing_indices columns{0ll, std::int64_t(t.columns()), 1ll};
        return slice3d_basic_basic_basic<T>(t,
            util::slicing_helpers::extract_slicing(
                indices, t.rows(), name, codename),
            columns, pages, detail::slice_identity<T>{}, name, codename);
    }

    ///////////////////////////////////////////////////////////////////////////
    template <typename T>
    ir::node_data<T> slice2d_extract3d(ir::node_data<T> const& data,
        execution_tree::primitive_argument_type const& rows,
        execution_tree::primitive_argument_type const& columns,
        std::string const& name, std::string const& codename)
    {
        HPX_THROW_EXCEPTION(hpx::bad_parameter,
            "phylanx::execution_tree::slice2d_extract3d",
            util::generate_error_message(
                "unsupported indexing type", name, codename));
    }

    ///////////////////////////////////////////////////////////////////////////
    // Slice 3D data with one (row) index, extracts full columns
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
    // This is the main entry point for 3d slicing
    template <typename T, typename Data, typename F>
    ir::node_data<T> slice3d(Data&& t, primitive_argument_type const& rows,
        primitive_argument_type const& columns,
        primitive_argument_type const& pages, F const& f,
        std::string const& name, std::string const& codename)
    {
        if (is_list_operand_strict(rows) && is_list_operand_strict(columns) &&
            is_list_operand_strict(pages))
        {
            if (detail::is_advanced_slicing_index(rows) ==
                    detail::slicing_index_basic &&
                detail::is_advanced_slicing_index(columns) ==
                    detail::slicing_index_basic &&
                detail::is_advanced_slicing_index(pages) ==
                    detail::slicing_index_basic)
            {
                std::size_t num_rows = t.columns();
                std::size_t num_cols = t.columns();
                std::size_t num_pages = t.pages();
                auto row_indices = util::slicing_helpers::extract_slicing(
                    rows, num_rows, name, codename);
                auto col_indices = util::slicing_helpers::extract_slicing(
                    columns, num_cols, name, codename);
                auto page_indices = util::slicing_helpers::extract_slicing(
                    pages, num_pages, name, codename);

                return slice3d_basic_basic_basic<T>(std::forward<Data>(t),
                    row_indices, col_indices, page_indices, f, name, codename);
            }
        }
        else if (is_integer_operand(rows) && is_integer_operand(columns) &&
            is_integer_operand(pages))
        {
            return slice3d_integer_integer_integer<T>(std::forward<Data>(t),
                extract_integer_value(rows, name, codename),
                extract_integer_value(columns, name, codename),
                extract_integer_value(pages, name, codename), f, name,
                codename);
        }

        HPX_THROW_EXCEPTION(hpx::bad_parameter,
            "phylanx::execution_tree::slice3d",
            util::generate_error_message(
                "unsupported indexing type", name, codename));
    }

    ///////////////////////////////////////////////////////////////////////////
    template <typename T>
    ir::node_data<T> slice3d_extract3d(ir::node_data<T> const& data,
        execution_tree::primitive_argument_type const& rows,
        execution_tree::primitive_argument_type const& columns,
        execution_tree::primitive_argument_type const& pages,
        std::string const& name, std::string const& codename)
    {
        return slice3d<T>(data.tensor(), rows, columns, pages,
            detail::slice_identity<T>{}, name, codename);
    }

    ///////////////////////////////////////////////////////////////////////////
    // Slice 3d data with two indices
    template <typename T>
    ir::node_data<T> slice3d_assign3d(ir::node_data<T>&& data,
        execution_tree::primitive_argument_type const& rows,
        execution_tree::primitive_argument_type const& columns,
        execution_tree::primitive_argument_type const& pages,
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
