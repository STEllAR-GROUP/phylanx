// Copyright (c) 2018 Hartmut Kaiser
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(PHYLANX_IR_NODE_SLICE_NODE_DATA_2D_SEP_18_2018_0125PM)
#define PHYLANX_IR_NODE_SLICE_NODE_DATA_2D_SEP_18_2018_0125PM

#include <phylanx/config.hpp>
#include <phylanx/execution_tree/primitives/base_primitive.hpp>
#include <phylanx/execution_tree/primitives/detail/slice_assign.hpp>
#include <phylanx/execution_tree/primitives/detail/slice_identity.hpp>
#include <phylanx/execution_tree/primitives/node_data_helpers.hpp>
#include <phylanx/ir/node_data.hpp>
#include <phylanx/ir/ranges.hpp>
#include <phylanx/util/slicing_helpers.hpp>

#include <hpx/throw_exception.hpp>
#include <hpx/util/assert.hpp>

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <string>
#include <type_traits>
#include <utility>

#include <blaze/Math.h>
#include <blaze/math/Elements.h>

// Slicing functionality for 2d data
namespace phylanx { namespace execution_tree
{
    ///////////////////////////////////////////////////////////////////////////
    // return a slice from a 2d ir::node_data
    template <typename T, typename Data, typename F>
    ir::node_data<T> slice2d_basic_basic(Data&& m,
        ir::slicing_indices const& rows, ir::slicing_indices const& columns,
        F const& f, std::string const& name, std::string const& codename)
    {
        std::size_t numrows = m.rows();
        if (rows.start() >= std::int64_t(numrows) ||
            rows.span() > std::int64_t(numrows))
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "phylanx::execution_tree::slice2d_basic_basic",
                util::generate_error_message(
                    "cannot extract the requested matrix elements",
                    name, codename));
        }

        std::int64_t row_start = rows.start();
        std::int64_t row_stop = rows.stop();
        std::int64_t row_step = rows.step();

        if (row_step == 0 && !rows.single_value())
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "phylanx::execution_tree::slice2d_basic_basic",
                util::generate_error_message(
                    "row-step can not be zero", name, codename));
        }

        std::size_t numcols = m.columns();
        if (columns.start() >= std::int64_t(numcols) ||
            columns.span() > std::int64_t(numcols))
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "phylanx::execution_tree::slice2d_basic_basic",
                util::generate_error_message(
                    "cannot extract the requested matrix elements",
                    name, codename));
        }

        std::int64_t col_start = columns.start();
        std::int64_t col_stop = columns.stop();
        std::int64_t col_step = columns.step();

        if (col_step == 0 && !columns.single_value())
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "phylanx::execution_tree::slice2d_basic_basic",
                util::generate_error_message(
                    "column-step can not be zero", name, codename));
        }

        std::size_t num_matrix_rows = m.rows();
        std::size_t num_matrix_cols = m.columns();

        // return a value and not a vector if you are not given a list
        if (rows.single_value())
        {
            auto row = blaze::row(m, row_start);

            // handle single value slicing result
            if (columns.single_value())
            {
                return f.scalar(m, row[col_start]);
            }

            // extract a consecutive sub-vector (sub-row)
            if (col_step == 1)
            {
                HPX_ASSERT(col_stop > col_start);
                auto sv =
                    blaze::subvector(row, col_start, col_stop - col_start);
                return f.trans_vector(m, std::move(sv));
            }

            // general case, pick arbitrary elements from selected row
            auto indices = util::slicing_helpers::create_list_slice(
                col_start, col_stop, col_step);
            auto sv = blaze::elements(row, indices);

            return f.trans_vector(m, std::move(sv));
        }
        else if (columns.single_value())
        {
            // handle single column case
            auto col = blaze::column(m, col_start);

            // extract a consecutive sub-vector (sub-column)
            if (row_step == 1)
            {
                HPX_ASSERT(row_stop > row_start);
                auto sv =
                    blaze::subvector(col, row_start, row_stop - row_start);
                return f.vector(m, std::move(sv));
            }

            // general case, pick arbitrary elements from selected column
            auto indices = util::slicing_helpers::create_list_slice(
                row_start, row_stop, row_step);
            auto sv = blaze::elements(col, indices);

            return f.vector(m, std::move(sv));
        }

        // extract various sub-matrices of the given matrix
        if (col_step == 1)
        {
            HPX_ASSERT(col_stop > col_start);

            if (row_step == 1)
            {
                HPX_ASSERT(row_stop > row_start);
                auto result = blaze::submatrix(m, row_start,
                    col_start, row_stop - row_start, col_stop - col_start);
                return f.matrix(m, std::move(result));
            }

            auto sm = blaze::submatrix(m, 0ll, col_start,
                num_matrix_rows, col_stop - col_start);

            auto indices = util::slicing_helpers::create_list_slice(
                row_start, row_stop, row_step);
            auto result = blaze::rows(sm, indices);

            return f.matrix(m, std::move(result));
        }
        else if (row_step == 1)
        {
            HPX_ASSERT(row_stop > row_start);

            auto sm = blaze::submatrix(m, row_start, 0ll,
                row_stop - row_start, num_matrix_cols);

            auto indices = util::slicing_helpers::create_list_slice(
                col_start, col_stop, col_step);
            auto result = blaze::columns(sm, indices);

            return f.matrix(m, std::move(result));
        }

        // general case, pick arbitrary elements from matrix
        auto row_indices = util::slicing_helpers::create_list_slice(
            row_start, row_stop, row_step);
        auto sm = blaze::rows(m, row_indices);

        auto column_indices = util::slicing_helpers::create_list_slice(
            col_start, col_stop, col_step);
        auto result = blaze::columns(sm, column_indices);

        return f.matrix(m, std::move(result));
    }

    ///////////////////////////////////////////////////////////////////////////
    namespace detail
    {
        template <typename T, typename Index, typename Enable = void>
        struct extract_columns;

        template <typename T, typename Index>
        struct extract_columns<T, Index,
            typename std::enable_if<sizeof(std::int64_t) == sizeof(Index)>::type>
        {
            template <typename F, typename Data, typename SubMatrix>
            ir::node_data<T> operator()(F const& f, Data&& m, SubMatrix&& sm,
                ir::node_data<std::int64_t>&& columns) const
            {
                auto result = blaze::columns(sm,
                    reinterpret_cast<std::size_t*>(columns.vector().data()),
                    columns.size());

                return f.matrix(m, std::move(result));
            }
        };

        template <typename T, typename Index>
        struct extract_columns<T, Index,
            typename std::enable_if<sizeof(std::int64_t) != sizeof(Index)>::type>
        {
            template <typename F, typename Data, typename SubMatrix>
            ir::node_data<T> operator()(F const& f, Data&& m, SubMatrix&& sm,
                ir::node_data<std::int64_t>&& columns) const
            {
                std::unique_ptr<std::size_t[]> indices(
                    std::make_unique<std::size_t[]>(columns.size()));

                std::int64_t* p = columns.vector().data();
                std::copy(p, p + columns.size(), indices.get());

                auto result = blaze::columns(sm, indices.get(), columns.size());

                return f.matrix(m, std::move(result));
            }
        };
    }

    template <typename T, typename Data, typename F>
    ir::node_data<T> slice2d_basic_integer(Data&& m,
        ir::slicing_indices const& rows,
        ir::node_data<std::int64_t> && columns, F const& f,
        std::string const& name, std::string const& codename)
    {
        std::size_t numrows = m.rows();
        if (rows.start() >= std::int64_t(numrows) ||
            rows.span() > std::int64_t(numrows))
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "phylanx::execution_tree::slice2d_basic_integer",
                util::generate_error_message(
                    "cannot extract the requested matrix elements",
                    name, codename));
        }

        std::int64_t row_start = rows.start();
        std::int64_t row_stop = rows.stop();
        std::int64_t row_step = rows.step();

        if (row_step == 0 && !rows.single_value())
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "phylanx::execution_tree::slice2d_basic_integer",
                util::generate_error_message(
                    "row-step can not be zero", name, codename));
        }

        std::size_t numcols = m.columns();
        std::size_t index_size = columns.size();
        for (std::size_t i = 0; i != index_size; ++i)
        {
            columns[i] =
                detail::check_index(columns[i], numcols, name, codename);
        }

        // return a value and not a vector
        if (rows.single_value())
        {
            auto row = blaze::row(m, row_start);

            // handle single value slicing result
            if (columns.size() == 1)
            {
                return f.scalar(m, row[columns[0]]);
            }

            // general case, pick arbitrary elements from selected row
            auto sv =
                blaze::elements(row, columns.vector().data(), columns.size());

            return f.trans_vector(m, std::move(sv));
        }
        else if (columns.size() == 1)
        {
            auto col = blaze::column(m, columns[0]);

            // extract a consecutive sub-vector (sub-column)
            if (row_step == 1)
            {
                HPX_ASSERT(row_stop > row_start);
                auto sv =
                    blaze::subvector(col, row_start, row_stop - row_start);
                return f.vector(m, std::move(sv));
            }

            // general case, pick arbitrary elements from selected column
            auto indices = util::slicing_helpers::create_list_slice(
                row_start, row_stop, row_step);
            auto sv = blaze::elements(col, indices);

            return f.vector(m, std::move(sv));
        }

        // extract various sub-matrices of the given matrix
        if (row_step == 1)
        {
            HPX_ASSERT(row_stop > row_start);

            auto&& sm = blaze::submatrix(
                m, row_start, 0ll, row_stop - row_start, m.columns());

            return detail::extract_columns<T, std::size_t>{}(
                f, std::move(m), std::move(sm), std::move(columns));
        }

        // general case, pick arbitrary elements from matrix
        auto row_indices = util::slicing_helpers::create_list_slice(
            row_start, row_stop, row_step);
        auto sm = blaze::rows(m, row_indices);

        auto result =
            blaze::columns(sm, columns.vector().data(), columns.size());

        return f.matrix(m, std::move(result));
    }

    template <typename T, typename Data, typename F>
    ir::node_data<T> slice2d_basic_boolean(Data&& m,
        ir::slicing_indices const& rows,
        ir::node_data<std::uint8_t> && columns, F const& f,
        std::string const& name, std::string const& codename)
    {
        std::size_t numrows = m.rows();
        if (rows.start() >= std::int64_t(numrows) ||
            rows.span() > std::int64_t(numrows))
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "phylanx::execution_tree::slice2d_basic_boolean",
                util::generate_error_message(
                    "cannot extract the requested matrix elements",
                    name, codename));
        }

        std::int64_t row_start = rows.start();
        std::int64_t row_stop = rows.stop();
        std::int64_t row_step = rows.step();

        if (row_step == 0 && !rows.single_value())
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "phylanx::execution_tree::slice2d_basic_boolean",
                util::generate_error_message(
                    "row-step can not be zero", name, codename));
        }

        HPX_THROW_EXCEPTION(hpx::not_implemented,
            "phylanx::execution_tree::slice2d_basic_boolean",
            util::generate_error_message(
                "this operation is not supported (yet)", name, codename));

        return {};
    }

    ///////////////////////////////////////////////////////////////////////////
    template <typename T, typename Data, typename F>
    ir::node_data<T> slice2d_basic_column(Data&& m,
        ir::slicing_indices const& columns, F const& f,
        std::string const& name, std::string const& codename)
    {
        std::size_t numcols = m.columns();
        if (columns.start() >= std::int64_t(numcols) ||
            columns.span() > std::int64_t(numcols))
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "phylanx::execution_tree::slice2d_basic_column",
                util::generate_error_message(
                    "cannot extract the requested matrix elements",
                    name, codename));
        }

        std::int64_t col_start = columns.start();
        std::int64_t col_stop = columns.stop();
        std::int64_t col_step = columns.step();

        if (col_step == 0 && !columns.single_value())
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "phylanx::execution_tree::slice2d_basic_basic",
                util::generate_error_message(
                    "column-step can not be zero", name, codename));
        }

        if (columns.single_value())
        {
            // handle single column case
            auto col = blaze::column(m, col_start);
            return f.vector(m, std::move(col));
        }

        // extract various sub-matrices of the given matrix
        if (col_step == 1)
        {
            HPX_ASSERT(col_stop > col_start);

            auto sm = blaze::submatrix(
                m, 0ll, col_start, m.rows(), col_stop - col_start);

            return f.matrix(m, std::move(sm));
        }

        // general case, pick arbitrary columns from matrix
        auto indices = util::slicing_helpers::create_list_slice(
            col_start, col_stop, col_step);

        auto result = blaze::columns(m, indices.data(), indices.size());
        return f.matrix(m, std::move(result));
    }

    ///////////////////////////////////////////////////////////////////////////
    namespace detail
    {
        template <typename T, typename Index, typename Enable = void>
        struct extract_rows;

        template <typename T, typename Index>
        struct extract_rows<T, Index,
            typename std::enable_if<sizeof(std::int64_t) == sizeof(Index)>::type>
        {
            template <typename F, typename Data, typename SubMatrix>
            ir::node_data<T> operator()(F const& f, Data&& m, SubMatrix&& sm,
                ir::node_data<std::int64_t>&& rows) const
            {
                auto result = blaze::rows(sm,
                    reinterpret_cast<std::size_t*>(rows.vector().data()),
                    rows.size());

                return f.matrix(m, std::move(result));
            }
        };

        template <typename T, typename Index>
        struct extract_rows<T, Index,
            typename std::enable_if<sizeof(std::int64_t) != sizeof(Index)>::type>
        {
            template <typename F, typename Data, typename SubMatrix>
            ir::node_data<T> operator()(F const& f, Data&& m, SubMatrix&& sm,
                ir::node_data<std::int64_t>&& rows) const
            {
                std::unique_ptr<std::size_t[]> indices(
                    std::make_unique<std::size_t[]>(rows.size()));

                std::int64_t* p = rows.vector().data();
                std::copy(p, p + rows.size(), indices.get());

                auto result = blaze::rows(sm, indices.get(), rows.size());

                return f.matrix(m, std::move(result));
            }
        };
    }

    template <typename T, typename Data, typename F>
    ir::node_data<T> slice2d_integer_basic(Data&& m,
        ir::node_data<std::int64_t> && rows,
        ir::slicing_indices const& columns, F const& f,
        std::string const& name, std::string const& codename)
    {
        std::size_t numcols = m.columns();
        if (columns.start() >= std::int64_t(numcols) ||
            columns.span() > std::int64_t(numcols))
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "phylanx::execution_tree::slice2d_integer_basic",
                util::generate_error_message(
                    "cannot extract the requested matrix elements",
                    name, codename));
        }

        std::int64_t col_start = columns.start();
        std::int64_t col_stop = columns.stop();
        std::int64_t col_step = columns.step();

        if (col_step == 0 && !columns.single_value())
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "phylanx::execution_tree::slice2d_integer_basic",
                util::generate_error_message(
                    "column-step can not be zero", name, codename));
        }

        std::size_t numrows = m.rows();
        std::size_t index_size = rows.size();
        for (std::size_t i = 0; i != index_size; ++i)
        {
            rows[i] =
                detail::check_index(rows[i], numrows, name, codename);
        }

        if (rows.size() == 1)
        {
            auto row = blaze::row(m, rows[0]);

            // handle single value slicing result
            if (columns.single_value())
            {
                return f.scalar(m, row[col_start]);
            }

            // general case, pick arbitrary elements from selected row
            auto indices = util::slicing_helpers::create_list_slice(
                col_start, col_stop, col_step);
            auto sv = blaze::elements(row, indices);

            return f.trans_vector(m, std::move(sv));
        }
        else if (columns.single_value())
        {
            // handle single column case
            auto col = blaze::column(m, col_start);
            auto sv = blaze::elements(col, rows.vector().data(), rows.size());

            return f.vector(m, std::move(sv));
        }

        // extract various sub-matrices of the given matrix
        if (col_step == 1)
        {
            HPX_ASSERT(col_stop > col_start);

            auto sm = blaze::submatrix(m, 0ll, col_start,
                m.rows(), col_stop - col_start);

            return detail::extract_rows<T, std::size_t>{}(
                f, std::move(m), std::move(sm), std::move(rows));
        }

        // general case, pick arbitrary elements from matrix
        auto sm = blaze::rows(m, rows.vector().data(), rows.size());

        auto column_indices = util::slicing_helpers::create_list_slice(
            col_start, col_stop, col_step);
        auto result = blaze::columns(sm, column_indices);

        return f.matrix(m, std::move(result));
    }

    ///////////////////////////////////////////////////////////////////////////
    template <typename T, typename Data, typename F>
    ir::node_data<T> slice2d_boolean_basic(Data&& m,
        ir::node_data<std::uint8_t> && rows,
        ir::slicing_indices const& columns, F const& f,
        std::string const& name, std::string const& codename)
    {
        std::size_t numcols = m.columns();
        if (columns.start() >= std::int64_t(numcols) ||
            columns.span() > std::int64_t(numcols))
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "phylanx::execution_tree::slice2d_boolean_basic",
                util::generate_error_message(
                    "cannot extract the requested matrix elements",
                    name, codename));
        }

        std::int64_t col_start = columns.start();
        std::int64_t col_stop = columns.stop();
        std::int64_t col_step = columns.step();

        if (col_step == 0 && !columns.single_value())
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "phylanx::execution_tree::slice2d_boolean_basic",
                util::generate_error_message(
                    "column-step can not be zero", name, codename));
        }

        HPX_THROW_EXCEPTION(hpx::not_implemented,
            "phylanx::execution_tree::slice2d_boolean_basic",
            util::generate_error_message(
                "this operation is not supported (yet)", name, codename));

        return {};
    }

    ///////////////////////////////////////////////////////////////////////////
    template <typename T, typename Data, typename F>
    ir::node_data<T> slice2d_integer_integer(Data&& m,
        ir::node_data<std::int64_t> && rows,
        ir::node_data<std::int64_t> && columns, F const& f,
        std::string const& name, std::string const& codename)
    {
        std::size_t numrows = m.rows();
        std::size_t rows_index_size = rows.size();
        for (std::size_t i = 0; i != rows_index_size; ++i)
        {
            rows[i] = detail::check_index(rows[i], numrows, name, codename);
        }

        std::size_t numcols = m.columns();
        std::size_t columns_index_size = columns.size();
        for (std::size_t i = 0; i != columns_index_size; ++i)
        {
            columns[i] =
                detail::check_index(columns[i], numcols, name, codename);
        }

        // broadcast both index arrays, use result to index elements in matrix
        std::size_t largest_dimensionality =
            extract_largest_dimension(name, codename, rows, columns);
        if (largest_dimensionality > 1)
        {
            HPX_THROW_EXCEPTION(hpx::not_implemented,
                "phylanx::execution_tree::slice2d_integer_integer",
                util::generate_error_message(
                    "this operation is not supported (yet)", name, codename));
        }

        if (largest_dimensionality == 0)
        {
            return f.scalar(m, m(rows[0], columns[0]));
        }

        auto sizes = extract_largest_dimensions(name, codename, rows, columns);
        auto row_indices =
            extract_value_vector<T>(std::move(rows), sizes[0], name, codename);
        auto column_indices = extract_value_vector<T>(
            std::move(columns), sizes[0], name, codename);

        typename ir::node_data<T>::storage1d_type result(row_indices.size());
        for (std::size_t i = 0; i != row_indices.size(); ++i)
        {
            result[i] = m(row_indices[i], column_indices[i]);
        }

        return f.vector(m, std::move(result));
    }

    template <typename T, typename Data, typename F>
    ir::node_data<T> slice2d_integer_boolean(Data&& m,
        ir::node_data<std::int64_t> && rows,
        ir::node_data<std::uint8_t> && columns, F const& f,
        std::string const& name, std::string const& codename)
    {
        std::size_t numrows = m.rows();
        std::size_t rows_index_size = rows.size();
        for (std::size_t i = 0; i != rows_index_size; ++i)
        {
            rows[i] = detail::check_index(rows[i], numrows, name, codename);
        }

        HPX_THROW_EXCEPTION(hpx::not_implemented,
            "phylanx::execution_tree::slice2d_integer_boolean",
            util::generate_error_message(
                "this operation is not supported (yet)", name, codename));

        return {};
    }

    template <typename T, typename Data, typename F>
    ir::node_data<T> slice2d_boolean_integer(Data&& m,
        ir::node_data<std::uint8_t> && rows,
        ir::node_data<std::int64_t> && columns, F const& f,
        std::string const& name, std::string const& codename)
    {
        std::size_t numcols = m.columns();
        std::size_t columns_index_size = columns.size();
        for (std::size_t i = 0; i != columns_index_size; ++i)
        {
            columns[i] =
                detail::check_index(columns[i], numcols, name, codename);
        }

        HPX_THROW_EXCEPTION(hpx::not_implemented,
            "phylanx::execution_tree::slice2d_boolean_integer",
            util::generate_error_message(
                "this operation is not supported (yet)", name, codename));

        return {};
    }

    template <typename T, typename Data, typename F>
    ir::node_data<T> slice2d_boolean_boolean(Data&& m,
        ir::node_data<std::uint8_t> && rows,
        ir::node_data<std::uint8_t> && columns, F const& f,
        std::string const& name, std::string const& codename)
    {
        HPX_THROW_EXCEPTION(hpx::not_implemented,
            "phylanx::execution_tree::slice2d_boolean_boolean",
            util::generate_error_message(
                "this operation is not supported (yet)", name, codename));

        return {};
    }

    ///////////////////////////////////////////////////////////////////////////
    template <typename T, typename Data, typename F>
    ir::node_data<T> slice2d_integer_0d(Data&& m,
        ir::node_data<std::int64_t>&& rows, bool explicit_nil, F const& f,
        std::string const& name, std::string const& codename)
    {
        HPX_ASSERT(rows.size() == 1);

        auto row = blaze::row(
            m, detail::check_index(rows[0], m.rows(), name, codename));
        if (!explicit_nil)
        {
            return f.vector(m, std::move(row));
        }

        typename ir::node_data<T>::storage2d_type result(1, m.columns());
        blaze::row(result, 0) = row;
        return f.matrix(m, std::move(result));
    }

    template <typename T, typename Data, typename F>
    ir::node_data<T> slice2d_integer_1d(Data&& m,
        ir::node_data<std::int64_t>&& rows, bool explicit_nil, F const& f,
        std::string const& name, std::string const& codename)
    {
        for (std::size_t i = 0; i != rows.size(); ++i)
        {
            rows[i] = detail::check_index(rows[i], m.rows(), name, codename);
        }

        // general case, pick arbitrary rows from matrix
        auto rows_result = blaze::rows(m, rows.vector().data(), rows.size());
        if (!explicit_nil)
        {
            return f.matrix(m, std::move(rows_result));
        }

#if defined(PHYLANX_HAVE_BLAZE_TENSOR)
        typename ir::node_data<T>::storage3d_type result(
            1, rows.size(), m.columns());
        blaze::pageslice(result, 0) = rows_result;
        return f.tensor(m, std::move(result));
#else
    HPX_THROW_EXCEPTION(hpx::not_implemented,
        "phylanx::execution_tree::slice2d_integer_1d",
        util::generate_error_message(
            "this operation is not supported (yet)", name, codename));
#endif
    }

#if defined(PHYLANX_HAVE_BLAZE_TENSOR)
    template <typename T, typename Data, typename F>
    ir::node_data<T> slice2d_integer_2d(Data&& m,
        ir::node_data<std::int64_t>&& rows, bool explicit_nil, F const& f,
        std::string const& name, std::string const& codename)
    {
        if (!explicit_nil && rows.size() == 1)
        {
            typename ir::node_data<T>::storage3d_type result(1, 1, m.columns());
            blaze::row(blaze::pageslice(result, 0), 0) = blaze::row(
                m, detail::check_index(rows[0], m.rows(), name, codename));
            return f.tensor(m, std::move(result));
        }

        HPX_THROW_EXCEPTION(hpx::not_implemented,
            "phylanx::execution_tree::slice2d_integer_2d",
            util::generate_error_message(
                "this operation is not supported (yet)", name, codename));
    }
#endif

    template <typename T, typename Data, typename F>
    ir::node_data<T> slice2d_integer(Data&& m,
        ir::node_data<std::int64_t>&& rows, bool explicit_nil, F const& f,
        std::string const& name, std::string const& codename)
    {
        switch (rows.num_dimensions())
        {
        case 0:
            return slice2d_integer_0d<T>(std::forward<Data>(m),
                std::move(rows), explicit_nil, f, name, codename);

        case 1:
            return slice2d_integer_1d<T>(std::forward<Data>(m),
                std::move(rows), explicit_nil, f, name, codename);

#if defined(PHYLANX_HAVE_BLAZE_TENSOR)
        case 2:
            return slice2d_integer_2d<T>(std::forward<Data>(m),
                std::move(rows), explicit_nil, f, name, codename);
#endif
        default:
            break;
        }

        HPX_THROW_EXCEPTION(hpx::not_implemented,
            "phylanx::execution_tree::slice2d_integer",
            util::generate_error_message(
                "this operation is not supported (yet)", name, codename));
    }

    ///////////////////////////////////////////////////////////////////////////
    template <typename T, typename Data, typename F>
    ir::node_data<T> slice2d_integer_column_0d(Data&& m,
        ir::node_data<std::int64_t>&& columns, F const& f,
        std::string const& name, std::string const& codename)
    {
        HPX_ASSERT(columns.size() == 1);

        auto column = blaze::column(
            m, detail::check_index(columns[0], m.columns(), name, codename));
        return f.vector(m, std::move(column));
    }

    template <typename T, typename Data, typename F>
    ir::node_data<T> slice2d_integer_column_1d(Data&& m,
        ir::node_data<std::int64_t>&& columns, F const& f,
        std::string const& name, std::string const& codename)
    {
        for (std::size_t i = 0; i != columns.size(); ++i)
        {
            columns[i] =
                detail::check_index(columns[i], m.columns(), name, codename);
        }

        // general case, pick arbitrary rows from matrix
        auto columns_result =
            blaze::columns(m, columns.vector().data(), columns.size());
        return f.matrix(m, std::move(columns_result));
    }

#if defined(PHYLANX_HAVE_BLAZE_TENSOR)
    template <typename T, typename Data, typename F>
    ir::node_data<T> slice2d_integer_column_2d(Data&& m,
        ir::node_data<std::int64_t>&& columns, F const& f,
        std::string const& name, std::string const& codename)
    {
        if (columns.size() == 1)
        {
            typename ir::node_data<T>::storage3d_type result(1, m.rows(), 1);
            blaze::column(blaze::pageslice(result, 0), 0) = blaze::column(
                m, detail::check_index(columns[0], m.columns(), name, codename));
            return f.tensor(m, std::move(result));
        }

        HPX_THROW_EXCEPTION(hpx::not_implemented,
            "phylanx::execution_tree::slice2d_integer_column_2d",
            util::generate_error_message(
                "this operation is not supported (yet)", name, codename));
    }
#endif

    template <typename T, typename Data, typename F>
    ir::node_data<T> slice2d_integer_column(Data&& m,
        ir::node_data<std::int64_t>&& columns, F const& f,
        std::string const& name, std::string const& codename)
    {
        switch (columns.num_dimensions())
        {
        case 0:
            return slice2d_integer_column_0d<T>(std::forward<Data>(m),
                std::move(columns), f, name, codename);

        case 1:
            return slice2d_integer_column_1d<T>(std::forward<Data>(m),
                std::move(columns), f, name, codename);

#if defined(PHYLANX_HAVE_BLAZE_TENSOR)
        case 2:
            return slice2d_integer_column_2d<T>(std::forward<Data>(m),
                std::move(columns), f, name, codename);
#endif
        default:
            break;
        }

        HPX_THROW_EXCEPTION(hpx::not_implemented,
            "phylanx::execution_tree::slice2d_integer_column",
            util::generate_error_message(
                "this operation is not supported (yet)", name, codename));
    }

    ///////////////////////////////////////////////////////////////////////////
    template <typename T, typename Data, typename F>
    ir::node_data<T> slice2d_boolean(Data&& m,
        ir::node_data<std::uint8_t> && columns, bool explicit_nil, F const& f,
        std::string const& name, std::string const& codename)
    {
        return slice2d_integer<T>(std::forward<Data>(m),
            util::slicing_helpers::create_list_slice(columns),
            explicit_nil, f, name, codename);
    }

    ///////////////////////////////////////////////////////////////////////////
    template <typename T, typename Data, typename F>
    ir::node_data<T> slice2d(Data&& m,
        ir::slicing_indices const& rows, primitive_argument_type const& columns,
        F const& f, std::string const& name, std::string const& codename)
    {
        if (is_list_operand_strict(columns))
        {
            switch (detail::extract_slicing_index_type(columns, name, codename))
            {
            case detail::slicing_index_basic:
                {
                    std::size_t num_cols = m.columns();
                    return slice2d_basic_basic<T>(std::forward<Data>(m), rows,
                        util::slicing_helpers::extract_slicing(
                            columns, num_cols, name, codename),
                        f, name, codename);
                }

            case detail::slicing_index_advanced_integer:
                return slice2d_basic_integer<T>(std::forward<Data>(m), rows,
                    extract_integer_value_strict(
                        detail::extract_advanced_integer_index(
                            columns, name, codename),
                        name, codename),
                    f, name, codename);

            case detail::slicing_index_advanced_boolean:
                return slice2d_basic_boolean<T>(std::forward<Data>(m), rows,
                    extract_boolean_value_strict(
                        detail::extract_advanced_boolean_index(
                            columns, name, codename),
                        name, codename),
                    f, name, codename);
            }
        }
        else if (is_boolean_operand_strict(columns))
        {
            return slice2d_basic_boolean<T>(std::forward<Data>(m),
                rows, extract_boolean_value_strict(columns, name, codename), f,
                name, codename);
        }
        else if (is_integer_operand(columns))
        {
            return slice2d_basic_integer<T>(std::forward<Data>(m),
                rows, extract_integer_value(columns, name, codename), f,
                name, codename);
        }
        else if (!valid(columns))
        {
            std::size_t num_cols = m.columns();
            return slice2d_basic_basic<T>(std::forward<Data>(m), rows,
                util::slicing_helpers::extract_slicing(
                    columns, num_cols, name, codename),
                f, name, codename);
        }

        HPX_THROW_EXCEPTION(hpx::bad_parameter,
            "phylanx::execution_tree::slice2d",
            util::generate_error_message(
                "unsupported indexing type", name, codename));
    }

    template <typename T, typename Data, typename F>
    ir::node_data<T> slice2d(Data&& m,
        primitive_argument_type const& rows, ir::slicing_indices const& columns,
        F const& f, std::string const& name, std::string const& codename)
    {
        if (is_list_operand_strict(rows))
        {
            switch (detail::extract_slicing_index_type(rows, name, codename))
            {
            case detail::slicing_index_basic:
                {
                    std::size_t num_rows = m.rows();
                    return slice2d_basic_basic<T>(std::forward<Data>(m),
                        util::slicing_helpers::extract_slicing(
                            rows, num_rows, name, codename),
                        columns, f, name, codename);
                }

            case detail::slicing_index_advanced_integer:
                return slice2d_integer_basic<T>(std::forward<Data>(m),
                    extract_integer_value_strict(
                        detail::extract_advanced_integer_index(
                            rows, name, codename),
                        name, codename),
                    columns, f, name, codename);

            case detail::slicing_index_advanced_boolean:
                return slice2d_boolean_basic<T>(std::forward<Data>(m),
                    extract_boolean_value_strict(
                        detail::extract_advanced_boolean_index(
                            rows, name, codename),
                        name, codename),
                    columns, f, name, codename);
            }
        }
        else if (is_boolean_operand_strict(rows))
        {
            return slice2d_boolean_basic<T>(std::forward<Data>(m),
                extract_boolean_value_strict(rows, name, codename),
                columns, f, name, codename);
        }
        else if (is_integer_operand(rows))
        {
            return slice2d_integer_basic<T>(std::forward<Data>(m),
                extract_integer_value(rows, name, codename), columns, f,
                name, codename);
        }
        else if (!valid(rows))
        {
            return slice2d_basic_column<T>(
                std::forward<Data>(m), columns, f, name, codename);
        }

        HPX_THROW_EXCEPTION(hpx::bad_parameter,
            "phylanx::execution_tree::slice2d",
            util::generate_error_message(
                "unsupported indexing type", name, codename));
    }

    ///////////////////////////////////////////////////////////////////////////
    namespace detail
    {
        template <typename T>
        blaze::CustomMatrix<T, true, true>
        create_ref(blaze::DynamicMatrix<T>& m)
        {
            return blaze::CustomMatrix<T, true, true>(
                m.data(), m.rows(), m.columns(), m.spacing());
        }

        template <typename T>
        blaze::CustomMatrix<T, true, true> create_ref(
            blaze::CustomMatrix<T, true, true> const& m)
        {
            return m;
        }
    }

    // This is the main entry point for 2d slicing
    template <typename T, typename Data, typename F>
    ir::node_data<T> slice2d(Data&& m,
        primitive_argument_type const& rows,
        primitive_argument_type const& columns, F const& f,
        std::string const& name, std::string const& codename)
    {
        if (is_list_operand_strict(rows))
        {
            switch (detail::extract_slicing_index_type(rows, name, codename))
            {
            case detail::slicing_index_basic:
                {
                    std::size_t num_rows = m.rows();
                    return slice2d<T>(std::forward<Data>(m),
                        util::slicing_helpers::extract_slicing(
                            rows, num_rows, name, codename),
                        columns, f, name, codename);
                }

            case detail::slicing_index_advanced_integer:
                return slice2d<T>(std::forward<Data>(m),
                    extract_integer_value_strict(
                        detail::extract_advanced_integer_index(
                            rows, name, codename),
                        name, codename),
                    columns, f, name, codename);

            case detail::slicing_index_advanced_boolean:
                return slice2d<T>(std::forward<Data>(m),
                    extract_boolean_value_strict(
                        detail::extract_advanced_boolean_index(
                            rows, name, codename),
                        name, codename),
                    columns, f, name, codename);
            }
        }
        else if (is_list_operand_strict(columns))
        {
            switch (detail::extract_slicing_index_type(columns, name, codename))
            {
            case detail::slicing_index_basic:
                {
                    std::size_t num_cols = m.columns();
                    return slice2d<T>(std::forward<Data>(m), rows,
                        util::slicing_helpers::extract_slicing(
                            columns, num_cols, name, codename),
                        f, name, codename);
                }

            case detail::slicing_index_advanced_integer:
                return slice2d<T>(std::forward<Data>(m), rows,
                    extract_integer_value_strict(
                        detail::extract_advanced_integer_index(
                            columns, name, codename),
                        name, codename),
                    f, name, codename);

            case detail::slicing_index_advanced_boolean:
                return slice2d<T>(std::forward<Data>(m), rows,
                    extract_boolean_value_strict(
                        detail::extract_advanced_boolean_index(
                            columns, name, codename),
                        name, codename),
                    f, name, codename);
            }
        }
        else if (is_boolean_operand_strict(rows))
        {
            if (is_boolean_operand_strict(columns))
            {
                return slice2d<T>(std::forward<Data>(m),
                    extract_boolean_value_strict(rows, name, codename),
                    extract_boolean_value_strict(columns, name, codename),
                    f, name, codename);
            }
            if (is_integer_operand(columns))
            {
                return slice2d<T>(std::forward<Data>(m),
                    extract_boolean_value_strict(rows, name, codename),
                    extract_integer_value(columns, name, codename),
                    f, name, codename);
            }
            if (!valid(columns))
            {
                HPX_THROW_EXCEPTION(hpx::not_implemented,
                    "phylanx::execution_tree::slice2d",
                    util::generate_error_message(
                        "this operation is not supported (yet)",
                        name, codename));
            }
        }
        else if (is_integer_operand(rows))
        {
            if (is_boolean_operand_strict(columns))
            {
                return slice2d_integer_boolean<T>(
                    std::forward<Data>(m),
                    extract_integer_value(rows, name, codename),
                    extract_boolean_value_strict(columns, name, codename), f,
                    name, codename);
            }
            if (is_integer_operand(columns))
            {
                return slice2d_integer_integer<T>(
                    std::forward<Data>(m),
                    extract_integer_value(rows, name, codename),
                    extract_integer_value(columns, name, codename), f,
                    name, codename);
            }
            if (!valid(columns))
            {
                return slice2d_integer<T>(std::forward<Data>(m),
                    extract_integer_value(rows, name, codename),
                    is_explicit_nil(columns), f, name, codename);
            }
        }
        else if (!valid(rows))
        {
            if (is_boolean_operand_strict(columns))
            {
                // an explicit 'nil' is equivalent to np.newaxis
                return slice2d_boolean<T>(std::forward<Data>(m),
                    extract_boolean_value_strict(columns, name, codename),
                    is_explicit_nil(rows), f, name, codename);
            }
            if (is_integer_operand(columns))
            {
                if (is_explicit_nil(rows))
                {
                    return slice2d_integer<T>(std::forward<Data>(m),
                        extract_integer_value(columns, name, codename),
                        true, f, name, codename);
                }

                // special handling for column_slice primtive
                return slice2d_integer_column<T>(std::forward<Data>(m),
                    extract_integer_value(columns, name, codename),
                    f, name, codename);
            }
            if (!valid(columns))
            {
                // just return the full matrix
                return f.matrix(m, detail::create_ref(m));
            }
        }

        HPX_THROW_EXCEPTION(hpx::bad_parameter,
            "phylanx::execution_tree::slice2d",
            util::generate_error_message(
                "unsupported indexing type", name, codename));
    }

    ///////////////////////////////////////////////////////////////////////////
    template <typename T>
    ir::node_data<T> slice1d_extract2d(ir::node_data<T> const& data,
        execution_tree::primitive_argument_type const& indices,
        std::string const& name, std::string const& codename)
    {
        return slice2d<T>(data.matrix(), indices, primitive_argument_type{},
            detail::slice_identity<T>{}, name, codename);
    }

    ///////////////////////////////////////////////////////////////////////////
    template <typename T>
    ir::node_data<T> slice2d_extract2d(ir::node_data<T> const& data,
        execution_tree::primitive_argument_type const& rows,
        execution_tree::primitive_argument_type const& columns,
        std::string const& name, std::string const& codename)
    {
        return slice2d<T>(data.matrix(), rows, columns,
            detail::slice_identity<T>{}, name, codename);
    }

    ///////////////////////////////////////////////////////////////////////////
    // Slice 2d data with two indices
    template <typename T>
    ir::node_data<T> slice2d_assign2d(ir::node_data<T>&& data,
        execution_tree::primitive_argument_type const& rows,
        execution_tree::primitive_argument_type const& columns,
        ir::node_data<T>&& value, std::string const& name,
        std::string const& codename)
    {
        switch (value.num_dimensions())
        {
        case 0: HPX_FALLTHROUGH;
        case 1: HPX_FALLTHROUGH;
        case 2:
            {
                auto m = data.matrix();
                std::size_t numrows = m.rows();
                std::size_t numcols = m.columns();

                std::size_t rows_size =
                    detail::slicing_size(rows, numrows, name, codename);
                std::size_t columns_size =
                    detail::slicing_size(columns, numcols, name, codename);

                typename ir::node_data<T>::storage2d_type result;
                extract_value_matrix(result, std::move(value), rows_size,
                    columns_size, name, codename);

                ir::node_data<T> rhs(std::move(result));
                if (data.is_ref())
                {
                    return slice2d<T>(std::move(m), rows, columns,
                        detail::slice_assign_matrix<T>{rhs}, name, codename);
                }

                return slice2d<T>(data.matrix_non_ref(), rows, columns,
                    detail::slice_assign_matrix<T>{rhs}, name, codename);
            }
            break;

        default:
            break;
        }

        HPX_THROW_EXCEPTION(hpx::invalid_status,
            "phylanx::execution_tree::slice2d_assign2d",
            util::generate_error_message(
                "source ir::node_data object holds unsupported data type", name,
                codename));
    }

    ///////////////////////////////////////////////////////////////////////////
    // Slice 2d data with one (row) index, extracts full columns
    template <typename T>
    ir::node_data<T> slice1d_assign2d(ir::node_data<T>&& data,
        execution_tree::primitive_argument_type const& indices,
        ir::node_data<T>&& value, std::string const& name,
        std::string const& codename)
    {
        switch (value.num_dimensions())
        {
        case 0: HPX_FALLTHROUGH;
        case 1: HPX_FALLTHROUGH;
        case 2:
            {
                auto m = data.matrix();
                std::size_t columns = m.columns();

                std::size_t result_size =
                    detail::slicing_size(indices, m.rows(), name, codename);

                typename ir::node_data<T>::storage2d_type result;
                extract_value_matrix(result, std::move(value),
                    result_size, columns, name, codename);

                ir::node_data<T> rhs(std::move(result));
                if (data.is_ref())
                {
                    return slice2d<T>(std::move(m), indices,
                        ir::slicing_indices{0ll, std::int64_t(columns), 1ll},
                        detail::slice_assign_matrix<T>{rhs}, name, codename);
                }

                return slice2d<T>(data.matrix_non_ref(), indices,
                    ir::slicing_indices{0ll, std::int64_t(columns), 1ll},
                    detail::slice_assign_matrix<T>{rhs}, name, codename);
            }

        default:
            break;
        }

        HPX_THROW_EXCEPTION(hpx::invalid_status,
            "phylanx::execution_tree::slice1d_assign2d",
            util::generate_error_message(
                "source ir::node_data object holds unsupported data type",
                name, codename));
    }
}}

#endif
