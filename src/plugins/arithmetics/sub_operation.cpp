// Copyright (c) 2017-2018 Hartmut Kaiser
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>
#include <phylanx/ir/node_data.hpp>
#include <phylanx/plugins/arithmetics/sub_operation.hpp>

#include <hpx/include/lcos.hpp>
#include <hpx/include/naming.hpp>
#include <hpx/include/util.hpp>
#include <hpx/throw_exception.hpp>

#include <cstddef>
#include <cstdint>
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
    namespace detail
    {
        struct sub0dnd_simd
        {
        public:
            explicit sub0dnd_simd(double scalar)
                : scalar_(scalar)
            {
            }

            template <typename T>
            BLAZE_ALWAYS_INLINE auto operator()(T const& a) const
            ->  decltype(std::declval<double>() - a)
            {
                return scalar_ - a;
            }

            template <typename T>
            static constexpr bool simdEnabled()
            {
                return blaze::HasSIMDSub<T, double>::value;
            }

            template <typename T>
            BLAZE_ALWAYS_INLINE decltype(auto) load(T const& a) const
            {
                BLAZE_CONSTRAINT_MUST_BE_SIMD_PACK(T);
                return blaze::set(scalar_) - a;
            }

        private:
            double scalar_;
        };

        struct subnd0d_simd
        {
        public:
            explicit subnd0d_simd(double scalar)
                : scalar_(scalar)
            {
            }

            template <typename T>
            BLAZE_ALWAYS_INLINE auto operator()(T const& a) const
                -> decltype(a - std::declval<double>())
            {
                return a - scalar_;
            }

            template <typename T>
            static constexpr bool simdEnabled()
            {
                return blaze::HasSIMDSub<T, double>::value;
            }

            template <typename T>
            BLAZE_ALWAYS_INLINE decltype(auto) load(T const& a) const
            {
                BLAZE_CONSTRAINT_MUST_BE_SIMD_PACK(T);
                return a - blaze::set(scalar_);
            }

        private:
            double scalar_;
        };
    }

    ///////////////////////////////////////////////////////////////////////////
    match_pattern_type const sub_operation::match_data =
    {
        hpx::util::make_tuple("__sub",
            std::vector<std::string>{"_1 - __2", "__sub(_1, __2)"},
            &create_sub_operation, &create_primitive<sub_operation>)
    };

    //////////////////////////////////////////////////////////////////////////
    sub_operation::sub_operation(
            std::vector<primitive_argument_type> && operands,
            std::string const& name, std::string const& codename)
      : primitive_component_base(std::move(operands), name, codename)
    {}

    ///////////////////////////////////////////////////////////////////////////
    primitive_argument_type sub_operation::sub0d0d(
        arg_type&& lhs, arg_type&& rhs) const
    {
        lhs.scalar() -= rhs.scalar();
        return primitive_argument_type(std::move(lhs));
    }

    primitive_argument_type sub_operation::sub0d0d(args_type && args) const
    {

        return primitive_argument_type{std::accumulate(
            args.begin() + 1, args.end(), std::move(args[0]),
            [](arg_type& result, arg_type const& curr) -> arg_type
            {
                result.scalar() -= curr.scalar();
                return std::move(result);
            })};
    }

    primitive_argument_type sub_operation::sub0d1d(
        arg_type&& lhs, arg_type&& rhs) const
    {
        if (rhs.is_ref())
        {
            rhs = blaze::map(rhs.vector(), detail::sub0dnd_simd(lhs.scalar()));
        }
        else
        {
            rhs.vector() =
                blaze::map(rhs.vector(), detail::sub0dnd_simd(lhs.scalar()));
        }

        return primitive_argument_type(std::move(rhs));
    }

    primitive_argument_type sub_operation::sub0d2d(
        arg_type&& lhs, arg_type&& rhs) const
    {
        if (rhs.is_ref())
        {
            rhs = blaze::map(rhs.matrix(), detail::sub0dnd_simd(lhs.scalar()));
        }
        else
        {
            rhs.matrix() = blaze::map(
                rhs.matrix(), detail::sub0dnd_simd(lhs.scalar()));
        }

        return primitive_argument_type(std::move(rhs));
    }

    primitive_argument_type sub_operation::sub0d(
        arg_type&& lhs, arg_type&& rhs) const
    {
        std::size_t rhs_dims = rhs.num_dimensions();
        switch (rhs_dims)
        {
        case 0:
            return sub0d0d(std::move(lhs), std::move(rhs));

        case 1:
            return sub0d1d(std::move(lhs), std::move(rhs));

        case 2:
            return sub0d2d(std::move(lhs), std::move(rhs));

        default:
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "sub_operation::sub0d",
                execution_tree::generate_error_message(
                    "the operands have incompatible number of dimensions",
                    name_, codename_));
        }
    }

    primitive_argument_type sub_operation::sub0d(args_type && args) const
    {
        std::size_t rhs_dims = args[1].num_dimensions();
        switch (rhs_dims)
        {
        case 0:
            return sub0d0d(std::move(args));

        default:
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "sub_operation::sub0d",
                execution_tree::generate_error_message(
                    "the operands have incompatible number of dimensions",
                    name_, codename_));
        }
    }

    ///////////////////////////////////////////////////////////////////////////
    primitive_argument_type sub_operation::sub1d0d(
        arg_type&& lhs, arg_type&& rhs) const
    {
        if (lhs.is_ref())
        {
            lhs = blaze::map(lhs.vector(), detail::subnd0d_simd(rhs.scalar()));
        }
        else
        {
            lhs.vector() =
                blaze::map(lhs.vector(), detail::subnd0d_simd(rhs.scalar()));
        }
        return primitive_argument_type(std::move(lhs));
    }

    primitive_argument_type sub_operation::sub1d1d(
        arg_type&& lhs, arg_type&& rhs) const
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
                    rhs = lhs.vector() - rhs.vector();
                }
                else
                {
                    rhs.vector() = lhs.vector() - rhs.vector();
                }
                return primitive_argument_type(std::move(rhs));
            }
            else
            {
                lhs.vector() -= rhs.vector();
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
                    rhs = blaze::map(
                        rhs.vector(), detail::sub0dnd_simd(lhs.vector()[0]));
                }
                else
                {
                    rhs.vector() = blaze::map(
                        rhs.vector(), detail::sub0dnd_simd(lhs.vector()[0]));
                }
                return primitive_argument_type{std::move(rhs)};
            }
            else if (rhs_size == 1)
            {
                if (lhs.is_ref())
                {
                    lhs = blaze::map(
                        lhs.vector(), detail::subnd0d_simd(rhs.vector()[0]));
                }
                else
                {
                    lhs.vector() = blaze::map(
                        lhs.vector(), detail::subnd0d_simd(rhs.vector()[0]));
                }
                return primitive_argument_type{std::move(lhs)};
            }
        }

        HPX_THROW_EXCEPTION(hpx::bad_parameter,
            "sub_operation::sub1d1d",
            execution_tree::generate_error_message(
                "the dimensions of the operands do not match", name_,
                codename_));
    }

    primitive_argument_type sub_operation::sub1d1d(args_type && args) const
    {
        std::size_t const operand_size = args[0].dimension(0);
        for (auto const& i : args)
        {
            if (i.dimension(0) != operand_size)
            {
                HPX_THROW_EXCEPTION(hpx::bad_parameter,
                    "sub_operation::sub1d1d",
                    execution_tree::generate_error_message(
                        "the dimensions of the operands do not match",
                        name_, codename_));
            }
        }

        return primitive_argument_type(std::accumulate(
            args.begin() + 1, args.end(), std::move(args[0]),
            [](arg_type& result, arg_type const& curr) -> arg_type
            {
                if (result.is_ref())
                {
                    result = result.vector() - curr.vector();
                }
                else
                {
                    result.vector() -= curr.vector();
                }
                return std::move(result);
            }));
    }

    primitive_argument_type sub_operation::sub1d2d(
        arg_type&& lhs, arg_type&& rhs) const
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
                        blaze::trans(lhs_v) - blaze::row(rhs_m, i);
                }
                return primitive_argument_type{std::move(result)};
            }
            else
            {
                for (std::size_t i = 0; i != rhs_m.rows(); ++i)
                {
                    blaze::row(rhs_m, i) =
                        blaze::trans(lhs_v) - blaze::row(rhs_m, i);
                }

                return primitive_argument_type{std::move(rhs)};
            }
        }
        // If the vector is effectively a scalar
        else if (lhs_v.size() == 1)
        {
            if (rhs.is_ref())
            {
                rhs = blaze::map(rhs_m, detail::sub0dnd_simd(lhs_v[0]));

                return primitive_argument_type{std::move(rhs)};
            }
            else
            {
                rhs_m = blaze::map(rhs_m, detail::sub0dnd_simd(lhs_v[0]));

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
                blaze::column(result, i) -= blaze::column(rhs_m, 0);
            }

            return primitive_argument_type{std::move(result)};
        }

        HPX_THROW_EXCEPTION(hpx::bad_parameter,
            "add_operation::sub1d2d",
            execution_tree::generate_error_message(
                "vector size does not match number of matrix columns",
                name_, codename_));
    }

    primitive_argument_type sub_operation::sub1d(
        arg_type&& lhs, arg_type&& rhs) const
    {
        std::size_t rhs_dims = rhs.num_dimensions();

        switch (rhs_dims)
        {
        case 0:
            return sub1d0d(std::move(lhs), std::move(rhs));

        case 1:
            return sub1d1d(std::move(lhs), std::move(rhs));

        case 2:
            return sub1d2d(std::move(lhs), std::move(rhs));

        default:
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "sub_operation::sub1d",
                execution_tree::generate_error_message(
                    "the operands have incompatible number of dimensions",
                    name_, codename_));
        }
    }

    primitive_argument_type sub_operation::sub1d(args_type && args) const
    {
        std::size_t rhs_dims = args[1].num_dimensions();

        switch (rhs_dims)
        {
        case 1:
            return sub1d1d(std::move(args));

        default:
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "sub_operation::sub1d",
                execution_tree::generate_error_message(
                    "the operands have incompatible number of dimensions",
                    name_, codename_));
        }
    }

    ///////////////////////////////////////////////////////////////////////////
    primitive_argument_type sub_operation::sub2d0d(
        arg_type&& lhs, arg_type&& rhs) const
    {
        if (lhs.is_ref())
        {
            lhs = blaze::map(lhs.matrix(), detail::subnd0d_simd(rhs.scalar()));
        }
        else
        {
            lhs.matrix() =
                blaze::map(lhs.matrix(), detail::subnd0d_simd(rhs.scalar()));
        }
        return primitive_argument_type(std::move(lhs));
    }

    primitive_argument_type sub_operation::sub2d1d(
        arg_type&& lhs, arg_type&& rhs) const
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
                        blaze::row(lhs_m, i) - blaze::trans(rhs_v);
                }
                return primitive_argument_type{std::move(result)};
            }
            else
            {
                for (std::size_t i = 0; i != lhs_m.rows(); ++i)
                {
                    blaze::row(lhs_m, i) -= blaze::trans(rhs_v);
                }

                return primitive_argument_type{std::move(lhs)};
            }
        }
        // If the vector is effectively a scalar
        else if (rhs_v.size() == 1)
        {
            if (lhs.is_ref())
            {
                lhs = blaze::map(lhs_m, detail::subnd0d_simd(rhs_v[0]));

                return primitive_argument_type{std::move(lhs)};
            }
            else
            {
                lhs_m = blaze::map(lhs_m, detail::subnd0d_simd(rhs_v[0]));

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
                blaze::row(result, i) -= blaze::trans(rhs_v);
            }

            return primitive_argument_type{std::move(result)};
        }

        HPX_THROW_EXCEPTION(hpx::bad_parameter,
            "sub_operation::sub2d1d",
            execution_tree::generate_error_message(
                "vector size does not match either the number of matrix "
                "columns nor rows.",
                name_, codename_));
    }

    sub_operation::stretch_operand sub_operation::get_stretch_dimension(
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

    primitive_argument_type sub_operation::sub2d2d_no_stretch(
        arg_type&& lhs, arg_type&& rhs) const
    {
        // Avoid overwriting references, avoid memory reallocation when possible
        if (lhs.is_ref())
        {
            // Cannot reuse the memory if an operand is a reference
            if (rhs.is_ref())
            {
                rhs = lhs.matrix() - rhs.matrix();
            }
            // Reuse the memory from rhs operand
            else
            {
                rhs.matrix() = lhs.matrix() - rhs.matrix();
            }
            return primitive_argument_type(std::move(rhs));
        }
        // Reuse the memory from lhs operand
        else
        {
            lhs.matrix() -= rhs.matrix();
        }

        return primitive_argument_type(std::move(lhs));
    }

    primitive_argument_type sub_operation::sub2d2d_lhs_both(
        arg_type&& lhs, arg_type&& rhs) const
    {
        auto lhs_m = lhs.matrix();
        auto rhs_m = rhs.matrix();

        if (lhs.is_ref())
        {
            lhs = blaze::map(rhs_m, detail::sub0dnd_simd(lhs_m(0, 0)));
        }
        else
        {
            lhs.matrix() = blaze::map(rhs_m, detail::sub0dnd_simd(lhs_m(0, 0)));
        }

        return primitive_argument_type{std::move(lhs)};
    }

    primitive_argument_type sub_operation::sub2d2d_rhs_both(
        arg_type&& lhs, arg_type&& rhs) const
    {
        auto lhs_m = lhs.matrix();
        auto rhs_m = rhs.matrix();

        if (lhs.is_ref())
        {
            lhs = blaze::map(lhs_m, detail::subnd0d_simd(rhs_m(0, 0)));
        }
        else
        {
            lhs.matrix() = blaze::map(lhs_m, detail::subnd0d_simd(rhs_m(0, 0)));
        }

        return primitive_argument_type{std::move(lhs)};
    }

    primitive_argument_type sub_operation::sub2d2d_lhs_row_rhs_col(
        arg_type&& lhs, arg_type&& rhs) const
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
            blaze::column(result, i) -= blaze::column(rhs_m, 0);
        }
        return primitive_argument_type{std::move(result)};
    }

    primitive_argument_type sub_operation::sub2d2d_lhs_row(
        arg_type&& lhs, arg_type&& rhs) const
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
                    blaze::row(lhs_m, 0) - blaze::row(rhs_m, i);
            }
            return primitive_argument_type{std::move(result)};
        }
        else
        {
            for (std::size_t i = 0; i != rhs.dimension(0); ++i)
            {
                blaze::row(rhs_m, i) =
                    blaze::row(lhs_m, 0) - blaze::row(rhs_m, i);
            }
            return primitive_argument_type{std::move(rhs)};
        }
    }

    primitive_argument_type sub_operation::sub2d2d_lhs_col_rhs_row(
        arg_type&& lhs, arg_type&& rhs) const
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
            blaze::row(result, i) -= blaze::row(rhs_m, 0);
        }
        return primitive_argument_type{std::move(result)};
    }

    primitive_argument_type sub_operation::sub2d2d_rhs_row(
        arg_type&& lhs, arg_type&& rhs) const
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
                    blaze::row(lhs_m, i) - blaze::row(rhs_m, 0);
            }
            return primitive_argument_type{std::move(result)};
        }
        else
        {
            for (std::size_t i = 0; i != lhs.dimension(0); ++i)
            {
                blaze::row(lhs_m, i) =
                    blaze::row(lhs_m, i) - blaze::row(rhs_m, 0);
            }
            return primitive_argument_type{std::move(lhs)};
        }
    }

    primitive_argument_type sub_operation::sub2d2d_lhs_col(
        arg_type&& lhs, arg_type&& rhs) const
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
                    blaze::column(lhs_m, 0) - blaze::column(rhs_m, i);
            }
            return primitive_argument_type{std::move(result)};
        }
        else
        {
            for (std::size_t i = 0; i != rhs.dimension(1); ++i)
            {
                blaze::column(rhs_m, i) =
                    blaze::column(lhs_m, 0) - blaze::column(rhs_m, i);
            }
            return primitive_argument_type{std::move(rhs)};
        }
    }

    primitive_argument_type sub_operation::sub2d2d_rhs_col(
        arg_type&& lhs, arg_type&& rhs) const
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
                    blaze::column(lhs_m, i) - blaze::column(rhs_m, 0);
            }
            return primitive_argument_type{std::move(result)};
        }
        else
        {
            for (std::size_t i = 0; i != lhs.dimension(1); ++i)
            {
                blaze::column(lhs_m, i) =
                    blaze::column(lhs_m, i) - blaze::column(rhs_m, 0);
            }
            return primitive_argument_type{std::move(lhs)};
        }
    }

    primitive_argument_type sub_operation::sub2d2d(
        arg_type&& lhs, arg_type&& rhs) const
    {
        auto lhs_size = lhs.dimensions();
        auto rhs_size = rhs.dimensions();

        // Dimensions are identical
        if (lhs_size == rhs_size)
        {
            return sub2d2d_no_stretch(std::move(lhs), std::move(rhs));
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
                    return sub2d2d_lhs_both(std::move(lhs), std::move(rhs));
                }
                // If lhs rows and rhs cols need stretching
                else if (stretch_cols == stretch_operand::rhs)
                {
                    return sub2d2d_lhs_row_rhs_col(
                        std::move(lhs), std::move(rhs));
                }
                // If only lhs rows need stretching
                else
                {
                    return sub2d2d_lhs_row(std::move(lhs), std::move(rhs));
                }
            }
            // If lhs cols needs stretching
            else if (stretch_rows == stretch_operand::rhs)
            {
                // If lhs cols and rhs rows need stretching
                if (stretch_cols == stretch_operand::lhs)
                {
                    return sub2d2d_lhs_col_rhs_row(
                        std::move(lhs), std::move(rhs));
                }
                // If both rhs axes need stretching
                else if (stretch_cols == stretch_operand::rhs)
                {
                    return sub2d2d_rhs_both(std::move(lhs), std::move(rhs));
                }
                // If only rhs rows need stretching
                else
                {
                    return sub2d2d_rhs_row(std::move(lhs), std::move(rhs));
                }
            }
            else
            {
                // If only lhs cols need stretching
                if (stretch_cols == stretch_operand::lhs)
                {
                    return sub2d2d_lhs_col(std::move(lhs), std::move(rhs));
                }
                // If only rhs cols need stretching
                else if (stretch_cols == stretch_operand::rhs)
                {
                    return sub2d2d_rhs_col(std::move(lhs), std::move(rhs));
                }
                // Otherwise no axis can be stretched
            }
        }

        HPX_THROW_EXCEPTION(hpx::bad_parameter,
            "sub_operation::sub2d2d",
            execution_tree::generate_error_message(
                "the dimensions of the operands do not match", name_,
                codename_));
    }

    primitive_argument_type sub_operation::sub2d2d(args_type && args) const
    {
        auto const operand_size = args[0].dimensions();
        for (auto const& i : args)
        {
            if (i.dimensions() != operand_size)
            {
                HPX_THROW_EXCEPTION(hpx::bad_parameter,
                    "sub_operation::sub2d2d",
                    execution_tree::generate_error_message(
                        "the dimensions of the operands do not match",
                        name_, codename_));
            }
        }

        return primitive_argument_type{std::accumulate(
            args.begin() + 1, args.end(), std::move(args[0]),
            [](arg_type& result, arg_type const& curr) -> arg_type
            {
                if (result.is_ref())
                {
                    result = result.matrix() - curr.matrix();
                }
                else
                {
                    result.matrix() -= curr.matrix();
                }
                return std::move(result);
            })};
    }

    primitive_argument_type sub_operation::sub2d(
        arg_type&& lhs, arg_type&& rhs) const
    {
        std::size_t rhs_dims = rhs.num_dimensions();
        switch (rhs_dims)
        {
        case 0:
            return sub2d0d(std::move(lhs), std::move(rhs));

        case 1:
            return sub2d1d(std::move(lhs), std::move(rhs));

        case 2:
            return sub2d2d(std::move(lhs), std::move(rhs));

        default:
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "sub_operation::sub2d",
                execution_tree::generate_error_message(
                    "the operands have incompatible number of dimensions",
                    name_, codename_));
        }
    }

    primitive_argument_type sub_operation::sub2d(args_type && args) const
    {
        std::size_t rhs_dims = args[1].num_dimensions();
        switch (rhs_dims)
        {
        case 2:
            return sub2d2d(std::move(args));

        default:
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "sub_operation::sub2d",
                execution_tree::generate_error_message(
                    "the operands have incompatible number of dimensions",
                    name_, codename_));
        }
    }

    ///////////////////////////////////////////////////////////////////////////
    hpx::future<primitive_argument_type> sub_operation::eval(
        std::vector<primitive_argument_type> const& operands,
        std::vector<primitive_argument_type> const& args) const
    {
        if (operands.size() < 2)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "sub_operation::eval",
                execution_tree::generate_error_message(
                    "the sub_operation primitive requires at least two operands",
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
                "sub_operation::eval",
                execution_tree::generate_error_message(
                    "the sub_operation primitive requires that the "
                    "arguments given by the operands array are valid",
                    name_, codename_));
        }

        auto this_ = this->shared_from_this();
        if (operands.size() == 2)
        {
            return hpx::dataflow(hpx::launch::sync, hpx::util::unwrapping(
                [this_](arg_type&& lhs, arg_type&& rhs)
                -> primitive_argument_type
                {
                    std::size_t lhs_dims = lhs.num_dimensions();
                    switch (lhs_dims)
                    {
                    case 0:
                        return this_->sub0d(std::move(lhs), std::move(rhs));

                    case 1:
                        return this_->sub1d(std::move(lhs), std::move(rhs));

                    case 2:
                        return this_->sub2d(std::move(lhs), std::move(rhs));

                    default:
                        HPX_THROW_EXCEPTION(hpx::bad_parameter,
                            "sub_operation::eval",
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
            [this_](args_type&& args) -> primitive_argument_type
            {
                std::size_t lhs_dims = args[0].num_dimensions();
                switch (lhs_dims)
                {
                case 0:
                    return this_->sub0d(std::move(args));

                case 1:
                    return this_->sub1d(std::move(args));

                case 2:
                    return this_->sub2d(std::move(args));

                default:
                    HPX_THROW_EXCEPTION(hpx::bad_parameter,
                        "sub_operation::eval",
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

    //////////////////////////////////////////////////////////////////////////
    // Implement '-' for all possible combinations of lhs and rhs
    hpx::future<primitive_argument_type> sub_operation::eval(
        std::vector<primitive_argument_type> const& args) const
    {
        if (operands_.empty())
        {
            return eval(args, noargs);
        }
        return eval(operands_, args);
    }
}}}
