//  Copyright (c) 2017-2019 Hartmut Kaiser
//  Copyright (c) 2018 Shahrzad Shirzad
//  Copyright (c) 2018 Tianyi Zhang
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(PHYLANX_PRIMITIVES_COMPARISON_IMPL_SEP_02_2018_0443PM)
#define PHYLANX_PRIMITIVES_COMPARISON_IMPL_SEP_02_2018_0443PM

#include <phylanx/config.hpp>
#include <phylanx/execution_tree/primitives/node_data_helpers.hpp>
#include <phylanx/ir/node_data.hpp>
#include <phylanx/ir/ranges.hpp>
#include <phylanx/plugins/booleans/comparison.hpp>

#include <hpx/include/lcos.hpp>
#include <hpx/include/naming.hpp>
#include <hpx/include/util.hpp>
#include <hpx/errors/throw_exception.hpp>

#include <array>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <numeric>
#include <string>
#include <utility>
#include <vector>

#include <blaze/Math.h>
#include <blaze_tensor/Math.h>

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
    primitive_argument_type comparison<Op>::comparison0d(
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

    ///////////////////////////////////////////////////////////////////////////
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
    primitive_argument_type comparison<Op>::comparison1d(ir::node_data<T>&& lhs,
        ir::node_data<T>&& rhs, bool propagate_type,
        std::array<std::size_t, PHYLANX_MAX_DIMENSIONS> const& sizes) const
    {
        if (lhs.dimensions() != rhs.dimensions())
        {
            blaze::DynamicVector<T> lhs_data, rhs_data;

            extract_value_vector(
                lhs_data, std::move(lhs), sizes[0], name_, codename_);
            extract_value_vector(
                rhs_data, std::move(rhs), sizes[0], name_, codename_);

            if (propagate_type)
            {
                return primitive_argument_type(ir::node_data<T>{
                    blaze::map(lhs_data, rhs_data, [&](T x, T y) -> T {
                        return Op{}(x, y) ? T(1) : T(0);
                    })});
            }

            return primitive_argument_type(
                ir::node_data<std::uint8_t>{blaze::map(lhs_data, rhs_data,
                    [&](T x, T y) -> std::uint8_t { return Op{}(x, y); })});
        }

        return comparison1d1d(std::move(lhs), std::move(rhs), propagate_type);
    }

    ///////////////////////////////////////////////////////////////////////////
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

    template <typename Op>
    template <typename T>
    primitive_argument_type comparison<Op>::comparison2d(ir::node_data<T>&& lhs,
        ir::node_data<T>&& rhs, bool propagate_type,
        std::array<std::size_t, PHYLANX_MAX_DIMENSIONS> const& sizes) const
    {
        if (lhs.dimensions() != rhs.dimensions())
        {
            blaze::DynamicMatrix<T> lhs_data, rhs_data;

            extract_value_matrix(
                lhs_data, std::move(lhs), sizes[0], sizes[1], name_, codename_);
            extract_value_matrix(
                rhs_data, std::move(rhs), sizes[0], sizes[1], name_, codename_);

            if (propagate_type)
            {
                return primitive_argument_type(ir::node_data<T>{
                    blaze::map(lhs_data, rhs_data, [&](T x, T y) -> T {
                        return Op{}(x, y) ? T(1) : T(0);
                    })});
            }

            return primitive_argument_type(
                ir::node_data<std::uint8_t>{blaze::map(lhs_data, rhs_data,
                    [&](T x, T y) -> std::uint8_t { return Op{}(x, y); })});
        }

        return comparison2d2d(std::move(lhs), std::move(rhs), propagate_type);
    }

    ///////////////////////////////////////////////////////////////////////////
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
    primitive_argument_type comparison<Op>::comparison3d(ir::node_data<T>&& lhs,
        ir::node_data<T>&& rhs, bool propagate_type,
        std::array<std::size_t, PHYLANX_MAX_DIMENSIONS> const& sizes) const
    {
        if (lhs.dimensions() != rhs.dimensions())
        {
            blaze::DynamicTensor<T> lhs_data, rhs_data;

            extract_value_tensor(lhs_data, std::move(lhs), sizes[0], sizes[1],
                sizes[2], name_, codename_);
            extract_value_tensor(rhs_data, std::move(rhs), sizes[0], sizes[1],
                sizes[2], name_, codename_);

            if (propagate_type)
            {
                return primitive_argument_type(ir::node_data<T>{
                    blaze::map(lhs_data, rhs_data, [&](T x, T y) -> T {
                        return Op{}(x, y) ? T(1) : T(0);
                    })});
            }

            return primitive_argument_type(
                ir::node_data<std::uint8_t>{blaze::map(lhs_data, rhs_data,
                    [&](T x, T y) -> std::uint8_t { return Op{}(x, y); })});
        }

        return comparison3d3d(std::move(lhs), std::move(rhs), propagate_type);
    }

    ///////////////////////////////////////////////////////////////////////////
    template <typename Op>
    template <typename T>
    primitive_argument_type comparison<Op>::comparison4d4d(
        ir::node_data<T>&& lhs, ir::node_data<T>&& rhs,
        bool propagate_type) const
    {
        auto lhs_size = lhs.dimensions();
        auto rhs_size = rhs.dimensions();

        if (lhs_size != rhs_size)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "comparison<Op>::comparison4d4d",
                util::generate_error_message(
                    "the dimensions of the operands do not match",
                    name_, codename_));
        }

        // TODO: SIMD functionality should be added, blaze implementation
        //       is not currently available
        if (lhs.is_ref())
        {
            lhs = blaze::map(lhs.quatern(), rhs.quatern(),
                [&](T x, T y) -> std::uint8_t { return Op{}(x, y); });
        }
        else
        {
            lhs.quatern() = blaze::map(lhs.quatern(), rhs.quatern(),
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
    primitive_argument_type comparison<Op>::comparison4d(ir::node_data<T>&& lhs,
        ir::node_data<T>&& rhs, bool propagate_type,
        std::array<std::size_t, PHYLANX_MAX_DIMENSIONS> const& sizes) const
    {
        if (lhs.dimensions() != rhs.dimensions())
        {
            blaze::DynamicArray<4UL, T> lhs_data, rhs_data;

            extract_value_quatern(lhs_data, std::move(lhs), sizes[0], sizes[1],
                sizes[2], sizes[3], name_, codename_);
            extract_value_quatern(rhs_data, std::move(rhs), sizes[0], sizes[1],
                sizes[2], sizes[3], name_, codename_);

            if (propagate_type)
            {
                return primitive_argument_type(ir::node_data<T>{
                    blaze::map(lhs_data, rhs_data, [&](T x, T y) -> T {
                        return Op{}(x, y) ? T(1) : T(0);
                    })});
            }

            return primitive_argument_type(
                ir::node_data<std::uint8_t>{blaze::map(lhs_data, rhs_data,
                    [&](T x, T y) -> std::uint8_t { return Op{}(x, y); })});
        }

        return comparison4d4d(std::move(lhs), std::move(rhs), propagate_type);
    }

    ///////////////////////////////////////////////////////////////////////////
    template <typename Op>
    template <typename T>
    primitive_argument_type comparison<Op>::comparison_all(
        ir::node_data<T>&& lhs, ir::node_data<T>&& rhs,
        bool propagate_type) const
    {
        auto sizes = extract_largest_dimensions(name_, codename_, lhs, rhs);
        switch (extract_largest_dimension(name_, codename_, lhs, rhs))
        {
        case 0:
            return comparison0d(std::move(lhs), std::move(rhs), propagate_type);

        case 1:
            return comparison1d(
                std::move(lhs), std::move(rhs), propagate_type, sizes);

        case 2:
            return comparison2d(
                std::move(lhs), std::move(rhs), propagate_type, sizes);

        case 3:
            return comparison3d(
                std::move(lhs), std::move(rhs), propagate_type, sizes);

        case 4:
            return comparison4d(
                std::move(lhs), std::move(rhs), propagate_type, sizes);

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
        primitive_argument_type operator()(T1&&, T2&&) const
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
        primitive_argument_type operator()(T&& lhs, T&& rhs) const
        {
            return primitive_argument_type(
                ir::node_data<std::uint8_t>{Op{}(lhs, rhs)});
        }

        primitive_argument_type operator()(
            util::recursive_wrapper<
                hpx::shared_future<primitive_argument_type>>&&,
            util::recursive_wrapper<
                hpx::shared_future<primitive_argument_type>>&&) const
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "comparison<Op>::eval",
                util::generate_error_message(
                    "left hand side and right hand side are incompatible "
                        "and can't be compared",
                    comparison_.name_, comparison_.codename_));
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
