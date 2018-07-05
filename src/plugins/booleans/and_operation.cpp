//  Copyright (c) 2017-2018 Hartmut Kaiser
//  Copyright (c) 2018 Shahrzad Shirzad
//  Copyright (c) 2018 Tiany Zhang
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>
#include <phylanx/ir/node_data.hpp>
#include <phylanx/plugins/booleans/and_operation.hpp>
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
    struct and_operation::visit_and
    {
        template <typename T1, typename T2>
        primitive_argument_type operator()(T1, T2) const
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "and::eval",
                execution_tree::generate_error_message(
                    "left hand side and right hand side are incompatible "
                    "and can't be compared",
                    and_.name_, and_.codename_));
        }

        primitive_argument_type operator()(
            ir::node_data<primitive_argument_type>&&,
            ir::node_data<primitive_argument_type>&&) const
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "and::eval",
                execution_tree::generate_error_message(
                    "left hand side and right hand side can't be compared",
                    and_.name_, and_.codename_));
        }

        primitive_argument_type operator()(
            std::vector<ast::expression>&&,
            std::vector<ast::expression>&&) const
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "and::eval",
                execution_tree::generate_error_message(
                    "left hand side and right hand side can't be compared",
                    and_.name_, and_.codename_));
        }

        primitive_argument_type operator()(
            ast::expression&&, ast::expression&&) const
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "and::eval",
                execution_tree::generate_error_message(
                    "left hand side and right hand side can't be compared",
                    and_.name_, and_.codename_));
        }

        primitive_argument_type operator()(
            primitive&&, primitive&&) const
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "and::eval",
                execution_tree::generate_error_message(
                    "left hand side and right hand side can't be compared",
                    and_.name_, and_.codename_));
        }

        template <typename T>
        primitive_argument_type operator()(
            T&& lhs, T&& rhs) const
        {
            return primitive_argument_type(ir::node_data<std::uint8_t>{lhs && rhs});
        }

        primitive_argument_type operator()(
            ast::nil lhs, ast::nil rhs) const
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "and::eval",
                execution_tree::generate_error_message(
                    "left hand side and right hand side can't be compared",
                    and_.name_, and_.codename_));
        }

        primitive_argument_type operator()(ir::range lhs, ir::range rhs) const
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "and::eval",
                execution_tree::generate_error_message(
                    "left hand side and right hand side can't be compared",
                    and_.name_, and_.codename_));
        }

        primitive_argument_type operator()(
            std::string lhs, std::string rhs) const
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "and::eval",
                execution_tree::generate_error_message(
                    "left hand side and right hand side can't be compared",
                    and_.name_, and_.codename_));
        }

        primitive_argument_type operator()(ir::node_data<double>&& lhs,
            ir::node_data<std::int64_t>&& rhs) const
        {
            if (lhs.num_dimensions() != 0 || rhs.num_dimensions() != 0)
            {
                return and_.and_all(
                    std::move(lhs), ir::node_data<double>(std::move(rhs)));
            }
            return primitive_argument_type(
                ir::node_data<std::uint8_t>{(lhs[0] != 0) && (rhs[0] != 0)});
        }

        primitive_argument_type operator()(ir::node_data<std::int64_t>&& lhs,
            ir::node_data<double>&& rhs) const
        {
            if (lhs.num_dimensions() != 0 || rhs.num_dimensions() != 0)
            {
                return and_.and_all(
                    ir::node_data<double>(std::move(lhs)), std::move(rhs));
            }
            return primitive_argument_type(
                ir::node_data<std::uint8_t>{(rhs[0] != 0) && (lhs[0] != 0)});
        }

        primitive_argument_type operator()(
            ir::node_data<double>&& lhs,
            ir::node_data<double>&& rhs) const
        {
            return and_.and_all(std::move(lhs),
                std::move(rhs));
        }

        primitive_argument_type operator()(
            and_operation::operand_type&& lhs, and_operation::operand_type&& rhs) const
        {
            return and_.and_all(std::move(lhs), std::move(rhs));
        }

        primitive_argument_type operator()(ir::node_data<std::int64_t>&& lhs,
            ir::node_data<std::int64_t>&& rhs) const
        {
            return and_.and_all(std::move(lhs), std::move(rhs));
        }

        and_operation const& and_;
    };

    ///////////////////////////////////////////////////////////////////////////
    match_pattern_type const and_operation::match_data =
    {
        hpx::util::make_tuple("__and",
            std::vector<std::string>{"_1 && __2", "__and(_1, __2)"},
            &create_and_operation, &create_primitive<and_operation>)
    };

    ///////////////////////////////////////////////////////////////////////////
    and_operation::and_operation(std::vector<primitive_argument_type>&& operands,
            std::string const& name, std::string const& codename)
      : primitive_component_base(std::move(operands), name, codename)
    {}

    ///////////////////////////////////////////////////////////////////////////
    template <typename T>
    primitive_argument_type and_operation::and0d1d(
        ir::node_data<T>&& lhs, ir::node_data<T>&& rhs) const
    {
        // TODO: SIMD functionality should be added, blaze implementation
        // is not currently available
        if (rhs.is_ref())
        {
            rhs = blaze::map(
                rhs.vector(), [&](bool x) { return (x && lhs.scalar()); });
        }
        rhs.vector() = blaze::map(
            rhs.vector(), [&](bool x) { return (x && lhs.scalar()); });

        return primitive_argument_type(
            ir::node_data<std::uint8_t>{std::move(rhs)});
    }

    template <typename T>
    primitive_argument_type and_operation::and0d2d(
        ir::node_data<T>&& lhs, ir::node_data<T>&& rhs) const
    {
        // TODO: SIMD functionality should be added, blaze implementation
        // is not currently available
        if (rhs.is_ref())
        {
            rhs = blaze::map(
                rhs.matrix(), [&](bool x) { return (x && lhs.scalar()); });
        }
        rhs.matrix() = blaze::map(
            rhs.matrix(), [&](bool x) { return (x && lhs.scalar()); });

        return primitive_argument_type(
            ir::node_data<std::uint8_t>{std::move(rhs)});
    }

    template <typename T>
    primitive_argument_type and_operation::and0d(
        ir::node_data<T>&& lhs, ir::node_data<T>&& rhs) const
    {
        std::size_t rhs_dims = rhs.num_dimensions();
        switch (rhs_dims)
        {
        case 0:
            return primitive_argument_type(
                ir::node_data<std::uint8_t>{lhs.scalar() && rhs.scalar()});

        case 1:
            return and0d1d(std::move(lhs), std::move(rhs));

        case 2:
            return and0d2d(std::move(lhs), std::move(rhs));

        default:
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "and::and0d",
                execution_tree::generate_error_message(
                    "the operands have incompatible number of dimensions",
                    name_, codename_));
        }
    }

    template <typename T>
    primitive_argument_type and_operation::and1d0d(
        ir::node_data<T>&& lhs, ir::node_data<T>&& rhs) const
    {
        // TODO: SIMD functionality should be added, blaze implementation
        // is not currently available
        if (lhs.is_ref())
        {
            lhs = blaze::map(
                lhs.vector(), [&](bool x) { return (x && rhs.scalar()); });
        }
        lhs.vector() = blaze::map(
            lhs.vector(), [&](bool x) { return (x && rhs.scalar()); });

        return primitive_argument_type(
            ir::node_data<std::uint8_t>{std::move(lhs)});
    }

    template <typename T>
    primitive_argument_type and_operation::and1d1d(
        ir::node_data<T>&& lhs, ir::node_data<T>&& rhs) const
    {
        std::size_t lhs_size = lhs.dimension(0);
        std::size_t rhs_size = rhs.dimension(0);

        if (lhs_size != rhs_size)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "and::and1d1d",
                execution_tree::generate_error_message(
                    "the dimensions of the operands do not match",
                    name_, codename_));
        }

        // TODO: SIMD functionality should be added, blaze implementation
        // is not currently available
        if (lhs.is_ref())
        {
            lhs = blaze::map(lhs.vector(), rhs.vector(),
                [&](bool x, bool y) { return (x && y); });
        }
        lhs.vector() = blaze::map(lhs.vector(), rhs.vector(),
            [&](bool x, bool y) { return (x && y); });

        return primitive_argument_type(
            ir::node_data<std::uint8_t>{std::move(lhs)});
    }

    template <typename T>
    primitive_argument_type and_operation::and1d2d(
        ir::node_data<T>&& lhs, ir::node_data<T>&& rhs) const
    {
        auto cv = lhs.vector();
        auto cm = rhs.matrix();

        if (cv.size() != cm.columns())
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "and::and1d2d",
                execution_tree::generate_error_message(
                    "the dimensions of the operands do not match",
                    name_, codename_));
        }

        // TODO: SIMD functionality should be added, blaze implementation
        // is not currently available
        if (rhs.is_ref())
        {
            blaze::DynamicMatrix<bool> m{ cm.rows(), cm.columns() };
            for (size_t i = 0UL; i < cm.rows(); i++)
                blaze::row(m, i) =
                        blaze::map(blaze::row(cm, i),
                                   blaze::trans(cv),
                                   [](bool x, bool y) { return x && y; });
            return primitive_argument_type(
                    ir::node_data<std::uint8_t>{std::move(m)});
        }
        for (size_t i = 0UL; i < rhs.matrix().rows(); i++)
            blaze::row(cm, i) =
                blaze::map(blaze::row(cm, i),
                    blaze::trans(cv),
                    [](bool x, bool y) { return x && y; });

        return primitive_argument_type(
            ir::node_data<std::uint8_t>{std::move(rhs)});
    }

    template <typename T>
    primitive_argument_type and_operation::and1d(
        ir::node_data<T>&& lhs, ir::node_data<T>&& rhs) const
    {
        std::size_t rhs_dims = rhs.num_dimensions();
        switch (rhs_dims)
        {
        case 0:
            return and1d0d(std::move(lhs), std::move(rhs));

        case 1:
            return and1d1d(std::move(lhs), std::move(rhs));

        case 2:
            return and1d2d(std::move(lhs), std::move(rhs));

        default:
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "and::and1d",
                execution_tree::generate_error_message(
                    "the operands have incompatible number of "
                        "dimensions",
                    name_, codename_));
        }
    }

    template <typename T>
    primitive_argument_type and_operation::and2d0d(
        ir::node_data<T>&& lhs, ir::node_data<T>&& rhs) const
    {
        std::size_t lhs_size = lhs.dimension(0);
        std::size_t rhs_size = rhs.dimension(0);

        // TODO: SIMD functionality should be added, blaze implementation
        // is not currently available
        if (lhs.is_ref())
        {
            lhs = blaze::map(
                lhs.matrix(), [&](bool x) { return (x && rhs.scalar()); });
        }
        lhs.matrix() = blaze::map(lhs.matrix(),
            [&](bool x) { return (x && rhs.scalar()); });

        return primitive_argument_type(
            ir::node_data<std::uint8_t>{std::move(lhs)});
    }

    template <typename T>
    primitive_argument_type and_operation::and2d1d(
        ir::node_data<T>&& lhs, ir::node_data<T>&& rhs) const
    {
        auto cv = rhs.vector();
        auto cm = lhs.matrix();

        if (cv.size() != cm.columns())
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "and::and2d1d",
                execution_tree::generate_error_message(
                    "the dimensions of the operands do not match",
                    name_, codename_));
        }

        // TODO: SIMD functionality should be added, blaze implementation
        // is not currently available
        if (lhs.is_ref())
        {
            blaze::DynamicMatrix<bool> m{cm.rows(), cm.columns()};
            for (size_t i = 0UL; i < cm.rows(); i++)
                blaze::row(m, i) = blaze::map(blaze::row(cm, i),
                    blaze::trans(cv),
                    [](bool x, bool y) { return x && y; });
            return primitive_argument_type(ir::node_data<std::uint8_t>{std::move(m)});
        }
        for (size_t i = 0UL; i < cm.rows(); i++)
            blaze::row(cm, i) = blaze::map(blaze::row(cm, i),
                blaze::trans(cv),
                [](bool x, bool y) { return x && y; });

        return primitive_argument_type(
            ir::node_data<std::uint8_t>{std::move(lhs)});
    }

    template <typename T>
    primitive_argument_type and_operation::and2d2d(
        ir::node_data<T>&& lhs, ir::node_data<T>&& rhs) const
    {
        auto lhs_size = lhs.dimensions();
        auto rhs_size = rhs.dimensions();

        if (lhs_size != rhs_size)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "and::and2d2d",
                execution_tree::generate_error_message(
                    "the dimensions of the operands do not match",
                    name_, codename_));
        }

        // TODO: SIMD functionality should be added, blaze implementation
        // is not currently available
        if (lhs.is_ref())
        {
            lhs = blaze::map(lhs.matrix(), rhs.matrix(),
                [&](bool x, bool y) { return (x && y); });
        }

        lhs.matrix() = blaze::map(lhs.matrix(), rhs.matrix(),
            [&](bool x, bool y) { return (x && y); });

        return primitive_argument_type(
            ir::node_data<std::uint8_t>{std::move(lhs)});
    }

    template <typename T>
    primitive_argument_type and_operation::and2d(
        ir::node_data<T>&& lhs, ir::node_data<T>&& rhs) const
    {
        std::size_t rhs_dims = rhs.num_dimensions();
        switch (rhs_dims)
        {
        case 0:
            return and2d0d(std::move(lhs), std::move(rhs));

        case 1:
            return and2d1d(std::move(lhs), std::move(rhs));

        case 2:
            return and2d2d(std::move(lhs), std::move(rhs));

        default:
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "and::and2d",
                execution_tree::generate_error_message(
                    "the operands have incompatible number of "
                        "dimensions",
                    name_, codename_));
        }
    }

    template <typename T>
    primitive_argument_type and_operation::and_all(
        ir::node_data<T>&& lhs, ir::node_data<T>&& rhs) const
    {
        std::size_t lhs_dims = lhs.num_dimensions();
        switch (lhs_dims)
        {
        case 0:
            return and0d(std::move(lhs), std::move(rhs));

        case 1:
            return and1d(std::move(lhs), std::move(rhs));

        case 2:
            return and2d(std::move(lhs), std::move(rhs));

        default:
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "and::and_all",
                execution_tree::generate_error_message(
                    "left hand side operand has unsupported number of "
                        "dimensions",
                    name_, codename_));
        }
    }

    hpx::future<primitive_argument_type> and_operation::eval(
        std::vector<primitive_argument_type> const& operands,
        std::vector<primitive_argument_type> const& args) const
    {
        // TODO: support for operands.size() > 2
        if (operands.size() != 2)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "and::eval",
                execution_tree::generate_error_message(
                    "the and primitive requires exactly two "
                        "operands",
                    name_, codename_));
        }

        if (!valid(operands[0]) || !valid(operands[1]))
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "and::eval",
                execution_tree::generate_error_message(
                    "the and primitive requires that the "
                        "arguments given by the operands array "
                        "are valid",
                    name_, codename_));
        }

        auto this_ = this->shared_from_this();
        return hpx::dataflow(hpx::launch::sync,
            hpx::util::unwrapping(
                [this_](primitive_argument_type&& op1,
                        primitive_argument_type&& op2)
                ->  primitive_argument_type
                {
                    return primitive_argument_type(
                        util::visit(visit_and{*this_},
                            std::move(op1.variant()),
                            std::move(op2.variant())));
                }),
            literal_operand(operands[0], args, name_, codename_),
            literal_operand(operands[1], args, name_, codename_));
    }

    // implement '&&' for all possible combinations of lhs and rhs
    hpx::future<primitive_argument_type> and_operation::eval(
        std::vector<primitive_argument_type> const& args) const
    {
        if (this->no_operands())
        {
            return eval(args, noargs);
        }
        return eval(this->operands(), args);
    }
}}}
