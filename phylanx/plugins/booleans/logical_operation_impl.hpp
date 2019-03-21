//  Copyright (c) 2017-2018 Hartmut Kaiser
//  Copyright (c) 2018 Shahrzad Shirzad
//  Copyright (c) 2018 Tiany Zhang
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(PHYLANX_PRIMITIVES_LOGICAL_OPERATION_IMPL_SEP_02_2018_0703PM)
#define PHYLANX_PRIMITIVES_LOGICAL_OPERATION_IMPL_SEP_02_2018_0703PM

#include <phylanx/config.hpp>
#include <phylanx/ir/node_data.hpp>
#include <phylanx/plugins/booleans/logical_operation.hpp>
#include <phylanx/ir/ranges.hpp>

#include <hpx/include/lcos.hpp>
#include <hpx/include/naming.hpp>
#include <hpx/include/util.hpp>
#include <hpx/throw_exception.hpp>

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <string>
#include <utility>
#include <vector>

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

        primitive_argument_type operator()(std::vector<ast::expression>&&,
            std::vector<ast::expression>&&) const
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "logical::eval",
                util::generate_error_message(
                    "left hand side logical right hand side can't be compared",
                    logical_.name_, logical_.codename_));
        }

        primitive_argument_type operator()(
            ast::expression&&, ast::expression&&) const
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
    primitive_argument_type logical_operation<Op>::logical0d1d(
        ir::node_data<T>&& lhs, ir::node_data<T>&& rhs) const
    {
        // TODO: SIMD functionality should be added, blaze implementation
        // is not currently available
        if (rhs.is_ref())
        {
            rhs = blaze::map(rhs.vector(),
                [&](bool x) { return Op{}(x, lhs.scalar()); });
        }
        else
        {
            rhs.vector() = blaze::map(rhs.vector(),
                [&](bool x) { return Op{}(x, lhs.scalar()); });
        }

        return primitive_argument_type(
            ir::node_data<std::uint8_t>{std::move(rhs)});
    }

    template <typename Op>
    template <typename T>
    primitive_argument_type logical_operation<Op>::logical0d2d(
        ir::node_data<T>&& lhs, ir::node_data<T>&& rhs) const
    {
        // TODO: SIMD functionality should be added, blaze implementation
        // is not currently available
        if (rhs.is_ref())
        {
            rhs = blaze::map(rhs.matrix(),
                [&](bool x) { return Op{}(x, lhs.scalar()); });
        }
        else
        {
            rhs.matrix() = blaze::map(rhs.matrix(),
                [&](bool x) { return Op{}(x, lhs.scalar()); });
        }

        return primitive_argument_type(
            ir::node_data<std::uint8_t>{std::move(rhs)});
    }

    template <typename Op>
    template <typename T>
    primitive_argument_type logical_operation<Op>::logical0d(
        ir::node_data<T>&& lhs, ir::node_data<T>&& rhs) const
    {
        std::size_t rhs_dims = rhs.num_dimensions();
        switch (rhs_dims)
        {
        case 0:
            return primitive_argument_type(
                ir::node_data<std::uint8_t>{Op{}(lhs.scalar(), rhs.scalar())});

        case 1:
            return logical0d1d(std::move(lhs), std::move(rhs));

        case 2:
            return logical0d2d(std::move(lhs), std::move(rhs));

        default:
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "logical::logical0d",
                util::generate_error_message(
                    "the operands have incompatible number of dimensions",
                    name_, codename_));
        }
    }

    template <typename Op>
    template <typename T>
    primitive_argument_type logical_operation<Op>::logical1d0d(
        ir::node_data<T>&& lhs, ir::node_data<T>&& rhs) const
    {
        // TODO: SIMD functionality should be added, blaze implementation
        // is not currently available
        if (lhs.is_ref())
        {
            lhs = blaze::map(lhs.vector(),
                [&](bool x) { return Op{}(x, rhs.scalar()); });
        }
        else
        {
            lhs.vector() = blaze::map(lhs.vector(),
                [&](bool x) { return Op{}(x, rhs.scalar()); });
        }

        return primitive_argument_type(
            ir::node_data<std::uint8_t>{std::move(lhs)});
    }

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
                [&](bool x, bool y) { return Op{}(x, y); });
        }
        else
        {
            lhs.vector() = blaze::map(lhs.vector(), rhs.vector(),
                [&](bool x, bool y) { return Op{}(x, y); });
        }

        return primitive_argument_type(
            ir::node_data<std::uint8_t>{std::move(lhs)});
    }

    template <typename Op>
    template <typename T>
    primitive_argument_type logical_operation<Op>::logical1d2d(
        ir::node_data<T>&& lhs, ir::node_data<T>&& rhs) const
    {
        auto cv = lhs.vector();
        auto cm = rhs.matrix();

        if (cv.size() != cm.columns())
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "logical::logical1d2d",
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
                    blaze::map(blaze::row(cm, i), blaze::trans(cv),
                        [](bool x, bool y) { return Op{}(x, y); });
            }
            return primitive_argument_type(
                ir::node_data<std::uint8_t>{std::move(m)});
        }

        for (std::size_t i = 0UL; i != rhs.matrix().rows(); ++i)
        {
            blaze::row(cm, i) =
                blaze::map(blaze::row(cm, i), blaze::trans(cv),
                    [](bool x, bool y) { return Op{}(x, y); });
        }
        return primitive_argument_type(
            ir::node_data<std::uint8_t>{std::move(rhs)});
    }

    template <typename Op>
    template <typename T>
    primitive_argument_type logical_operation<Op>::logical1d(
        ir::node_data<T>&& lhs, ir::node_data<T>&& rhs) const
    {
        std::size_t rhs_dims = rhs.num_dimensions();
        switch (rhs_dims)
        {
        case 0:
            return logical1d0d(std::move(lhs), std::move(rhs));

        case 1:
            return logical1d1d(std::move(lhs), std::move(rhs));

        case 2:
            return logical1d2d(std::move(lhs), std::move(rhs));

        default:
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "logical::logical1d",
                util::generate_error_message(
                    "the operands have incompatible number of dimensions",
                    name_, codename_));
        }
    }

    template <typename Op>
    template <typename T>
    primitive_argument_type logical_operation<Op>::logical2d0d(
        ir::node_data<T>&& lhs, ir::node_data<T>&& rhs) const
    {
        std::size_t lhs_size = lhs.dimension(0);
        std::size_t rhs_size = rhs.dimension(0);

        // TODO: SIMD functionality should be added, blaze implementation
        // is not currently available
        if (lhs.is_ref())
        {
            lhs = blaze::map(lhs.matrix(),
                [&](bool x) { return Op{}(x, rhs.scalar()); });
        }
        else
        {
            lhs.matrix() = blaze::map(lhs.matrix(),
                [&](bool x) { return Op{}(x, rhs.scalar()); });
        }

        return primitive_argument_type(
            ir::node_data<std::uint8_t>{std::move(lhs)});
    }

    template <typename Op>
    template <typename T>
    primitive_argument_type logical_operation<Op>::logical2d1d(
        ir::node_data<T>&& lhs, ir::node_data<T>&& rhs) const
    {
        auto cv = rhs.vector();
        auto cm = lhs.matrix();

        if (cv.size() != cm.columns())
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "logical::logical2d1d",
                generate_error_message(
                    "the dimensions of the operands do not match"));
        }

        // TODO: SIMD functionality should be added, blaze implementation
        // is not currently available
        if (lhs.is_ref())
        {
            blaze::DynamicMatrix<std::uint8_t> m{cm.rows(), cm.columns()};
            for (std::size_t i = 0UL; i != cm.rows(); ++i)
            {
                blaze::row(m, i) =
                    blaze::map(blaze::row(cm, i), blaze::trans(cv),
                        [](bool x, bool y) { return Op{}(x, y); });
            }
            return primitive_argument_type(
                ir::node_data<std::uint8_t>{std::move(m)});
        }

        for (std::size_t i = 0UL; i != cm.rows(); ++i)
        {
            blaze::row(cm, i) =
                blaze::map(blaze::row(cm, i), blaze::trans(cv),
                    [](bool x, bool y) { return Op{}(x, y); });
        }
        return primitive_argument_type(
            ir::node_data<std::uint8_t>{std::move(lhs)});
    }

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
                [&](bool x, bool y) { return Op{}(x, y); });
        }
        else
        {
            lhs.matrix() = blaze::map(lhs.matrix(), rhs.matrix(),
                [&](bool x, bool y) { return Op{}(x, y); });
        }

        return primitive_argument_type(
            ir::node_data<std::uint8_t>{std::move(lhs)});
    }

    template <typename Op>
    template <typename T>
    primitive_argument_type logical_operation<Op>::logical2d(
        ir::node_data<T>&& lhs, ir::node_data<T>&& rhs) const
    {
        std::size_t rhs_dims = rhs.num_dimensions();
        switch (rhs_dims)
        {
        case 0:
            return logical2d0d(std::move(lhs), std::move(rhs));

        case 1:
            return logical2d1d(std::move(lhs), std::move(rhs));

        case 2:
            return logical2d2d(std::move(lhs), std::move(rhs));

        default:
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "logical::logical2d",
                generate_error_message(
                    "the operands have incompatible number of dimensions"));
        }
    }

    template <typename Op>
    template <typename T>
    primitive_argument_type logical_operation<Op>::logical_all(
        ir::node_data<T>&& lhs, ir::node_data<T>&& rhs) const
    {
        std::size_t lhs_dims = lhs.num_dimensions();
        switch (lhs_dims)
        {
        case 0:
            return logical0d(std::move(lhs), std::move(rhs));

        case 1:
            return logical1d(std::move(lhs), std::move(rhs));

        case 2:
            return logical2d(std::move(lhs), std::move(rhs));

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
