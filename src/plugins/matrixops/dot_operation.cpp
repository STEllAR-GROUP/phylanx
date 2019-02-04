//  Copyright (c) 2017-2018 Hartmut Kaiser
//  Copyright (c) 2017 Parsa Amini
//  Copyright (c) 2019 Bita Hasheminezhad
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

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
#include <utility>
#include <vector>

#if defined(PHYLANX_HAVE_BLAZE_TENSOR)
#include <blaze_tensor/Math.h>
#endif
///////////////////////////////////////////////////////////////////////////////
namespace phylanx { namespace execution_tree { namespace primitives
{
    ///////////////////////////////////////////////////////////////////////////
    std::vector<match_pattern_type> const dot_operation::match_data = {
        match_pattern_type{"outer", std::vector<std::string>{"outer(_1, _2)"},
            &create_dot_operation, &create_primitive<dot_operation>, R"(
            a, b
            Args:

                a (array) : a scalar, vector, matrix or a tensor.Input is
                    flattened if not already 1-dimensional.
                b (array) : a scalar, vector, matrix or a tensor.Input is
                    flattened if not already 1-dimensional.

            Returns:

            Computes the outer product of two arrays. Always returns a matrix)"},

        match_pattern_type{"dot", std::vector<std::string>{"dot(_1, _2)"},
            &create_dot_operation, &create_primitive<dot_operation>, R"(
            a, b
            Args:

                a (array) : a scalar, vector, matrix or a tensor
                b (array) : a scalar, vector, matrix or a tensor

            Returns:

            The dot product of two arrays: `a` and `b`. The dot product of an
            N-D array and an M-D array is of dimension N+M-2)"},

        match_pattern_type{"tensordot",
            std::vector<std::string>{
                "tensordot(_1, _2)", "tensordot(_1, _2,_3)"},
            &create_dot_operation, &create_primitive<dot_operation>, R"(
            a, b, axes
            Args:

                a (array) : a vector, matrix or a tensor
                b (array) : a vector, matrix or a tensor
                axes(optional, integer or tuple of integers): if a scalar N, sum
                    over the last N axes of a and the first N axes of b in
                    order. The sizes of the corresponding axes must match.

            Returns:

            The tensor dot product along specified axes for arrays>=1-D.)"}};

    ///////////////////////////////////////////////////////////////////////////
    dot_operation::dot_mode extract_dot_mode(std::string const& name)
    {
        dot_operation::dot_mode result = dot_operation::dot_product;

        if (name.find("outer") != std::string::npos)
        {
            result = dot_operation::outer_product;
        }
        else if (name.find("tensordot") != std::string::npos)
        {
            result = dot_operation::doubledot_product;
        }
        return result;
    }

    dot_operation::dot_operation(
            primitive_arguments_type&& operands,
            std::string const& name, std::string const& codename)
      : primitive_component_base(std::move(operands), name, codename)
        , mode_(extract_dot_mode(name_))
    {}

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

    template <typename T>
    primitive_argument_type dot_operation::dot2d2d(
        ir::node_data<T>&& lhs, ir::node_data<T>&& rhs) const
    {
        if (lhs.dimension(1) != rhs.dimension(0))
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "dot_operation::dot2d2d",
                generate_error_message(
                    "the operands have incompatible number of dimensions"));
        }

        lhs = lhs.matrix() * rhs.matrix();
        return primitive_argument_type{std::move(lhs)};
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
            return dot2d2d(std::move(lhs), std::move(rhs));

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
    template <typename T>
    primitive_argument_type dot_operation::outer1d1d(
        ir::node_data<T>&& lhs, ir::node_data<T>&& rhs) const
    {
        blaze::DynamicMatrix<T> result =
            blaze::outer(lhs.vector(), rhs.vector());

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
            //slice = blaze::trans(blaze::outer(v, blaze::row(m, i)));
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
            return outer1d1d(std::move(lhs), std::move(rhs));

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
    primitive_argument_type dot_operation::contraction2d2d(
        ir::node_data<T>&& lhs, ir::node_data<T>&& rhs) const
    {
        if (lhs.dimension(0) != rhs.dimension(0) ||
            lhs.dimension(1) != rhs.dimension(1))
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "dot_operation::contraction2d2d",
                generate_error_message("shape-mismatch for sum"));
        }

        HPX_THROW_EXCEPTION(hpx::bad_parameter,
            "dot_operation::contraction2d2d",
            generate_error_message(" ravel "));
    }

#if defined(PHYLANX_HAVE_BLAZE_TENSOR)
    template <typename T>
    primitive_argument_type dot_operation::contraction2d3d(
        ir::node_data<T>&& lhs, ir::node_data<T>&& rhs) const
    {
        if (lhs.dimension(0) != rhs.dimension(0) ||
            lhs.dimension(1) != rhs.dimension(1))
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "dot_operation::contraction2d3d",
                generate_error_message("shape-mismatch for sum"));
        }

        HPX_THROW_EXCEPTION(hpx::bad_parameter,
            "dot_operation::contraction2d3d",
            generate_error_message(" depends on  contraction2d2d "));
        auto t = rhs.tensor();
        blaze::DynamicVector<T> result(t.columns());
        for (std::size_t i = 0; i != t.columns(); ++i)
        {
            auto slice = blaze::columnslice(t, i);
            //result[i] = contraction2d2d(slice, lhs.matrix());
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
            return contraction2d2d(std::move(lhs), std::move(rhs));

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
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "dot_operation::contraction3d2d",
                generate_error_message("shape-mismatch for sum"));
        }

        HPX_THROW_EXCEPTION(hpx::bad_parameter,
            "dot_operation::contraction3d2d",
            generate_error_message(" depends on contraction2d2d "));
        auto t = lhs.tensor();
        blaze::DynamicVector<T> result(t.pages());
        for (std::size_t i = 0; i != t.pages(); ++i)
        {
            auto slice = blaze::pageslice(t, i);
            //result[i] = contraction2d2d(slice, rhs.matrix());
        }
        return primitive_argument_type{std::move(result)};
    }


    template <typename T>
    primitive_argument_type dot_operation::contraction3d3d(
        ir::node_data<T>&& lhs, ir::node_data<T>&& rhs) const
    {
        if (lhs.dimension(0) != rhs.dimension(0) ||
            lhs.dimension(1) != rhs.dimension(1))
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "dot_operation::contraction3d3d",
                generate_error_message("shape-mismatch for sum"));
        }

        HPX_THROW_EXCEPTION(hpx::bad_parameter,
            "dot_operation::contraction3d3d",
            generate_error_message("  "));

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
    primitive_argument_type dot_operation::dot0d(
        primitive_argument_type&& lhs, primitive_argument_type&& rhs) const
    {
        switch (extract_common_type(lhs, rhs))
        {
        case node_data_type_bool:
            return dot0d(
                extract_boolean_value(std::move(lhs), name_, codename_),
                extract_boolean_value(std::move(rhs), name_, codename_));

        case node_data_type_int64:
            return dot0d(
                extract_integer_value(std::move(lhs), name_, codename_),
                extract_integer_value(std::move(rhs), name_, codename_));

        case node_data_type_unknown: HPX_FALLTHROUGH;
        case node_data_type_double:
            return dot0d(
                extract_numeric_value(std::move(lhs), name_, codename_),
                extract_numeric_value(std::move(rhs), name_, codename_));

        default:
            break;
        }

        HPX_THROW_EXCEPTION(hpx::bad_parameter,
            "dot_operation::dot0d",
            generate_error_message(
                "the dot primitive requires for all arguments to "
                    "be numeric data types"));
    }

    primitive_argument_type dot_operation::dot1d(
        primitive_argument_type&& lhs, primitive_argument_type&& rhs) const
    {
        switch (extract_common_type(lhs, rhs))
        {
        case node_data_type_bool:
            return dot1d(
                extract_boolean_value(std::move(lhs), name_, codename_),
                extract_boolean_value(std::move(rhs), name_, codename_));

        case node_data_type_int64:
            return dot1d(
                extract_integer_value(std::move(lhs), name_, codename_),
                extract_integer_value(std::move(rhs), name_, codename_));

        case node_data_type_unknown: HPX_FALLTHROUGH;
        case node_data_type_double:
            return dot1d(
                extract_numeric_value(std::move(lhs), name_, codename_),
                extract_numeric_value(std::move(rhs), name_, codename_));

        default:
            break;
        }

        HPX_THROW_EXCEPTION(hpx::bad_parameter,
            "dot_operation::dot1d",
            generate_error_message(
                "the dot primitive requires for all arguments to "
                    "be numeric data types"));
    }

    primitive_argument_type dot_operation::dot2d(
        primitive_argument_type&& lhs, primitive_argument_type&& rhs) const
    {
        switch (extract_common_type(lhs, rhs))
        {
        case node_data_type_bool:
            return dot2d(
                extract_boolean_value(std::move(lhs), name_, codename_),
                extract_boolean_value(std::move(rhs), name_, codename_));

        case node_data_type_int64:
            return dot2d(
                extract_integer_value(std::move(lhs), name_, codename_),
                extract_integer_value(std::move(rhs), name_, codename_));

        case node_data_type_unknown: HPX_FALLTHROUGH;
        case node_data_type_double:
            return dot2d(
                extract_numeric_value(std::move(lhs), name_, codename_),
                extract_numeric_value(std::move(rhs), name_, codename_));

        default:
            break;
        }

        HPX_THROW_EXCEPTION(hpx::bad_parameter,
            "dot_operation::dot2d",
            generate_error_message(
                "the dot primitive requires for all arguments to "
                    "be numeric data types"));
    }

#if defined(PHYLANX_HAVE_BLAZE_TENSOR)
    primitive_argument_type dot_operation::dot3d(
        primitive_argument_type&& lhs, primitive_argument_type&& rhs) const
    {
        switch (extract_common_type(lhs, rhs))
        {
        case node_data_type_bool:
            return dot3d(
                extract_boolean_value(std::move(lhs), name_, codename_),
                extract_boolean_value(std::move(rhs), name_, codename_));

        case node_data_type_int64:
            return dot3d(
                extract_integer_value(std::move(lhs), name_, codename_),
                extract_integer_value(std::move(rhs), name_, codename_));

        case node_data_type_unknown: HPX_FALLTHROUGH;
        case node_data_type_double:
            return dot3d(
                extract_numeric_value(std::move(lhs), name_, codename_),
                extract_numeric_value(std::move(rhs), name_, codename_));

        default:
            break;
        }

        HPX_THROW_EXCEPTION(hpx::bad_parameter,
            "dot_operation::dot3d",
            generate_error_message(
                "the dot primitive requires for all arguments to "
                "be numeric data types"));
    }
#endif

    primitive_argument_type dot_operation::outer1d(
        primitive_argument_type&& lhs, primitive_argument_type&& rhs) const
    {
        switch (extract_common_type(lhs, rhs))
        {
        case node_data_type_bool:
            return outer1d(
                extract_boolean_value(std::move(lhs), name_, codename_),
                extract_boolean_value(std::move(rhs), name_, codename_));

        case node_data_type_int64:
            return outer1d(
                extract_integer_value(std::move(lhs), name_, codename_),
                extract_integer_value(std::move(rhs), name_, codename_));

        case node_data_type_unknown:
            HPX_FALLTHROUGH;
        case node_data_type_double:
            return outer1d(
                extract_numeric_value(std::move(lhs), name_, codename_),
                extract_numeric_value(std::move(rhs), name_, codename_));

        default:
            break;
        }

        HPX_THROW_EXCEPTION(hpx::bad_parameter,
            "dot_operation::outer1d",
            generate_error_message(
                "the dot_operation primitive requires for all arguments to "
                "be numeric data types"));
    }

    primitive_argument_type dot_operation::outer2d(
        primitive_argument_type&& lhs, primitive_argument_type&& rhs) const
    {
        switch (extract_common_type(lhs, rhs))
        {
        case node_data_type_bool:
            return outer2d(
                extract_boolean_value(std::move(lhs), name_, codename_),
                extract_boolean_value(std::move(rhs), name_, codename_));

        case node_data_type_int64:
            return outer2d(
                extract_integer_value(std::move(lhs), name_, codename_),
                extract_integer_value(std::move(rhs), name_, codename_));

        case node_data_type_unknown:
            HPX_FALLTHROUGH;
        case node_data_type_double:
            return outer2d(
                extract_numeric_value(std::move(lhs), name_, codename_),
                extract_numeric_value(std::move(rhs), name_, codename_));

        default:
            break;
        }

        HPX_THROW_EXCEPTION(hpx::bad_parameter,
            "dot_operation::outer2d",
            generate_error_message(
                "the dot_operation primitive requires for all arguments to "
                "be numeric data types"));
    }

#if defined(PHYLANX_HAVE_BLAZE_TENSOR)
    primitive_argument_type dot_operation::outer3d(
        primitive_argument_type&& lhs, primitive_argument_type&& rhs) const
    {
        switch (extract_common_type(lhs, rhs))
        {
        case node_data_type_bool:
            return outer3d(
                extract_boolean_value(std::move(lhs), name_, codename_),
                extract_boolean_value(std::move(rhs), name_, codename_));

        case node_data_type_int64:
            return outer3d(
                extract_integer_value(std::move(lhs), name_, codename_),
                extract_integer_value(std::move(rhs), name_, codename_));

        case node_data_type_unknown:
            HPX_FALLTHROUGH;
        case node_data_type_double:
            return outer3d(
                extract_numeric_value(std::move(lhs), name_, codename_),
                extract_numeric_value(std::move(rhs), name_, codename_));

        default:
            break;
        }

        HPX_THROW_EXCEPTION(hpx::bad_parameter,
            "dot_operation::outer3d",
            generate_error_message(
                "the dot_operation primitive requires for all arguments to "
                "be numeric data types"));
    }
#endif

    primitive_argument_type dot_operation::contraction2d(
        primitive_argument_type&& lhs, primitive_argument_type&& rhs) const
    {
        switch (extract_common_type(lhs, rhs))
        {
        case node_data_type_bool:
            return contraction2d(
                extract_boolean_value(std::move(lhs), name_, codename_),
                extract_boolean_value(std::move(rhs), name_, codename_));

        case node_data_type_int64:
            return contraction2d(
                extract_integer_value(std::move(lhs), name_, codename_),
                extract_integer_value(std::move(rhs), name_, codename_));

        case node_data_type_unknown:
            HPX_FALLTHROUGH;
        case node_data_type_double:
            return contraction2d(
                extract_numeric_value(std::move(lhs), name_, codename_),
                extract_numeric_value(std::move(rhs), name_, codename_));

        default:
            break;
        }

        HPX_THROW_EXCEPTION(hpx::bad_parameter,
            "dot_operation::contraction2d",
            generate_error_message(
                "the dot_operation primitive requires for all arguments to "
                "be numeric data types"));
    }

#if defined(PHYLANX_HAVE_BLAZE_TENSOR)
    primitive_argument_type dot_operation::contraction3d(
        primitive_argument_type&& lhs, primitive_argument_type&& rhs) const
    {
        switch (extract_common_type(lhs, rhs))
        {
        case node_data_type_bool:
            return contraction3d(
                extract_boolean_value(std::move(lhs), name_, codename_),
                extract_boolean_value(std::move(rhs), name_, codename_));

        case node_data_type_int64:
            return contraction3d(
                extract_integer_value(std::move(lhs), name_, codename_),
                extract_integer_value(std::move(rhs), name_, codename_));

        case node_data_type_unknown:
            HPX_FALLTHROUGH;
        case node_data_type_double:
            return contraction3d(
                extract_numeric_value(std::move(lhs), name_, codename_),
                extract_numeric_value(std::move(rhs), name_, codename_));

        default:
            break;
        }

        HPX_THROW_EXCEPTION(hpx::bad_parameter,
            "dot_operation::contraction3d",
            generate_error_message(
                "the dot_operation primitive requires for all arguments to "
                "be numeric data types"));
    }
#endif

    ///////////////////////////////////////////////////////////////////////////
    primitive_argument_type dot_operation::dot_nd(
        primitive_argument_type&& lhs, primitive_argument_type&& rhs) const
    {
        switch (extract_numeric_value_dimension(lhs, name_, codename_))
        {
        case 0:
            return dot0d(std::move(lhs), std::move(rhs));

        case 1:
            return dot1d(std::move(lhs), std::move(rhs));

        case 2:
            return dot2d(std::move(lhs), std::move(rhs));

#if defined(PHYLANX_HAVE_BLAZE_TENSOR)
        case 3:
            return dot3d(std::move(lhs), std::move(rhs));
#endif

        default:
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "dot_operation::dot_nd",
                generate_error_message("left hand side operand has unsupported "
                                       "number of dimensions"));
        }
    }

    primitive_argument_type dot_operation::outer_nd(
        primitive_argument_type&& lhs, primitive_argument_type&& rhs) const
    {
        switch (extract_numeric_value_dimension(lhs, name_, codename_))
        {
        case 0:
            //outer0d has the same functionality as dot0d
            return dot0d(std::move(lhs), std::move(rhs));

        case 1:
            return outer1d(std::move(lhs), std::move(rhs));

        case 2:
            return outer2d(std::move(lhs), std::move(rhs));

#if defined(PHYLANX_HAVE_BLAZE_TENSOR)
        case 3:
            return outer3d(std::move(lhs), std::move(rhs));
#endif

        default:
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "dot_operation::outer_nd",
                generate_error_message("left hand side operand has unsupported "
                                       "number of dimensions"));
        }
    }

    primitive_argument_type dot_operation::outer_nd_helper(
        primitive_argument_type&& lhs, primitive_argument_type&& rhs) const
    {
        if (extract_numeric_value_dimension(lhs, name_, codename_) < 2 &&
            extract_numeric_value_dimension(rhs, name_, codename_) < 2)
            return outer_nd(std::move(lhs), std::move(rhs));

        HPX_THROW_EXCEPTION(hpx::bad_parameter,
            "dot_operation::outer_nd_helper",
            generate_error_message("one of the operands has unsupported "
                                   "number of dimensions"));
    }

    primitive_argument_type dot_operation::contraction_nd(
        primitive_argument_type&& lhs, primitive_argument_type&& rhs) const
    {
        switch (extract_numeric_value_dimension(lhs, name_, codename_))
        {

        case 2:
            return contraction2d(std::move(lhs), std::move(rhs));

#if defined(PHYLANX_HAVE_BLAZE_TENSOR)
        case 3:
            return contraction3d(std::move(lhs), std::move(rhs));
#endif

        default:
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "dot_operation::contraction_nd",
                generate_error_message("left hand side operand has unsupported "
                                       "number of dimensions"));
        }
    }
    ///////////////////////////////////////////////////////////////////////////
    primitive_argument_type dot_operation::tensordot_scalar_axis(
        primitive_argument_type&& lhs, primitive_argument_type&& rhs,
        ir::range&& axes) const
    {
        auto axis = extract_scalar_integer_value_strict(*axes.begin());
        if (axis < 0)
            axis = 0;
        if (extract_numeric_value_dimension(lhs) < axis ||
            extract_numeric_value_dimension(rhs) < axis)
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "dot_operation::tensordot_scalar_axis",
                generate_error_message(
                    "the given axes should not be "
                    "greater than any of operands dimensions"));
        else
        {
            switch (axis)
            {
            case 0:
                return outer_nd(std::move(lhs), std::move(rhs));

            case 1:
                return dot_nd(std::move(lhs), std::move(rhs));

            case 2:
                return contraction_nd(std::move(lhs), std::move(rhs));

            default:
                HPX_THROW_EXCEPTION(hpx::bad_parameter,
                    "dot_operation::tensordot_scalar_axis",
                    generate_error_message("the given axes is out of range. A "
                                           "scalar axis should be <3"));
            }
        }
    }

    primitive_argument_type dot_operation::tensordot_range_axes(
        primitive_argument_type&& lhs, primitive_argument_type&& rhs,
        ir::range&& axes) const
    {
        auto it = axes.begin();
        auto axes_a_size = extract_list_value_size(*it);
        auto axes_b_size = extract_list_value_size(*++it);
        if (axes_a_size == 1 && axes_b_size == 1)
        {
            auto axis_b = extract_scalar_integer_value(*it);
            auto axis_a = extract_scalar_integer_value(*axes.begin());
            std::cout << axis_a << std::endl;
            std::cout << axis_b << std::endl;
        }
        HPX_THROW_EXCEPTION(hpx::bad_parameter,
            "dot_operation::tensordot_range_axes",
            generate_error_message("other cases"));
    }

    ///////////////////////////////////////////////////////////////////////////
    hpx::future<primitive_argument_type> dot_operation::eval(
        primitive_arguments_type const& operands,
        primitive_arguments_type const& args, eval_context ctx) const
    {
        if (operands.size() != 2 && operands.size() != 3)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "dot_operation::eval",
                generate_error_message(
                    "the dot_operation primitive requires exactly "
                        "two or three operands"));
        }

        if (!valid(operands[0]) || !valid(operands[1]))
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "dot_operation::eval",
                generate_error_message(
                    "the dot_operation primitive requires that the "
                        "arguments given by the operands array are valid"));
        }

        auto this_ = this->shared_from_this();
        if (operands.size() == 2)
        {
            return hpx::dataflow(hpx::launch::sync, hpx::util::unwrapping(
                [this_ = std::move(this_)](primitive_argument_type&& op1,
                    primitive_argument_type&& op2)
                ->primitive_argument_type
            {
                if (this_->mode_ == outer_product)

                    return this_->outer_nd_helper(
                        std::move(op1), std::move(op2));

                else if (this_->mode_ == dot_product)

                    return this_->dot_nd(std::move(op1), std::move(op2));

                else if (this_->mode_ == doubledot_product)

                    return this_->contraction_nd(
                        std::move(op1), std::move(op2));

                HPX_THROW_EXCEPTION(hpx::bad_parameter,
                    "dot_operation::eval",
                    this_->generate_error_message(
                        "unsupported dot mode requested"));
            }),
                value_operand(operands[0], args, name_, codename_, ctx),
                value_operand(operands[1], args, name_, codename_, ctx));
        }
        else if (operands.size() == 3 && valid(operands[2]))
        {
            if (this_->mode_ == dot_product || this_->mode_ == outer_product)
                HPX_THROW_EXCEPTION(hpx::bad_parameter,
                    "dot_operation::eval",
                    this_->generate_error_message(
                        "the dot/outer product requires exactly two operands"));
            else if (this_->mode_ == doubledot_product)
            {
                return hpx::dataflow(hpx::launch::sync,
                    hpx::util::unwrapping(
                        [this_ = std::move(this_)](
                            primitive_argument_type&& op1,
                            primitive_argument_type&& op2,
                            ir::range&& axes) -> primitive_argument_type {
                    switch (axes.size())
                    {
                    case 1:
                        return this_->tensordot_scalar_axis(
                            std::move(op1), std::move(op2), std::move(axes));

                    case 2:
                        return this_->tensordot_range_axes(
                            std::move(op1), std::move(op2), std::move(axes));

                    default:
                        HPX_THROW_EXCEPTION(hpx::bad_parameter,
                            "dot_operation::eval",
                            this_->generate_error_message(
                                "the axes can only be an integer, or a tuple "
                                "indicating a_axes and b_axes where a_axes and "
                                "b_axes can be integers or tuples of "
                                "integers"));
                    }

                        }),
                    value_operand(operands[0], args, name_, codename_, ctx),
                    value_operand(operands[1], args, name_, codename_, ctx),
                    list_operand(operands[2], args, name_, codename_, ctx));
            }
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "dot_operation::eval",
                this_->generate_error_message(
                    "unsupported dot mode requested"));
        }
        HPX_THROW_EXCEPTION(hpx::bad_parameter,
            "dot_operation::eval",
            generate_error_message(
                "the dot_operation primitive requires that the "
                "arguments given by the operands array are valid"));
    }
}}}
