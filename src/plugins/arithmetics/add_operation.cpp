// Copyright (c) 2017-2018 Hartmut Kaiser
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>
#include <phylanx/ir/node_data.hpp>
#include <phylanx/plugins/arithmetics/add_operation.hpp>

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
    namespace detail
    {
        struct add_simd
        {
        public:
            explicit add_simd(double scalar)
                : scalar_(scalar)
            {
            }

            template <typename T>
            BLAZE_ALWAYS_INLINE auto operator()(T const& a) const
                ->  decltype(a + std::declval<double>())
            {
                return a + scalar_;
            }

            template <typename T>
            static constexpr bool simdEnabled()
            {
                return blaze::HasSIMDAdd<T, double>::value;
            }

            template <typename T>
            BLAZE_ALWAYS_INLINE decltype(auto) load(T const& a) const
            {
                BLAZE_CONSTRAINT_MUST_BE_SIMD_PACK(T);
                return a + blaze::set(scalar_);
            }

        private:
            double scalar_;
        };
    }

    ///////////////////////////////////////////////////////////////////////////
    match_pattern_type const add_operation::match_data =
    {
        hpx::util::make_tuple("__add",
            std::vector<std::string>{"_1 + __2", "__add(_1, __2)"},
            &create_add_operation, &create_primitive<add_operation>)
    };

    //////////////////////////////////////////////////////////////////////////
    add_operation::add_operation(
            std::vector<primitive_argument_type> && operands,
            std::string const& name, std::string const& codename)
      : primitive_component_base(std::move(operands), name, codename)
    {}

    ///////////////////////////////////////////////////////////////////////////
    primitive_argument_type add_operation::add0d0d(
        arg_type&& lhs, arg_type&& rhs) const
    {
        lhs.scalar() += rhs.scalar();
        return primitive_argument_type(std::move(lhs));
    }

    primitive_argument_type add_operation::add0d0d(args_type && args) const
    {
        arg_type& first_term = *args.begin();

        return primitive_argument_type{std::accumulate(
            args.begin() + 1, args.end(), std::move(first_term),
            [](arg_type& result, arg_type const& curr) -> arg_type
            {
                result.scalar() += curr.scalar();
                return std::move(result);
            })};
    }

    primitive_argument_type add_operation::add0d1d(
        arg_type&& lhs, arg_type&& rhs) const
    {
        if (rhs.is_ref())
        {
            rhs = blaze::map(rhs.vector(), detail::add_simd(lhs.scalar()));
        }
        else
        {
            rhs.vector() =
                blaze::map(rhs.vector(), detail::add_simd(lhs.scalar()));
        }
        return primitive_argument_type(std::move(rhs));
    }

    primitive_argument_type add_operation::add0d2d(
        arg_type&& lhs, arg_type&& rhs) const
    {
        if (rhs.is_ref())
        {
            rhs = blaze::map(rhs.matrix(), detail::add_simd(lhs.scalar()));
        }
        else
        {
            rhs.matrix() = blaze::map(
                rhs.matrix(), detail::add_simd(lhs.scalar()));
        }
        return primitive_argument_type(std::move(rhs));
    }

    primitive_argument_type add_operation::add0d(
        arg_type&& lhs, arg_type&& rhs) const
    {
        std::size_t rhs_dims = rhs.num_dimensions();
        switch (rhs_dims)
        {
        case 0:
            return add0d0d(std::move(lhs), std::move(rhs));

        case 1:
            return add0d1d(std::move(lhs), std::move(rhs));

        case 2:
            return add0d2d(std::move(lhs), std::move(rhs));

        default:
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "add_operation::add0d",
                "the operands have incompatible number of dimensions");
        }
    }

    primitive_argument_type add_operation::add0d(args_type && args) const
    {
        std::size_t rhs_dims = args[1].num_dimensions();
        switch (rhs_dims)
        {
        case 0:
            return add0d0d(std::move(args));

        default:
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "add_operation::add0d",
                execution_tree::generate_error_message(
                    "the operands have incompatible number of dimensions",
                    name_, codename_));
        }
    }

    ///////////////////////////////////////////////////////////////////////////
    primitive_argument_type add_operation::add1d0d(
        arg_type&& lhs, arg_type&& rhs) const
    {
        if (lhs.is_ref())
        {
            lhs = blaze::map(lhs.vector(), detail::add_simd(rhs.scalar()));
        }
        else
        {
            lhs.vector() =
                blaze::map(lhs.vector(), detail::add_simd(rhs.scalar()));
        }
        return primitive_argument_type(std::move(lhs));
    }

    primitive_argument_type add_operation::add1d1d(
        arg_type&& lhs, arg_type&& rhs) const
    {
        std::size_t lhs_size = lhs.dimension(0);
        std::size_t rhs_size = rhs.dimension(0);

        if (lhs_size != rhs_size)
        {
            // Propagation rule 2
            if (lhs_size == 1)
            {
                if (rhs.is_ref())
                {
                    rhs = blaze::map(
                        rhs.vector(), detail::add_simd(lhs.vector()[0]));
                }
                else
                {
                    rhs.vector() += blaze::map(
                        rhs.vector(), detail::add_simd(lhs.vector()[0]));
                }
            }
            else if (rhs_size == 1)
            {
                if (lhs.is_ref())
                {
                    lhs = blaze::map(
                        lhs.vector(), detail::add_simd(rhs.vector()[0]));
                }
                else
                {
                    lhs.vector() += blaze::map(
                        lhs.vector(), detail::add_simd(rhs.vector()[0]));
                }
            }
            else
            {
                HPX_THROW_EXCEPTION(hpx::bad_parameter,
                    "add_operation::add1d1d",
                    execution_tree::generate_error_message(
                        "the dimensions of the operands do not match", name_,
                        codename_));
            }
        }
        // Dimensions are identical
        else
        {
            // Avoid overwriting references, avoid memory reallocation when possible
            if (lhs.is_ref())
            {
                // Cannot reuse the memory if an operand is a reference
                if (rhs.is_ref())
                {
                    rhs = lhs.vector() + rhs.vector();
                }
                // Reuse the memory from rhs operand
                else
                {
                    rhs.vector() = lhs.vector() + rhs.vector();
                }
                return primitive_argument_type(std::move(rhs));
            }
            // Reuse the memory from lhs operand
            else
            {
                lhs.vector() += rhs.vector();
            }
        }

        return primitive_argument_type(std::move(lhs));
    }

    primitive_argument_type add_operation::add1d1d(args_type && args) const
    {
        std::size_t const operand_size = args[0].dimension(0);
        bool operands_same_size = true;
        for (auto const& i : args)
        {
            if (i.dimension(0) != operand_size)
            {
                operands_same_size = false;
                break;
            }
        }

        if (!operands_same_size)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "add_operation::add1d1d",
                execution_tree::generate_error_message(
                    "the dimensions of the operands do not match", name_,
                    codename_));
        }

        arg_type& first_term = *args.begin();
        return primitive_argument_type(std::accumulate(
            args.begin() + 1, args.end(), std::move(first_term),
            [](arg_type& result, arg_type const& curr) -> arg_type
            {
                if (result.is_ref())
                {
                    result = result.vector() + curr.vector();
                }
                else
                {
                    result.vector() += curr.vector();
                }
                return std::move(result);
            }));
    }

    primitive_argument_type add_operation::add1d2d(
        arg_type&& lhs, arg_type&& rhs) const
    {
        auto lhs_v = lhs.vector();
        auto rhs_m = rhs.matrix();

        // If the vector is effectively a scalar
        if (lhs_v.size() == 1)
        {
            if (rhs.is_ref())
            {
                rhs = blaze::map(rhs_m, detail::add_simd(lhs_v[0]));

                return primitive_argument_type{std::move(rhs)};
            }
            else
            {
                rhs_m = blaze::map(rhs_m, detail::add_simd(lhs_v[0]));

                return primitive_argument_type{std::move(rhs)};
            }
        }
        else if (rhs_m.columns() == lhs_v.size())
        {
            if (rhs.is_ref())
            {
                blaze::DynamicMatrix<double> result{
                    rhs_m.rows(), rhs_m.columns()};
                for (std::size_t i = 0; i != rhs_m.rows(); ++i)
                {
                    blaze::row(result, i) =
                        blaze::row(rhs_m, i) + blaze::trans(lhs_v);
                }
                return primitive_argument_type{std::move(result)};
            }
            else
            {
                for (std::size_t i = 0; i != rhs_m.rows(); ++i)
                {
                    blaze::row(rhs_m, i) += blaze::trans(lhs_v);
                }

                return primitive_argument_type{std::move(rhs)};
            }
        }
        else if (rhs_m.columns() == 1)
        {
            blaze::DynamicMatrix<double> result(rhs_m.rows(), lhs_v.size());

            for (std::size_t i = 0; i < result.columns(); ++i)
            {
                blaze::column(result, i) = blaze::column(rhs_m, 0);
            }

            for (std::size_t i = 0; i < result.rows(); ++i)
            {
                blaze::row(result, i) += blaze::trans(lhs_v);
            }

            return primitive_argument_type{std::move(result)};
        }

        HPX_THROW_EXCEPTION(hpx::bad_parameter,
            "add_operation::add1d2d",
            execution_tree::generate_error_message(
                "vector size does not match number of matrix columns",
                name_, codename_));
    }

    primitive_argument_type add_operation::add1d(
        arg_type&& lhs, arg_type&& rhs) const
    {
        std::size_t rhs_dims = rhs.num_dimensions();

        switch (rhs_dims)
        {
        case 0:
            return add1d0d(std::move(lhs), std::move(rhs));

        case 1:
            return add1d1d(std::move(lhs), std::move(rhs));

        case 2:
            return add1d2d(std::move(lhs), std::move(rhs));

        default:
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "add_operation::add1d",
                execution_tree::generate_error_message(
                    "the operands have incompatible number of dimensions",
                    name_, codename_));
        }
    }

    primitive_argument_type add_operation::add1d(args_type && args) const
    {
        std::size_t rhs_dims = args[1].num_dimensions();

        switch (rhs_dims)
        {
        case 1:
            return add1d1d(std::move(args));

        default:
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "add_operation::add1d",
                execution_tree::generate_error_message(
                    "the operands have incompatible number of dimensions",
                    name_, codename_));
        }
    }

    ///////////////////////////////////////////////////////////////////////////
    primitive_argument_type add_operation::add2d0d(
        arg_type&& lhs, arg_type&& rhs) const
    {
        if (lhs.is_ref())
        {
            lhs = blaze::map(lhs.matrix(), detail::add_simd(rhs.scalar()));
        }
        else
        {
            lhs.matrix() =
                blaze::map(lhs.matrix(), detail::add_simd(rhs.scalar()));
        }
        return primitive_argument_type(std::move(lhs));
    }

    primitive_argument_type add_operation::add2d1d(
        arg_type&& lhs, arg_type&& rhs) const
    {
        auto lhs_m = lhs.matrix();
        auto rhs_v = rhs.vector();

        // If the vector is effectively a scalar
        if (rhs_v.size() == 1)
        {
            if (lhs.is_ref())
            {
                lhs = blaze::map(lhs_m, detail::add_simd(rhs_v[0]));

                return primitive_argument_type{std::move(lhs)};
            }
            else
            {
                lhs_m = blaze::map(lhs_m, detail::add_simd(rhs_v[0]));

                return primitive_argument_type{std::move(lhs)};
            }
        }
        else if (lhs_m.columns() == rhs_v.size())
        {
            if (lhs.is_ref())
            {
                blaze::DynamicMatrix<double> result{
                    lhs_m.rows(), lhs_m.columns()};
                for (std::size_t i = 0; i != lhs_m.rows(); ++i)
                {
                    blaze::row(result, i) =
                        blaze::row(lhs_m, i) + blaze::trans(rhs_v);
                }
                return primitive_argument_type{std::move(result)};
            }
            else
            {
                for (std::size_t i = 0; i != lhs_m.rows(); ++i)
                {
                    blaze::row(lhs_m, i) += blaze::trans(rhs_v);
                }

                return primitive_argument_type{std::move(lhs)};
            }
        }
        else if (lhs_m.columns() == 1)
        {
            blaze::DynamicMatrix<double> result(lhs_m.rows(), rhs_v.size());

            for (std::size_t i = 0; i< result.columns(); ++i)
            {
                blaze::column(result, i) = blaze::column(lhs_m, 0);
            }

            for (std::size_t i = 0; i < result.rows(); ++i)
            {
                blaze::row(result, i) += blaze::trans(rhs_v);
            }

            return primitive_argument_type{std::move(result)};
        }

        HPX_THROW_EXCEPTION(hpx::bad_parameter,
            "add_operation::add2d1d",
            execution_tree::generate_error_message(
                "vector size does not match either the number of matrix "
                "columns nor rows.",
                name_, codename_));
    }

    add_operation::stretch_operand add_operation::get_stretch_dimension(
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

    primitive_argument_type add_operation::add2d2d_no_stretch(
        arg_type&& lhs, arg_type&& rhs) const
    {
        // Avoid overwriting references, avoid memory reallocation when possible
        if (lhs.is_ref())
        {
            // Cannot reuse the memory if an operand is a reference
            if (rhs.is_ref())
            {
                rhs = lhs.matrix() + rhs.matrix();
            }
            // Reuse the memory from rhs operand
            else
            {
                rhs.matrix() = lhs.matrix() + rhs.matrix();
            }
            return primitive_argument_type(std::move(rhs));
        }
        // Reuse the memory from lhs operand
        else
        {
            lhs.matrix() += rhs.matrix();
        }

        return primitive_argument_type(std::move(lhs));
    }

    primitive_argument_type add_operation::add2d2d_stretch_rows(
        arg_type&& lhs, arg_type&& rhs, stretch_operand stretch_rows) const
    {
        auto lhs_m = lhs.matrix();
        auto rhs_m = rhs.matrix();

        // Stretch lhs x axis
        if (stretch_rows == stretch_operand::lhs)
        {
            // Ensure we can reuse memory
            if (rhs.is_ref())
            {
                blaze::DynamicMatrix<double> result(
                    rhs.dimension(0), rhs.dimension(1));

                for (std::size_t i = 0; i != result.rows(); ++i)
                {
                    blaze::row(result, i) =
                        blaze::row(lhs_m, 0) +
                        blaze::row(rhs_m, i);
                }
                return primitive_argument_type{std::move(result)};
            }
            else
            {
                for (std::size_t i = 0; i != rhs.dimension(0); ++i)
                {
                    blaze::row(rhs_m, i) =
                        blaze::row(lhs_m, 0) +
                        blaze::row(rhs_m, i);
                }
                return primitive_argument_type{std::move(rhs)};
            }
        }
        // Stretch rhs x axis
        else if (stretch_rows == stretch_operand::rhs)
        {
            // Ensure we can reuse memory
            if (lhs.is_ref())
            {
                blaze::DynamicMatrix<double> result(
                    lhs.dimension(0), lhs.dimension(1));

                for (std::size_t i = 0; i != result.rows(); ++i)
                {
                    blaze::row(result, i) =
                        blaze::row(lhs_m, i) +
                        blaze::row(rhs_m, 0);
                }
                return primitive_argument_type{std::move(result)};
            }
            else
            {
                for (std::size_t i = 0; i != lhs.dimension(0); ++i)
                {
                    blaze::row(lhs_m, i) =
                        blaze::row(lhs_m, i) +
                        blaze::row(rhs_m, 0);
                }
                return primitive_argument_type{std::move(lhs)};
            }
        }
        HPX_THROW_EXCEPTION(hpx::bad_parameter,
            "add_operation::add2d2d_stretch_rows",
            execution_tree::generate_error_message(
                "the dimensions of the operands do not match", name_,
                codename_));
    }

    primitive_argument_type add_operation::add2d2d_stretch_cols(
        arg_type&& lhs, arg_type&& rhs, stretch_operand stretch_cols) const
    {
        auto lhs_m = lhs.matrix();
        auto rhs_m = rhs.matrix();

        // Stretch lhs y axis
        if (stretch_cols == stretch_operand::lhs)
        {
            // Avoid overwriting references, avoid memory
            // reallocation when possible
            if (rhs.is_ref())
            {
                blaze::DynamicMatrix<double> result(
                    rhs.dimension(0), rhs.dimension(1));

                for (std::size_t i = 0; i != result.columns(); ++i)
                {
                    blaze::column(result, i) =
                        blaze::column(lhs_m, 0) +
                        blaze::column(rhs_m, i);
                }
                return primitive_argument_type{std::move(result)};
            }
            else
            {
                for (std::size_t i = 0; i != rhs.dimension(1); ++i)
                {
                    blaze::column(rhs_m, i) =
                        blaze::column(lhs_m, 0) + blaze::column(rhs_m, i);
                }
                return primitive_argument_type{std::move(rhs)};
            }
        }
        // Stretch rhs y axis
        else if (stretch_cols == stretch_operand::rhs)
        {
            // Avoid overwriting references, avoid memory
            // reallocation when possible
            if (lhs.is_ref())
            {
                blaze::DynamicMatrix<double> result(
                    lhs.dimension(0), lhs.dimension(1));

                for (std::size_t i = 0; i != result.columns(); ++i)
                {
                    blaze::column(result, i) =
                        blaze::column(lhs_m, i) +
                        blaze::column(rhs_m, 0);
                }
                return primitive_argument_type{std::move(result)};
            }
            else
            {
                for (std::size_t i = 0; i != lhs.dimension(1); ++i)
                {
                    blaze::column(lhs_m, i) =
                        blaze::column(lhs_m, i) +
                        blaze::column(rhs_m, 0);
                }
                return primitive_argument_type{std::move(lhs)};
            }
        }
        HPX_THROW_EXCEPTION(hpx::bad_parameter,
            "add_operation::add2d2d_stretch_cols",
            execution_tree::generate_error_message(
                "the dimensions of the operands do not match", name_,
                codename_));
    }

    primitive_argument_type add_operation::add2d2d_stretch_both(arg_type&& lhs,
        arg_type&& rhs, stretch_operand stretch_rows,
        stretch_operand stretch_cols) const
    {
        auto lhs_m = lhs.matrix();
        auto rhs_m = rhs.matrix();

        // If lhs rows needs stretching
        if (stretch_rows == stretch_operand::lhs)
        {
            // If both lhs axes need stretching
            if (stretch_cols == stretch_operand::lhs)
            {
                if (lhs.is_ref())
                {
                    lhs = blaze::map(rhs_m, detail::add_simd(lhs_m(0, 0)));
                }
                else
                {
                    lhs.matrix() = blaze::map(rhs_m, detail::add_simd(lhs_m(0, 0)));
                }

                return primitive_argument_type{std::move(lhs)};
            }
            // If lhs rows and rhs cols need stretching
            else
            {
                blaze::DynamicMatrix<double> result(rhs_m.rows(), lhs_m.columns());
                for (std::size_t i = 0; i < result.rows(); ++i)
                {
                    blaze::row(result, i) = blaze::row(lhs_m, 0);
                }
                for (std::size_t i = 0; i < result.columns(); ++i)
                {
                    blaze::column(result, i) += blaze::column(rhs_m, 0);
                }
                return primitive_argument_type{std::move(result)};
            }
        }
        // If lhs cols needs stretching
        else
        {
            // If lhs cols and rhs rows need stretching
            if (stretch_cols == stretch_operand::lhs)
            {
                blaze::DynamicMatrix<double> result(
                    lhs_m.rows(), rhs_m.columns());
                for (std::size_t i = 0; i < result.columns(); ++i)
                {
                    blaze::column(result, i) = blaze::column(lhs_m, 0);
                }
                for (std::size_t i = 0; i < result.rows(); ++i)
                {
                    blaze::row(result, i) += blaze::row(rhs_m, 0);
                }
                return primitive_argument_type{std::move(result)};
            }
            // If both rhs axes need stretching
            else
            {
                if (lhs.is_ref())
                {
                    lhs = blaze::map(lhs_m, detail::add_simd(rhs_m(0, 0)));
                }
                else
                {
                    lhs.matrix() = blaze::map(lhs_m, detail::add_simd(rhs_m(0, 0)));
                }

                return primitive_argument_type{std::move(lhs)};
            }
        }

        HPX_THROW_EXCEPTION(hpx::bad_parameter,
            "add_operation::add2d2d_stretch_both",
            execution_tree::generate_error_message(
                "the dimensions of the operands do not match", name_,
                codename_));
    }

    primitive_argument_type add_operation::add2d2d(
        arg_type&& lhs, arg_type&& rhs) const
    {
        auto lhs_size = lhs.dimensions();
        auto rhs_size = rhs.dimensions();

        if (lhs_size != rhs_size)
        {
            // Broadcasting rules
            // In case x dimensions do not match
            stretch_operand stretch_rows =
                get_stretch_dimension(lhs_size[0], rhs_size[0]);
            // In case y dimensions do not match
            stretch_operand stretch_cols =
                get_stretch_dimension(lhs_size[1], rhs_size[1]);

            // Stretch if stretching is possible
            if (stretch_rows != stretch_operand::neither ||
                stretch_cols != stretch_operand::neither)
            {
                // If only y axis needs stretching
                if (stretch_rows == stretch_operand::neither)
                {
                    return add2d2d_stretch_cols(
                            std::move(lhs), std::move(rhs), stretch_cols);
                }
                // If only x axis needs stretching
                else if(stretch_cols == stretch_operand::neither)
                {
                    return add2d2d_stretch_rows(
                            std::move(lhs), std::move(rhs), stretch_rows);
                }
                // If both lhs and rhs must be stretched
                // Separated because memory cannot be reused
                else
                {
                    return add2d2d_stretch_both(std::move(lhs), std::move(rhs),
                        stretch_rows, stretch_cols);
                }
            }
        }
        // Dimensions are identical
        else
        {
            return add2d2d_no_stretch(std::move(lhs), std::move(rhs));
        }

        HPX_THROW_EXCEPTION(hpx::bad_parameter,
            "add_operation::add2d2d",
            execution_tree::generate_error_message(
                "the dimensions of the operands do not match", name_,
                codename_));
    }

    primitive_argument_type add_operation::add2d2d(args_type && args) const
    {
        auto const operand_size = args[0].dimensions();
        bool operands_same_size = true;
        for (auto const& i : args)
        {
            if (i.dimensions() != operand_size)
            {
                operands_same_size = false;
                break;
            }
        }

        if (!operands_same_size)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "add_operation::add2d2d",
                execution_tree::generate_error_message(
                    "the dimensions of the operands do not match",
                    name_, codename_));
        }

        arg_type& first_term = *args.begin();
        return primitive_argument_type{std::accumulate(
            args.begin() + 1, args.end(), std::move(first_term),
            [](arg_type& result, arg_type const& curr) -> arg_type
            {
                if (result.is_ref())
                {
                    result = result.matrix() + curr.matrix();
                }
                else
                {
                    result.matrix() += curr.matrix();
                }
                return std::move(result);
            })};
    }

    primitive_argument_type add_operation::add2d(
        arg_type&& lhs, arg_type&& rhs) const
    {
        std::size_t rhs_dims = rhs.num_dimensions();
        switch (rhs_dims)
        {
        case 0:
            return add2d0d(std::move(lhs), std::move(rhs));

        case 2:
            return add2d2d(std::move(lhs), std::move(rhs));

        case 1:
            return add2d1d(std::move(lhs), std::move(rhs));

        default:
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "add_operation::add2d",
                execution_tree::generate_error_message(
                    "the operands have incompatible number of dimensions",
                    name_, codename_));
        }
    }

    primitive_argument_type add_operation::add2d(args_type && args) const
    {
        std::size_t rhs_dims = args[1].num_dimensions();
        switch (rhs_dims)
        {
        case 2:
            return add2d2d(std::move(args));

        default:
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "add_operation::add2d",
                execution_tree::generate_error_message(
                    "the operands have incompatible number of dimensions",
                    name_, codename_));
        }
    }

    hpx::future<primitive_argument_type> add_operation::eval(
        std::vector<primitive_argument_type> const& operands,
        std::vector<primitive_argument_type> const& args) const
    {
        if (operands.size() < 2)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "add_operation::eval",
                execution_tree::generate_error_message(
                    "the add_operation primitive requires at least two "
                        "operands",
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
                "add_operation::eval",
                execution_tree::generate_error_message(
                    "the add_operation primitive requires that the "
                    "arguments given by the operands array are valid",
                    name_, codename_));
        }

        auto this_ = this->shared_from_this();
        if (operands.size() == 2)
        {
            // special case for 2 operands
            return hpx::dataflow(hpx::launch::sync, hpx::util::unwrapping(
                [this_](arg_type&& lhs, arg_type&& rhs)
                -> primitive_argument_type
                {
                    std::size_t lhs_dims = lhs.num_dimensions();
                    switch (lhs_dims)
                    {
                    case 0:
                        return this_->add0d(std::move(lhs), std::move(rhs));

                    case 1:
                        return this_->add1d(std::move(lhs), std::move(rhs));

                    case 2:
                        return this_->add2d(std::move(lhs), std::move(rhs));

                    default:
                        HPX_THROW_EXCEPTION(hpx::bad_parameter,
                            "add_operation::eval",
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
                    return this_->add0d(std::move(args));

                case 1:
                    return this_->add1d(std::move(args));

                case 2:
                    return this_->add2d(std::move(args));

                default:
                    HPX_THROW_EXCEPTION(hpx::bad_parameter,
                        "add_operation::eval",
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

    // Implement '+' for all possible combinations of lhs and rhs
    hpx::future<primitive_argument_type> add_operation::eval(
        std::vector<primitive_argument_type> const& args) const
    {
        if (operands_.empty())
        {
            return eval(args, noargs);
        }
        return eval(operands_, args);
    }
}}}
