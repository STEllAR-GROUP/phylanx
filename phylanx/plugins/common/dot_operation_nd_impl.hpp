//  Copyright (c) 2017-2019 Hartmut Kaiser
//  Copyright (c) 2017 Parsa Amini
//  Copyright (c) 2019 Bita Hasheminezhad
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(PHYLANX_MATRIXOPS_DOT_OPERATIONS_ND_IMPL_JUN_26_2019_1123AM)
#define PHYLANX_MATRIXOPS_DOT_OPERATIONS_ND_IMPL_JUN_26_2019_1123AM

#include <phylanx/config.hpp>
#include <phylanx/ir/node_data.hpp>
#include <phylanx/plugins/common/export_definitions.hpp>
#include <phylanx/plugins/common/dot_operation_nd.hpp>
#include <phylanx/util/generate_error_message.hpp>

#include <hpx/throw_exception.hpp>

#if defined(PHYLANX_HAVE_BLAZE_TENSOR)
#include <blaze_tensor/Math.h>
#endif

///////////////////////////////////////////////////////////////////////////////
namespace phylanx { namespace common
{
    ///////////////////////////////////////////////////////////////////////////
    template <typename T>
    PHYLANX_COMMON_EXPORT execution_tree::primitive_argument_type dot0d0d(
        ir::node_data<T>&& lhs, ir::node_data<T>&& rhs)
    {
        lhs.scalar() *= rhs.scalar();
        return execution_tree::primitive_argument_type{std::move(lhs)};
    }

    template <typename T>
    PHYLANX_COMMON_EXPORT execution_tree::primitive_argument_type dot0d1d(
        ir::node_data<T>&& lhs, ir::node_data<T>&& rhs)
    {
        if (rhs.is_ref())
        {
            rhs = rhs.vector() * lhs.scalar();
        }
        else
        {
            rhs.vector() *= lhs.scalar();
        }

        return execution_tree::primitive_argument_type{std::move(rhs)};
    }

    template <typename T>
    PHYLANX_COMMON_EXPORT execution_tree::primitive_argument_type dot0d2d(
        ir::node_data<T>&& lhs, ir::node_data<T>&& rhs)
    {
        if (rhs.is_ref())
        {
            rhs = rhs.matrix() * lhs.scalar();
        }
        else
        {
            rhs.matrix() *= lhs.scalar();
        }

        return execution_tree::primitive_argument_type{std::move(rhs)};
    }

#if defined(PHYLANX_HAVE_BLAZE_TENSOR)
    template <typename T>
    PHYLANX_COMMON_EXPORT execution_tree::primitive_argument_type dot0d3d(
        ir::node_data<T>&& lhs, ir::node_data<T>&& rhs)
    {
        if (rhs.is_ref())
        {
            rhs = rhs.tensor() * lhs.scalar();
        }
        else
        {
            rhs.tensor() *= lhs.scalar();
        }

        return execution_tree::primitive_argument_type{std::move(rhs)};
    }
#endif

    template <typename T>
    PHYLANX_COMMON_EXPORT execution_tree::primitive_argument_type dot0d(
        ir::node_data<T>&& lhs, ir::node_data<T>&& rhs, std::string const& name,
        std::string const& codename)
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
                util::generate_error_message(
                    "the operands have incompatible number of dimensions",
                    name, codename));
        }
    }

    ///////////////////////////////////////////////////////////////////////////
    template <typename T>
    PHYLANX_COMMON_EXPORT execution_tree::primitive_argument_type dot1d0d(
        ir::node_data<T>&& lhs, ir::node_data<T>&& rhs)
    {
        if (lhs.is_ref())
        {
            lhs = lhs.vector() * rhs.scalar();
        }
        else
        {
            lhs.vector() *= rhs.scalar();
        }

        return execution_tree::primitive_argument_type{std::move(lhs)};
    }

    template <typename T>
    PHYLANX_COMMON_EXPORT execution_tree::primitive_argument_type dot1d1d(
        ir::node_data<T>&& lhs, ir::node_data<T>&& rhs, std::string const& name,
        std::string const& codename)
    {
        if (lhs.size() != rhs.size())
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "dot1d1d",
                util::generate_error_message(
                    "the operands have incompatible number of dimensions",
                    name, codename));
        }

        // lhs.dimension(0) == rhs.dimension(0)
        lhs = T(blaze::dot(lhs.vector(), rhs.vector()));
        return execution_tree::primitive_argument_type{std::move(lhs)};
    }

    template <typename T>
    PHYLANX_COMMON_EXPORT execution_tree::primitive_argument_type dot1d2d(
        ir::node_data<T>&& lhs, ir::node_data<T>&& rhs, std::string const& name,
        std::string const& codename)
    {
        if (lhs.size() != rhs.dimension(0))
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "dot1d2d",
                util::generate_error_message(
                    "the operands have incompatible number of dimensions",
                    name, codename));
        }
        // lhs = blaze::trans(rhs.matrix()) * lhs.vector();
        lhs = blaze::trans(blaze::trans(lhs.vector()) * rhs.matrix());
        return execution_tree::primitive_argument_type{std::move(lhs)};
    }

#if defined(PHYLANX_HAVE_BLAZE_TENSOR)
    template <typename T>
    PHYLANX_COMMON_EXPORT execution_tree::primitive_argument_type dot1d3d(
        ir::node_data<T>&& lhs, ir::node_data<T>&& rhs, std::string const& name,
        std::string const& codename)
    {
        if (lhs.size() != rhs.dimension(1))
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "dot1d3d",
                util::generate_error_message(
                    "the operands have incompatible number of dimensions",
                    name, codename));
        }

        auto t = rhs.tensor();
        blaze::DynamicMatrix<T> result(t.pages(), t.columns());

        for (std::size_t i = 0; i != t.pages(); ++i)
            blaze::row(blaze::submatrix(result, i, 0, 1, t.columns()), 0) =
                blaze::trans(lhs.vector()) * blaze::pageslice(t, i);

        return execution_tree::primitive_argument_type{std::move(result)};
    }
#endif

    // lhs_num_dims == 1
    template <typename T>
    PHYLANX_COMMON_EXPORT execution_tree::primitive_argument_type dot1d(
        ir::node_data<T>&& lhs, ir::node_data<T>&& rhs, std::string const& name,
        std::string const& codename)
    {
        switch (rhs.num_dimensions())
        {
        case 0:
            // If is_vector(lhs) && is_scalar(rhs)
            return dot1d0d(std::move(lhs), std::move(rhs));

        case 1:
            // If is_vector(lhs) && is_vector(rhs)
            return dot1d1d(std::move(lhs), std::move(rhs), name, codename);

        case 2:
            // If is_vector(lhs) && is_matrix(rhs)
            return dot1d2d(std::move(lhs), std::move(rhs), name, codename);

#if defined(PHYLANX_HAVE_BLAZE_TENSOR)
        case 3:
            // If is_vector(lhs) && is_tensor(rhs)
            return dot1d3d(std::move(lhs), std::move(rhs), name, codename);
#endif

        default:
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "dot_operation::dot1d",
                util::generate_error_message(
                    "the operands have incompatible number of dimensions",
                    name, codename));
        }
    }

    ///////////////////////////////////////////////////////////////////////////
    template <typename T>
    PHYLANX_COMMON_EXPORT execution_tree::primitive_argument_type dot2d0d(
        ir::node_data<T>&& lhs, ir::node_data<T>&& rhs)
    {
        if (lhs.is_ref())
        {
            lhs = lhs.matrix() * rhs.scalar();
        }
        else
        {
            lhs.matrix() *= rhs.scalar();
        }
        return execution_tree::primitive_argument_type{std::move(lhs)};
    }

    template <typename T>
    PHYLANX_COMMON_EXPORT execution_tree::primitive_argument_type dot2d1d(
        ir::node_data<T>&& lhs, ir::node_data<T>&& rhs, std::string const& name,
        std::string const& codename)
    {
        if (lhs.dimension(1) != rhs.size())
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "dot2d1d",
                util::generate_error_message(
                    "the operands have incompatible number of dimensions",
                    name, codename));
        }

        rhs = lhs.matrix() * rhs.vector();
        return execution_tree::primitive_argument_type{std::move(rhs)};
    }

    template <typename Matrix1, typename Matrix2>
    PHYLANX_COMMON_EXPORT execution_tree::primitive_argument_type dot2d2d(
        Matrix1&& lhs, Matrix2&& rhs, std::string const& name,
        std::string const& codename)
    {
        if (lhs.columns() != rhs.rows())
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "dot2d2d",
                util::generate_error_message(
                    "the operands have incompatible number of dimensions",
                    name, codename));
        }
        using T = blaze::ElementType_t<typename std::decay<Matrix1>::type>;
        blaze::DynamicMatrix<T> result = lhs * rhs;
        return execution_tree::primitive_argument_type{std::move(result)};
    }

    template <typename T>
    PHYLANX_COMMON_EXPORT execution_tree::primitive_argument_type dot2d2d(
        ir::node_data<T>&& lhs, ir::node_data<T>&& rhs, std::string const& name,
        std::string const& codename)
    {
        return dot2d2d(lhs.matrix(), rhs.matrix(), name, codename);
    }

    template <typename T>
    PHYLANX_COMMON_EXPORT execution_tree::primitive_argument_type dot2dt2d(
        ir::node_data<T>&& lhs, ir::node_data<T>&& rhs, std::string const& name,
        std::string const& codename)
    {
        return dot2d2d(blaze::trans(lhs.matrix()), rhs.matrix(), name, codename);
    }

    template <typename T>
    PHYLANX_COMMON_EXPORT execution_tree::primitive_argument_type dot2d2dt(
        ir::node_data<T>&& lhs, ir::node_data<T>&& rhs, std::string const& name,
        std::string const& codename)
    {
        return dot2d2d(lhs.matrix(), blaze::trans(rhs.matrix()), name, codename);
    }

#if defined(PHYLANX_HAVE_BLAZE_TENSOR)
    template <typename T>
    PHYLANX_COMMON_EXPORT execution_tree::primitive_argument_type dot2d3d(
        ir::node_data<T>&& lhs, ir::node_data<T>&& rhs, std::string const& name,
        std::string const& codename)
    {
        if (lhs.dimension(1) != rhs.dimension(1))
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "dot2d3d",
                util::generate_error_message(
                    "the operands have incompatible number of dimensions",
                    name, codename));
        }

        auto m = lhs.matrix();
        auto t = rhs.tensor();

        blaze::DynamicTensor<T> result(m.rows(), t.pages(), t.columns());

        for (std::size_t i = 0; i != t.pages(); ++i)
            blaze::rowslice(
                blaze::subtensor(result, 0, i, 0, m.rows(), 1, t.columns()),
                0) = blaze::trans(m * blaze::pageslice(t, i));

        return execution_tree::primitive_argument_type{std::move(result)};
    }
#endif

    // lhs_num_dims == 2
    // Multiply a matrix with a vector
    // Regular matrix multiplication
    template <typename T>
    PHYLANX_COMMON_EXPORT execution_tree::primitive_argument_type dot2d(
        ir::node_data<T>&& lhs, ir::node_data<T>&& rhs, std::string const& name,
        std::string const& codename)
    {
        switch (rhs.num_dimensions())
        {
        case 0:
            // If is_matrix(lhs) && is_scalar(rhs)
            return dot2d0d(std::move(lhs), std::move(rhs));

        case 1:
            // If is_matrix(lhs) && is_vector(rhs)
            return dot2d1d(std::move(lhs), std::move(rhs), name, codename);

        case 2:
            // If is_matrix(lhs) && is_matrix(rhs)
            return dot2d2d(lhs.matrix(), rhs.matrix(), name, codename);

#if defined(PHYLANX_HAVE_BLAZE_TENSOR)
        case 3:
            // If is_matrix(lhs) && is_tensor(rhs)
            return dot2d3d(std::move(lhs), std::move(rhs), name, codename);
#endif

        default:
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "dot_operation::dot2d",
                util::generate_error_message(
                    "the operands have incompatible number of dimensions",
                    name, codename));
        }
    }

    ///////////////////////////////////////////////////////////////////////////
#if defined(PHYLANX_HAVE_BLAZE_TENSOR)
    // lhs_num_dims == 3
    template <typename T>
    PHYLANX_COMMON_EXPORT execution_tree::primitive_argument_type dot3d(
        ir::node_data<T>&& lhs, ir::node_data<T>&& rhs, std::string const& name,
        std::string const& codename)
    {
        switch (rhs.num_dimensions())
        {
        case 0:
            // If is_tensor(lhs) && is_scalar(rhs)
            return dot3d0d(std::move(lhs), std::move(rhs));

        case 1:
            // If is_tensor(lhs) && is_vector(rhs)
            return dot3d1d(
                std::move(lhs), std::move(rhs), name, codename);

        case 2:
            // If is_tensor(lhs) && is_matrix(rhs)
            return dot3d2d(
                std::move(lhs), std::move(rhs), name, codename);

        case 3:
            // If is_tensor(lhs) && is_tensor(rhs)
            return dot3d3d(
                std::move(lhs), std::move(rhs), name, codename);

        default:
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "dot_operation::dot3d",
                util::generate_error_message(
                    "the operands have incompatible number of dimensions",
                    name, codename));
        }
    }

    template <typename T>
    PHYLANX_COMMON_EXPORT execution_tree::primitive_argument_type dot3d0d(
        ir::node_data<T>&& lhs, ir::node_data<T>&& rhs)
    {
        if (lhs.is_ref())
        {
            lhs = lhs.tensor() * rhs.scalar();
        }
        else
        {
            lhs.tensor() *= rhs.scalar();
        }
        return execution_tree::primitive_argument_type{std::move(lhs)};
    }

    template <typename T>
    PHYLANX_COMMON_EXPORT execution_tree::primitive_argument_type dot3d1d(
        ir::node_data<T>&& lhs, ir::node_data<T>&& rhs, std::string const& name,
        std::string const& codename)
    {
        if (lhs.dimension(2) != rhs.size())
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "dot3d1d",
                util::generate_error_message(
                    "the operands have incompatible number of dimensions",
                    name, codename));
        }
        auto t = lhs.tensor();
        blaze::DynamicMatrix<T> result(t.pages(), t.rows());

        for (std::size_t i = 0; i != t.pages(); ++i)
            blaze::row(blaze::submatrix(result, i, 0, 1, t.rows()), 0) =
                blaze::trans(blaze::pageslice(t, i) * rhs.vector());

        return execution_tree::primitive_argument_type{std::move(result)};
    }

    template <typename T>
    PHYLANX_COMMON_EXPORT execution_tree::primitive_argument_type dot3d2d(
        ir::node_data<T>&& lhs, ir::node_data<T>&& rhs, std::string const& name,
        std::string const& codename)
    {
        if (lhs.dimension(2) != rhs.dimension(0))
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "dot3d2d",
                util::generate_error_message(
                    "the operands have incompatible number of dimensions",
                    name, codename));
        }

        auto m = rhs.matrix();
        auto t = lhs.tensor();

        blaze::DynamicTensor<T> result(t.pages(), t.rows(), m.columns());

        for (std::size_t i = 0; i != t.pages(); ++i)
            blaze::pageslice(
                blaze::subtensor(result, i, 0, 0, 1, t.rows(), m.columns()),
                0) = blaze::pageslice(t, i) * m;

        return execution_tree::primitive_argument_type{std::move(result)};
    }

    template <typename T>
    PHYLANX_COMMON_EXPORT execution_tree::primitive_argument_type dot3d3d(
        ir::node_data<T>&& lhs, ir::node_data<T>&& rhs, std::string const& name,
        std::string const& codename)
    {
        if (lhs.dimension(2) != rhs.dimension(1))
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "dot3d3d",
                util::generate_error_message(
                    "the operands have incompatible number of dimensions",
                    name, codename));
        }
        HPX_THROW_EXCEPTION(hpx::bad_parameter,
            "dot3d3d",
            util::generate_error_message(
                "it is not supported by Phylanx yet", name, codename));
    }
#endif
}}

#endif
