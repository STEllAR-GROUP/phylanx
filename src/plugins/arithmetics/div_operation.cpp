// Copyright (c) 2017-2018 Hartmut Kaiser
// Copyright (c) 2018 Parsa Amini
// Copyright (c) 2018 Shahrzad Shirzad
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>
#include <phylanx/ir/node_data.hpp>
#include <phylanx/plugins/arithmetics/div_operation.hpp>
#include <phylanx/util/detail/div_simd.hpp>

#include <hpx/include/lcos.hpp>
#include <hpx/include/naming.hpp>
#include <hpx/include/util.hpp>
#include <hpx/throw_exception.hpp>

#include <cstddef>
#include <memory>
#include <numeric>
#include <string>
#include <utility>
#include <vector>

#include <blaze/Math.h>

///////////////////////////////////////////////////////////////////////////////
namespace phylanx { namespace execution_tree { namespace primitives
{
    ///////////////////////////////////////////////////////////////////////////
    match_pattern_type const div_operation::match_data =
    {
        hpx::util::make_tuple("__div",
            std::vector<std::string>{"_1 / __2", "__div(_1, __2)"},
            &create_div_operation, &create_primitive<div_operation>)
    };

    ///////////////////////////////////////////////////////////////////////////
    div_operation::div_operation(std::vector<primitive_argument_type>&& operands,
            std::string const& name, std::string const& codename)
      : primitive_component_base(std::move(operands), name, codename)
    {}

    ///////////////////////////////////////////////////////////////////////////
    primitive_argument_type div_operation::div0d0d(
        operand_type&& lhs, operand_type&& rhs) const
    {
        lhs.scalar() /= rhs.scalar();
        return primitive_argument_type(std::move(lhs));
    }

    primitive_argument_type div_operation::div0d0d(operands_type && ops) const
    {
        operand_type& lhs = ops[0];
        operand_type& rhs = ops[1];

        return primitive_argument_type(std::accumulate(
            ops.begin() + 1, ops.end(), std::move(lhs),
            [](operand_type& result, operand_type const& curr)
            ->  operand_type
            {
                result[0] /= curr[0];
                return std::move(result);
            }));
    }

    primitive_argument_type div_operation::div0d1d(
        operand_type&& lhs, operand_type&& rhs) const
    {
        if (rhs.is_ref())
        {
            rhs = blaze::map(rhs.vector(),
                phylanx::util::detail::div0dnd_simd(lhs.scalar()));
        }
        else
        {
            rhs.vector() = blaze::map(rhs.vector(),
                phylanx::util::detail::div0dnd_simd(lhs.scalar()));
        }
        return primitive_argument_type(std::move(rhs));
    }

    primitive_argument_type div_operation::div0d2d(
        operand_type&& lhs, operand_type&& rhs) const
    {
        if (rhs.is_ref())
        {
            rhs = blaze::map(rhs.matrix(),
                phylanx::util::detail::div0dnd_simd(lhs.scalar()));
        }
        else
        {
            rhs.matrix() = blaze::map(rhs.matrix(),
                phylanx::util::detail::div0dnd_simd(lhs.scalar()));
        }
        return primitive_argument_type(std::move(rhs));
    }

    primitive_argument_type div_operation::div0d(
        operand_type&& lhs, operand_type&& rhs) const
    {
        std::size_t rhs_dims = rhs.num_dimensions();
        switch(rhs_dims)
        {
        case 0:
            return div0d0d(std::move(lhs), std::move(rhs));

        case 1:
            return div0d1d(std::move(lhs), std::move(rhs));

        case 2:
            return div0d2d(std::move(lhs), std::move(rhs));

        default:
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "div_operation::div0d",
                execution_tree::generate_error_message(
                    "the operands have incompatible number of dimensions",
                    name_, codename_));
        }
    }

    primitive_argument_type div_operation::div0d(operands_type && ops) const
    {
        std::size_t rhs_dims = ops[1].num_dimensions();
        switch(rhs_dims)
        {
        case 0:
            return div0d0d(std::move(ops));

        default:
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "div_operation::div0d",
                execution_tree::generate_error_message(
                    "the operands have incompatible number of dimensions",
                    name_, codename_));
        }
    }

    ///////////////////////////////////////////////////////////////////////////
    primitive_argument_type div_operation::div1d0d(
        operand_type&& lhs, operand_type&& rhs) const
    {
        if (lhs.is_ref())
        {
            lhs = blaze::map(lhs.vector(),
                phylanx::util::detail::divnd0d_simd(rhs.scalar()));
        }
        else
        {
            lhs.vector() = blaze::map(lhs.vector(),
                phylanx::util::detail::divnd0d_simd(rhs.scalar()));
        }

        return primitive_argument_type(std::move(lhs));
    }

    primitive_argument_type div_operation::div1d1d(
        operand_type&& lhs, operand_type&& rhs) const
    {
        std::size_t lhs_size = lhs.dimension(0);
        std::size_t rhs_size = rhs.dimension(0);

        // Broadcasting rule 1: Dimensions are identical
        if (lhs_size == rhs_size)
        {
            if (lhs.is_ref())
            {
                if (rhs.is_ref())
                {
                    rhs = lhs.vector() / rhs.vector();
                }
                else
                {
                    rhs.vector() = lhs.vector() / rhs.vector();
                }
                return primitive_argument_type(std::move(rhs));
            }
            else
            {
                lhs.vector() = blaze::map(lhs.vector(), rhs.vector(),
                    phylanx::util::detail::divndnd_simd());
            }
            return primitive_argument_type{std::move(lhs)};
        }
        // Broadcasting rule 2: One or two of the operand dimensions equal one
        else
        {
            if (lhs_size == 1)
            {
                if (rhs.is_ref())
                {
                    rhs = blaze::map(rhs.vector(),
                        phylanx::util::detail::div0dnd_simd(lhs.vector()[0]));
                }
                else
                {
                    rhs.vector() = blaze::map(rhs.vector(),
                        phylanx::util::detail::div0dnd_simd(lhs.vector()[0]));
                }
                return primitive_argument_type{std::move(rhs)};
            }
            else if (rhs_size == 1)
            {
                if (lhs.is_ref())
                {
                    lhs = blaze::map(lhs.vector(),
                        phylanx::util::detail::divnd0d_simd(rhs.vector()[0]));
                }
                else
                {
                    lhs.vector() = blaze::map(lhs.vector(),
                        phylanx::util::detail::divnd0d_simd(rhs.vector()[0]));
                }
                return primitive_argument_type{std::move(lhs)};
            }
        }

        HPX_THROW_EXCEPTION(hpx::bad_parameter,
            "div_operation::div1d1d",
            execution_tree::generate_error_message(
                "the dimensions of the operands do not match",
                name_, codename_));
    }

    primitive_argument_type div_operation::div1d1d(operands_type && ops) const
    {
        std::size_t const operand_size = ops[0].dimension(0);
        for (auto const& i : ops)
        {
            if (i.dimension(0) != operand_size)
            {
                HPX_THROW_EXCEPTION(hpx::bad_parameter,
                    "div_operation::div1d1d",
                    execution_tree::generate_error_message(
                        "the dimensions of the operands do not match",
                        name_, codename_));
            }
        }

        return primitive_argument_type(std::accumulate(ops.begin() + 1,
            ops.end(), std::move(ops[0]),
            [](operand_type& result,
                operand_type const& curr) -> operand_type
            {
                if (result.is_ref())
                {
                    result = blaze::map(result.vector(), curr.vector(),
                        phylanx::util::detail::divndnd_simd());
                }
                else
                {
                    result.vector() = blaze::map(result.vector(), curr.vector(),
                        phylanx::util::detail::divndnd_simd());
                }
                return std::move(result);
            }));
    }

    primitive_argument_type div_operation::div1d2d(
        operand_type&& lhs, operand_type&& rhs) const
    {
        auto lhs_v = lhs.vector();
        auto rhs_m = rhs.matrix();

        // If dimensions match
        if (rhs_m.columns() == lhs_v.size())
        {
            if (rhs.is_ref())
            {
                blaze::DynamicMatrix<double> result{
                    rhs_m.rows(), rhs_m.columns()};
                for (std::size_t i = 0; i != rhs_m.rows(); ++i)
                {
                    blaze::row(result, i) =
                        blaze::trans(lhs_v) / blaze::row(rhs_m, i);
                }
                return primitive_argument_type{std::move(result)};
            }
            else
            {
                for (std::size_t i = 0; i != rhs_m.rows(); ++i)
                {
                    blaze::row(rhs_m, i) =
                        blaze::trans(lhs_v) / blaze::row(rhs_m, i);
                }

                return primitive_argument_type{std::move(rhs)};
            }
        }
        // If the vector is effectively a scalar
        else if (lhs_v.size() == 1)
        {
            if (rhs.is_ref())
            {
                rhs = blaze::map(
                    rhs_m, phylanx::util::detail::div0dnd_simd(lhs_v[0]));

                return primitive_argument_type{std::move(rhs)};
            }
            else
            {
                rhs_m = blaze::map(
                    rhs_m, phylanx::util::detail::div0dnd_simd(lhs_v[0]));

                return primitive_argument_type{std::move(rhs)};
            }
        }
        // If the matrix has only one column
        else if (rhs_m.columns() == 1)
        {
            blaze::DynamicMatrix<double> result(rhs_m.rows(), lhs_v.size());

            // Replicate lhs vector
            for (std::size_t i = 0; i < result.rows(); ++i)
            {
                blaze::row(result, i) = blaze::trans(lhs_v);
            }

            // Replicate first and only column of rhs matrix
            for (std::size_t i = 0; i < result.columns(); ++i)
            {
                blaze::column(result, i) /= blaze::column(rhs_m, 0);
            }

            return primitive_argument_type{std::move(result)};
        }

        HPX_THROW_EXCEPTION(hpx::bad_parameter,
            "add_operation::div1d2d",
            execution_tree::generate_error_message(
                "vector size does not match number of matrix columns",
                name_, codename_));
    }

    primitive_argument_type div_operation::div1d(
        operand_type&& lhs, operand_type&& rhs) const
    {
        std::size_t rhs_dims = rhs.num_dimensions();
        switch(rhs_dims)
        {
        case 0:
            return div1d0d(std::move(lhs), std::move(rhs));

        case 1:
            return div1d1d(std::move(lhs), std::move(rhs));

        case 2:
            return div1d2d(std::move(lhs), std::move(rhs));

        default:
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "div_operation::div1d",
                execution_tree::generate_error_message(
                    "the operands have incompatible number of dimensions",
                    name_, codename_));
        }
    }

    primitive_argument_type div_operation::div1d(operands_type && ops) const
    {
        std::size_t rhs_dims = ops[1].num_dimensions();
        switch(rhs_dims)
        {
        case 1:
            return div1d1d(std::move(ops));

        default:
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "div_operation::div1d",
                execution_tree::generate_error_message(
                    "the operands have incompatible number of dimensions",
                    name_, codename_));
        }
    }

    ///////////////////////////////////////////////////////////////////////////
    primitive_argument_type div_operation::div2d0d(
        operand_type&& lhs, operand_type&& rhs) const
    {
        if (lhs.is_ref())
        {
            lhs = blaze::map(lhs.matrix(),
                phylanx::util::detail::divnd0d_simd(rhs.scalar()));
        }
        else
        {
            lhs.matrix() = blaze::map(lhs.matrix(),
                phylanx::util::detail::divnd0d_simd(rhs.scalar()));
        }

        return primitive_argument_type(std::move(lhs));
    }

    primitive_argument_type div_operation::div2d1d(
        operand_type&& lhs, operand_type&& rhs) const
    {
        auto lhs_m = lhs.matrix();
        auto rhs_v = rhs.vector();

        // If dimensions match
        if (rhs_v.size() == lhs_m.columns())
        {
            if (lhs.is_ref())
            {
                blaze::DynamicMatrix<double> result{
                    lhs_m.rows(), lhs_m.columns()};
                for (std::size_t i = 0; i != lhs_m.rows(); ++i)
                {
                    blaze::row(result, i) =
                        blaze::row(lhs_m, i) / blaze::trans(rhs_v);
                }
                return primitive_argument_type{std::move(result)};
            }
            else
            {
                for (std::size_t i = 0; i != lhs_m.rows(); ++i)
                {
                    blaze::row(lhs_m, i) /= blaze::trans(rhs_v);
                }

                return primitive_argument_type{std::move(lhs)};
            }
        }
        // If the vector is effectively a scalar
        else if (rhs_v.size() == 1)
        {
            if (lhs.is_ref())
            {
                lhs = blaze::map(
                    lhs_m, phylanx::util::detail::divnd0d_simd(rhs_v[0]));

                return primitive_argument_type{std::move(lhs)};
            }
            else
            {
                lhs_m = blaze::map(
                    lhs_m, phylanx::util::detail::divnd0d_simd(rhs_v[0]));

                return primitive_argument_type{std::move(lhs)};
            }
        }
        // If the matrix has only one column
        else if (lhs_m.columns() == 1)
        {
            blaze::DynamicMatrix<double> result(lhs_m.rows(), rhs_v.size());

            for (std::size_t i = 0; i < result.columns(); ++i)
            {
                blaze::column(result, i) = blaze::column(lhs_m, 0);
            }

            for (std::size_t i = 0; i < result.rows(); ++i)
            {
                blaze::row(result, i) /= blaze::trans(rhs_v);
            }

            return primitive_argument_type{std::move(result)};
        }

        HPX_THROW_EXCEPTION(hpx::bad_parameter,
            "div_operation::div2d1d",
            execution_tree::generate_error_message(
                "vector size does not match either the number of matrix "
                "columns nor rows.",
                name_, codename_));
    }

    div_operation::stretch_operand div_operation::get_stretch_dimension(
        std::size_t lhs, std::size_t rhs) const
    {
        // 0: No copy, 1: stretch lhs, 2: stretch rhs
        if (lhs != rhs)
        {
            // lhs x dimension must be stretched
            if (lhs == 1)
            {
                return stretch_operand::lhs;
            }
            // rhs x dimensions must be stretched
            else if (rhs == 1)
            {
                return stretch_operand::rhs;
            }
        }
        return stretch_operand::neither;
    }

    primitive_argument_type div_operation::div2d2d_no_stretch(
        operand_type&& lhs, operand_type&& rhs) const
    {
        // Avoid overwriting references, avoid memory reallocation when possible
        if (lhs.is_ref())
        {
            // Cannot reuse the memory if an operand is a reference
            if (rhs.is_ref())
            {
                rhs = blaze::map(lhs.matrix(), rhs.matrix(),
                    phylanx::util::detail::divndnd_simd());
            }
            // Reuse the memory from rhs operand
            else
            {
                rhs.matrix() = blaze::map(lhs.matrix(), rhs.matrix(),
                    phylanx::util::detail::divndnd_simd());
            }
            return primitive_argument_type(std::move(rhs));
        }
        // Reuse the memory from lhs operand
        else
        {
            lhs.matrix() = blaze::map(lhs.matrix(), rhs.matrix(),
                phylanx::util::detail::divndnd_simd());
        }

        return primitive_argument_type(std::move(lhs));
    }

    primitive_argument_type div_operation::div2d2d_lhs_both(
        operand_type&& lhs, operand_type&& rhs) const
    {
        auto lhs_m = lhs.matrix();
        auto rhs_m = rhs.matrix();

        if (lhs.is_ref())
        {
            lhs = blaze::map(
                rhs_m, phylanx::util::detail::div0dnd_simd(lhs_m(0, 0)));
        }
        else
        {
            lhs.matrix() = blaze::map(
                rhs_m, phylanx::util::detail::div0dnd_simd(lhs_m(0, 0)));
        }

        return primitive_argument_type{std::move(lhs)};
    }

    primitive_argument_type div_operation::div2d2d_rhs_both(
        operand_type&& lhs, operand_type&& rhs) const
    {
        auto lhs_m = lhs.matrix();
        auto rhs_m = rhs.matrix();

        if (lhs.is_ref())
        {
            lhs = blaze::map(
                lhs_m, phylanx::util::detail::divnd0d_simd(rhs_m(0, 0)));
        }
        else
        {
            lhs.matrix() = blaze::map(
                lhs_m, phylanx::util::detail::divnd0d_simd(rhs_m(0, 0)));
        }

        return primitive_argument_type{std::move(lhs)};
    }

    primitive_argument_type div_operation::div2d2d_lhs_row_rhs_col(
        operand_type&& lhs, operand_type&& rhs) const
    {
        auto lhs_m = lhs.matrix();
        auto rhs_m = rhs.matrix();

        blaze::DynamicMatrix<double> result(rhs_m.rows(), lhs_m.columns());
        for (std::size_t i = 0; i < result.rows(); ++i)
        {
            blaze::row(result, i) = blaze::row(lhs_m, 0);
        }
        for (std::size_t i = 0; i < result.columns(); ++i)
        {
            blaze::column(result, i) /= blaze::column(rhs_m, 0);
        }
        return primitive_argument_type{std::move(result)};
    }

    primitive_argument_type div_operation::div2d2d_lhs_row(
        operand_type&& lhs, operand_type&& rhs) const
    {
        auto lhs_m = lhs.matrix();
        auto rhs_m = rhs.matrix();

        // Ensure we can reuse memory
        if (rhs.is_ref())
        {
            blaze::DynamicMatrix<double> result(
                rhs.dimension(0), rhs.dimension(1));

            for (std::size_t i = 0; i != result.rows(); ++i)
            {
                blaze::row(result, i) =
                    blaze::row(lhs_m, 0) / blaze::row(rhs_m, i);
            }
            return primitive_argument_type{std::move(result)};
        }
        else
        {
            for (std::size_t i = 0; i != rhs.dimension(0); ++i)
            {
                blaze::row(rhs_m, i) =
                    blaze::row(lhs_m, 0) / blaze::row(rhs_m, i);
            }
            return primitive_argument_type{std::move(rhs)};
        }
    }

    primitive_argument_type div_operation::div2d2d_lhs_col_rhs_row(
        operand_type&& lhs, operand_type&& rhs) const
    {
        auto lhs_m = lhs.matrix();
        auto rhs_m = rhs.matrix();

        blaze::DynamicMatrix<double> result(lhs_m.rows(), rhs_m.columns());
        for (std::size_t i = 0; i < result.columns(); ++i)
        {
            blaze::column(result, i) = blaze::column(lhs_m, 0);
        }
        for (std::size_t i = 0; i < result.rows(); ++i)
        {
            blaze::row(result, i) /= blaze::row(rhs_m, 0);
        }
        return primitive_argument_type{std::move(result)};
    }

    primitive_argument_type div_operation::div2d2d_rhs_row(
        operand_type&& lhs, operand_type&& rhs) const
    {
        auto lhs_m = lhs.matrix();
        auto rhs_m = rhs.matrix();

        // Ensure we can reuse memory
        if (lhs.is_ref())
        {
            blaze::DynamicMatrix<double> result(
                lhs.dimension(0), lhs.dimension(1));

            for (std::size_t i = 0; i != result.rows(); ++i)
            {
                blaze::row(result, i) =
                    blaze::row(lhs_m, i) / blaze::row(rhs_m, 0);
            }
            return primitive_argument_type{std::move(result)};
        }
        else
        {
            for (std::size_t i = 0; i != lhs.dimension(0); ++i)
            {
                blaze::row(lhs_m, i) =
                    blaze::row(lhs_m, i) / blaze::row(rhs_m, 0);
            }
            return primitive_argument_type{std::move(lhs)};
        }
    }

    primitive_argument_type div_operation::div2d2d_lhs_col(
        operand_type&& lhs, operand_type&& rhs) const
    {
        auto lhs_m = lhs.matrix();
        auto rhs_m = rhs.matrix();

        // Avoid overwriting references, avoid memory
        // reallocation when possible
        if (rhs.is_ref())
        {
            blaze::DynamicMatrix<double> result(
                rhs.dimension(0), rhs.dimension(1));

            for (std::size_t i = 0; i != result.columns(); ++i)
            {
                blaze::column(result, i) =
                    blaze::column(lhs_m, 0) / blaze::column(rhs_m, i);
            }
            return primitive_argument_type{std::move(result)};
        }
        else
        {
            for (std::size_t i = 0; i != rhs.dimension(1); ++i)
            {
                blaze::column(rhs_m, i) =
                    blaze::column(lhs_m, 0) / blaze::column(rhs_m, i);
            }
            return primitive_argument_type{std::move(rhs)};
        }
    }

    primitive_argument_type div_operation::div2d2d_rhs_col(
        operand_type&& lhs, operand_type&& rhs) const
    {
        auto lhs_m = lhs.matrix();
        auto rhs_m = rhs.matrix();

        // Avoid overwriting references, avoid memory
        // reallocation when possible
        if (lhs.is_ref())
        {
            blaze::DynamicMatrix<double> result(
                lhs.dimension(0), lhs.dimension(1));

            for (std::size_t i = 0; i != result.columns(); ++i)
            {
                blaze::column(result, i) =
                    blaze::column(lhs_m, i) / blaze::column(rhs_m, 0);
            }
            return primitive_argument_type{std::move(result)};
        }
        else
        {
            for (std::size_t i = 0; i != lhs.dimension(1); ++i)
            {
                blaze::column(lhs_m, i) =
                    blaze::column(lhs_m, i) / blaze::column(rhs_m, 0);
            }
            return primitive_argument_type{std::move(lhs)};
        }
    }

    primitive_argument_type div_operation::div2d2d(
        operand_type&& lhs, operand_type&& rhs) const
    {
        auto lhs_size = lhs.dimensions();
        auto rhs_size = rhs.dimensions();

        // Dimensions are identical
        if (lhs_size == rhs_size)
        {
            return div2d2d_no_stretch(std::move(lhs), std::move(rhs));
        }
        // Check if broadcasting rules apply
        else
        {
            // Number of rows do not match
            stretch_operand stretch_rows =
                get_stretch_dimension(lhs_size[0], rhs_size[0]);
            // Number of columns do not match
            stretch_operand stretch_cols =
                get_stretch_dimension(lhs_size[1], rhs_size[1]);

            // Shorthand
            auto lhs_m = lhs.matrix();
            auto rhs_m = rhs.matrix();

            // If lhs rows needs stretching
            if (stretch_rows == stretch_operand::lhs)
            {
                // If both lhs axes need stretching
                if (stretch_cols == stretch_operand::lhs)
                {
                    return div2d2d_lhs_both(std::move(lhs), std::move(rhs));
                }
                // If lhs rows and rhs cols need stretching
                else if (stretch_cols == stretch_operand::rhs)
                {
                    return div2d2d_lhs_row_rhs_col(
                        std::move(lhs), std::move(rhs));
                }
                // If only lhs rows need stretching
                else
                {
                    return div2d2d_lhs_row(std::move(lhs), std::move(rhs));
                }
            }
            // If lhs cols needs stretching
            else if (stretch_rows == stretch_operand::rhs)
            {
                // If lhs cols and rhs rows need stretching
                if (stretch_cols == stretch_operand::lhs)
                {
                    return div2d2d_lhs_col_rhs_row(
                        std::move(lhs), std::move(rhs));
                }
                // If both rhs axes need stretching
                else if (stretch_cols == stretch_operand::rhs)
                {
                    return div2d2d_rhs_both(std::move(lhs), std::move(rhs));
                }
                // If only rhs rows need stretching
                else
                {
                    return div2d2d_rhs_row(std::move(lhs), std::move(rhs));
                }
            }
            else
            {
                // If only lhs cols need stretching
                if (stretch_cols == stretch_operand::lhs)
                {
                    return div2d2d_lhs_col(std::move(lhs), std::move(rhs));
                }
                // If only rhs cols need stretching
                else if (stretch_cols == stretch_operand::rhs)
                {
                    return div2d2d_rhs_col(std::move(lhs), std::move(rhs));
                }
                // Otherwise no axis can be stretched
            }
        }

        HPX_THROW_EXCEPTION(hpx::bad_parameter,
            "div_operation::div2d2d",
            execution_tree::generate_error_message(
                "the dimensions of the operands do not match",
                name_, codename_));
    }

    primitive_argument_type div_operation::div2d2d(operands_type && ops) const
    {
        operand_type& lhs = ops[0];
        operand_type& rhs = ops[1];

        auto lhs_size = lhs.dimensions();
        auto rhs_size = rhs.dimensions();

        if (lhs_size != rhs_size)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "div_operation::div2d2d",
                execution_tree::generate_error_message(
                    "the dimensions of the operands do not match",
                    name_, codename_));
        }

        operand_type& first_term = *ops.begin();
        return primitive_argument_type(std::accumulate(ops.begin() + 1,
            ops.end(), std::move(first_term),
            [](operand_type& result, operand_type const& curr) -> operand_type {
                if (result.is_ref())
                {
                    result = blaze::map(result.matrix(), curr.matrix(),
                        phylanx::util::detail::divndnd_simd());
                }
                else
                {
                    result.matrix() = blaze::map(result.matrix(), curr.matrix(),
                        phylanx::util::detail::divndnd_simd());
                }
                return std::move(result);
            }));
    }

    primitive_argument_type div_operation::div2d(
        operand_type&& lhs, operand_type&& rhs) const
    {
        std::size_t rhs_dims = rhs.num_dimensions();
        switch(rhs_dims)
        {
        case 0:
            return div2d0d(std::move(lhs), std::move(rhs));

        case 1:
            return div2d1d(std::move(lhs), std::move(rhs));

        case 2:
            return div2d2d(std::move(lhs), std::move(rhs));

        default:
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "div_operation::div2d",
                execution_tree::generate_error_message(
                    "the operands have incompatible number of dimensions",
                    name_, codename_));
        }
    }

    primitive_argument_type div_operation::div2d(operands_type && ops) const
    {
        std::size_t rhs_dims = ops[1].num_dimensions();
        switch(rhs_dims)
        {
        case 2:
            return div2d2d(std::move(ops));

        default:
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "div_operation::div2d",
                execution_tree::generate_error_message(
                    "the operands have incompatible number of dimensions",
                    name_, codename_));
        }
    }

    ///////////////////////////////////////////////////////////////////////////
    hpx::future<primitive_argument_type> div_operation::eval(
        std::vector<primitive_argument_type> const& operands,
        std::vector<primitive_argument_type> const& args) const
    {
        if (operands.size() < 2)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "div_operation::eval",
                execution_tree::generate_error_message(
                    "the div_operation primitive requires at least "
                        "two operands",
                    name_, codename_));
        }

        bool arguments_valid = true;
        for (std::size_t i = 0; i != operands.size(); ++i)
        {
            if (!valid(operands[i]))
            {
                arguments_valid = false;
            }
        }

        if (!arguments_valid)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "div_operation::eval",
                execution_tree::generate_error_message(
                    "the div_operation primitive requires that the "
                        "arguments given by the operands array are "
                        "valid",
                    name_, codename_));
        }

        auto this_ = this->shared_from_this();
        if (operands.size() == 2)
        {
            return hpx::dataflow(hpx::launch::sync, hpx::util::unwrapping(
                [this_](operand_type&& lhs, operand_type&& rhs)
                ->  primitive_argument_type
                {
                    std::size_t lhs_dims = lhs.num_dimensions();
                    switch (lhs_dims)
                    {
                    case 0:
                        return this_->div0d(std::move(lhs), std::move(rhs));

                    case 1:
                        return this_->div1d(std::move(lhs), std::move(rhs));

                    case 2:
                        return this_->div2d(std::move(lhs), std::move(rhs));

                    default:
                        HPX_THROW_EXCEPTION(hpx::bad_parameter,
                            "div_operation::eval",
                            execution_tree::generate_error_message(
                                "left hand side operand has unsupported "
                                    "number of dimensions",
                                this_->name_, this_->codename_));
                    }
                }),
                numeric_operand(operands[0], args, name_, codename_),
                numeric_operand(operands[1], args, name_, codename_));
        }

        return hpx::dataflow(hpx::launch::sync, hpx::util::unwrapping(
            [this_](operands_type&& ops) -> primitive_argument_type
            {
                std::size_t lhs_dims = ops[0].num_dimensions();
                switch (lhs_dims)
                {
                case 0:
                    return this_->div0d(std::move(ops));

                case 1:
                    return this_->div1d(std::move(ops));

                case 2:
                    return this_->div2d(std::move(ops));

                default:
                    HPX_THROW_EXCEPTION(hpx::bad_parameter,
                        "div_operation::eval",
                        execution_tree::generate_error_message(
                            "left hand side operand has unsupported "
                                "number of dimensions",
                            this_->name_, this_->codename_));
                }
            }),
            detail::map_operands(
                operands, functional::numeric_operand{}, args,
                name_, codename_));
    }

    // implement '/' for all possible combinations of lhs and rhs
    hpx::future<primitive_argument_type> div_operation::eval(
        std::vector<primitive_argument_type> const& args) const
    {
        if (this->no_operands())
        {
            return eval(args, noargs);
        }
        return eval(this->operands(), args);
    }
}}}
