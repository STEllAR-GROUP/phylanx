// Copyright (c) 2018 Hartmut Kaiser
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>
#include <phylanx/execution_tree/primitives/base_primitive.hpp>
#include <phylanx/execution_tree/primitives/slice_node_data.hpp>
#include <phylanx/ir/node_data.hpp>
#include <phylanx/ir/ranges.hpp>
#include <phylanx/util/slicing_helpers.hpp>

#include <hpx/exception.hpp>
#include <hpx/util/assert.hpp>

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

#include <blaze/Math.h>
#include <blaze/math/Elements.h>

namespace phylanx { namespace execution_tree
{
    ///////////////////////////////////////////////////////////////////////////
    // Extracting slice functionality
    template <typename T, typename F>
    ir::node_data<T> slice0d(T data, ir::slicing_indices const& indices,
        F const& f, std::string const& name, std::string const& codename)
    {
        if (indices.start() != 0 || indices.span() != 1 || indices.step() != 1)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "phylanx::execution_tree::slicing0d",
                util::generate_error_message(
                    "cannot extract anything but the first element from a "
                    "scalar",
                    name, codename));
        }
        return f.scalar(data, data);
    }

    ///////////////////////////////////////////////////////////////////////////
    // return a slice from a 1d ir::node_data
    template <typename T, typename Vector, typename F>
    ir::node_data<T> slice1d(
        Vector&& data, ir::slicing_indices const& indices, F const& f,
        std::string const& name, std::string const& codename)
    {
        std::size_t size = data.size();
        if (indices.start() >= size || indices.span() > size ||
            indices.stop() > size)
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
            return f.scalar(data, data[start]);
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

        auto sv = blaze::elements(
            data, util::slicing_helpers::create_list_slice(start, stop, step));

        return f.vector(data, std::move(sv));
    }

    ///////////////////////////////////////////////////////////////////////////
    // return a slice from a 2d ir::node_data
    template <typename T, typename Matrix, typename F>
    ir::node_data<T> slice2d(Matrix&& input_matrix,
        ir::slicing_indices const& rows, ir::slicing_indices const& columns,
        F const& f, std::string const& name, std::string const& codename)
    {
        std::size_t numrows = input_matrix.rows();
        if (rows.start() >= numrows || rows.span() > numrows ||
            rows.stop() > numrows)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "phylanx::execution_tree::slicing2d",
                util::generate_error_message(
                    "cannot extract the requested matrix elements",
                    name, codename));
        }
        std::size_t numcols = input_matrix.columns();
        if (columns.start() >= numcols || columns.span() > numcols ||
            columns.stop() > numcols)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "phylanx::execution_tree::slicing1d",
                util::generate_error_message(
                    "cannot extract the requested matrix elements",
                    name, codename));
        }

        std::int64_t row_start = rows.start();
        std::int64_t row_stop = rows.stop();
        std::int64_t row_step = rows.step();

        std::int64_t col_start = columns.start();
        std::int64_t col_stop = columns.stop();
        std::int64_t col_step = columns.step();

        if (row_step == 0 && !rows.single_value())
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "phylanx::execution_tree::slicing2d",
                util::generate_error_message(
                    "row-step can not be zero", name, codename));
        }

        if (col_step == 0 && !columns.single_value())
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "phylanx::execution_tree::slicing2d",
                util::generate_error_message(
                    "column-step can not be zero", name, codename));
        }

        std::size_t num_matrix_rows = input_matrix.rows();
        std::size_t num_matrix_cols = input_matrix.columns();

        // return a value and not a vector if you are not given a list
        if (rows.single_value())
        {
            auto row = blaze::row(input_matrix, row_start);

            // handle single value slicing result
            if (columns.single_value())
            {
                return f.scalar(input_matrix, row[col_start]);
            }

            // extract a consecutive sub-vector (sub-row)
            if (col_step == 1)
            {
                HPX_ASSERT(col_stop > col_start);
                auto sv =
                    blaze::subvector(row, col_start, col_stop - col_start);
                return f.trans_vector(input_matrix, std::move(sv));
            }

            // general case, pick arbitrary elements from selected row
            auto sv = blaze::elements(row,
                util::slicing_helpers::create_list_slice(
                    col_start, col_stop, col_step));

            return f.trans_vector(input_matrix, std::move(sv));
        }
        else if (columns.single_value())
        {
            // handle single column case
            auto col = blaze::column(input_matrix, col_start);

            // extract a consecutive sub-vector (sub-column)
            if (row_step == 1)
            {
                HPX_ASSERT(row_stop > row_start);
                auto sv =
                    blaze::subvector(col, row_start, row_stop - row_start);
                return f.vector(input_matrix, std::move(sv));
            }

            // general case, pick arbitrary elements from selected column
            auto sv = blaze::elements(col,
                util::slicing_helpers::create_list_slice(
                    row_start, row_stop, row_step));

            return f.vector(input_matrix, std::move(sv));
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
                return f.matrix(input_matrix, std::move(result));
            }

            auto sm = blaze::submatrix(input_matrix, 0ll, col_start,
                num_matrix_rows, col_stop - col_start);

            auto result = blaze::rows(sm,
                util::slicing_helpers::create_list_slice(
                    row_start, row_stop, row_step));

            return f.matrix(input_matrix, std::move(result));
        }
        else if (row_step == 1)
        {
            HPX_ASSERT(row_stop > row_start);

            auto sm = blaze::submatrix(input_matrix, row_start, 0ll,
                row_stop - row_start, num_matrix_cols);

            auto result = blaze::columns(sm,
                util::slicing_helpers::create_list_slice(
                    col_start, col_stop, col_step));

            return f.matrix(input_matrix, std::move(result));
        }

        // general case, pick arbitrary elements from matrix
        auto sm = blaze::rows(input_matrix,
            util::slicing_helpers::create_list_slice(
                row_start, row_stop, row_step));

        auto result = blaze::columns(sm,
            util::slicing_helpers::create_list_slice(
                col_start, col_stop, col_step));

        return f.matrix(input_matrix, std::move(result));
    }

    ///////////////////////////////////////////////////////////////////////////
    // return a slice of the given ir::node_data instance
    namespace detail
    {
        template <typename T>
        struct slice_identity
        {
            template <typename Data, typename Scalar>
            ir::node_data<T> scalar(Data const&, Scalar const& value) const
            {
                return ir::node_data<T>{value};
            }

            template <typename Data, typename View>
            ir::node_data<T> vector(Data const&, View&& view) const
            {
                return ir::node_data<T>{
                    blaze::DynamicVector<T>{std::forward<View>(view)}};
            }

            template <typename Data, typename View>
            ir::node_data<T> trans_vector(Data const&, View&& view) const
            {
                return ir::node_data<T>{blaze::DynamicVector<T>{
                    blaze::trans(std::forward<View>(view))}};
            }

            template <typename Data, typename View>
            ir::node_data<T> matrix(Data const&, View&& view) const
            {
                return ir::node_data<T>{
                    blaze::DynamicMatrix<T>{std::forward<View>(view)}};
            }
        };
    }

    template <typename T>
    ir::node_data<T> slice(ir::node_data<T> const& data,
        execution_tree::primitive_argument_type const& indices,
        std::string const& name, std::string const& codename)
    {
        switch (data.index())
        {
        case 0:
            return slice0d(data.scalar(),
                util::slicing_helpers::extract_slicing(indices, 1),
                detail::slice_identity<T>{}, name, codename);

        case 1: HPX_FALLTHROUGH;
        case 3:
            {
                auto v = data.vector();
                return slice1d<T>(v,
                    util::slicing_helpers::extract_slicing(indices, v.size()),
                    detail::slice_identity<T>{}, name, codename);
            }

        case 2: HPX_FALLTHROUGH;
        case 4:
            {
                auto m = data.matrix();
                ir::slicing_indices columns{0ll, std::int64_t(m.columns()), 1ll};
                return slice2d<T>(m,
                    util::slicing_helpers::extract_slicing(indices, m.rows()),
                    columns, detail::slice_identity<T>{}, name, codename);
            }

        default:
            break;
        }

        HPX_THROW_EXCEPTION(hpx::invalid_status,
            "phylanx::execution_tree::slice",
            util::generate_error_message(
                "target ir::node_data object holds unsupported data type", name,
                codename));
    }

    template <typename T>
    ir::node_data<T> slice(ir::node_data<T> const& data,
        execution_tree::primitive_argument_type const& rows,
        execution_tree::primitive_argument_type const& columns,
        std::string const& name, std::string const& codename)
    {
        switch (data.index())
        {
        case 2: HPX_FALLTHROUGH;
        case 4:
            {
                auto m = data.matrix();
                return slice2d<T>(m,
                    util::slicing_helpers::extract_slicing(rows, m.rows()),
                    util::slicing_helpers::extract_slicing(columns, m.columns()),
                    detail::slice_identity<T>{}, name, codename);
            }

        case 1: HPX_FALLTHROUGH;
        case 3:
            {
                auto v = data.vector();
                if (valid(rows))
                {
                    HPX_ASSERT(!valid(columns));
                    return slice1d<T>(v,
                        util::slicing_helpers::extract_slicing(rows, v.size()),
                        detail::slice_identity<T>{}, name, codename);
                }

                if (valid(columns))
                {
                    HPX_ASSERT(!valid(rows));
                    return slice1d<T>(v,
                        util::slicing_helpers::extract_slicing(columns, v.size()),
                        detail::slice_identity<T>{}, name, codename);
                }
            }
            break;

        case 0: HPX_FALLTHROUGH;
        default:
            break;
        }

        HPX_THROW_EXCEPTION(hpx::invalid_status,
            "phylanx::execution_tree::slice",
            util::generate_error_message(
                "target ir::node_data object holds data type that does not support "
                "2d slicing",
                name, codename));
    }

    ///////////////////////////////////////////////////////////////////////////
    // explicit instantiations of the slice (extract) functionality
    template PHYLANX_EXPORT ir::node_data<std::uint8_t> slice<std::uint8_t>(
        ir::node_data<std::uint8_t> const& data,
        execution_tree::primitive_argument_type const& indices,
        std::string const& name, std::string const& codename);

    template PHYLANX_EXPORT ir::node_data<double> slice<double>(
        ir::node_data<double> const& data,
        execution_tree::primitive_argument_type const& indices,
        std::string const& name, std::string const& codename);

    template PHYLANX_EXPORT ir::node_data<std::int64_t> slice<std::int64_t>(
        ir::node_data<std::int64_t> const& data,
        execution_tree::primitive_argument_type const& indices,
        std::string const& name, std::string const& codename);

    template PHYLANX_EXPORT ir::node_data<std::uint8_t> slice<std::uint8_t>(
        ir::node_data<std::uint8_t> const& data,
        execution_tree::primitive_argument_type const& rows,
        execution_tree::primitive_argument_type const& columns,
        std::string const& name, std::string const& codename);

    template PHYLANX_EXPORT ir::node_data<double> slice<double>(
        ir::node_data<double> const& data,
        execution_tree::primitive_argument_type const& rows,
        execution_tree::primitive_argument_type const& columns,
        std::string const& name, std::string const& codename);

    template PHYLANX_EXPORT ir::node_data<std::int64_t> slice<std::int64_t>(
        ir::node_data<std::int64_t> const& data,
        execution_tree::primitive_argument_type const& rows,
        execution_tree::primitive_argument_type const& columns,
        std::string const& name, std::string const& codename);

    ///////////////////////////////////////////////////////////////////////////
    // Modifying slice functionality
    namespace detail
    {
        template <typename Lhs, typename Rhs>
        void check_vector_sizes(Lhs const& lhs, Rhs const& rhs)
        {
            if (lhs.size() != rhs.size())
            {
                std::ostringstream msg;
                msg << "size mismatch during vector assignment, sizes: "
                    << lhs.size() << ", " << rhs.size();

                HPX_THROW_EXCEPTION(hpx::bad_parameter,
                    "phylanx::execution_tree::detail::check_vector_sizes",
                    msg.str());
            }
        }

        template <typename Lhs, typename Rhs>
        void check_matrix_sizes(Lhs const& lhs, Rhs const& rhs)
        {
            if (lhs.rows() != rhs.rows() || lhs.columns() != rhs.columns())
            {
                std::ostringstream msg;
                msg << "size mismatch during matrix assignment, "
                    << "rows: " << lhs.rows() << ", " << rhs.rows()
                    << "columns: " << lhs.columns() << ", " << rhs.columns();

                HPX_THROW_EXCEPTION(hpx::bad_parameter,
                    "phylanx::execution_tree::detail::check_matrix_sizes",
                    msg.str());
            }
        }

        template <typename T>
        struct slice_assign
        {
            ir::node_data<T>& rhs_;

            template <typename Data, typename Scalar>
            ir::node_data<T> scalar(Data& data, Scalar& value) const
            {
                value = rhs_.scalar();
                return ir::node_data<T>{std::move(data)};
            }

            template <typename Data, typename View>
            ir::node_data<T> vector(Data& data, View&& view) const
            {
                std::size_t size = view.size();
                auto v = rhs_.vector();

                check_vector_sizes(view, v);

                for (std::size_t i = 0; i != size; ++i)
                {
                    view[i] = v[i];
                }
                return ir::node_data<T>{std::move(data)};
            }

            template <typename Data, typename View>
            ir::node_data<T> trans_vector(Data& data, View&& view) const
            {
                std::size_t size = view.size();
                auto v = rhs_.vector();

                check_vector_sizes(view, v);

                auto tv = blaze::trans(v);
                for (std::size_t i = 0; i != size; ++i)
                {
                    view[i] = tv[i];
                }
                return ir::node_data<T>{std::move(data)};
            }

            template <typename Data, typename View>
            ir::node_data<T> matrix(Data& data, View&& view) const
            {
                auto m = rhs_.matrix();

                check_matrix_sizes(view, m);

                view = m;
                return ir::node_data<T>{std::move(data)};
            }
        };
    }

    template <typename T>
    ir::node_data<T> slice(ir::node_data<T>&& data,
        execution_tree::primitive_argument_type const& indices,
        ir::node_data<T>&& value, std::string const& name,
        std::string const& codename)
    {
        switch (data.index())
        {
        case 0:
            return slice0d(data.scalar(),
                util::slicing_helpers::extract_slicing(indices, 1),
                detail::slice_assign<T>{value}, name, codename);

        case 1: HPX_FALLTHROUGH;
        case 3:
            {
                if (data.is_ref())
                {
                    auto v = data.vector();
                    std::size_t size = v.size();
                    return slice1d<T>(std::move(v),
                        util::slicing_helpers::extract_slicing(indices, size),
                        detail::slice_assign<T>{value}, name, codename);
                }

                auto& v = data.vector_non_ref();
                std::size_t size = v.size();
                return slice1d<T>(std::move(v),
                    util::slicing_helpers::extract_slicing(indices, size),
                    detail::slice_assign<T>{value}, name, codename);
            }

        case 2: HPX_FALLTHROUGH;
        case 4:
            {
                if (data.is_ref())
                {
                    auto m = data.matrix();
                    std::size_t rows = m.rows();
                    ir::slicing_indices columns{
                        0ll, std::int64_t(m.columns()), 1ll};
                    return slice2d<T>(std::move(m),
                        util::slicing_helpers::extract_slicing(indices, rows),
                        columns, detail::slice_assign<T>{value}, name, codename);
                }

                auto& m = data.matrix_non_ref();
                std::size_t rows = m.rows();
                ir::slicing_indices columns{
                    0ll, std::int64_t(m.columns()), 1ll};
                return slice2d<T>(std::move(m),
                    util::slicing_helpers::extract_slicing(indices, rows),
                    columns, detail::slice_assign<T>{value}, name, codename);
            }

        default:
            break;
        }

        HPX_THROW_EXCEPTION(hpx::invalid_status,
            "phylanx::execution_tree::slice",
            util::generate_error_message(
                "target ir::node_data object holds unsupported data type", name,
                codename));
    }

    template <typename T>
    ir::node_data<T> slice(ir::node_data<T>&& data,
        execution_tree::primitive_argument_type const& rows,
        execution_tree::primitive_argument_type const& columns,
        ir::node_data<T>&& value, std::string const& name,
        std::string const& codename)
    {
        switch (data.index())
        {
        case 2: HPX_FALLTHROUGH;
        case 4:
            {
                if (data.is_ref())
                {
                    auto m = data.matrix();
                    std::size_t numrows = m.rows();
                    std::size_t numcols = m.columns();
                    return slice2d<T>(std::move(m),
                        util::slicing_helpers::extract_slicing(rows, numrows),
                        util::slicing_helpers::extract_slicing(columns, numcols),
                        detail::slice_assign<T>{value}, name, codename);
                }

                auto& m = data.matrix_non_ref();
                std::size_t numrows = m.rows();
                std::size_t numcols = m.columns();
                return slice2d<T>(std::move(m),
                    util::slicing_helpers::extract_slicing(rows, numrows),
                    util::slicing_helpers::extract_slicing(columns, numcols),
                    detail::slice_assign<T>{value}, name, codename);
            }

        case 1: HPX_FALLTHROUGH;
        case 3:
            {
                if (data.is_ref())
                {
                    auto v = data.vector();
                    std::size_t size = v.size();
                    if (valid(rows))
                    {
                        HPX_ASSERT(!valid(columns));
                        return slice1d<T>(std::move(v),
                            util::slicing_helpers::extract_slicing(rows, size),
                            detail::slice_assign<T>{value}, name, codename);
                    }

                    if (valid(columns))
                    {
                        HPX_ASSERT(!valid(rows));
                        return slice1d<T>(std::move(v),
                            util::slicing_helpers::extract_slicing(columns, size),
                            detail::slice_assign<T>{value}, name, codename);
                    }
                }
                else
                {
                    auto& v = data.vector_non_ref();
                    std::size_t size = v.size();
                    if (valid(rows))
                    {
                        HPX_ASSERT(!valid(columns));
                        return slice1d<T>(std::move(v),
                            util::slicing_helpers::extract_slicing(rows, size),
                            detail::slice_assign<T>{value}, name, codename);
                    }

                    if (valid(columns))
                    {
                        HPX_ASSERT(!valid(rows));
                        return slice1d<T>(std::move(v),
                            util::slicing_helpers::extract_slicing(columns, size),
                            detail::slice_assign<T>{value}, name, codename);
                    }
                }
            }
            break;

        case 0: HPX_FALLTHROUGH;
        default:
            break;
        }

        HPX_THROW_EXCEPTION(hpx::invalid_status,
            "phylanx::execution_tree::slice",
            util::generate_error_message(
                "target ir::node_data object holds data type that does not support "
                "2d slicing",
                name, codename));
    }

    ///////////////////////////////////////////////////////////////////////////
    // explicit instantiations of the slice (modify) functionality
    template PHYLANX_EXPORT ir::node_data<std::uint8_t> slice<std::uint8_t>(
        ir::node_data<std::uint8_t>&& data,
        execution_tree::primitive_argument_type const& indices,
        ir::node_data<std::uint8_t>&& value, std::string const& name,
        std::string const& codename);

    template PHYLANX_EXPORT ir::node_data<double> slice<double>(
        ir::node_data<double>&& data,
        execution_tree::primitive_argument_type const& indices,
        ir::node_data<double>&& value, std::string const& name,
        std::string const& codename);

    template PHYLANX_EXPORT ir::node_data<std::int64_t> slice<std::int64_t>(
        ir::node_data<std::int64_t>&& data,
        execution_tree::primitive_argument_type const& indices,
        ir::node_data<std::int64_t>&& value, std::string const& name,
        std::string const& codename);

    template PHYLANX_EXPORT ir::node_data<std::uint8_t> slice<std::uint8_t>(
        ir::node_data<std::uint8_t>&& data,
        execution_tree::primitive_argument_type const& rows,
        execution_tree::primitive_argument_type const& columns,
        ir::node_data<std::uint8_t>&& value, std::string const& name,
        std::string const& codename);

    template PHYLANX_EXPORT ir::node_data<double> slice<double>(
        ir::node_data<double>&& data,
        execution_tree::primitive_argument_type const& rows,
        execution_tree::primitive_argument_type const& columns,
        ir::node_data<double>&& value, std::string const& name,
        std::string const& codename);

    template PHYLANX_EXPORT ir::node_data<std::int64_t> slice<std::int64_t>(
        ir::node_data<std::int64_t>&& data,
        execution_tree::primitive_argument_type const& rows,
        execution_tree::primitive_argument_type const& columns,
        ir::node_data<std::int64_t>&& value, std::string const& name,
        std::string const& codename);
}}
