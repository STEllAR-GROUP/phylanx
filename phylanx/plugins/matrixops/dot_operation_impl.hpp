//  Copyright (c) 2017-2018 Hartmut Kaiser
//  Copyright (c) 2017 Parsa Amini
//  Copyright (c) 2019 Bita Hasheminezhad
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(PHYLANX_MATRIXOPS_TENSORDOT_IMPL_2019_02_13_1235PM)
#define PHYLANX_MATRIXOPS_TENSORDOT_IMPL_2019_02_13_1235PM

#include <phylanx/config.hpp>
#include <phylanx/execution_tree/primitives/node_data_helpers.hpp>
#include <phylanx/ir/node_data.hpp>
#include <phylanx/plugins/matrixops/dot_operation.hpp>

#include <hpx/include/lcos.hpp>
#include <hpx/include/naming.hpp>
#include <hpx/include/util.hpp>
#include <hpx/throw_exception.hpp>

#include <cstddef>
#include <memory>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

#if defined(PHYLANX_HAVE_BLAZE_TENSOR)
#include <blaze_tensor/Math.h>
#endif

///////////////////////////////////////////////////////////////////////////////
namespace phylanx { namespace execution_tree { namespace primitives
{
    template <typename T>
    blaze::DynamicVector<T> dot_operation::convert_to_1d(
        ir::node_data<T>&& arr) const
    {
        switch (arr.num_dimensions())
        {
        case 0:
            return blaze::DynamicVector<T>(1, arr.scalar());
        case 1:
            return arr.vector();
#if defined(PHYLANX_HAVE_BLAZE_TENSOR)
        case 2:
            return blaze::trans(blaze::ravel(arr.matrix()));

        case 3:
            return blaze::trans(blaze::ravel(arr.tensor()));
#endif
        default:
             HPX_THROW_EXCEPTION(hpx::bad_parameter,
            "dot_operation::convert_to_1d",
            generate_error_message("the operand has >3 "
                                   "dimensions which is not supported"));
        }
    }

    ///////////////////////////////////////////////////////////////////////////
    template <typename T>
    primitive_argument_type dot_operation::dot0d0d(
        ir::node_data<T>&& lhs, ir::node_data<T>&& rhs) const
    {
        lhs.scalar() *= rhs.scalar();
        return primitive_argument_type{std::move(lhs)};
    }

    template <typename T>
    primitive_argument_type dot_operation::dot0d1d(
        ir::node_data<T>&& lhs, ir::node_data<T>&& rhs) const
    {
        if (rhs.is_ref())
        {
            rhs = rhs.vector() * lhs.scalar();
        }
        else
        {
            rhs.vector() *= lhs.scalar();
        }

        return primitive_argument_type{std::move(rhs)};
    }

    template <typename T>
    primitive_argument_type dot_operation::dot0d2d(
        ir::node_data<T>&& lhs, ir::node_data<T>&& rhs) const
    {
        if (rhs.is_ref())
        {
            rhs = rhs.matrix() * lhs.scalar();
        }
        else
        {
            rhs.matrix() *= lhs.scalar();
        }

        return primitive_argument_type{std::move(rhs)};
    }

#if defined(PHYLANX_HAVE_BLAZE_TENSOR)
    template <typename T>
    primitive_argument_type dot_operation::dot0d3d(
        ir::node_data<T>&& lhs, ir::node_data<T>&& rhs) const
    {
        if (rhs.is_ref())
        {
            rhs = rhs.tensor() * lhs.scalar();
        }
        else
        {
            rhs.tensor() *= lhs.scalar();
        }

        return primitive_argument_type{std::move(rhs)};
    }
#endif

    template <typename T>
    primitive_argument_type dot_operation::dot0d(
        ir::node_data<T>&& lhs, ir::node_data<T>&& rhs) const
    {
        switch (rhs.num_dimensions())
        {
        case 0:
            // If is_scalar(lhs) && is_scalar(rhs)
            return dot0d0d(std::move(lhs), std::move(rhs));

        case 1:
            // If is_scalar(lhs) && is_vector(rhs)
            return dot0d1d(std::move(lhs), std::move(rhs));

        case 2:
            // If is_scalar(lhs) && is_matrix(rhs)
            return dot0d2d(std::move(lhs), std::move(rhs));

#if defined(PHYLANX_HAVE_BLAZE_TENSOR)
        case 3:
            // If is_scalar(lhs) && is_tensor(rhs)
            return dot0d3d(std::move(lhs), std::move(rhs));
#endif

        default:
            // lhs_order == 1 && rhs_order != 2
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "dot_operation::dot0d",
                generate_error_message(
                    "the operands have incompatible number of dimensions"));
        }
    }

    ///////////////////////////////////////////////////////////////////////////
    template <typename T>
    primitive_argument_type dot_operation::dot1d0d(
        ir::node_data<T>&& lhs, ir::node_data<T>&& rhs) const
    {
        if (lhs.is_ref())
        {
            lhs = lhs.vector() * rhs.scalar();
        }
        else
        {
            lhs.vector() *= rhs.scalar();
        }

        return primitive_argument_type{std::move(lhs)};
    }

    template <typename T>
    primitive_argument_type dot_operation::dot1d1d(
        ir::node_data<T>&& lhs, ir::node_data<T>&& rhs) const
    {
        if (lhs.size() != rhs.size())
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "dot_operation::dot1d1d",
                generate_error_message(
                    "the operands have incompatible number of dimensions"));
        }

        // lhs.dimension(0) == rhs.dimension(0)
        lhs = T(blaze::dot(lhs.vector(), rhs.vector()));
        return primitive_argument_type{std::move(lhs)};
    }

    template <typename T>
    primitive_argument_type dot_operation::dot1d2d(
        ir::node_data<T>&& lhs, ir::node_data<T>&& rhs) const
    {
        if (lhs.size() != rhs.dimension(0))
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "dot_operation::dot1d2d",
                generate_error_message(
                    "the operands have incompatible number of dimensions"));
        }
        // lhs = blaze::trans(rhs.matrix()) * lhs.vector();
        lhs = blaze::trans(blaze::trans(lhs.vector()) * rhs.matrix());
        return primitive_argument_type{std::move(lhs)};
    }

#if defined(PHYLANX_HAVE_BLAZE_TENSOR)
    template <typename T>
    primitive_argument_type dot_operation::dot1d3d(
        ir::node_data<T>&& lhs, ir::node_data<T>&& rhs) const
    {
        if (lhs.size() != rhs.dimension(1))
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "dot_operation::dot1d3d",
                generate_error_message(
                    "the operands have incompatible number of dimensions"));
        }

        auto t = rhs.tensor();
        blaze::DynamicMatrix<T> result(t.pages(), t.columns());

        for (std::size_t i = 0; i != t.pages(); ++i)
            blaze::row(blaze::submatrix(result, i, 0, 1, t.columns()), 0) =
                blaze::trans(lhs.vector()) * blaze::pageslice(t, i);

        return primitive_argument_type{std::move(result)};
    }
#endif

    // lhs_num_dims == 1
    // Case 1: Inner product of two vectors
    // Case 2: Inner product of a vector and an array of vectors
    // Case 3: Inner product of a matrix (tensor slice)
    template <typename T>
    primitive_argument_type dot_operation::dot1d(
        ir::node_data<T>&& lhs, ir::node_data<T>&& rhs) const
    {
        switch (rhs.num_dimensions())
        {
        case 0:
            // If is_vector(lhs) && is_scalar(rhs)
            return dot1d0d(std::move(lhs), std::move(rhs));

        case 1:
            // If is_vector(lhs) && is_vector(rhs)
            return dot1d1d(std::move(lhs), std::move(rhs));

        case 2:
            // If is_vector(lhs) && is_matrix(rhs)
            return dot1d2d(std::move(lhs), std::move(rhs));

#if defined(PHYLANX_HAVE_BLAZE_TENSOR)
        case 3:
            // If is_vector(lhs) && is_tensor(rhs)
            return dot1d3d(std::move(lhs), std::move(rhs));
#endif

        default:
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "dot_operation::dot1d",
                generate_error_message(
                    "the operands have incompatible number of dimensions"));
        }
    }

    ///////////////////////////////////////////////////////////////////////////
    template <typename T>
    primitive_argument_type dot_operation::dot2d0d(
        ir::node_data<T>&& lhs, ir::node_data<T>&& rhs) const
    {
        if (lhs.is_ref())
        {
            lhs = lhs.matrix() * rhs.scalar();
        }
        else
        {
            lhs.matrix() *= rhs.scalar();
        }
        return primitive_argument_type{std::move(lhs)};
    }

    template <typename T>
    primitive_argument_type dot_operation::dot2d1d(
        ir::node_data<T>&& lhs, ir::node_data<T>&& rhs) const
    {
        if (lhs.dimension(1) != rhs.size())
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "dot_operation::dot2d1d",
                generate_error_message(
                    "the operands have incompatible number of dimensions"));
        }

        rhs = lhs.matrix() * rhs.vector();
        return primitive_argument_type{std::move(rhs)};
    }

    template <typename Matrix1, typename Matrix2>
    primitive_argument_type dot_operation::dot2d2d(
        Matrix1&& lhs, Matrix2&& rhs) const
    {
        if (lhs.columns() != rhs.rows())
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "dot_operation::dot2d2d",
                generate_error_message(
                    "the operands have incompatible number of dimensions"));
        }
        using T = blaze::ElementType_t<typename std::decay<Matrix1>::type>;
        blaze::DynamicMatrix<T> result = lhs * rhs;
        return primitive_argument_type{std::move(result)};
    }

#if defined(PHYLANX_HAVE_BLAZE_TENSOR)
    template <typename T>
    primitive_argument_type dot_operation::dot2d3d(
        ir::node_data<T>&& lhs, ir::node_data<T>&& rhs) const
    {
        if (lhs.dimension(1) != rhs.dimension(1))
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "dot_operation::dot2d3d",
                generate_error_message(
                    "the operands have incompatible number of dimensions"));
        }

        auto m = lhs.matrix();
        auto t = rhs.tensor();

        blaze::DynamicTensor<T> result(m.rows(), t.pages(), t.columns());

        for (std::size_t i = 0; i != t.pages(); ++i)
            blaze::rowslice(
                blaze::subtensor(result, 0, i, 0, m.rows(), 1, t.columns()),
                0) = blaze::trans(m * blaze::pageslice(t, i));

        return primitive_argument_type{std::move(result)};
    }
#endif

    // lhs_num_dims == 2
    // Multiply a matrix with a vector
    // Regular matrix multiplication
    template <typename T>
    primitive_argument_type dot_operation::dot2d(
        ir::node_data<T>&& lhs, ir::node_data<T>&& rhs) const
    {
        switch (rhs.num_dimensions())
        {
        case 0:
            // If is_matrix(lhs) && is_scalar(rhs)
            return dot2d0d(std::move(lhs), std::move(rhs));

        case 1:
            // If is_matrix(lhs) && is_vector(rhs)
            return dot2d1d(std::move(lhs), std::move(rhs));

        case 2:
            // If is_matrix(lhs) && is_matrix(rhs)
            return dot2d2d(lhs.matrix(), rhs.matrix());

#if defined(PHYLANX_HAVE_BLAZE_TENSOR)
        case 3:
            // If is_matrix(lhs) && is_tensor(rhs)
            return dot2d3d(std::move(lhs), std::move(rhs));
#endif

        default:
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "dot_operation::dot2d",
                generate_error_message(
                    "the operands have incompatible number of dimensions"));
        }
    }

    ///////////////////////////////////////////////////////////////////////////
#if defined(PHYLANX_HAVE_BLAZE_TENSOR)
    template <typename T>
    primitive_argument_type dot_operation::dot3d0d(
        ir::node_data<T>&& lhs, ir::node_data<T>&& rhs) const
    {
        if (lhs.is_ref())
        {
            lhs = lhs.tensor() * rhs.scalar();
        }
        else
        {
            lhs.tensor() *= rhs.scalar();
        }
        return primitive_argument_type{std::move(lhs)};
    }

    template <typename T>
    primitive_argument_type dot_operation::dot3d1d(
        ir::node_data<T>&& lhs, ir::node_data<T>&& rhs) const
    {
        if (lhs.dimension(2) != rhs.size())
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "dot_operation::dot3d1d",
                generate_error_message(
                    "the operands have incompatible number of dimensions"));
        }
        auto t = lhs.tensor();
        blaze::DynamicMatrix<T> result(t.pages(), t.rows());

        for (std::size_t i = 0; i != t.pages(); ++i)
            blaze::row(blaze::submatrix(result, i, 0, 1, t.rows()), 0) =
                blaze::trans(blaze::pageslice(t, i) * rhs.vector());

        return primitive_argument_type{std::move(result)};
    }

    template <typename T>
    primitive_argument_type dot_operation::dot3d2d(
        ir::node_data<T>&& lhs, ir::node_data<T>&& rhs) const
    {
        if (lhs.dimension(2) != rhs.dimension(0))
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "dot_operation::dot3d2d",
                generate_error_message(
                    "the operands have incompatible number of dimensions"));
        }

        auto m = rhs.matrix();
        auto t = lhs.tensor();

        blaze::DynamicTensor<T> result(t.pages(), t.rows(), m.columns());

        for (std::size_t i = 0; i != t.pages(); ++i)
            blaze::pageslice(
                blaze::subtensor(result, i, 0, 0, 1, t.rows(), m.columns()),
                0) = blaze::pageslice(t, i) * m;

        return primitive_argument_type{std::move(result)};
    }

    template <typename T>
    primitive_argument_type dot_operation::dot3d3d(
        ir::node_data<T>&& lhs, ir::node_data<T>&& rhs) const
    {
        if (lhs.dimension(2) != rhs.dimension(1))
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "dot_operation::dot3d3d",
                generate_error_message(
                    "the operands have incompatible number of dimensions"));
        }
        HPX_THROW_EXCEPTION(hpx::bad_parameter,
            "dot_operation::dot3d3d",
            generate_error_message(
                "it is not supported by Phylanx yet"));
    }

    // lhs_num_dims == 3
    template <typename T>
    primitive_argument_type dot_operation::dot3d(
        ir::node_data<T>&& lhs, ir::node_data<T>&& rhs) const
    {
        switch (rhs.num_dimensions())
        {
        case 0:
            // If is_tensor(lhs) && is_scalar(rhs)
            return dot3d0d(std::move(lhs), std::move(rhs));

        case 1:
            // If is_tensor(lhs) && is_vector(rhs)
            return dot3d1d(std::move(lhs), std::move(rhs));

        case 2:
            // If is_tensor(lhs) && is_matrix(rhs)
            return dot3d2d(std::move(lhs), std::move(rhs));

        case 3:
            // If is_tensor(lhs) && is_tensor(rhs)
            return dot3d3d(std::move(lhs), std::move(rhs));

        default:
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "dot_operation::dot3d",
                generate_error_message(
                    "the operands have incompatible number of dimensions"));
        }
    }
#endif

    ///////////////////////////////////////////////////////////////////////////
    template <typename Vector1, typename Vector2>
    primitive_argument_type dot_operation::outer1d1d(
        Vector1&& lhs, Vector2&& rhs) const
    {
        using T = blaze::ElementType_t<Vector1>;
        blaze::DynamicMatrix<T> result = blaze::outer(lhs, rhs);
        return primitive_argument_type{std::move(result)};
    }

#if defined(PHYLANX_HAVE_BLAZE_TENSOR)
    template <typename T>
    primitive_argument_type dot_operation::outer1d2d(
        ir::node_data<T>&& lhs, ir::node_data<T>&& rhs) const
    {
        auto v = lhs.vector();
        auto m = rhs.matrix();

        blaze::DynamicTensor<T> result(v.size(), m.rows(), m.columns());
        for (std::size_t i = 0; i != m.rows(); ++i)
        {
            auto slice = blaze::rowslice(result, i);
            slice = blaze::outer(blaze::row(m, i), v);
        }
        return primitive_argument_type{std::move(result)};
    }
#endif

    template <typename T>
    primitive_argument_type dot_operation::outer1d(
        ir::node_data<T>&& lhs, ir::node_data<T>&& rhs) const
    {
        switch (rhs.num_dimensions())
        {
        case 0:
            // If is_vector(lhs) && is_scalar(rhs) -> a vector
            // has the same functionality as dot1d0d
            return dot1d0d(std::move(lhs), std::move(rhs));

        case 1:
            // If is_vector(lhs) && is_vector(rhs) -> a matrix
            return outer1d1d(lhs.vector(), rhs.vector());

#if defined(PHYLANX_HAVE_BLAZE_TENSOR)
        case 2:
            // If is_vector(lhs) && is_matrix(rhs) -> a tensor
            return outer1d2d(std::move(lhs), std::move(rhs));
#endif

        default:
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "dot_operation::outer1d",
                generate_error_message(
                    "the result has >3 dimensions which is not supported"));
        }
    }

    ///////////////////////////////////////////////////////////////////////////
#if defined(PHYLANX_HAVE_BLAZE_TENSOR)
    template <typename T>
    primitive_argument_type dot_operation::outer2d1d(
        ir::node_data<T>&& lhs, ir::node_data<T>&& rhs) const
    {
        auto v = rhs.vector();
        auto m = lhs.matrix();

        blaze::DynamicTensor<T> result(m.rows(), m.columns(), v.size());
        for (std::size_t i = 0; i != m.rows(); ++i)
        {
            auto slice = blaze::pageslice(result, i);
            slice = blaze::outer(blaze::row(m, i), v);
        }
        return primitive_argument_type{std::move(result)};
    }
#endif

    template <typename T>
    primitive_argument_type dot_operation::outer2d(
        ir::node_data<T>&& lhs, ir::node_data<T>&& rhs) const
    {
        switch (rhs.num_dimensions())
        {
        case 0:
            // If is_matrix(lhs) && is_scalar(rhs) -> a matrix
            // has the same functionality as dot2d0d
            return dot2d0d(std::move(lhs), std::move(rhs));

#if defined(PHYLANX_HAVE_BLAZE_TENSOR)
        case 1:
            // If is_matrix(lhs) && is_vector(rhs) -> a tensor
            return outer2d1d(std::move(lhs), std::move(rhs));
#endif

        default:
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "dot_operation::outer2d",
                generate_error_message(
                    "the result has >3 dimensions which is not supported"));
        }
    }

    ///////////////////////////////////////////////////////////////////////////
#if defined(PHYLANX_HAVE_BLAZE_TENSOR)
    template <typename T>
    primitive_argument_type dot_operation::outer3d(
        ir::node_data<T>&& lhs, ir::node_data<T>&& rhs) const
    {
        switch (rhs.num_dimensions())
        {
        case 0:
            // If is_matrix(lhs) && is_scalar(rhs) -> a tensor
            // has the same functionality as dot3d0d
            return dot3d0d(std::move(lhs), std::move(rhs));

        default:
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "dot_operation::outer3d",
                generate_error_message(
                    "the result has >3 dimensions which is not supported"));
        }
    }
#endif

    ///////////////////////////////////////////////////////////////////////////
    template <typename T>
    primitive_argument_type dot_operation::outer_nd_helper(
        ir::node_data<T>&& lhs, ir::node_data<T>&& rhs) const
    {
        return outer1d1d(
            convert_to_1d(std::move(lhs)), convert_to_1d(std::move(rhs)));
    }

    template <typename Matrix1, typename Matrix2>
    blaze::ElementType_t<typename std::decay<Matrix1>::type>
        dot_operation::contraction2d2d(Matrix1&& lhs, Matrix2&& rhs) const
    {
        if (lhs.rows() != rhs.rows() || lhs.columns() != rhs.columns())
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "dot_operation::contraction2d2d",
                generate_error_message("shape-mismatch for sum"));

        return blaze::sum(lhs % rhs);
    }

#if defined(PHYLANX_HAVE_BLAZE_TENSOR)
    template <typename T>
    primitive_argument_type dot_operation::contraction2d3d(
        ir::node_data<T>&& lhs, ir::node_data<T>&& rhs) const
    {
        if (lhs.dimension(0) != rhs.dimension(0) ||
            lhs.dimension(1) != rhs.dimension(1))
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "dot_operation::contraction2d3d",
                generate_error_message("shape-mismatch for sum"));

        auto t = rhs.tensor();
        blaze::DynamicVector<T> result(t.columns());
        for (std::size_t i = 0; i != t.columns(); ++i)
        {
            auto slice = blaze::columnslice(t, i);
            result[i] = contraction2d2d(std::move(slice), lhs.matrix());
        }
        return primitive_argument_type{std::move(result)};
    }
#endif

    template <typename T>
    primitive_argument_type dot_operation::contraction2d(
        ir::node_data<T>&& lhs, ir::node_data<T>&& rhs) const
    {
        switch (rhs.num_dimensions())
        {
        case 2:
            // If is_matrix(lhs) && is_matrix(rhs) -> a scalar
            return primitive_argument_type{
                contraction2d2d(lhs.matrix(), rhs.matrix())};

#if defined(PHYLANX_HAVE_BLAZE_TENSOR)
        case 3:
            // If is_matrix(lhs) && is_tensor(rhs) -> a vector
            return contraction2d3d(std::move(lhs), std::move(rhs));
#endif

        default:
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "dot_operation::contraction2d",
                generate_error_message("the left operand has >3 dimensions "
                                       "which is not supported"));
        }
    }

    ///////////////////////////////////////////////////////////////////////////
#if defined(PHYLANX_HAVE_BLAZE_TENSOR)
    template <typename T>
    primitive_argument_type dot_operation::contraction3d2d(
        ir::node_data<T>&& lhs, ir::node_data<T>&& rhs) const
    {
        if (lhs.dimension(1) != rhs.dimension(0) ||
            lhs.dimension(2) != rhs.dimension(1))
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "dot_operation::contraction3d2d",
                generate_error_message("shape-mismatch for sum"));

        auto t = lhs.tensor();
        blaze::DynamicVector<T> result(t.pages());
        for (std::size_t i = 0; i != t.pages(); ++i)
        {
            auto slice = blaze::pageslice(t, i);
            result[i] = contraction2d2d(std::move(slice), rhs.matrix());
        }
        return primitive_argument_type{std::move(result)};
    }


    template <typename T>
    primitive_argument_type dot_operation::contraction3d3d(
        ir::node_data<T>&& lhs, ir::node_data<T>&& rhs) const
    {
        if (lhs.dimension(1) != rhs.dimension(0) ||
            lhs.dimension(2) != rhs.dimension(1))
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "dot_operation::contraction3d3d",
                generate_error_message("shape-mismatch for sum"));

        auto t1 = lhs.tensor();
        auto t2 = rhs.tensor();
        blaze::DynamicMatrix<T> result(t1.pages(), t2.columns(), T(0));
        for (std::size_t i = 0UL; i != t1.pages(); ++i)
        {
            auto slice1 = blaze::pageslice(t1, i);
            for (std::size_t j = 0UL; j != t2.columns(); ++j)
            {
                auto slice2 = blaze::columnslice(t2, j);
                result(i, j) =
                    contraction2d2d(std::move(slice1), std::move(slice2));
            }
        }
        return primitive_argument_type{std::move(result)};
    }


    template <typename T>
    primitive_argument_type dot_operation::contraction3d(
        ir::node_data<T>&& lhs, ir::node_data<T>&& rhs) const
    {
        switch (rhs.num_dimensions())
        {
        case 2:
            // If is_tensor(lhs) && is_matrix(rhs) -> a vector
            return contraction3d2d(std::move(lhs), std::move(rhs));

        case 3:
            // If is_tensor(lhs) && is_tensor(rhs) -> a matrix
            return contraction3d3d(std::move(lhs), std::move(rhs));


        default:
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "dot_operation::contraction3d",
                generate_error_message("the left operand has >3 dimensions "
                                       "which is not supported"));
        }
    }
#endif

    ///////////////////////////////////////////////////////////////////////////
    template <typename T>
    primitive_argument_type dot_operation::tensordot2d2d_0_1(
        ir::node_data<T>&& lhs, ir::node_data<T>&& rhs) const
    {
        if (lhs.dimension(0) != rhs.dimension(1))
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "dot_operation::tensordot2d2d_0_1",
                generate_error_message(
                    "the operands have incompatible number of dimensions"));
        }

        lhs = blaze::trans(rhs.matrix() * lhs.matrix());
        return primitive_argument_type{std::move(lhs)};
    }

#if defined(PHYLANX_HAVE_BLAZE_TENSOR)
    template <typename T>
    primitive_argument_type dot_operation::tensordot1d3d_0_0(
        ir::node_data<T>&& lhs, ir::node_data<T>&& rhs) const
    {
        if (lhs.size() != rhs.dimension(0))
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "dot_operation::tensordot1d3d_0_0",
                generate_error_message(
                    "the operands have incompatible number of dimensions"));
        }

        auto t = rhs.tensor();
        blaze::DynamicMatrix<T> result(t.rows(), t.columns());

        for (std::size_t i = 0; i != t.rows(); ++i)
            blaze::row(blaze::submatrix(result, i, 0, 1, t.columns()), 0) =
                blaze::trans(blaze::rowslice(t, i) * (lhs.vector()));

        return primitive_argument_type{std::move(result)};
    }

    template <typename T>
    primitive_argument_type dot_operation::tensordot2d3d_0_0(
        ir::node_data<T>&& lhs, ir::node_data<T>&& rhs) const
    {
        if (lhs.dimension(0) != rhs.dimension(0))
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "dot_operation::tensordot2d3d_0_0",
                generate_error_message(
                    "the operands have incompatible number of dimensions"));
        }

        auto m = lhs.matrix();
        auto t = rhs.tensor();

        blaze::DynamicTensor<T> result(m.columns(), t.rows(), t.columns());

        for (std::size_t i = 0; i != t.rows(); ++i)
            blaze::rowslice(
                blaze::subtensor(result, 0, i, 0, m.columns(), 1, t.columns()),
                0) = blaze::rowslice(t, i) * m;

        return primitive_argument_type{std::move(result)};
    }

    template <typename T>
    primitive_argument_type dot_operation::tensordot2d3d_0_1(
        ir::node_data<T>&& lhs, ir::node_data<T>&& rhs) const
    {
        if (lhs.dimension(0) != rhs.dimension(1))
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "dot_operation::tensordot2d3d_0_1",
                generate_error_message(
                    "the operands have incompatible number of dimensions"));
        }

        auto m = lhs.matrix();
        auto t = rhs.tensor();

        blaze::DynamicTensor<T> result(m.columns(), t.pages(), t.columns());

        for (std::size_t i = 0; i != t.columns(); ++i)
            blaze::columnslice(
                blaze::subtensor(result, 0, 0, i, m.columns(), t.pages(), 1),
                0) = blaze::trans(blaze::columnslice(t, i) * m);

        return primitive_argument_type{std::move(result)};
    }

    template <typename T>
    primitive_argument_type dot_operation::tensordot2d3d_0_2(
        ir::node_data<T>&& lhs, ir::node_data<T>&& rhs) const
    {
        if (lhs.dimension(0) != rhs.dimension(2))
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "dot_operation::tensordot2d3d_0_2",
                generate_error_message(
                    "the operands have incompatible number of dimensions"));
        }

        auto m = lhs.matrix();
        auto t = rhs.tensor();

        blaze::DynamicTensor<T> result(m.columns(), t.pages(), t.rows());

        for (std::size_t i = 0; i != t.pages(); ++i)
            blaze::rowslice(
                blaze::subtensor(result, 0, i, 0, m.columns(), 1, t.rows()),
                0) = blaze::pageslice(t, i) * m;

        return primitive_argument_type{std::move(result)};
    }

    template <typename T>
    primitive_argument_type dot_operation::tensordot2d3d_1_0(
        ir::node_data<T>&& lhs, ir::node_data<T>&& rhs) const
    {
        if (lhs.dimension(1) != rhs.dimension(0))
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "dot_operation::tensordot2d3d_1_0",
                generate_error_message(
                    "the operands have incompatible number of dimensions"));
        }

        auto m = lhs.matrix();
        auto t = rhs.tensor();

        blaze::DynamicTensor<T> result(m.rows(), t.rows(), t.columns());

        for (std::size_t i = 0; i != t.columns(); ++i)
            blaze::columnslice(
                blaze::subtensor(result, 0, 0, i, m.rows(), t.rows(), 1), 0) =
                m * blaze::columnslice(t, i);

        return primitive_argument_type{std::move(result)};
    }

    template <typename T>
    primitive_argument_type dot_operation::tensordot2d3d_1_2(
        ir::node_data<T>&& lhs, ir::node_data<T>&& rhs) const
    {
        if (lhs.dimension(1) != rhs.dimension(2))
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "dot_operation::tensordot2d3d_1_2",
                generate_error_message(
                    "the operands have incompatible number of dimensions"));
        }

        auto m = lhs.matrix();
        auto t = rhs.tensor();

        blaze::DynamicTensor<T> result(m.rows(), t.pages(), t.rows());

        for (std::size_t i = 0; i != t.rows(); ++i)
            blaze::columnslice(
                blaze::subtensor(result, 0, 0, i, m.rows(), t.pages(), 1), 0) =
                m * blaze::rowslice(t, i);

        return primitive_argument_type{std::move(result)};
    }

    template <typename T>
    primitive_argument_type dot_operation::tensordot3d2d_0_0(
        ir::node_data<T>&& lhs, ir::node_data<T>&& rhs) const
    {
        if (lhs.dimension(0) != rhs.dimension(0))
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "dot_operation::tensordot3d2d_0_0",
                generate_error_message(
                    "the operands have incompatible number of dimensions"));
        }

        auto m = rhs.matrix();
        auto t = lhs.tensor();

        blaze::DynamicTensor<T> result(t.rows(), t.columns(), m.columns());

        for (std::size_t i = 0; i != t.rows(); ++i)
            blaze::pageslice(
                blaze::subtensor(result, i, 0, 0, 1, t.columns(), m.columns()),
                0) = blaze::rowslice(t, i) * m;

        return primitive_argument_type{std::move(result)};
    }

    template <typename T>
    primitive_argument_type dot_operation::tensordot3d2d_0_1(
        ir::node_data<T>&& lhs, ir::node_data<T>&& rhs) const
    {
        if (lhs.dimension(0) != rhs.dimension(1))
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "dot_operation::tensordot3d2d_0_1",
                generate_error_message(
                    "the operands have incompatible number of dimensions"));
        }

        auto m = rhs.matrix();
        auto t = lhs.tensor();

        blaze::DynamicTensor<T> result(t.rows(), t.columns(), m.rows());

        for (std::size_t i = 0; i != t.columns(); ++i)
            blaze::rowslice(
                blaze::subtensor(result, 0, i, 0, t.rows(), 1, m.rows()), 0) =
                m * blaze::columnslice(t, i);

        return primitive_argument_type{std::move(result)};
    }

    template <typename T>
    primitive_argument_type dot_operation::tensordot3d2d_1_0(
        ir::node_data<T>&& lhs, ir::node_data<T>&& rhs) const
    {
        if (lhs.dimension(1) != rhs.dimension(0))
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "dot_operation::tensordot3d2d_1_0",
                generate_error_message(
                    "the operands have incompatible number of dimensions"));
        }

        auto m = rhs.matrix();
        auto t = lhs.tensor();

        blaze::DynamicTensor<T> result(t.pages(), t.columns(), m.columns());

        for (std::size_t i = 0; i != t.columns(); ++i)
            blaze::rowslice(
                blaze::subtensor(result, 0, i, 0, t.pages(), 1, m.columns()),
                0) = blaze::trans(blaze::columnslice(t, i) * m);

        return primitive_argument_type{std::move(result)};
    }

    template <typename T>
    primitive_argument_type dot_operation::tensordot3d2d_1_1(
        ir::node_data<T>&& lhs, ir::node_data<T>&& rhs) const
    {
        if (lhs.dimension(1) != rhs.dimension(1))
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "dot_operation::tensordot3d2d_1_1",
                generate_error_message(
                    "the operands have incompatible number of dimensions"));
        }

        auto m = rhs.matrix();
        auto t = lhs.tensor();

        blaze::DynamicTensor<T> result(t.pages(), t.columns(), m.rows());

        for (std::size_t i = 0; i != t.pages(); ++i)
            blaze::pageslice(
                blaze::subtensor(result, i, 0, 0, 1, t.columns(), m.rows()),
                0) = blaze::trans(m * blaze::pageslice(t, i));

        return primitive_argument_type{std::move(result)};
    }

    template <typename T>
    primitive_argument_type dot_operation::tensordot3d2d_2_1(
        ir::node_data<T>&& lhs, ir::node_data<T>&& rhs) const
    {
        if (lhs.dimension(2) != rhs.dimension(1))
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "dot_operation::tensordot3d2d_2_1",
                generate_error_message(
                    "the operands have incompatible number of dimensions"));
        }

        auto m = rhs.matrix();
        auto t = lhs.tensor();

        blaze::DynamicTensor<T> result(t.pages(), t.rows(), m.rows());

        for (std::size_t i = 0; i != t.pages(); ++i)
            blaze::pageslice(
                blaze::subtensor(result, i, 0, 0, 1, t.rows(), m.rows()), 0) =
                blaze::pageslice(t, i) * blaze::trans(m);

        return primitive_argument_type{std::move(result)};
    }
#endif

    ///////////////////////////////////////////////////////////////////////////
    template <typename T>
    primitive_argument_type dot_operation::tensordot_range_of_scalars(
        ir::node_data<T>&& lhs, ir::node_data<T>&& rhs, val_type axis_a,
        val_type axis_b) const
    {
        std::size_t a_dims =
            extract_numeric_value_dimension(lhs, name_, codename_);
        std::size_t b_dims =
            extract_numeric_value_dimension(rhs, name_, codename_);

        if (a_dims == 0 || b_dims == 0)
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "dot_operation::tensordot_range_of_scalars",
                generate_error_message("tuple index out of range. No axis is "
                                       "defined for a 0-d array"));

        if (a_dims == 1)
        {
            if (b_dims == 1)
            {
                if (axis_a == 0 && axis_b == 0)
                    return dot1d1d(std::move(lhs), std::move(rhs));

                else
                    HPX_THROW_EXCEPTION(hpx::bad_parameter,
                        "dot_operation::tensordot_range_of_scalars",
                        generate_error_message(
                            "tuple of axes is out of range. For both vectors, "
                            "axes can be -1 or 0"));
            }
            else if (b_dims == 2)
            {
                if (axis_a == 0 && axis_b == 0)
                    return dot1d2d(std::move(lhs), std::move(rhs));

                else if(axis_a == 0 && axis_b == 1)
                    return dot2d1d(std::move(rhs), std::move(lhs));

                HPX_THROW_EXCEPTION(hpx::bad_parameter,
                    "dot_operation::tensordot_range_of_scalars",
                    generate_error_message(
                        "tuple of axes is out of range. For the left hand side "
                        "vector axis can be 0 or -1 and for the right hand "
                        "side matrix axis can be between -2 and 1"));
            }
#if defined(PHYLANX_HAVE_BLAZE_TENSOR)
            else if (b_dims == 3)
            {
                if (axis_a == 0 && axis_b == 0)
                    return tensordot1d3d_0_0(std::move(lhs), std::move(rhs));

                else if(axis_a == 0 && axis_b == 1)
                    return dot1d3d(std::move(lhs), std::move(rhs));

                else if(axis_a == 0 && axis_b == 2)
                    return dot3d1d(std::move(rhs), std::move(lhs));

                HPX_THROW_EXCEPTION(hpx::bad_parameter,
                    "dot_operation::tensordot_range_of_scalars",
                    generate_error_message(
                        "tuple of axes is out of range. For the left hand side "
                        "vector axis can be 0 or -1 and for the right hand "
                        "side tensor axis can be between -3 and 2"));
            }
#endif
        }
        else if(a_dims==2)
        {
            if (b_dims == 1)
            {
                if (axis_a == 0 && axis_b == 0)
                    return dot1d2d(std::move(rhs), std::move(lhs));

                else if (axis_a == 1 && axis_b == 0)
                    return dot2d1d(std::move(lhs), std::move(rhs));

                HPX_THROW_EXCEPTION(hpx::bad_parameter,
                    "dot_operation::tensordot_range_of_scalars",
                    generate_error_message(
                        "tuple of axes is out of range. For the left hand side "
                        "matrix axis can be between -2 and 1 and for the right "
                        "hand side vector axis can be -1 or 0"));
            }
            else if (b_dims == 2)
            {
                if (axis_a == 0 && axis_b == 0)
                    return dot2d2d(blaze::trans(lhs.matrix()), rhs.matrix());

                else if(axis_a == 0 && axis_b == 1)
                    return tensordot2d2d_0_1(std::move(lhs), std::move(rhs));

                else if(axis_a == 1 && axis_b == 0)
                    return dot2d2d(lhs.matrix(), rhs.matrix());

                else if(axis_a == 1 && axis_b == 1)
                    return dot2d2d(lhs.matrix(), blaze::trans(rhs.matrix()));

                HPX_THROW_EXCEPTION(hpx::bad_parameter,
                    "dot_operation::tensordot_range_of_scalars",
                    generate_error_message("tuple of axes is out of range. For "
                        "both matrices axes can be between "
                        "-2 and 1"));
            }
#if defined(PHYLANX_HAVE_BLAZE_TENSOR)
            else if (b_dims == 3)
            {
                if (axis_a == 0 && axis_b == 0)
                    return tensordot2d3d_0_0(std::move(lhs), std::move(rhs));

                else if(axis_a == 0 && axis_b == 1)
                    return tensordot2d3d_0_1(std::move(lhs), std::move(rhs));

                else if(axis_a == 0 && axis_b == 2)
                    return tensordot2d3d_0_2(std::move(lhs), std::move(rhs));

                else if(axis_a == 1 && axis_b == 0)
                    return tensordot2d3d_1_0(std::move(lhs), std::move(rhs));

                else if(axis_a == 1 && axis_b == 1)
                    return dot2d3d(std::move(lhs), std::move(rhs));

                else if(axis_a == 1 && axis_b == 2)
                    return tensordot2d3d_1_2(std::move(lhs), std::move(rhs));

                HPX_THROW_EXCEPTION(hpx::bad_parameter,
                    "dot_operation::tensordot_range_of_scalars",
                    generate_error_message(
                        "tuple of axes is out of range. For the left hand side "
                        "matrix axis can be between -2 and 1 and for the right "
                        "hand side tensor axis can be between -3 and 2"));
            }
#endif
        }
#if defined(PHYLANX_HAVE_BLAZE_TENSOR)
        else if(a_dims==3)
        {
            if (b_dims == 1)
            {
                if (axis_a == 0 && axis_b == 0)
                    return tensordot1d3d_0_0(std::move(rhs), std::move(lhs));

                else if (axis_a == 1 && axis_b == 0)
                    return dot1d3d(std::move(rhs), std::move(lhs));

                else if (axis_a == 2 && axis_b == 0)
                    return dot3d1d(std::move(lhs), std::move(rhs));

                HPX_THROW_EXCEPTION(hpx::bad_parameter,
                    "dot_operation::tensordot_range_of_scalars",
                    generate_error_message(
                        "tuple of axes is out of range. For the left hand side "
                        "tensor axis can be between -3 and 2 and for the right "
                        "hand side vector axis can be -1 or 0"));
            }
            else if (b_dims == 2)
            {
                if (axis_a == 0 && axis_b == 0)
                    return tensordot3d2d_0_0(std::move(lhs), std::move(rhs));

                else if(axis_a == 0 && axis_b == 1)
                    return tensordot3d2d_0_1(std::move(lhs), std::move(rhs));

                else if(axis_a == 1 && axis_b == 0)
                    return tensordot3d2d_1_0(std::move(lhs), std::move(rhs));

                else if(axis_a == 1 && axis_b == 1)
                    return tensordot3d2d_1_1(std::move(lhs), std::move(rhs));

                else if(axis_a == 2 && axis_b == 0)
                    return dot3d2d(std::move(lhs), std::move(rhs));

                else if(axis_a == 2 && axis_b == 1)
                    return tensordot3d2d_2_1(std::move(lhs), std::move(rhs));

                HPX_THROW_EXCEPTION(hpx::bad_parameter,
                    "dot_operation::tensordot_range_of_scalars",
                    generate_error_message(
                        "tuple of axes is out of range. For the left hand side "
                        "tensor axis can be between -3 and 2 and for the right "
                        "hand side matrix axis can be between -2 and 1"));
            }
            else
            {
                HPX_THROW_EXCEPTION(hpx::bad_parameter,
                    "dot_operation::tensordot_range_of_scalars",
                    generate_error_message(
                        "the right hand side has >=3 dimensions "
                        "which results in >=4 dimensions having a 3d left hand "
                        "side operand. This is not supported"));
            }
        }
#endif
        HPX_THROW_EXCEPTION(hpx::bad_parameter,
            "dot_operation::tensordot_range_of_scalars",
            generate_error_message(
                "operands with >3 dimensions are not supported"));
    }
}}}

#endif
