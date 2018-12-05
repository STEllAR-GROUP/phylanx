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
    ir::node_data<T> slice3d_basic_basic_basic(Data&& m,
        ir::slicing_indices const& rows, ir::slicing_indices const& columns,
        ir::slicing_indices const& pages, F const& f, std::string const& name,
        std::string const& codename)
    {
//         std::size_t numrows = m.rows();
//         if (rows.start() >= std::int64_t(numrows) ||
//             rows.span() > std::int64_t(numrows))
//         {
//             HPX_THROW_EXCEPTION(hpx::bad_parameter,
//                 "phylanx::execution_tree::slice3d_basic_basic",
//                 util::generate_error_message(
//                     "cannot extract the requested matrix elements",
//                     name, codename));
//         }
//
//         std::int64_t row_start = rows.start();
//         std::int64_t row_stop = rows.stop();
//         std::int64_t row_step = rows.step();
//
//         if (row_step == 0 && !rows.single_value())
//         {
//             HPX_THROW_EXCEPTION(hpx::bad_parameter,
//                 "phylanx::execution_tree::slice3d_basic_basic",
//                 util::generate_error_message(
//                     "row-step can not be zero", name, codename));
//         }
//
//         std::size_t numcols = m.columns();
//         if (columns.start() >= std::int64_t(numcols) ||
//             columns.span() > std::int64_t(numcols))
//         {
//             HPX_THROW_EXCEPTION(hpx::bad_parameter,
//                 "phylanx::execution_tree::slice3d_basic_basic",
//                 util::generate_error_message(
//                     "cannot extract the requested matrix elements",
//                     name, codename));
//         }
//
//         std::int64_t col_start = columns.start();
//         std::int64_t col_stop = columns.stop();
//         std::int64_t col_step = columns.step();
//
//         if (col_step == 0 && !columns.single_value())
//         {
//             HPX_THROW_EXCEPTION(hpx::bad_parameter,
//                 "phylanx::execution_tree::slice3d_basic_basic",
//                 util::generate_error_message(
//                     "column-step can not be zero", name, codename));
//         }
//
//         std::size_t num_matrix_rows = m.rows();
//         std::size_t num_matrix_cols = m.columns();
//
//         // return a value and not a vector if you are not given a list
//         if (rows.single_value())
//         {
//             auto row = blaze::row(m, row_start);
//
//             // handle single value slicing result
//             if (columns.single_value())
//             {
//                 return f.scalar(m, row[col_start]);
//             }
//
//             // extract a consecutive sub-vector (sub-row)
//             if (col_step == 1)
//             {
//                 HPX_ASSERT(col_stop > col_start);
//                 auto sv =
//                     blaze::subvector(row, col_start, col_stop - col_start);
//                 return f.trans_vector(m, std::move(sv));
//             }
//
//             // general case, pick arbitrary elements from selected row
//             auto indices = util::slicing_helpers::create_list_slice(
//                 col_start, col_stop, col_step);
//             auto sv = blaze::elements(row, indices);
//
//             return f.trans_vector(m, std::move(sv));
//         }
//         else if (columns.single_value())
//         {
//             // handle single column case
//             auto col = blaze::column(m, col_start);
//
//             // extract a consecutive sub-vector (sub-column)
//             if (row_step == 1)
//             {
//                 HPX_ASSERT(row_stop > row_start);
//                 auto sv =
//                     blaze::subvector(col, row_start, row_stop - row_start);
//                 return f.vector(m, std::move(sv));
//             }
//
//             // general case, pick arbitrary elements from selected column
//             auto indices = util::slicing_helpers::create_list_slice(
//                 row_start, row_stop, row_step);
//             auto sv = blaze::elements(col, indices);
//
//             return f.vector(m, std::move(sv));
//         }
//
//         // extract various sub-matrices of the given matrix
//         if (col_step == 1)
//         {
//             HPX_ASSERT(col_stop > col_start);
//
//             if (row_step == 1)
//             {
//                 HPX_ASSERT(row_stop > row_start);
//                 auto result = blaze::submatrix(m, row_start,
//                     col_start, row_stop - row_start, col_stop - col_start);
//                 return f.matrix(m, std::move(result));
//             }
//
//             auto sm = blaze::submatrix(m, 0ll, col_start,
//                 num_matrix_rows, col_stop - col_start);
//
//             auto indices = util::slicing_helpers::create_list_slice(
//                 row_start, row_stop, row_step);
//             auto result = blaze::rows(sm, indices);
//
//             return f.matrix(m, std::move(result));
//         }
//         else if (row_step == 1)
//         {
//             HPX_ASSERT(row_stop > row_start);
//
//             auto sm = blaze::submatrix(m, row_start, 0ll,
//                 row_stop - row_start, num_matrix_cols);
//
//             auto indices = util::slicing_helpers::create_list_slice(
//                 col_start, col_stop, col_step);
//             auto result = blaze::columns(sm, indices);
//
//             return f.matrix(m, std::move(result));
//         }
//
//         // general case, pick arbitrary elements from matrix
//         auto row_indices = util::slicing_helpers::create_list_slice(
//             row_start, row_stop, row_step);
//         auto sm = blaze::rows(m, row_indices);
//
//         auto column_indices = util::slicing_helpers::create_list_slice(
//             col_start, col_stop, col_step);
//         auto result = blaze::columns(sm, column_indices);
//
//         return f.matrix(m, std::move(result));
        HPX_THROW_EXCEPTION(hpx::bad_parameter,
            "phylanx::execution_tree::slice3d_basic_basic_basic",
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
    ir::node_data<T> slice3d(Data&& m, primitive_argument_type const& rows,
        primitive_argument_type const& columns,
        primitive_argument_type const& pages, F const& f,
        std::string const& name, std::string const& codename)
    {
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
