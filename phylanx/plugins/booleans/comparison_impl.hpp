//  Copyright (c) 2017-2019 Hartmut Kaiser
//  Copyright (c) 2018 Shahrzad Shirzad
//  Copyright (c) 2018 Tianyi Zhang
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(PHYLANX_PRIMITIVES_COMPARISON_IMPL_SEP_02_2018_0443PM)
#define PHYLANX_PRIMITIVES_COMPARISON_IMPL_SEP_02_2018_0443PM

#include <phylanx/config.hpp>
#include <phylanx/ir/node_data.hpp>
#include <phylanx/ir/ranges.hpp>
#include <phylanx/plugins/booleans/comparison.hpp>

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
#if defined(PHYLANX_HAVE_BLAZE_TENSOR)
#include <blaze_tensor/Math.h>
#endif

///////////////////////////////////////////////////////////////////////////////
namespace phylanx { namespace execution_tree { namespace primitives
{
    ///////////////////////////////////////////////////////////////////////////
    template <typename Op>
    comparison<Op>::comparison(primitive_arguments_type&& operands,
            std::string const& name, std::string const& codename)
      : primitive_component_base(std::move(operands), name, codename)
    {}

    ///////////////////////////////////////////////////////////////////////////
    template <typename Op>
    template <typename T>
    primitive_argument_type comparison<Op>::comparison0d0d(
        ir::node_data<T>&& lhs, ir::node_data<T>&& rhs,
        bool propagate_type) const
    {
        if (propagate_type)
        {
            return primitive_argument_type(ir::node_data<T>{
                Op{}(lhs.scalar(), rhs.scalar()) ? T(1) : T(0)});
        }

        return primitive_argument_type(
            ir::node_data<std::uint8_t>{Op{}(lhs.scalar(), rhs.scalar())});
    }

    template <typename Op>
    template <typename T>
    primitive_argument_type comparison<Op>::comparison0d1d(
        ir::node_data<T>&& lhs, ir::node_data<T>&& rhs,
        bool propagate_type) const
    {
        // TODO: SIMD functionality should be added, blaze implementation
        // is not currently available
        if (rhs.is_ref())
        {
            rhs = blaze::map(rhs.vector(),
                [&](T x) { return Op{}(lhs.scalar(), x); });
        }
        else
        {
            rhs.vector() = blaze::map(rhs.vector(),
                [&](T x) { return Op{}(lhs.scalar(), x); });
        }

        if (propagate_type)
        {
            return primitive_argument_type(ir::node_data<T>{std::move(rhs)});
        }
        return primitive_argument_type(
            ir::node_data<std::uint8_t>{std::move(rhs)});
    }

    template <typename Op>
    template <typename T>
    primitive_argument_type comparison<Op>::comparison0d2d(
        ir::node_data<T>&& lhs, ir::node_data<T>&& rhs,
        bool propagate_type) const
    {
        // TODO: SIMD functionality should be added, blaze implementation
        // is not currently available
        if (rhs.is_ref())
        {
            rhs = blaze::map(rhs.matrix(),
                [&](T x) -> std::uint8_t { return Op{}(lhs.scalar(), x); });
        }
        else
        {
            rhs.matrix() = blaze::map(rhs.matrix(),
                [&](T x) -> std::uint8_t { return Op{}(lhs.scalar(), x); });
        }

        if (propagate_type)
        {
            return primitive_argument_type(ir::node_data<T>{std::move(rhs)});
        }

        return primitive_argument_type(
            ir::node_data<std::uint8_t>{std::move(rhs)});
    }

#if defined(PHYLANX_HAVE_BLAZE_TENSOR)
    template <typename Op>
    template <typename T>
    primitive_argument_type comparison<Op>::comparison0d3d(
        ir::node_data<T>&& lhs, ir::node_data<T>&& rhs,
        bool propagate_type) const
    {
        // TODO: SIMD functionality should be added, blaze implementation
        // is not currently available
        if (rhs.is_ref())
        {
            rhs = blaze::map(rhs.tensor(),
                [&](T x) -> std::uint8_t { return Op{}(lhs.scalar(), x); });
        }
        else
        {
            rhs.tensor() = blaze::map(rhs.tensor(),
                [&](T x) -> std::uint8_t { return Op{}(lhs.scalar(), x); });
        }

        if (propagate_type)
        {
            return primitive_argument_type(ir::node_data<T>{std::move(rhs)});
        }

        return primitive_argument_type(
            ir::node_data<std::uint8_t>{std::move(rhs)});
    }
#endif

    template <typename Op>
    template <typename T>
    primitive_argument_type comparison<Op>::comparison0d(
        ir::node_data<T>&& lhs, ir::node_data<T>&& rhs,
        bool propagate_type) const
    {
        std::size_t rhs_dims = rhs.num_dimensions();
        switch(rhs_dims)
        {
        case 0:
            return comparison0d0d(
                std::move(lhs), std::move(rhs), propagate_type);

        case 1:
            return comparison0d1d(
                std::move(lhs), std::move(rhs), propagate_type);

        case 2:
            return comparison0d2d(
                std::move(lhs), std::move(rhs), propagate_type);

#if defined(PHYLANX_HAVE_BLAZE_TENSOR)
        case 3:
            return comparison0d3d(
                std::move(lhs), std::move(rhs), propagate_type);
#endif

        default:
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "comparison<Op>::comparison0d",
                util::generate_error_message(
                    "the operands have incompatible number of "
                        "dimensions",
                    name_, codename_));
        }
    }

    template <typename Op>
    template <typename T>
    primitive_argument_type comparison<Op>::comparison1d0d(
        ir::node_data<T>&& lhs, ir::node_data<T>&& rhs,
        bool propagate_type) const
    {
        // TODO: SIMD functionality should be added, blaze implementation
        // is not currently available
        if (lhs.is_ref())
        {
            lhs = blaze::map(lhs.vector(),
                [&](T x) -> std::uint8_t { return Op{}(x, rhs.scalar()); });
        }
        else
        {
            lhs.vector() = blaze::map(lhs.vector(),
                [&](T x) -> std::uint8_t { return Op{}(x, rhs.scalar()); });
        }

        if (propagate_type)
        {
            return primitive_argument_type(ir::node_data<T>{std::move(lhs)});
        }

        return primitive_argument_type(
            ir::node_data<std::uint8_t>{std::move(lhs)});
    }

    template <typename Op>
    template <typename T>
    primitive_argument_type comparison<Op>::comparison1d1d(
        ir::node_data<T>&& lhs, ir::node_data<T>&& rhs,
        bool propagate_type) const
    {
        std::size_t lhs_size = lhs.dimension(0);
        std::size_t rhs_size = rhs.dimension(0);

        if (lhs_size != rhs_size)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "comparison<Op>::comparison1d1d",
                util::generate_error_message(
                    "the dimensions of the operands do not match",
                    name_, codename_));
        }

        // TODO: SIMD functionality should be added, blaze implementation
        // is not currently available
        if (lhs.is_ref())
        {
            lhs = blaze::map(lhs.vector(), rhs.vector(),
                [&](T x, T y) -> std::uint8_t { return Op{}(x, y); });
        }
        else
        {
            lhs.vector() = blaze::map(lhs.vector(), rhs.vector(),
                [&](T x, T y) -> std::uint8_t { return Op{}(x, y); });
        }

        if (propagate_type)
        {
            return primitive_argument_type(ir::node_data<T>{std::move(lhs)});
        }

        return primitive_argument_type(
            ir::node_data<std::uint8_t>{std::move(lhs)});
    }

    template <typename Op>
    template <typename T>
    primitive_argument_type comparison<Op>::comparison1d2d(
        ir::node_data<T>&& lhs, ir::node_data<T>&& rhs,
        bool propagate_type) const
    {
        auto cv = lhs.vector();
        auto cm = rhs.matrix();

        if (cv.size() != cm.columns())
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "comparison<Op>::comparison1d2d",
                util::generate_error_message(
                    "the dimensions of the operands do not match",
                    name_, codename_));
        }

        // TODO: SIMD functionality should be added, blaze implementation
        // is not currently available
        if (rhs.is_ref())
        {
            blaze::DynamicMatrix<std::uint8_t> m{cm.rows(), cm.columns()};

            for (std::size_t i = 0UL; i != cm.rows(); ++i)
            {
                blaze::row(m, i) =
                    blaze::map(blaze::trans(cv), blaze::row(cm, i),
                        [](T x, T y) -> std::uint8_t { return Op{}(x, y); });
            }

            if (propagate_type)
            {
                return primitive_argument_type(ir::node_data<T>{std::move(m)});
            }

            return primitive_argument_type(
                ir::node_data<std::uint8_t>{std::move(m)});
        }

        for (std::size_t i = 0UL; i != cm.rows(); ++i)
        {
            blaze::row(cm, i) = blaze::map(blaze::trans(cv), blaze::row(cm, i),
                [](T x, T y) -> std::uint8_t { return Op{}(x, y); });
        }

        if (propagate_type)
        {
            return primitive_argument_type(ir::node_data<T>{std::move(rhs)});
        }

        return primitive_argument_type(
            ir::node_data<std::uint8_t>{std::move(rhs)});
    }

#if defined(PHYLANX_HAVE_BLAZE_TENSOR)
    template <typename Op>
    template <typename T>
    primitive_argument_type comparison<Op>::comparison1d3d(
        ir::node_data<T>&& lhs, ir::node_data<T>&& rhs,
        bool propagate_type) const
    {
        auto cv = lhs.vector();
        auto ct = rhs.tensor();

        if (cv.size() != ct.columns())
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "comparison<Op>::comparison1d3d",
                util::generate_error_message(
                    "the dimensions of the operands do not match",
                    name_, codename_));
        }

        // TODO: SIMD functionality should be added, blaze implementation
        // is not currently available
        if (rhs.is_ref())
        {
            blaze::DynamicTensor<std::uint8_t> t{
                ct.pages(), ct.rows(), ct.columns()};

            for (std::size_t k = 0UL; k != ct.pages(); ++k)
            {
                auto lhs_page = blaze::pageslice(t, k);
                auto rhs_page = blaze::pageslice(ct, k);
                for (std::size_t i = 0UL; i != ct.rows(); ++i)
                {
                    blaze::row(lhs_page, i) = blaze::map(blaze::trans(cv),
                        blaze::row(rhs_page, i),
                        [](T x, T y) -> std::uint8_t { return Op{}(x, y); });
                }
            }

            if (propagate_type)
            {
                return primitive_argument_type(ir::node_data<T>{std::move(t)});
            }

            return primitive_argument_type(
                ir::node_data<std::uint8_t>{std::move(t)});
        }

        for (std::size_t k = 0UL; k != ct.pages(); ++k)
        {
            auto page = blaze::pageslice(ct, k);
            for (std::size_t i = 0UL; i != ct.rows(); ++i)
            {
                auto row = blaze::row(page, i);
                row = blaze::map(blaze::trans(cv), row,
                    [](T x, T y) -> std::uint8_t { return Op{}(x, y); });
            }
        }

        if (propagate_type)
        {
            return primitive_argument_type(ir::node_data<T>{std::move(rhs)});
        }

        return primitive_argument_type(
            ir::node_data<std::uint8_t>{std::move(rhs)});
    }
#endif

    template <typename Op>
    template <typename T>
    primitive_argument_type comparison<Op>::comparison1d(
        ir::node_data<T>&& lhs, ir::node_data<T>&& rhs,
        bool propagate_type) const
    {
        std::size_t rhs_dims = rhs.num_dimensions();
        switch(rhs_dims)
        {
        case 0:
            return comparison1d0d(
                std::move(lhs), std::move(rhs), propagate_type);

        case 1:
            return comparison1d1d(
                std::move(lhs), std::move(rhs), propagate_type);

        case 2:
            return comparison1d2d(
                std::move(lhs), std::move(rhs), propagate_type);

#if defined(PHYLANX_HAVE_BLAZE_TENSOR)
        case 3:
            return comparison1d3d(
                std::move(lhs), std::move(rhs), propagate_type);
#endif
        default:
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "comparison<Op>::comparison1d",
                util::generate_error_message(
                    "the operands have incompatible number of "
                        "dimensions",
                    name_, codename_));
        }
    }

    template <typename Op>
    template <typename T>
    primitive_argument_type comparison<Op>::comparison2d0d(
        ir::node_data<T>&& lhs, ir::node_data<T>&& rhs,
        bool propagate_type) const
    {
        std::size_t lhs_size = lhs.dimension(0);
        std::size_t rhs_size = rhs.dimension(0);

        // TODO: SIMD functionality should be added, blaze implementation
        // is not currently available
        if (lhs.is_ref())
        {
            lhs = blaze::map(lhs.matrix(),
                [&](T x) -> std::uint8_t { return Op{}(x, rhs.scalar()); });
        }
        else
        {
            lhs.matrix() = blaze::map(lhs.matrix(),
                [&](T x) -> std::uint8_t { return Op{}(x, rhs.scalar()); });
        }

        if (propagate_type)
        {
            return primitive_argument_type(ir::node_data<T>{std::move(lhs)});
        }

        return primitive_argument_type(
            ir::node_data<std::uint8_t>{std::move(lhs)});
    }

    template <typename Op>
    template <typename T>
    primitive_argument_type comparison<Op>::comparison2d1d(
        ir::node_data<T>&& lhs, ir::node_data<T>&& rhs,
        bool propagate_type) const
    {
        auto cv = rhs.vector();
        auto cm = lhs.matrix();

        if (cv.size() != cm.columns())
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "comparison<Op>::comparison2d1d",
                util::generate_error_message(
                    "the dimensions of the operands do not match",
                    name_, codename_));
        }

        // TODO: SIMD functionality should be added, blaze implementation
        // is not currently available
        if (lhs.is_ref())
        {
            blaze::DynamicMatrix<std::uint8_t> m{cm.rows(), cm.columns()};

            for (std::size_t i = 0UL; i != cm.rows(); ++i)
            {
                blaze::row(m, i) = blaze::map(blaze::row(cm, i),
                    blaze::trans(cv),
                    [](T x, T y) -> std::uint8_t { return Op{}(x, y); });
            }

            if (propagate_type)
            {
                return primitive_argument_type(ir::node_data<T>{std::move(m)});
            }

            return primitive_argument_type(
                ir::node_data<std::uint8_t>{std::move(m)});
        }

        for (std::size_t i = 0UL; i != cm.rows(); ++i)
        {
            blaze::row(cm, i) = blaze::map(blaze::row(cm, i),
                blaze::trans(cv),
                [](T x, T y) -> std::uint8_t { return Op{}(x, y); });
        }

        if (propagate_type)
        {
            return primitive_argument_type(ir::node_data<T>{std::move(lhs)});
        }

        return primitive_argument_type(
            ir::node_data<std::uint8_t>{std::move(lhs)});
    }

    template <typename Op>
    template <typename T>
    primitive_argument_type comparison<Op>::comparison2d2d(
        ir::node_data<T>&& lhs, ir::node_data<T>&& rhs,
        bool propagate_type) const
    {
        auto lhs_size = lhs.dimensions();
        auto rhs_size = rhs.dimensions();

        if (lhs_size != rhs_size)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "comparison<Op>::comparison2d2d",
                util::generate_error_message(
                    "the dimensions of the operands do not match",
                    name_, codename_));
        }

        // TODO: SIMD functionality should be added, blaze implementation
        //       is not currently available
        if (lhs.is_ref())
        {
            lhs = blaze::map(lhs.matrix(), rhs.matrix(),
                [&](T x, T y) -> std::uint8_t { return Op{}(x, y); });
        }
        else
        {
            lhs.matrix() = blaze::map(lhs.matrix(), rhs.matrix(),
                [&](T x, T y) -> std::uint8_t { return Op{}(x, y); });
        }

        if (propagate_type)
        {
            return primitive_argument_type(ir::node_data<T>{std::move(lhs)});
        }

        return primitive_argument_type(
            ir::node_data<std::uint8_t>{std::move(lhs)});
    }

#if defined(PHYLANX_HAVE_BLAZE_TENSOR)
    template <typename Op>
    template <typename T>
    primitive_argument_type comparison<Op>::comparison2d3d(
        ir::node_data<T>&& lhs, ir::node_data<T>&& rhs,
        bool propagate_type) const
    {
        auto cm = lhs.matrix();
        auto ct = rhs.tensor();

        if (cm.rows() != ct.rows() || cm.columns() != ct.columns())
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "comparison<Op>::comparison2d3d",
                util::generate_error_message(
                    "the dimensions of the operands do not match",
                    name_, codename_));
        }

        // TODO: SIMD functionality should be added, blaze implementation
        //       is not currently available
        if (rhs.is_ref())
        {
            blaze::DynamicTensor<std::uint8_t> t{
                ct.pages(), ct.rows(), ct.columns()};

            for (std::size_t k = 0UL; k != ct.pages(); ++k)
            {
                blaze::pageslice(t, k) = blaze::map(cm, blaze::pageslice(ct, k),
                    [&](T x, T y) -> std::uint8_t { return Op{}(x, y); });
            }

            if (propagate_type)
            {
                return primitive_argument_type(ir::node_data<T>{std::move(t)});
            }

            return primitive_argument_type(
                ir::node_data<std::uint8_t>{std::move(t)});
        }

        for (std::size_t k = 0UL; k != ct.pages(); ++k)
        {
            auto page = blaze::pageslice(ct, k);
            page = blaze::map(
                cm, page, [&](T x, T y) -> std::uint8_t { return Op{}(x, y); });
        }

        if (propagate_type)
        {
            return primitive_argument_type(ir::node_data<T>{std::move(rhs)});
        }

        return primitive_argument_type(
            ir::node_data<std::uint8_t>{std::move(rhs)});
    }
#endif

    template <typename Op>
    template <typename T>
    primitive_argument_type comparison<Op>::comparison2d(
        ir::node_data<T>&& lhs, ir::node_data<T>&& rhs,
        bool propagate_type) const
    {
        std::size_t rhs_dims = rhs.num_dimensions();
        switch(rhs_dims)
        {
        case 0:
            return comparison2d0d(
                std::move(lhs), std::move(rhs), propagate_type);

        case 1:
            return comparison2d1d(
                std::move(lhs), std::move(rhs), propagate_type);

        case 2:
            return comparison2d2d(
                std::move(lhs), std::move(rhs), propagate_type);

#if defined(PHYLANX_HAVE_BLAZE_TENSOR)
        case 3:
            return comparison2d3d(
                std::move(lhs), std::move(rhs), propagate_type);
#endif
        default:
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "comparison<Op>::comparison2d",
                util::generate_error_message(
                    "the operands have incompatible number of "
                        "dimensions",
                    name_, codename_));
        }
    }

#if defined(PHYLANX_HAVE_BLAZE_TENSOR)
    template <typename Op>
    template <typename T>
    primitive_argument_type comparison<Op>::comparison3d0d(
        ir::node_data<T>&& lhs, ir::node_data<T>&& rhs,
        bool propagate_type) const
    {
        // TODO: SIMD functionality should be added, blaze implementation
        // is not currently available
        if (lhs.is_ref())
        {
            lhs = blaze::map(lhs.tensor(),
                [&](T x) -> std::uint8_t { return Op{}(x, rhs.scalar()); });
        }
        else
        {
            lhs.tensor() = blaze::map(lhs.tensor(),
                [&](T x) -> std::uint8_t { return Op{}(x, rhs.scalar()); });
        }

        if (propagate_type)
        {
            return primitive_argument_type(ir::node_data<T>{std::move(lhs)});
        }

        return primitive_argument_type(
            ir::node_data<std::uint8_t>{std::move(lhs)});
    }

    template <typename Op>
    template <typename T>
    primitive_argument_type comparison<Op>::comparison3d1d(
        ir::node_data<T>&& lhs, ir::node_data<T>&& rhs,
        bool propagate_type) const
    {
        auto ct = lhs.tensor();
        auto cv = rhs.vector();

        if (cv.size() != ct.columns())
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "comparison<Op>::comparison3d1d",
                util::generate_error_message(
                    "the dimensions of the operands do not match",
                    name_, codename_));
        }

        // TODO: SIMD functionality should be added, blaze implementation
        // is not currently available
        if (lhs.is_ref())
        {
            blaze::DynamicTensor<std::uint8_t> t{
                ct.pages(), ct.rows(), ct.columns()};

            for (std::size_t k = 0UL; k != ct.pages(); ++k)
            {
                auto lhs_page = blaze::pageslice(t, k);
                auto rhs_page = blaze::pageslice(ct, k);
                for (std::size_t i = 0UL; i != ct.rows(); ++i)
                {
                    blaze::row(lhs_page, i) = blaze::map(
                        blaze::row(rhs_page, i), blaze::trans(cv),
                        [](T x, T y) -> std::uint8_t { return Op{}(x, y); });
                }
            }

            if (propagate_type)
            {
                return primitive_argument_type(ir::node_data<T>{std::move(t)});
            }

            return primitive_argument_type(
                ir::node_data<std::uint8_t>{std::move(t)});
        }

        for (std::size_t k = 0UL; k != ct.pages(); ++k)
        {
            auto page = blaze::pageslice(ct, k);
            for (std::size_t i = 0UL; i != ct.rows(); ++i)
            {
                auto row = blaze::row(page, i);
                row = blaze::map(row, blaze::trans(cv),
                    [](T x, T y) -> std::uint8_t { return Op{}(x, y); });
            }
        }

        if (propagate_type)
        {
            return primitive_argument_type(ir::node_data<T>{std::move(lhs)});
        }

        return primitive_argument_type(
            ir::node_data<std::uint8_t>{std::move(lhs)});
    }

    template <typename Op>
    template <typename T>
    primitive_argument_type comparison<Op>::comparison3d2d(
        ir::node_data<T>&& lhs, ir::node_data<T>&& rhs,
        bool propagate_type) const
    {
        auto ct = lhs.tensor();
        auto cm = rhs.matrix();

        if (cm.rows() != ct.rows() || cm.columns() != ct.columns())
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "comparison<Op>::comparison2d3d",
                util::generate_error_message(
                    "the dimensions of the operands do not match",
                    name_, codename_));
        }

        // TODO: SIMD functionality should be added, blaze implementation
        //       is not currently available
        if (lhs.is_ref())
        {
            blaze::DynamicTensor<std::uint8_t> t{
                ct.pages(), ct.rows(), ct.columns()};

            for (std::size_t k = 0UL; k != ct.pages(); ++k)
            {
                blaze::pageslice(t, k) = blaze::map(blaze::pageslice(ct, k), cm,
                    [&](T x, T y) -> std::uint8_t { return Op{}(x, y); });
            }

            if (propagate_type)
            {
                return primitive_argument_type(ir::node_data<T>{std::move(t)});
            }

            return primitive_argument_type(
                ir::node_data<std::uint8_t>{std::move(t)});
        }

        for (std::size_t k = 0UL; k != ct.pages(); ++k)
        {
            auto page = blaze::pageslice(ct, k);
            page = blaze::map(
                page, cm, [&](T x, T y) -> std::uint8_t { return Op{}(x, y); });
        }

        if (propagate_type)
        {
            return primitive_argument_type(ir::node_data<T>{std::move(lhs)});
        }

        return primitive_argument_type(
            ir::node_data<std::uint8_t>{std::move(lhs)});
    }

    template <typename Op>
    template <typename T>
    primitive_argument_type comparison<Op>::comparison3d3d(
        ir::node_data<T>&& lhs, ir::node_data<T>&& rhs,
        bool propagate_type) const
    {
        auto lhs_size = lhs.dimensions();
        auto rhs_size = rhs.dimensions();

        if (lhs_size != rhs_size)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "comparison<Op>::comparison3d3d",
                util::generate_error_message(
                    "the dimensions of the operands do not match",
                    name_, codename_));
        }

        // TODO: SIMD functionality should be added, blaze implementation
        //       is not currently available
        if (lhs.is_ref())
        {
            lhs = blaze::map(lhs.tensor(), rhs.tensor(),
                [&](T x, T y) -> std::uint8_t { return Op{}(x, y); });
        }
        else
        {
            lhs.tensor() = blaze::map(lhs.tensor(), rhs.tensor(),
                [&](T x, T y) -> std::uint8_t { return Op{}(x, y); });
        }

        if (propagate_type)
        {
            return primitive_argument_type(ir::node_data<T>{std::move(lhs)});
        }

        return primitive_argument_type(
            ir::node_data<std::uint8_t>{std::move(lhs)});
    }

    template <typename Op>
    template <typename T>
    primitive_argument_type comparison<Op>::comparison3d(
        ir::node_data<T>&& lhs, ir::node_data<T>&& rhs,
        bool propagate_type) const
    {
        std::size_t rhs_dims = rhs.num_dimensions();
        switch(rhs_dims)
        {
        case 0:
            return comparison3d0d(
                std::move(lhs), std::move(rhs), propagate_type);

        case 1:
            return comparison3d1d(
                std::move(lhs), std::move(rhs), propagate_type);

        case 2:
            return comparison3d2d(
                std::move(lhs), std::move(rhs), propagate_type);

        case 3:
            return comparison3d3d(
                std::move(lhs), std::move(rhs), propagate_type);

        default:
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "comparison<Op>::comparison2d",
                util::generate_error_message(
                    "the operands have incompatible number of "
                        "dimensions",
                    name_, codename_));
        }
    }
#endif

    template <typename Op>
    template <typename T>
    primitive_argument_type comparison<Op>::comparison_all(
        ir::node_data<T>&& lhs, ir::node_data<T>&& rhs,
        bool propagate_type) const
    {
        std::size_t lhs_dims = lhs.num_dimensions();
        switch (lhs_dims)
        {
        case 0:
            return comparison0d(std::move(lhs), std::move(rhs), propagate_type);

        case 1:
            return comparison1d(std::move(lhs), std::move(rhs), propagate_type);

        case 2:
            return comparison2d(std::move(lhs), std::move(rhs), propagate_type);

#if defined(PHYLANX_HAVE_BLAZE_TENSOR)
        case 3:
            return comparison3d(std::move(lhs), std::move(rhs), propagate_type);
#endif
        default:
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "comparison<Op>::comparison_all",
                util::generate_error_message(
                    "left hand side operand has unsupported number of "
                        "dimensions",
                    name_, codename_));
        }
    }

    template <typename Op>
    struct comparison<Op>::visit_comparison
    {
        template <typename T1, typename T2>
        primitive_argument_type operator()(T1, T2) const
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "comparison<Op>::eval",
                util::generate_error_message(
                    "left hand side and right hand side are incompatible "
                        "and can't be compared",
                    comparison_.name_, comparison_.codename_));
        }

        primitive_argument_type operator()(std::vector<ast::expression>&&,
            std::vector<ast::expression>&&) const
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "comparison<Op>::eval",
                util::generate_error_message(
                    "left hand side and right hand side are incompatible "
                        "and can't be compared",
                    comparison_.name_, comparison_.codename_));
        }

        primitive_argument_type operator()(
            ast::expression&&, ast::expression&&) const
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "comparison<Op>::eval",
                util::generate_error_message(
                    "left hand side and right hand side are incompatible "
                        "and can't be compared",
                    comparison_.name_, comparison_.codename_));
        }

        primitive_argument_type operator()(primitive&&, primitive&&) const
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "comparison<Op>::eval",
                util::generate_error_message(
                    "left hand side and right hand side are incompatible "
                        "and can't be compared",
                    comparison_.name_, comparison_.codename_));
        }

        template <typename T>
        primitive_argument_type operator()(T && lhs, T && rhs) const
        {
            return primitive_argument_type(
                ir::node_data<std::uint8_t>{Op{}(lhs, rhs)});
        }

        primitive_argument_type operator()(ir::range&&, ir::range&&) const
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "comparison<Op>::eval",
                util::generate_error_message(
                    "left hand side and right hand side are incompatible "
                        "and can't be compared",
                    comparison_.name_, comparison_.codename_));
        }

        primitive_argument_type operator()(
            ir::dictionary&&, ir::dictionary&&) const
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "comparison<Op>::eval",
                util::generate_error_message(
                    "left hand side and right hand side are incompatible "
                    "and can't be compared",
                    comparison_.name_, comparison_.codename_));
        }

        primitive_argument_type operator()(ir::node_data<double>&& lhs,
            ir::node_data<std::int64_t>&& rhs) const
        {
            if (lhs.num_dimensions() != 0 || rhs.num_dimensions() != 0)
            {
                return comparison_.comparison_all(std::move(lhs),
                    ir::node_data<double>(std::move(rhs)), propagate_type_);
            }

            if (propagate_type_)
            {
                return primitive_argument_type(
                    ir::node_data<double>{Op{}(lhs[0], rhs[0]) ? 1.0 : 0.0});
            }

            return primitive_argument_type(
                ir::node_data<std::uint8_t>{Op{}(lhs[0], rhs[0])});
        }

        primitive_argument_type operator()(ir::node_data<std::int64_t>&& lhs,
            ir::node_data<double>&& rhs) const
        {
            if (lhs.num_dimensions() != 0 || rhs.num_dimensions() != 0)
            {
                return comparison_.comparison_all(
                    ir::node_data<double>(std::move(lhs)), std::move(rhs),
                    propagate_type_);
            }

            if (propagate_type_)
            {
                return primitive_argument_type(
                    ir::node_data<double>{Op{}(lhs[0], rhs[0]) ? 1.0 : 0.0});
            }

            return primitive_argument_type(
                ir::node_data<std::uint8_t>{Op{}(lhs[0], rhs[0])});
        }

        primitive_argument_type operator()(ir::node_data<std::uint8_t>&& lhs,
            ir::node_data<std::int64_t>&& rhs) const
        {
            if (lhs.num_dimensions() != 0 || rhs.num_dimensions() != 0)
            {
                return comparison_.comparison_all(std::move(lhs),
                    ir::node_data<std::uint8_t>{
                        std::move(rhs) != ir::node_data<std::int64_t>(0)},
                    propagate_type_);
            }

            if (propagate_type_)
            {
                return primitive_argument_type(ir::node_data<std::int64_t>{
                    Op{}(lhs[0], rhs[0]) ? 1 : 0});
            }

            return primitive_argument_type(
                ir::node_data<std::uint8_t>{Op{}(lhs[0], rhs[0])});
        }

        primitive_argument_type operator()(ir::node_data<std::int64_t>&& lhs,
            ir::node_data<std::uint8_t>&& rhs) const
        {
            if (lhs.num_dimensions() != 0 || rhs.num_dimensions() != 0)
            {
                return comparison_.comparison_all(
                    ir::node_data<std::uint8_t>{
                        std::move(lhs) != ir::node_data<std::int64_t>(0)},
                    std::move(rhs), propagate_type_);
            }

            if (propagate_type_)
            {
                return primitive_argument_type(ir::node_data<std::int64_t>{
                    Op{}(lhs[0], rhs[0]) ? 1 : 0});
            }

            return primitive_argument_type(
                ir::node_data<std::uint8_t>{Op{}(lhs[0], rhs[0])});
        }

        primitive_argument_type operator()(ir::node_data<std::uint8_t>&& lhs,
            ir::node_data<double>&& rhs) const
        {
            if (lhs.num_dimensions() != 0 || rhs.num_dimensions() != 0)
            {
                return comparison_.comparison_all(std::move(lhs),
                    ir::node_data<std::uint8_t>{
                        std::move(rhs) != ir::node_data<double>(0)},
                    propagate_type_);
            }

            if (propagate_type_)
            {
                return primitive_argument_type(ir::node_data<double>{
                    Op{}(lhs[0], rhs[0]) ? 1.0 : 0.0});
            }

            return primitive_argument_type(
                ir::node_data<std::uint8_t>{Op{}(lhs[0], rhs[0])});
        }

        primitive_argument_type operator()(ir::node_data<double>&& lhs,
            ir::node_data<std::uint8_t>&& rhs) const
        {
            if (lhs.num_dimensions() != 0 || rhs.num_dimensions() != 0)
            {
                return comparison_.comparison_all(
                    ir::node_data<std::uint8_t>{
                        std::move(lhs) != ir::node_data<double>(0)},
                    std::move(rhs), propagate_type_);
            }

            if (propagate_type_)
            {
                return primitive_argument_type(ir::node_data<double>{
                    Op{}(lhs[0], rhs[0]) ? 1.0 : 0.0});
            }

            return primitive_argument_type(
                ir::node_data<std::uint8_t>{Op{}(lhs[0], rhs[0])});
        }

        primitive_argument_type operator()(
            ir::node_data<double>&& lhs, ir::node_data<double>&& rhs) const
        {
            return comparison_.comparison_all(
                std::move(lhs), std::move(rhs), propagate_type_);
        }

        primitive_argument_type operator()(ir::node_data<std::uint8_t>&& lhs,
            ir::node_data<std::uint8_t>&& rhs) const
        {
            return comparison_.comparison_all(
                std::move(lhs), std::move(rhs), propagate_type_);
        }

        primitive_argument_type operator()(ir::node_data<std::int64_t>&& lhs,
            ir::node_data<std::int64_t>&& rhs) const
        {
            return comparison_.comparison_all(
                std::move(lhs), std::move(rhs), propagate_type_);
        }

        comparison const& comparison_;
        bool propagate_type_;
    };

    template <typename Op>
    hpx::future<primitive_argument_type> comparison<Op>::eval(
        primitive_arguments_type const& operands,
        primitive_arguments_type const& args, eval_context ctx) const
    {
        if (operands.size() < 2 || operands.size() > 3)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "comparison<Op>::eval",
                generate_error_message(
                    "the comparison primitive requires two or three operands"));
        }

        // either operand is allowed to be 'nil'
        if (operands.size() == 3 && !valid(operands[2]))
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "comparison<Op>::eval",
                generate_error_message(
                    "the comparison primitive requires that the "
                        "arguments given by the operands array are valid"));
        }

        auto this_ = this->shared_from_this();
        bool propagate_type = (operands.size() == 3 &&
            phylanx::execution_tree::extract_scalar_boolean_value(operands[2]));

        return hpx::dataflow(hpx::launch::sync, hpx::util::unwrapping(
            [this_ = std::move(this_), propagate_type](
                    primitive_argument_type&& op1,
                    primitive_argument_type&& op2)
            ->  primitive_argument_type
            {
                return primitive_argument_type(
                    util::visit(visit_comparison{*this_, propagate_type},
                        std::move(op1.variant()), std::move(op2.variant())));
            }),
            value_operand(operands[0], args, name_, codename_, ctx),
            value_operand(operands[1], args, name_, codename_, ctx));
    }
}}}

#endif
