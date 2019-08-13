// Copyright (c) 2019 Bita Hasheminezhad
// Copyright (c) 2019 Hartmut Kaiser
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(PHYLANX_KERAS_SUPPORT_BATCH_DOT_OPERATION_IMPL)
#define PHYLANX_KERAS_SUPPORT_BATCH_DOT_OPERATION_IMPL

#include <phylanx/config.hpp>
#include <phylanx/execution_tree/primitives/node_data_helpers.hpp>
#include <phylanx/ir/node_data.hpp>
#include <phylanx/plugins/keras_support/batch_dot_operation.hpp>

#include <hpx/include/lcos.hpp>
#include <hpx/include/naming.hpp>
#include <hpx/include/util.hpp>
#include <hpx/throw_exception.hpp>

#include <array>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

#include <blaze/Math.h>
#include <blaze_tensor/Math.h>

///////////////////////////////////////////////////////////////////////////////
namespace phylanx { namespace execution_tree { namespace primitives
{
    ///////////////////////////////////////////////////////////////////////////
    template <typename T>
    primitive_argument_type batch_dot_operation::batch_dot2d2d(
        ir::node_data<T>&& lhs, ir::node_data<T>&& rhs) const
    {
        // axes = (1,1)
        auto m1 = lhs.matrix();
        auto m2 = rhs.matrix();

        blaze::DynamicMatrix<T> result(m1.rows(), 1);

        for (std::size_t i = 0; i != m1.rows(); ++i)

            blaze::row(result, i) =
                blaze::dot(blaze::row(m1, i), blaze::row(m2, i));

        return primitive_argument_type{std::move(result)};
    }

    template <typename T>
    primitive_argument_type batch_dot_operation::batch_dot2d3d(
        ir::node_data<T>&& lhs, ir::node_data<T>&& rhs) const
    {
        // axes = (1,1)
        auto m = lhs.matrix();
        auto t = rhs.tensor();

        blaze::DynamicMatrix<T> result(t.pages(), t.columns());

        for (std::size_t i = 0; i != m.rows(); ++i)

            blaze::row(result, i) = blaze::row(m, i) * blaze::pageslice(t, i);

        return primitive_argument_type{std::move(result)};
    }

    template <typename T>
    primitive_argument_type batch_dot_operation::batch_dot2d3d_axes12(
        ir::node_data<T>&& lhs, ir::node_data<T>&& rhs) const
    {
        // axes = (1,2)
        auto m = lhs.matrix();
        auto t = rhs.tensor();

        blaze::DynamicMatrix<T> result(t.pages(), t.rows());

        for (std::size_t i = 0; i != t.pages(); ++i)
            blaze::row(result, i) =
                blaze::row(m, i) * blaze::trans(blaze::pageslice(t, i));

        return primitive_argument_type{std::move(result)};
    }

    template <typename T>
    primitive_argument_type batch_dot_operation::batch_dot2d3d(
        ir::node_data<T>&& lhs, ir::node_data<T>&& rhs,
        std::size_t const& axis_a, std::size_t const& axis_b) const
    {
        if (axis_b == 2)
            return batch_dot2d3d_axes12(std::move(lhs), std::move(rhs));

        // axes = (1,1)
        return batch_dot2d3d(std::move(lhs), std::move(rhs));
    }

    ///////////////////////////////////////////////////////////////////////////
    template <typename T>
    primitive_argument_type batch_dot_operation::batch_dot2d4d(
        ir::node_data<T>&& lhs, ir::node_data<T>&& rhs) const
    {
        // axes = (1,2)
        auto m = lhs.matrix();
        auto q = rhs.quatern();
        std::size_t batch = m.rows();
        std::size_t pages = q.pages();

        blaze::DynamicTensor<T> result(batch, pages, q.columns());

        for (std::size_t i = 0; i != batch; ++i) {

            auto t = quatslice(q, i);
            for (std::size_t j = 0; j != pages; ++j)
                blaze::row(blaze::pageslice(result, i), j) =
                    blaze::row(m, i) * blaze::pageslice(t, j);
        }

        return primitive_argument_type{std::move(result)};
    }

    template <typename T>
    primitive_argument_type batch_dot_operation::batch_dot2d4d_axis1(
        ir::node_data<T>&& lhs, ir::node_data<T>&& rhs) const
    {
        // axes = (1,1)
        auto m = lhs.matrix();
        auto q = rhs.quatern();
        std::size_t batch = m.rows();
        std::size_t rows = q.rows();

        blaze::DynamicTensor<T> result(batch, rows, q.columns());

        for (std::size_t i = 0; i != batch; ++i) {

            auto t = quatslice(q, i);
            for (std::size_t j = 0; j != rows; ++j)
                blaze::row(blaze::pageslice(result, i), j) =
                blaze::row(m, i) * blaze::trans(blaze::rowslice(t, j));
        }

        return primitive_argument_type{std::move(result)};
    }

    template <typename T>
    primitive_argument_type batch_dot_operation::batch_dot2d4d_axes13(
        ir::node_data<T>&& lhs, ir::node_data<T>&& rhs) const
    {
        // axes = (1,3)
        auto m = lhs.matrix();
        auto q = rhs.quatern();
        std::size_t batch = m.rows();
        std::size_t pages = q.pages();

        blaze::DynamicTensor<T> result(batch, pages, q.rows());

        for (std::size_t i = 0; i != batch; ++i) {

            auto t = quatslice(q, i);
            for (std::size_t j = 0; j != pages; ++j)
                blaze::row(blaze::pageslice(result, i), j) = blaze::trans(
                    blaze::pageslice(t, j) * blaze::trans(blaze::row(m, i)));
        }

        return primitive_argument_type{std::move(result)};
    }

    template <typename T>
    primitive_argument_type batch_dot_operation::batch_dot2d4d(
        ir::node_data<T>&& lhs, ir::node_data<T>&& rhs,
        std::size_t const& axis_a, std::size_t const& axis_b) const
    {
        if (axis_b == 1)
            return batch_dot2d4d_axis1(std::move(lhs), std::move(rhs));

        else if (axis_b == 3)
            return batch_dot2d4d_axes13(std::move(lhs), std::move(rhs));

        // axes = (1,2)
        return batch_dot2d4d(std::move(lhs), std::move(rhs));
    }

    template <typename T>
    primitive_argument_type batch_dot_operation::batch_dot2d(
        ir::node_data<T>&& lhs, ir::node_data<T>&& rhs) const
    {
        switch (rhs.num_dimensions())
        {
        case 2:
            return batch_dot2d2d(std::move(lhs), std::move(rhs));

        case 3:
            return batch_dot2d3d(std::move(lhs), std::move(rhs));

        case 4:
            return batch_dot2d4d(std::move(lhs), std::move(rhs));

        default:
            break;
        }
        HPX_THROW_EXCEPTION(hpx::bad_parameter,
            "batch_dot_operation::batch_dot2d",
            generate_error_message(
                "the right operand has unsupported number of dimensions"));
    }

    template <typename T>
    primitive_argument_type batch_dot_operation::batch_dot2d(
        ir::node_data<T>&& lhs, ir::node_data<T>&& rhs,
        std::size_t const& axis_a, std::size_t const& axis_b) const
    {
        switch (rhs.num_dimensions())
        {
        case 2:
            return batch_dot2d2d(std::move(lhs), std::move(rhs));

        case 3:
            return batch_dot2d3d(
                std::move(lhs), std::move(rhs), axis_a, axis_b);

        case 4:
            return batch_dot2d4d(
                std::move(lhs), std::move(rhs), axis_a, axis_b);

        default:
            break;
        }
        HPX_THROW_EXCEPTION(hpx::bad_parameter,
            "batch_dot_operation::batch_dot2d",
            generate_error_message(
                "the right operand has unsupported number of dimensions"));
    }

    ///////////////////////////////////////////////////////////////////////////
    template <typename T>
    primitive_argument_type batch_dot_operation::batch_dot3d2d(
        ir::node_data<T>&& lhs, ir::node_data<T>&& rhs) const
    {
        // axes = (2,1)
        auto m = rhs.matrix();
        auto t = lhs.tensor();

        blaze::DynamicMatrix<T> result(t.pages(), t.rows());

        for (std::size_t i = 0; i != t.pages(); ++i)

            blaze::row(result, i) =
                blaze::row(m, i) * blaze::trans(blaze::pageslice(t, i));

        return primitive_argument_type{std::move(result)};
    }

    template <typename T>
    primitive_argument_type batch_dot_operation::batch_dot3d2d_axis1(
        ir::node_data<T>&& lhs, ir::node_data<T>&& rhs) const
    {
        // axes = (1,1)
        auto m = rhs.matrix();
        auto t = lhs.tensor();

        blaze::DynamicMatrix<T> result(t.pages(), t.columns());

        for (std::size_t i = 0; i != t.pages(); ++i)
            blaze::row(result, i) = blaze::row(m, i) * blaze::pageslice(t, i);

        return primitive_argument_type{std::move(result)};
    }

    template <typename T>
    primitive_argument_type batch_dot_operation::batch_dot3d2d(
        ir::node_data<T>&& lhs, ir::node_data<T>&& rhs,
        std::size_t const& axis_a, std::size_t const& axis_b) const
    {
        if (axis_a == 2)
            return batch_dot3d2d(std::move(lhs), std::move(rhs));

        // axes = (1,1)
        return batch_dot3d2d_axis1(std::move(lhs), std::move(rhs));
    }

    ///////////////////////////////////////////////////////////////////////////
    template <typename T>
    primitive_argument_type batch_dot_operation::batch_dot3d3d_axis1(
        ir::node_data<T>&& lhs, ir::node_data<T>&& rhs) const
    {
        // axes = (1,1)
        auto t1 = lhs.tensor();
        auto t2 = rhs.tensor();

        blaze::DynamicTensor<T> result(t1.pages(), t1.columns(), t2.columns());

        for (std::size_t i = 0; i != t1.pages(); ++i)

            blaze::pageslice(result, i) =
                blaze::trans(blaze::pageslice(t1, i)) * blaze::pageslice(t2, i);

        return primitive_argument_type{std::move(result)};
    }

    template <typename T>
    primitive_argument_type batch_dot_operation::batch_dot3d3d_axis2(
        ir::node_data<T>&& lhs, ir::node_data<T>&& rhs) const
    {
        // axes = (2,2)
        auto t1 = lhs.tensor();
        auto t2 = rhs.tensor();

        blaze::DynamicTensor<T> result(t1.pages(), t1.rows(), t2.rows());

        for (std::size_t i = 0; i != t1.pages(); ++i)

            blaze::pageslice(result, i) =
                blaze::pageslice(t1, i) * blaze::trans(blaze::pageslice(t2, i));

        return primitive_argument_type{std::move(result)};
    }

    template <typename T>
    primitive_argument_type batch_dot_operation::batch_dot3d3d_axes12(
        ir::node_data<T>&& lhs, ir::node_data<T>&& rhs) const
    {
        // axes = (1,2)
        auto t1 = lhs.tensor();
        auto t2 = rhs.tensor();

        blaze::DynamicTensor<T> result(t1.pages(), t1.columns(), t2.rows());

        for (std::size_t i = 0; i != t1.pages(); ++i)

            blaze::pageslice(result, i) =
                blaze::trans(blaze::pageslice(t2, i) * blaze::pageslice(t1, i));

        return primitive_argument_type{std::move(result)};
    }

    template <typename T>
    primitive_argument_type batch_dot_operation::batch_dot3d3d(
        ir::node_data<T>&& lhs, ir::node_data<T>&& rhs) const
    {
        // axes = (2,1)
        auto t1 = lhs.tensor();
        auto t2 = rhs.tensor();

        blaze::DynamicTensor<T> result(t1.pages(), t1.rows(), t2.columns());

        for (std::size_t i = 0; i != t1.pages(); ++i)

            blaze::pageslice(result, i) =
                blaze::pageslice(t1, i) * blaze::pageslice(t2, i);

        return primitive_argument_type{std::move(result)};
    }

    template <typename T>
    primitive_argument_type batch_dot_operation::batch_dot3d3d(
        ir::node_data<T>&& lhs, ir::node_data<T>&& rhs,
        std::size_t const& axis_a, std::size_t const& axis_b) const
    {
        if (axis_a == 1)
        {
            if(axis_b == 1)
                return batch_dot3d3d_axis1(std::move(lhs), std::move(rhs));

            // axis_b == 2
            return batch_dot3d3d_axes12(std::move(lhs), std::move(rhs));
        }

        // axis_a == 2
        if (axis_b == 2)
            return batch_dot3d3d_axis2(std::move(lhs), std::move(rhs));

        // axis_b == 1, the default case
        return batch_dot3d3d(std::move(lhs), std::move(rhs));

    }

    ///////////////////////////////////////////////////////////////////////////
    template <typename T>
    primitive_argument_type batch_dot_operation::batch_dot3d4d(
        ir::node_data<T>&& lhs, ir::node_data<T>&& rhs) const
    {
        // axes = (2,2)
        auto t = lhs.tensor();
        auto q = rhs.quatern();
        std::size_t batch = q.quats();
        std::size_t pages = q.pages();

        blaze::DynamicArray<4UL, T> result(
            batch, t.rows(), pages, q.columns());

        for (std::size_t i = 0; i != batch; ++i)
        {
            auto q_tensor = quatslice(q, i);
            auto res_tensor = quatslice(result, i);
            for (std::size_t j = 0; j != pages; ++j)
            {
                blaze::rowslice(res_tensor, j) = blaze::trans(
                    blaze::pageslice(t, i) * blaze::pageslice(q_tensor, j));
            }
        }
        return primitive_argument_type{std::move(result)};
    }

    template <typename T>
    primitive_argument_type batch_dot_operation::batch_dot3d4d_axis1(
        ir::node_data<T>&& lhs, ir::node_data<T>&& rhs) const
    {
        // axes = (1,1)
        auto t = lhs.tensor();
        auto q = rhs.quatern();
        std::size_t batch = q.quats();
        std::size_t t_columns = t.columns();
        std::size_t rows  = q.rows();

        blaze::DynamicArray<4UL, T> result(batch, t_columns, rows, q.columns());

        for (std::size_t i = 0; i != batch; ++i)
        {
            auto q_tensor = quatslice(q, i);
            auto res_tensor = quatslice(result, i);
            for (std::size_t j = 0; j != t_columns; ++j)
            {
                for (std::size_t k = 0; k != rows; ++k)
                {
                    blaze::row(blaze::pageslice(res_tensor, j), k) =
                        blaze::trans(blaze::rowslice(q_tensor, k) *
                            blaze::column(blaze::pageslice(t, i), j));
                }
            }
        }
        return primitive_argument_type{std::move(result)};
    }

    template <typename T>
    primitive_argument_type batch_dot_operation::batch_dot3d4d_axes12(
        ir::node_data<T>&& lhs, ir::node_data<T>&& rhs) const
    {
        // axes = (1,2)
        auto t = lhs.tensor();
        auto q = rhs.quatern();
        std::size_t batch = q.quats();
        std::size_t t_columns = t.columns();
        std::size_t q_columns = q.columns();

        blaze::DynamicArray<4UL, T> result(
            batch, t_columns, q.rows(), q_columns);

        for (std::size_t i = 0; i != batch; ++i)
        {
            auto q_tensor = quatslice(q, i);
            auto res_tensor = quatslice(result, i);
            for (std::size_t j = 0; j != t_columns; ++j)
            {
                for (std::size_t k = 0; k != q_columns; ++k)
                {
                    blaze::column(blaze::pageslice(res_tensor, j), k) =
                        blaze::columnslice(q_tensor, k) *
                        blaze::column(blaze::pageslice(t, i), j);
                }
            }
        }
        return primitive_argument_type{std::move(result)};
    }

    template <typename T>
    primitive_argument_type batch_dot_operation::batch_dot3d4d_axes13(
        ir::node_data<T>&& lhs, ir::node_data<T>&& rhs) const
    {
        // axes = (1,3)
        auto t = lhs.tensor();
        auto q = rhs.quatern();
        std::size_t batch = q.quats();
        std::size_t t_columns = t.columns();
        std::size_t rows = q.rows();

        blaze::DynamicArray<4UL, T> result(batch, t_columns, q.pages(), rows);

        for (std::size_t i = 0; i != batch; ++i)
        {
            auto q_tensor = quatslice(q, i);
            auto res_tensor = quatslice(result, i);
            for (std::size_t j = 0; j != t_columns; ++j)
            {
                for (std::size_t k = 0; k != rows; ++k)
                {
                    blaze::column(blaze::pageslice(res_tensor, j), k) =
                        blaze::trans(blaze::rowslice(q_tensor, k)) *
                        blaze::column(blaze::pageslice(t, i), j);
                }
            }
        }
        return primitive_argument_type{std::move(result)};
    }

    template <typename T>
    primitive_argument_type batch_dot_operation::batch_dot3d4d_axes21(
        ir::node_data<T>&& lhs, ir::node_data<T>&& rhs) const
    {
        // axes = (2,1)
        auto t = lhs.tensor();
        auto q = rhs.quatern();
        std::size_t batch = q.quats();
        std::size_t t_rows = t.rows();
        std::size_t q_rows = q.rows();

        blaze::DynamicArray<4UL, T> result(batch, t_rows, q_rows, q.columns());

        for (std::size_t i = 0; i != batch; ++i)
        {
            auto q_tensor = quatslice(q, i);
            auto res_tensor = quatslice(result, i);
            for (std::size_t j = 0; j != t_rows; ++j)
            {
                for (std::size_t k = 0; k != q_rows; ++k)
                {
                    blaze::row(blaze::pageslice(res_tensor, j), k) =
                        blaze::row(blaze::pageslice(t, i), j) *
                        blaze::trans(blaze::rowslice(q_tensor, k));
                }
            }
        }
        return primitive_argument_type{std::move(result)};
    }

    template <typename T>
    primitive_argument_type batch_dot_operation::batch_dot3d4d_axes23(
        ir::node_data<T>&& lhs, ir::node_data<T>&& rhs) const
    {
        // axes = (2,3)
        auto t = lhs.tensor();
        auto q = rhs.quatern();
        std::size_t batch = q.quats();
        std::size_t t_rows = t.rows();
        std::size_t pages = q.pages();

        blaze::DynamicArray<4UL, T> result(batch, t_rows, pages, q.rows());

        for (std::size_t i = 0; i != batch; ++i)
        {
            auto q_tensor = quatslice(q, i);
            auto res_tensor = quatslice(result, i);
            for (std::size_t j = 0; j != t_rows; ++j)
            {
                for (std::size_t k = 0; k != pages; ++k)
                {
                    blaze::row(blaze::pageslice(res_tensor, j), k) =
                        blaze::row(blaze::pageslice(t, i), j) *
                        blaze::trans(blaze::pageslice(q_tensor, k));
                }
            }
        }
        return primitive_argument_type{std::move(result)};
    }

    template <typename T>
    primitive_argument_type batch_dot_operation::batch_dot3d4d(
        ir::node_data<T>&& lhs, ir::node_data<T>&& rhs,
        std::size_t const& axis_a, std::size_t const& axis_b) const
    {
        if (axis_a == 1)
        {
            if (axis_b == 1)
                return batch_dot3d4d_axis1(std::move(lhs), std::move(rhs));

            else if (axis_b == 2)
                return batch_dot3d4d_axes12(std::move(lhs), std::move(rhs));

            // axis_b == 3
            return batch_dot3d4d_axes13(std::move(lhs), std::move(rhs));
        }

        // axis_a == 2
        if (axis_b == 1)
            return batch_dot3d4d_axes21(std::move(lhs), std::move(rhs));

        else if (axis_b == 3)
            return batch_dot3d4d_axes23(std::move(lhs), std::move(rhs));

        // axis_b == 2, the default case
        return batch_dot3d4d(std::move(lhs), std::move(rhs));
    }

    ///////////////////////////////////////////////////////////////////////////
    template <typename T>
    primitive_argument_type batch_dot_operation::batch_dot3d(
        ir::node_data<T>&& lhs, ir::node_data<T>&& rhs) const
    {
        switch (rhs.num_dimensions())
        {
        case 2:
            return batch_dot3d2d(std::move(lhs), std::move(rhs));

        case 3:
            return batch_dot3d3d(std::move(lhs), std::move(rhs));

        case 4:
            return batch_dot3d4d(std::move(lhs), std::move(rhs));

        default:
            break;
        }
        HPX_THROW_EXCEPTION(hpx::bad_parameter,
            "batch_dot_operation::batch_dot3d",
            generate_error_message(
                "the right operand has unsupported number of dimensions"));
    }

    template <typename T>
    primitive_argument_type batch_dot_operation::batch_dot3d(
        ir::node_data<T>&& lhs, ir::node_data<T>&& rhs,
        std::size_t const& axis_a, std::size_t const& axis_b) const
    {
        switch (rhs.num_dimensions())
        {
        case 2:
            return batch_dot3d2d(
                std::move(lhs), std::move(rhs), axis_a, axis_b);

        case 3:
            return batch_dot3d3d(
                std::move(lhs), std::move(rhs), axis_a, axis_b);

        case 4:
            return batch_dot3d4d(
                std::move(lhs), std::move(rhs), axis_a, axis_b);

        default:
            break;
        }
        HPX_THROW_EXCEPTION(hpx::bad_parameter,
            "batch_dot_operation::batch_dot3d",
            generate_error_message(
                "the right operand has unsupported number of dimensions"));
    }

    ///////////////////////////////////////////////////////////////////////////
    template <typename T>
    primitive_argument_type batch_dot_operation::batch_dot4d2d(
        ir::node_data<T>&& lhs, ir::node_data<T>&& rhs,
        std::size_t const& axis_a, std::size_t const& axis_b) const
    {
        if (axis_a == 1)
            return batch_dot2d4d_axis1(std::move(rhs), std::move(lhs));

        if (axis_a == 2)
            return batch_dot2d4d(std::move(rhs), std::move(lhs));

        // axes = (3,1)
        return batch_dot2d4d_axes13(std::move(rhs), std::move(lhs));
    }

    ///////////////////////////////////////////////////////////////////////////
    template <typename T>
    primitive_argument_type batch_dot_operation::batch_dot4d3d(
        ir::node_data<T>&& lhs, ir::node_data<T>&& rhs) const
    {
        // axes = (3,1)
        auto q = lhs.quatern();
        auto t = rhs.tensor();
        std::size_t batch = q.quats();
        std::size_t pages = q.pages();

        blaze::DynamicArray<4UL, T> result(batch, pages, q.rows(), t.columns());

        for (std::size_t i = 0; i != batch; ++i)
        {
            auto q_tensor = quatslice(q, i);
            auto res_tensor = quatslice(result, i);
            for (std::size_t j = 0; j != pages; ++j)
            {
                blaze::pageslice(res_tensor, j) =
                    blaze::pageslice(q_tensor, j) * blaze::pageslice(t, i);
            }
        }
        return primitive_argument_type{std::move(result)};
    }

    template <typename T>
    primitive_argument_type batch_dot_operation::batch_dot4d3d_axis1(
        ir::node_data<T>&& lhs, ir::node_data<T>&& rhs) const
    {
        // axes = (1,1)
        auto q = lhs.quatern();
        auto t = rhs.tensor();
        std::size_t batch = q.quats();
        std::size_t rows = q.rows();

        blaze::DynamicArray<4UL, T> result(
            batch, q.rows(), q.columns(), t.columns());

        for (std::size_t i = 0; i != batch; ++i)
        {
            auto q_tensor = quatslice(q, i);
            auto res_tensor = quatslice(result, i);
            for (std::size_t j = 0; j != rows; ++j)
            {
                blaze::pageslice(res_tensor, j) =
                    blaze::rowslice(q_tensor, j) * blaze::pageslice(t, i);
            }
        }
        return primitive_argument_type{std::move(result)};
    }

    template <typename T>
    primitive_argument_type batch_dot_operation::batch_dot4d3d_axes21(
        ir::node_data<T>&& lhs, ir::node_data<T>&& rhs) const
    {
        // axes = (2,1)
        auto q = lhs.quatern();
        auto t = rhs.tensor();
        std::size_t batch = q.quats();
        std::size_t pages = q.pages();

        blaze::DynamicArray<4UL, T> result(
            batch, pages, q.columns(), t.columns());

        for (std::size_t i = 0; i != batch; ++i)
        {
            auto q_tensor = quatslice(q, i);
            auto res_tensor = quatslice(result, i);
            for (std::size_t j = 0; j != pages; ++j)
            {
                blaze::pageslice(res_tensor, j) =
                    blaze::trans(blaze::pageslice(q_tensor, j)) *
                    blaze::pageslice(t, i);
            }
        }
        return primitive_argument_type{std::move(result)};
    }

    template <typename T>
    primitive_argument_type batch_dot_operation::batch_dot4d3d_axes12(
        ir::node_data<T>&& lhs, ir::node_data<T>&& rhs) const
    {
        // axes = (1,2)
        auto q = lhs.quatern();
        auto t = rhs.tensor();
        std::size_t batch = q.quats();
        std::size_t rows = q.rows();

        blaze::DynamicArray<4UL, T> result(batch, rows, q.columns(), t.rows());

        for (std::size_t i = 0; i != batch; ++i)
        {
            auto q_tensor = quatslice(q, i);
            auto res_tensor = quatslice(result, i);
            for (std::size_t j = 0; j != rows; ++j)
            {
                blaze::pageslice(res_tensor, j) = blaze::rowslice(q_tensor, j) *
                    blaze::trans(blaze::pageslice(t, i));
            }
        }
        return primitive_argument_type{std::move(result)};
    }

    template <typename T>
    primitive_argument_type batch_dot_operation::batch_dot4d3d_axes32(
        ir::node_data<T>&& lhs, ir::node_data<T>&& rhs) const
    {
        // axes = (3,2)
        auto q = lhs.quatern();
        auto t = rhs.tensor();
        std::size_t batch = q.quats();
        std::size_t pages = q.pages();

        blaze::DynamicArray<4UL, T> result(batch, pages, q.rows(), t.rows());

        for (std::size_t i = 0; i != batch; ++i)
        {
            auto q_tensor = quatslice(q, i);
            auto res_tensor = quatslice(result, i);
            for (std::size_t j = 0; j != pages; ++j)
            {
                blaze::pageslice(res_tensor, j) =
                    blaze::pageslice(q_tensor, j) *
                    blaze::trans(blaze::pageslice(t, i));
            }
        }
        return primitive_argument_type{std::move(result)};
    }

    template <typename T>
    primitive_argument_type batch_dot_operation::batch_dot4d3d_axis2(
        ir::node_data<T>&& lhs, ir::node_data<T>&& rhs) const
    {
        // axes = (2,2)
        auto q = lhs.quatern();
        auto t = rhs.tensor();
        std::size_t batch = q.quats();
        std::size_t pages = q.pages();

        blaze::DynamicArray<4UL, T> result(batch, pages, q.columns(), t.rows());

        for (std::size_t i = 0; i != batch; ++i)
        {
            auto q_tensor = quatslice(q, i);
            auto res_tensor = quatslice(result, i);
            for (std::size_t j = 0; j != pages; ++j)
            {
                blaze::pageslice(res_tensor, j) = blaze::trans(
                    blaze::pageslice(t, i) * blaze::pageslice(q_tensor, j));
            }
        }
        return primitive_argument_type{std::move(result)};
    }

    template <typename T>
    primitive_argument_type batch_dot_operation::batch_dot4d3d(
        ir::node_data<T>&& lhs, ir::node_data<T>&& rhs,
        std::size_t const& axis_a, std::size_t const& axis_b) const
    {
        if (axis_b == 1)
        {
            if (axis_a == 1)
                return batch_dot4d3d_axis1(std::move(lhs), std::move(rhs));

            else if (axis_a == 2)
                return batch_dot4d3d_axes21(std::move(lhs), std::move(rhs));

            // axis_a == 3, the default case
            return batch_dot4d3d(std::move(lhs), std::move(rhs));
        }

        // axis_b == 2
        if (axis_a == 1)
            return batch_dot4d3d_axes12(std::move(lhs), std::move(rhs));

        else if (axis_a == 3)
            return batch_dot4d3d_axes32(std::move(lhs), std::move(rhs));

        // axis_a == 2
        return batch_dot4d3d_axis2(std::move(lhs), std::move(rhs));
    }

    ///////////////////////////////////////////////////////////////////////////
    template <typename T>
    primitive_argument_type batch_dot_operation::batch_dot4d(
        ir::node_data<T>&& lhs, ir::node_data<T>&& rhs) const
    {
        switch (rhs.num_dimensions())
        {
        case 2:
            return batch_dot2d4d_axes13(std::move(rhs), std::move(lhs));

        case 3:
            return batch_dot4d3d(std::move(lhs), std::move(rhs));

        case 4:
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "batch_dot_operation::batch_dot4d",
                generate_error_message("batch_dot4d4d is not supported by this "
                                       "version of Phylanx"));

        default:
            break;
        }
        HPX_THROW_EXCEPTION(hpx::bad_parameter,
            "batch_dot_operation::batch_dot4d",
            generate_error_message(
                "the right operand has unsupported number of dimensions"));
    }

    template <typename T>
    primitive_argument_type batch_dot_operation::batch_dot4d(
        ir::node_data<T>&& lhs, ir::node_data<T>&& rhs,
        std::size_t const& axis_a, std::size_t const& axis_b) const
    {
        switch (rhs.num_dimensions())
        {
        case 2:
            return batch_dot4d2d(
                std::move(lhs), std::move(rhs), axis_a, axis_b);

        case 3:
            return batch_dot4d3d(
                std::move(lhs), std::move(rhs), axis_a, axis_b);

        case 4:
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "batch_dot_operation::batch_dot4d",
                generate_error_message("batch_dot4d4d is not supported by this "
                                       "version of Phylanx"));

        default:
            break;
        }
        HPX_THROW_EXCEPTION(hpx::bad_parameter,
            "batch_dot_operation::batch_dot4d",
            generate_error_message(
                "the right operand has unsupported number of dimensions"));
    }

    ///////////////////////////////////////////////////////////////////////////
    template <typename T>
    primitive_argument_type batch_dot_operation::batch_dot_nd(
        ir::node_data<T>&& lhs, ir::node_data<T>&& rhs) const
    {
        switch (lhs.num_dimensions())
        {
        case 2:
            return batch_dot2d(std::move(lhs), std::move(rhs));

        case 3:
            return batch_dot3d(std::move(lhs), std::move(rhs));

        case 4:
            return batch_dot4d(std::move(lhs), std::move(rhs));

        default:
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "batch_dot_operation::batch_dot_nd",
                generate_error_message(
                    "the left operand has unsupported number of dimensions"));
        }
    }

    template <typename T>
    primitive_argument_type batch_dot_operation::batch_dot_nd(
        ir::node_data<T>&& lhs, ir::node_data<T>&& rhs,
        std::size_t const& axis_a, std::size_t const& axis_b) const
    {
        switch (lhs.num_dimensions())
        {
        case 2:
            return batch_dot2d(std::move(lhs), std::move(rhs), axis_a, axis_b);

        case 3:
            return batch_dot3d(std::move(lhs), std::move(rhs), axis_a, axis_b);

        case 4:
            return batch_dot4d(std::move(lhs), std::move(rhs), axis_a, axis_b);

        default:
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "batch_dot_operation::batch_dot_nd",
                generate_error_message(
                    "the left operand has unsupported number of dimensions"));
        }
    }
}}}

#endif
