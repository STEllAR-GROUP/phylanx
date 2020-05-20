//  Copyright (c) 2017-2019 Hartmut Kaiser
//  Copyright (c) 2018 Shahrzad Shirzad
//  Copyright (c) 2018 Tiany Zhang
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(PHYLANX_PRIMITIVES_LOGICAL_OPERATION_IMPL_SEP_02_2018_0703PM)
#define PHYLANX_PRIMITIVES_LOGICAL_OPERATION_IMPL_SEP_02_2018_0703PM

#include <phylanx/config.hpp>
#include <phylanx/execution_tree/primitives/node_data_helpers.hpp>
#include <phylanx/ir/node_data.hpp>
#include <phylanx/ir/ranges.hpp>
#include <phylanx/plugins/booleans/logical_operation.hpp>

#include <hpx/include/lcos.hpp>
#include <hpx/include/naming.hpp>
#include <hpx/include/util.hpp>
#include <hpx/errors/throw_exception.hpp>

#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include <blaze/Math.h>
#include <blaze_tensor/Math.h>

///////////////////////////////////////////////////////////////////////////////
namespace phylanx { namespace execution_tree { namespace primitives
{
    template <typename Op>
    struct logical_operation<Op>::visit_logical
    {
        template <typename T1, typename T2>
        primitive_argument_type operator()(T1, T2) const
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "logical::eval",
                util::generate_error_message(
                    "left hand side logical right hand side are incompatible "
                    "logical can't be compared",
                    logical_.name_, logical_.codename_));
        }

        primitive_argument_type operator()(
            util::recursive_wrapper<
                hpx::shared_future<primitive_argument_type>>&&,
            util::recursive_wrapper<
                hpx::shared_future<primitive_argument_type>>&&) const
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "logical::eval",
                util::generate_error_message(
                    "left hand side logical right hand side can't be compared",
                    logical_.name_, logical_.codename_));
        }

        primitive_argument_type operator()(primitive&&, primitive&&) const
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "logical::eval",
                util::generate_error_message(
                    "left hand side logical right hand side can't be compared",
                    logical_.name_, logical_.codename_));
        }

        template <typename T>
        primitive_argument_type operator()(T&& lhs, T&& rhs) const
        {
            return primitive_argument_type(
                ir::node_data<std::uint8_t>{Op{}(lhs, rhs)});
        }

        primitive_argument_type operator()(
            ir::dictionary&& lhs, ir::dictionary&& rhs) const
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "logical::eval",
                util::generate_error_message(
                    "left hand side logical right hand side can't be compared",
                    logical_.name_, logical_.codename_));
        }

        primitive_argument_type operator()(ast::nil lhs, ast::nil rhs) const
        {
            return primitive_argument_type(
                ir::node_data<std::uint8_t>{Op{}(bool(lhs), bool(rhs))});
        }

        primitive_argument_type operator()(ir::range lhs, ir::range rhs) const
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "logical::eval",
                util::generate_error_message(
                    "left hand side logical right hand side can't be compared",
                    logical_.name_, logical_.codename_));
        }

        primitive_argument_type operator()(
            std::string lhs, std::string rhs) const
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "logical::eval",
                util::generate_error_message(
                    "left hand side logical right hand side can't be compared",
                    logical_.name_, logical_.codename_));
        }

        primitive_argument_type operator()(ir::node_data<double>&& lhs,
            ir::node_data<std::int64_t>&& rhs) const
        {
            if (lhs.num_dimensions() != 0 || rhs.num_dimensions() != 0)
            {
                return logical_.logical_all(
                    std::move(lhs), ir::node_data<double>(std::move(rhs)));
            }

            return primitive_argument_type(ir::node_data<std::uint8_t>{
                Op{}(rhs[0] != 0, lhs[0] != 0)});
        }

        primitive_argument_type operator()(ir::node_data<std::int64_t>&& lhs,
            ir::node_data<double>&& rhs) const
        {
            if (lhs.num_dimensions() != 0 || rhs.num_dimensions() != 0)
            {
                return logical_.logical_all(
                    ir::node_data<double>(std::move(lhs)), std::move(rhs));
            }
            return primitive_argument_type(ir::node_data<std::uint8_t>{
                Op{}(rhs[0] != 0, lhs[0] != 0)});
        }

        primitive_argument_type operator()(ir::node_data<double>&& lhs,
            ir::node_data<std::uint8_t>&& rhs) const
        {
            if (lhs.num_dimensions() != 0 || rhs.num_dimensions() != 0)
            {
                return logical_.logical_all(
                    std::move(lhs), ir::node_data<double>(std::move(rhs)));
            }

            return primitive_argument_type(ir::node_data<std::uint8_t>{
                Op{}(rhs[0] != 0, lhs[0] != 0)});
        }

        primitive_argument_type operator()(ir::node_data<std::uint8_t>&& lhs,
            ir::node_data<double>&& rhs) const
        {
            if (lhs.num_dimensions() != 0 || rhs.num_dimensions() != 0)
            {
                return logical_.logical_all(
                    ir::node_data<double>(std::move(lhs)), std::move(rhs));
            }
            return primitive_argument_type(ir::node_data<std::uint8_t>{
                Op{}(rhs[0] != 0, lhs[0] != 0)});
        }

        primitive_argument_type operator()(ir::node_data<std::int64_t>&& lhs,
            ir::node_data<std::uint8_t>&& rhs) const
        {
            if (lhs.num_dimensions() != 0 || rhs.num_dimensions() != 0)
            {
                return logical_.logical_all(std::move(lhs),
                    ir::node_data<std::int64_t>(std::move(rhs)));
            }

            return primitive_argument_type(ir::node_data<std::uint8_t>{
                Op{}(rhs[0] != 0, lhs[0] != 0)});
        }

        primitive_argument_type operator()(ir::node_data<std::uint8_t>&& lhs,
            ir::node_data<std::int64_t>&& rhs) const
        {
            if (lhs.num_dimensions() != 0 || rhs.num_dimensions() != 0)
            {
                return logical_.logical_all(
                    ir::node_data<std::int64_t>(std::move(lhs)),
                    std::move(rhs));
            }
            return primitive_argument_type(ir::node_data<std::uint8_t>{
                Op{}(rhs[0] != 0, lhs[0] != 0)});
        }

        template <typename T>
        primitive_argument_type operator()(ir::node_data<T>&& lhs,
            ir::node_data<T>&& rhs) const
        {
            return logical_.logical_all(std::move(lhs), std::move(rhs));
        }

        logical_operation const& logical_;
    };

    ///////////////////////////////////////////////////////////////////////////
    template <typename Op>
    logical_operation<Op>::logical_operation(primitive_arguments_type&& operands,
            std::string const& name, std::string const& codename)
      : primitive_component_base(std::move(operands), name, codename)
    {}

    ///////////////////////////////////////////////////////////////////////////
    template <typename Op>
    template <typename T>
    primitive_argument_type logical_operation<Op>::logical0d(
        ir::node_data<T>&& lhs, ir::node_data<T>&& rhs) const
    {
        return primitive_argument_type(
            ir::node_data<std::uint8_t>{Op{}(lhs.scalar(), rhs.scalar())});
    }

    ///////////////////////////////////////////////////////////////////////////
    template <typename Op>
    template <typename T>
    primitive_argument_type logical_operation<Op>::logical1d1d(
        ir::node_data<T>&& lhs, ir::node_data<T>&& rhs) const
    {
        std::size_t lhs_size = lhs.dimension(0);
        std::size_t rhs_size = rhs.dimension(0);

        if (lhs_size != rhs_size)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "logical::logical1d1d",
                util::generate_error_message(
                    "the dimensions of the operands do not match",
                    name_, codename_));
        }

        // TODO: SIMD functionality should be added, blaze implementation
        // is not currently available
        if (lhs.is_ref())
        {
            lhs = blaze::map(lhs.vector(), rhs.vector(),
                [&](bool x, bool y) -> std::uint8_t { return Op{}(x, y); });
        }
        else
        {
            lhs.vector() = blaze::map(lhs.vector(), rhs.vector(),
                [&](bool x, bool y) -> std::uint8_t { return Op{}(x, y); });
        }

        return primitive_argument_type(
            ir::node_data<std::uint8_t>{std::move(lhs)});
    }

    template <typename Op>
    template <typename T>
    primitive_argument_type logical_operation<Op>::logical1d(
        ir::node_data<T>&& lhs, ir::node_data<T>&& rhs,
        std::array<std::size_t, PHYLANX_MAX_DIMENSIONS> const& sizes) const
    {
        if (lhs.dimensions() != rhs.dimensions())
        {
            blaze::DynamicVector<T> lhs_data, rhs_data;

            extract_value_vector(
                lhs_data, std::move(lhs), sizes[0], name_, codename_);
            extract_value_vector(
                rhs_data, std::move(rhs), sizes[0], name_, codename_);

            return ir::node_data<std::uint8_t>{blaze::map(lhs_data, rhs_data,
                [&](bool x, bool y) -> std::uint8_t { return Op{}(x, y); })};
        }

        return logical1d1d(std::move(lhs), std::move(rhs));
    }

    ///////////////////////////////////////////////////////////////////////////
    template <typename Op>
    template <typename T>
    primitive_argument_type logical_operation<Op>::logical2d2d(
        ir::node_data<T>&& lhs, ir::node_data<T>&& rhs) const
    {
        auto lhs_size = lhs.dimensions();
        auto rhs_size = rhs.dimensions();

        if (lhs_size != rhs_size)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "logical::logical2d2d",
                generate_error_message(
                    "the dimensions of the operands do not match"));
        }

        // TODO: SIMD functionality should be added, blaze implementation
        // is not currently available
        if (lhs.is_ref())
        {
            lhs = blaze::map(lhs.matrix(), rhs.matrix(),
                [&](bool x, bool y) -> std::uint8_t { return Op{}(x, y); });
        }
        else
        {
            lhs.matrix() = blaze::map(lhs.matrix(), rhs.matrix(),
                [&](bool x, bool y) -> std::uint8_t { return Op{}(x, y); });
        }

        return primitive_argument_type(
            ir::node_data<std::uint8_t>{std::move(lhs)});
    }

    template <typename Op>
    template <typename T>
    primitive_argument_type logical_operation<Op>::logical2d(
        ir::node_data<T>&& lhs, ir::node_data<T>&& rhs,
        std::array<std::size_t, PHYLANX_MAX_DIMENSIONS> const& sizes) const
    {
        if (lhs.dimensions() != rhs.dimensions())
        {
            blaze::DynamicMatrix<T> lhs_data, rhs_data;

            extract_value_matrix(
                lhs_data, std::move(lhs), sizes[0], sizes[1], name_, codename_);
            extract_value_matrix(
                rhs_data, std::move(rhs), sizes[0], sizes[1], name_, codename_);

            return ir::node_data<std::uint8_t>{blaze::map(lhs_data, rhs_data,
                [&](bool x, bool y) -> std::uint8_t { return Op{}(x, y); })};
        }

        return logical2d2d(std::move(lhs), std::move(rhs));
    }

    template <typename Op>
    template <typename T>
    primitive_argument_type logical_operation<Op>::logical3d3d(
        ir::node_data<T>&& lhs, ir::node_data<T>&& rhs) const
    {
        auto lhs_size = lhs.dimensions();
        auto rhs_size = rhs.dimensions();

        if (lhs_size != rhs_size)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "logical_operation<Op>::logical3d3d",
                util::generate_error_message(
                    "the dimensions of the operands do not match",
                    name_, codename_));
        }

        // TODO: SIMD functionality should be added, blaze implementation
        //       is not currently available
        if (lhs.is_ref())
        {
            lhs = blaze::map(lhs.tensor(), rhs.tensor(),
                [&](bool x, bool y) -> std::uint8_t { return Op{}(x, y); });
        }
        else
        {
            lhs.tensor() = blaze::map(lhs.tensor(), rhs.tensor(),
                [&](bool x, bool y) -> std::uint8_t { return Op{}(x, y); });
        }

        return primitive_argument_type(
            ir::node_data<std::uint8_t>{std::move(lhs)});
    }

    template <typename Op>
    template <typename T>
    primitive_argument_type logical_operation<Op>::logical3d(
        ir::node_data<T>&& lhs, ir::node_data<T>&& rhs,
        std::array<std::size_t, PHYLANX_MAX_DIMENSIONS> const& sizes) const
    {
        if (lhs.dimensions() != rhs.dimensions())
        {
            blaze::DynamicTensor<T> lhs_data, rhs_data;

            extract_value_tensor(lhs_data, std::move(lhs), sizes[0], sizes[1],
                sizes[2], name_, codename_);
            extract_value_tensor(rhs_data, std::move(rhs), sizes[0], sizes[1],
                sizes[2], name_, codename_);

            return ir::node_data<std::uint8_t>{blaze::map(lhs_data, rhs_data,
                [&](bool x, bool y) -> std::uint8_t { return Op{}(x, y); })};
        }

        return logical3d3d(std::move(lhs), std::move(rhs));
    }

    template <typename Op>
    template <typename T>
    primitive_argument_type logical_operation<Op>::logical4d4d(
        ir::node_data<T>&& lhs, ir::node_data<T>&& rhs) const
    {
        auto lhs_size = lhs.dimensions();
        auto rhs_size = rhs.dimensions();

        if (lhs_size != rhs_size)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "logical_operation<Op>::logical4d4d",
                util::generate_error_message(
                    "the dimensions of the operands do not match",
                    name_, codename_));
        }

        // TODO: SIMD functionality should be added, blaze implementation
        //       is not currently available
        if (lhs.is_ref())
        {
            lhs = blaze::map(lhs.quatern(), rhs.quatern(),
                [&](bool x, bool y) -> std::uint8_t { return Op{}(x, y); });
        }
        else
        {
            lhs.quatern() = blaze::map(lhs.quatern(), rhs.quatern(),
                [&](bool x, bool y) -> std::uint8_t { return Op{}(x, y); });
        }

        return primitive_argument_type(
            ir::node_data<std::uint8_t>{std::move(lhs)});
    }

    template <typename Op>
    template <typename T>
    primitive_argument_type logical_operation<Op>::logical4d(
        ir::node_data<T>&& lhs, ir::node_data<T>&& rhs,
        std::array<std::size_t, PHYLANX_MAX_DIMENSIONS> const& sizes) const
    {
        if (lhs.dimensions() != rhs.dimensions())
        {
            blaze::DynamicArray<4UL, T> lhs_data, rhs_data;

            extract_value_quatern(lhs_data, std::move(lhs), sizes[0], sizes[1],
                sizes[2], sizes[3], name_, codename_);
            extract_value_quatern(rhs_data, std::move(rhs), sizes[0], sizes[1],
                sizes[2], sizes[3], name_, codename_);

            return ir::node_data<std::uint8_t>{blaze::map(lhs_data, rhs_data,
                [&](bool x, bool y) -> std::uint8_t { return Op{}(x, y); })};
        }

        return logical4d4d(std::move(lhs), std::move(rhs));
    }

    ///////////////////////////////////////////////////////////////////////////
    template <typename Op>
    template <typename T>
    primitive_argument_type logical_operation<Op>::logical_all(
        ir::node_data<T>&& lhs, ir::node_data<T>&& rhs) const
    {
        auto sizes = extract_largest_dimensions(name_, codename_, lhs, rhs);
        switch (extract_largest_dimension(name_, codename_, lhs, rhs))
        {
        case 0:
            return logical0d(std::move(lhs), std::move(rhs));

        case 1:
            return logical1d(std::move(lhs), std::move(rhs), sizes);

        case 2:
            return logical2d(std::move(lhs), std::move(rhs), sizes);

        case 3:
            return logical3d(std::move(lhs), std::move(rhs), sizes);

        case 4:
            return logical4d(std::move(lhs), std::move(rhs), sizes);

        default:
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "logical::logical_all",
                generate_error_message(
                    "left hand side operand of logical has unsupported number "
                        "of dimensions"));
        }
    }

    template <typename Op>
    hpx::future<primitive_argument_type> logical_operation<Op>::eval(
        primitive_arguments_type const& operands,
        primitive_arguments_type const& args, eval_context ctx) const
    {
        // TODO: support for operands.size() > 2
        if (operands.size() != 2)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "logical::eval",
                util::generate_error_message(
                    "the logical primitive requires exactly two operands",
                    name_, codename_));
        }

        if (!valid(operands[0]) || !valid(operands[1]))
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "logical::eval",
                util::generate_error_message(
                    "the logical primitive requires that the "
                        "arguments given by the operands array are valid",
                    name_, codename_));
        }

        auto this_ = this->shared_from_this();
        return hpx::dataflow(hpx::launch::sync, hpx::util::unwrapping(
            [this_ = std::move(this_)](primitive_argument_type&& op1,
                    primitive_argument_type&& op2)
            ->  primitive_argument_type
            {
                return primitive_argument_type(
                    util::visit(visit_logical{*this_},
                        std::move(op1.variant()), std::move(op2.variant())));
            }),
            value_operand(operands[0], args, name_, codename_, ctx),
            value_operand(operands[1], args, name_, codename_, ctx));
    }
}}}

#endif
