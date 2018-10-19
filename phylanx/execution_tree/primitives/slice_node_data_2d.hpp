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

#include <cstddef>
#include <cstdint>
#include <string>
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

            auto sm = blaze::submatrix(m, row_start, 0ll,
                row_stop - row_start, m.columns());

            static_assert(sizeof(std::int64_t) == sizeof(std::size_t),
                "sizeof(std::int64_t) == sizeof(std::size_t)");
            auto result = blaze::columns(sm,
                reinterpret_cast<std::size_t*>(columns.vector().data()),
                columns.size());

            return f.matrix(m, std::move(result));
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

        return {};
    }

    ///////////////////////////////////////////////////////////////////////////
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

            static_assert(sizeof(std::int64_t) == sizeof(std::size_t),
                "sizeof(std::int64_t) == sizeof(std::size_t)");
            auto result = blaze::rows(sm,
                reinterpret_cast<std::size_t*>(rows.vector().data()),
                rows.size());

            return f.matrix(m, std::move(result));
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
    ir::node_data<T> slice2d_integer_column(Data&& m,
        ir::node_data<std::int64_t> && columns, F const& f,
        std::string const& name, std::string const& codename)
    {
        std::size_t numcols = m.columns();
        std::size_t index_size = columns.size();

        if (index_size == 1)
        {
            // handle single column case
            auto col = blaze::column(
                m, detail::check_index(columns[0], numcols, name, codename));
            return f.vector(m, std::move(col));
        }

        for (std::size_t i = 0; i != index_size; ++i)
        {
            columns[i] =
                detail::check_index(columns[i], numcols, name, codename);
        }

        // general case, pick arbitrary columns from matrix
        auto result = blaze::columns(m, columns.vector().data(), columns.size());
        return f.matrix(m, std::move(result));
    }

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
            rows[i] =
                detail::check_index(rows[i], numrows, name, codename);
        }

        std::size_t numcols = m.columns();
        std::size_t columns_index_size = columns.size();
        for (std::size_t i = 0; i != columns_index_size; ++i)
        {
            columns[i] =
                detail::check_index(columns[i], numcols, name, codename);
        }

        // return a value and not a vector if you are not given a list
        if (rows.size() == 1)
        {
            auto row = blaze::row(m, rows[0]);

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

            // general case, pick arbitrary elements from selected column
            auto sv = blaze::elements(col, rows.vector().data(), rows.size());

            return f.vector(m, std::move(sv));
        }

        // general case, pick arbitrary elements from matrix
        auto sm = blaze::rows(m, rows.vector().data(), rows.size());
        auto result =
            blaze::columns(sm, columns.vector().data(), columns.size());

        return f.matrix(m, std::move(result));
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
            rows[i] =
                detail::check_index(rows[i], numrows, name, codename);
        }

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

        return {};
    }

    template <typename T, typename Data, typename F>
    ir::node_data<T> slice2d_boolean_boolean(Data&& m,
        ir::node_data<std::uint8_t> && rows,
        ir::node_data<std::uint8_t> && columns, F const& f,
        std::string const& name, std::string const& codename)
    {
        return {};
    }

    ///////////////////////////////////////////////////////////////////////////
    template <typename T, typename Data, typename F>
    ir::node_data<T> slice2d(Data&& m,
        ir::slicing_indices const& rows, primitive_argument_type const& columns,
        F const& f, std::string const& name, std::string const& codename)
    {
        if (is_list_operand_strict(columns))
        {
            switch (detail::is_advanced_slicing_index(columns))
            {
            case detail::slicing_index_basic:
                return slice2d_basic_basic<T>(std::forward<Data>(m),
                    rows,
                    util::slicing_helpers::extract_slicing(
                        columns, m.columns(), name, codename),
                    f, name, codename);

            case detail::slicing_index_advanced_integer:
                return slice2d_basic_integer<T>(
                    std::forward<Data>(m), rows,
                    extract_integer_value_strict(
                        detail::extract_advanced_integer_index(
                            columns, name, codename),
                        name, codename),
                    f, name, codename);

            case detail::slicing_index_advanced_boolean:
                return slice2d_basic_boolean<T>(
                    std::forward<Data>(m), rows,
                    extract_boolean_value_strict(
                        detail::extract_advanced_boolean_index(
                            columns, name, codename),
                        name, codename),
                    f, name, codename);
            }
        }
        else if (is_integer_operand_strict(columns))
        {
            return slice2d_basic_integer<T>(std::forward<Data>(m),
                rows, extract_integer_value_strict(columns, name, codename), f,
                name, codename);
        }
        else if (is_boolean_operand_strict(columns))
        {
            return slice2d_basic_boolean<T>(std::forward<Data>(m),
                rows, extract_boolean_value_strict(columns, name, codename), f,
                name, codename);
        }
        else if (!valid(columns))
        {
//             return slice1d<T>(blaze::elements(input_matrix, )
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
            switch (detail::is_advanced_slicing_index(rows))
            {
            case detail::slicing_index_basic:
                return slice2d_basic_basic<T>(std::forward<Data>(m),
                    util::slicing_helpers::extract_slicing(
                        rows, m.rows(), name, codename),
                    columns, f, name, codename);

            case detail::slicing_index_advanced_integer:
                return slice2d_integer_basic<T>(
                    std::forward<Data>(m),
                    extract_integer_value_strict(
                        detail::extract_advanced_integer_index(
                            rows, name, codename),
                        name, codename),
                    columns, f, name, codename);

            case detail::slicing_index_advanced_boolean:
                return slice2d_boolean_basic<T>(
                    std::forward<Data>(m),
                    extract_boolean_value_strict(
                        detail::extract_advanced_boolean_index(
                            rows, name, codename),
                        name, codename),
                    columns, f, name, codename);
            }
        }
        else if (is_integer_operand_strict(rows))
        {
            return slice2d_integer_basic<T>(std::forward<Data>(m),
                extract_integer_value_strict(rows, name, codename), columns, f,
                name, codename);
        }
        else if (is_boolean_operand_strict(rows))
        {
            return slice2d_boolean_basic<T>(std::forward<Data>(m),
                extract_boolean_value_strict(rows, name, codename),
                columns, f, name, codename);
        }
        else if (!valid(rows))
        {
        }

        HPX_THROW_EXCEPTION(hpx::bad_parameter,
            "phylanx::execution_tree::slice2d",
            util::generate_error_message(
                "unsupported indexing type", name, codename));
    }

    ///////////////////////////////////////////////////////////////////////////
    // This is the main entry point for 2d slicing
    template <typename T, typename Data, typename F>
    ir::node_data<T> slice2d(Data&& m,
        primitive_argument_type const& rows,
        primitive_argument_type const& columns, F const& f,
        std::string const& name, std::string const& codename)
    {
        if (is_list_operand_strict(rows))
        {
            switch (detail::is_advanced_slicing_index(rows))
            {
            case detail::slicing_index_basic:
                return slice2d<T>(std::forward<Data>(m),
                    util::slicing_helpers::extract_slicing(
                        rows, m.rows(), name, codename),
                    columns, f, name, codename);

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
            switch (detail::is_advanced_slicing_index(columns))
            {
            case detail::slicing_index_basic:
                return slice2d<T>(std::forward<Data>(m), rows,
                    util::slicing_helpers::extract_slicing(
                        columns, m.rows(), name, codename),
                    f, name, codename);

            case detail::slicing_index_advanced_integer:
                return slice2d<T>(std::forward<Data>(m),
                    rows,
                    extract_integer_value_strict(
                        detail::extract_advanced_integer_index(
                            columns, name, codename),
                        name, codename),
                    f, name, codename);

            case detail::slicing_index_advanced_boolean:
                return slice2d<T>(std::forward<Data>(m),
                    rows,
                    extract_boolean_value_strict(
                        detail::extract_advanced_boolean_index(
                            columns, name, codename),
                        name, codename),
                    f, name, codename);
            }
        }
        else if (is_integer_operand_strict(rows))
        {
            if (is_integer_operand_strict(columns))
            {
                return slice2d_integer_integer<T>(
                    std::forward<Data>(m),
                    extract_integer_value_strict(rows, name, codename),
                    extract_integer_value_strict(columns, name, codename), f,
                    name, codename);
            }
            if (is_boolean_operand_strict(columns))
            {
                return slice2d_integer_boolean<T>(
                    std::forward<Data>(m),
                    extract_integer_value_strict(rows, name, codename),
                    extract_boolean_value_strict(columns, name, codename), f,
                    name, codename);
            }
            if (!valid(columns))
            {
//                 return slice1d<T>(
//                     input_matrix.matrix(), rows, f, name, codename);
            }
        }
        else if (is_boolean_operand_strict(rows))
        {
            if (is_integer_operand_strict(columns))
            {
                return slice2d<T>(std::forward<Data>(m),
                    extract_boolean_value_strict(rows, name, codename),
                    extract_integer_value_strict(columns, name, codename),
                    f, name, codename);
            }
            if (is_boolean_operand_strict(columns))
            {
                return slice2d<T>(std::forward<Data>(m),
                    extract_boolean_value_strict(rows, name, codename),
                    extract_boolean_value_strict(columns, name, codename),
                    f, name, codename);
            }
            if (!valid(columns))
            {
            }
        }
        else if (!valid(rows))
        {
            if (is_integer_operand_strict(columns))
            {
                return slice2d_integer_column<T>(std::forward<Data>(m),
                    extract_integer_value_strict(columns, name, codename),
                    f, name, codename);
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
        auto m = data.matrix();
        ir::slicing_indices columns{0ll, std::int64_t(m.columns()), 1ll};
        return slice2d_basic_basic<T>(m,
            util::slicing_helpers::extract_slicing(
                indices, m.rows(), name, codename),
            columns, detail::slice_identity<T>{}, name, codename);
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
