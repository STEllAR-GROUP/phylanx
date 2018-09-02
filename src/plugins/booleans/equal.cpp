// Copyright (c) 2017-2018 Hartmut Kaiser
//  Copyright (c) 2018 Tianyi Zhang
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>
#include <phylanx/ir/node_data.hpp>
#include <phylanx/plugins/booleans/equal.hpp>

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

///////////////////////////////////////////////////////////////////////////////
namespace phylanx { namespace execution_tree { namespace primitives
{
    ///////////////////////////////////////////////////////////////////////////
    match_pattern_type const equal::match_data = {
        hpx::util::make_tuple("__eq",
            std::vector<std::string>{
                "_1 == _2", "__eq(_1, _2)", "__eq(_1, _2, _3)"},
            &create_equal, &create_primitive<equal>)
    };

    ///////////////////////////////////////////////////////////////////////////
    equal::equal(primitive_arguments_type&& operands,
            std::string const& name, std::string const& codename)
      : primitive_component_base(std::move(operands), name, codename)
    {}

    ///////////////////////////////////////////////////////////////////////////
    template <typename T>
    primitive_argument_type equal::equal0d0d(ir::node_data<T>&& lhs,
        ir::node_data<T>&& rhs, bool propagate_type) const
    {
        if (propagate_type)
        {
            return primitive_argument_type(ir::node_data<T>{
                (lhs.scalar() == rhs.scalar()) ? T(1) : T(0)});
        }
        return primitive_argument_type(
            ir::node_data<std::uint8_t>{lhs.scalar() == rhs.scalar()});
    }

    template <typename T>
    primitive_argument_type equal::equal0d1d(ir::node_data<T>&& lhs,
        ir::node_data<T>&& rhs, bool propagate_type) const
    {
        // TODO: SIMD functionality should be added, blaze implementation
        // is not currently available
        if (rhs.is_ref())
        {
            rhs = blaze::map(rhs.vector(),
                [&](T x) { return (x == lhs.scalar()); });
        }
        else
        {
            rhs.vector() = blaze::map(rhs.vector(),
                [&](T x) { return (x == lhs.scalar()); });
        }

        if (propagate_type)
        {
            return primitive_argument_type(ir::node_data<T>{std::move(rhs)});
        }

        return primitive_argument_type(
            ir::node_data<std::uint8_t>{std::move(rhs)});
    }

    template <typename T>
    primitive_argument_type equal::equal0d2d(ir::node_data<T>&& lhs,
        ir::node_data<T>&& rhs, bool propagate_type) const
    {
        // TODO: SIMD functionality should be added, blaze implementation
        // is not currently available
        if (rhs.is_ref())
        {
            rhs = blaze::map(rhs.matrix(),
                [&](T x) { return (x == lhs.scalar()); });
        }
        else
        {
            rhs.matrix() = blaze::map(rhs.matrix(),
                [&](T x) { return (x == lhs.scalar()); });
        }

        if (propagate_type)
        {
            return primitive_argument_type(ir::node_data<T>{std::move(rhs)});
        }

        return primitive_argument_type(
            ir::node_data<std::uint8_t>{std::move(rhs)});
    }

    template <typename T>
    primitive_argument_type equal::equal0d(ir::node_data<T>&& lhs,
        ir::node_data<T>&& rhs, bool propagate_type) const
    {
        std::size_t rhs_dims = rhs.num_dimensions();
        switch(rhs_dims)
        {
        case 0:
            return equal0d0d(std::move(lhs), std::move(rhs), propagate_type);

        case 1:
            return equal0d1d(std::move(lhs), std::move(rhs), propagate_type);

        case 2:
            return equal0d2d(std::move(lhs), std::move(rhs), propagate_type);

        default:
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "equal::equal0d",
                util::generate_error_message(
                    "the operands have incompatible number of dimensions",
                    name_, codename_));
        }
    }

    template <typename T>
    primitive_argument_type equal::equal1d0d(ir::node_data<T>&& lhs,
        ir::node_data<T>&& rhs, bool propagate_type) const
    {
        // TODO: SIMD functionality should be added, blaze implementation
        // is not currently available
        if (lhs.is_ref())
        {
            lhs = blaze::map(lhs.vector(),
                [&](T x) { return (x == rhs.scalar()); });
        }
        else
        {
            lhs.vector() = blaze::map(lhs.vector(),
                [&](T x) { return (x == rhs.scalar()); });
        }

        if (propagate_type)
        {
            return primitive_argument_type(ir::node_data<T>{std::move(lhs)});
        }

        return primitive_argument_type(
            ir::node_data<std::uint8_t>{std::move(lhs)});
    }

    template <typename T>
    primitive_argument_type equal::equal1d1d(ir::node_data<T>&& lhs,
        ir::node_data<T>&& rhs, bool propagate_type) const
    {
        std::size_t lhs_size = lhs.dimension(0);
        std::size_t rhs_size = rhs.dimension(0);

        if (lhs_size != rhs_size)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "equal::equal1d1d",
                util::generate_error_message(
                    "the dimensions of the operands do not match",
                    name_, codename_));
        }

        // TODO: SIMD functionality should be added, blaze implementation
        // is not currently available
        if (lhs.is_ref())
        {
            lhs = blaze::map(lhs.vector(), rhs.vector(),
                [&](T x, T y) { return (x == y); });
        }
        else
        {
            lhs.vector() = blaze::map(lhs.vector(), rhs.vector(),
                [&](T x, T y) { return (x == y); });
        }

        if (propagate_type)
        {
            return primitive_argument_type(ir::node_data<T>{std::move(lhs)});
        }
        return primitive_argument_type(
            ir::node_data<std::uint8_t>{std::move(lhs)});
    }

    template <typename T>
    primitive_argument_type equal::equal1d2d(ir::node_data<T>&& lhs,
        ir::node_data<T>&& rhs, bool propagate_type) const
    {
        auto cv = lhs.vector();
        auto cm = rhs.matrix();

        if (cv.size() != cm.columns())
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "equal::equal1d2d",
                util::generate_error_message(
                    "the dimensions of the operands do not match",
                    name_, codename_));
        }

        // TODO: SIMD functionality should be added, blaze implementation
        // is not currently available
        if (rhs.is_ref())
        {
            blaze::DynamicMatrix<double> m{cm.rows(), cm.columns()};

            for (size_t i = 0UL; i != cm.rows(); ++i)
            {
                blaze::row(m, i) = blaze::map(blaze::row(cm, i),
                    blaze::trans(cv),
                    [](T x, T y) { return x == y; });
            }

            if (propagate_type)
            {
                return primitive_argument_type(ir::node_data<T>{std::move(m)});
            }
            return primitive_argument_type(
                ir::node_data<std::uint8_t>{std::move(m)});
        }

        for (size_t i = 0UL; i != cm.rows(); ++i)
        {
            blaze::row(cm, i) = blaze::map(blaze::row(cm, i),
                blaze::trans(cv),
                [](T x, T y) { return x == y; });
        }

        if (propagate_type)
        {
            return primitive_argument_type(ir::node_data<T>{std::move(rhs)});
        }

        return primitive_argument_type(
            ir::node_data<std::uint8_t>{std::move(rhs)});
    }

    template <typename T>
    primitive_argument_type equal::equal1d(ir::node_data<T>&& lhs,
        ir::node_data<T>&& rhs, bool propagate_type) const
    {
        std::size_t rhs_dims = rhs.num_dimensions();
        switch(rhs_dims)
        {
        case 0:
            return equal1d0d(std::move(lhs), std::move(rhs), propagate_type);

        case 1:
            return equal1d1d(std::move(lhs), std::move(rhs), propagate_type);

        case 2:
            return equal1d2d(std::move(lhs), std::move(rhs), propagate_type);

        default:
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "equal::equal1d",
                util::generate_error_message(
                    "the operands have incompatible number of "
                        "dimensions",
                    name_, codename_));
        }
    }

    template <typename T>
    primitive_argument_type equal::equal2d0d(ir::node_data<T>&& lhs,
        ir::node_data<T>&& rhs, bool propagate_type) const
    {
        std::size_t lhs_size = lhs.dimension(0);
        std::size_t rhs_size = rhs.dimension(0);

        // TODO: SIMD functionality should be added, blaze implementation
        // is not currently available
        if (lhs.is_ref())
        {
            lhs = blaze::map(lhs.matrix(),
                [&](T x) { return (x == rhs.scalar()); });
        }
        else
        {
            lhs.matrix() = blaze::map(lhs.matrix(),
                [&](T x) { return (x == rhs.scalar()); });
        }

        if (propagate_type)
        {
            return primitive_argument_type(ir::node_data<T>{std::move(lhs)});
        }
        return primitive_argument_type(
            ir::node_data<std::uint8_t>{std::move(lhs)});
    }

    template <typename T>
    primitive_argument_type equal::equal2d1d(ir::node_data<T>&& lhs,
        ir::node_data<T>&& rhs, bool propagate_type) const
    {
        auto cv = rhs.vector();
        auto cm = lhs.matrix();

        if (cv.size() != cm.columns())
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "equal::equal2d1d",
                util::generate_error_message(
                    "the dimensions of the operands do not match",
                    name_, codename_));
        }

        // TODO: SIMD functionality should be added, blaze implementation
        // is not currently available
        if (lhs.is_ref())
        {
            blaze::DynamicMatrix<double> m{cm.rows(), cm.columns()};

            for (size_t i = 0UL; i != cm.rows(); ++i)
            {
                blaze::row(m, i) = blaze::map(blaze::row(cm, i),
                    blaze::trans(cv),
                    [](T x, T y) { return x == y; });
            }

            if (propagate_type)
            {
                return primitive_argument_type(ir::node_data<T>{std::move(m)});
            }

            return primitive_argument_type(
                ir::node_data<std::uint8_t>{std::move(m)});
        }

        for (size_t i = 0UL; i != cm.rows(); ++i)
        {
            blaze::row(cm, i) = blaze::map(blaze::row(cm, i),
                blaze::trans(cv),
                [](T x, T y) { return x == y; });
        }

        if (propagate_type)
        {
            return primitive_argument_type(ir::node_data<T>{std::move(lhs)});
        }

        return primitive_argument_type(
            ir::node_data<std::uint8_t>{std::move(lhs)});
    }

    template <typename T>
    primitive_argument_type equal::equal2d2d(ir::node_data<T>&& lhs,
        ir::node_data<T>&& rhs, bool propagate_type) const
    {
        auto lhs_size = lhs.dimensions();
        auto rhs_size = rhs.dimensions();

        if (lhs_size != rhs_size)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "equal::equal2d2d",
                util::generate_error_message(
                    "the dimensions of the operands do not match",
                    name_, codename_));
        }

        // TODO: SIMD functionality should be added, blaze implementation
        // is not currently available
        if (lhs.is_ref())
        {
            lhs = blaze::map(lhs.matrix(), rhs.matrix(),
                [&](T x, T y) { return (x == y); });
        }
        else
        {
            lhs.matrix() = blaze::map(lhs.matrix(), rhs.matrix(),
                [&](T x, T y) { return (x == y); });
        }

        if (propagate_type)
        {
            return primitive_argument_type(ir::node_data<T>{std::move(lhs)});
        }

        return primitive_argument_type(
            ir::node_data<std::uint8_t>{std::move(lhs)});
    }

    template <typename T>
    primitive_argument_type equal::equal2d(ir::node_data<T>&& lhs,
        ir::node_data<T>&& rhs, bool propagate_type) const
    {
        std::size_t rhs_dims = rhs.num_dimensions();
        switch(rhs_dims)
        {
        case 0:
            return equal2d0d(std::move(lhs), std::move(rhs), propagate_type);

        case 1:
            return equal2d1d(std::move(lhs), std::move(rhs), propagate_type);

        case 2:
            return equal2d2d(std::move(lhs), std::move(rhs), propagate_type);

        default:
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "equal::equal2d",
                util::generate_error_message(
                    "the operands have incompatible number of "
                        "dimensions",
                    name_, codename_));
        }
    }

    template <typename T>
    primitive_argument_type equal::equal_all(ir::node_data<T>&& lhs,
        ir::node_data<T>&& rhs, bool propagate_type) const
    {
        std::size_t lhs_dims = lhs.num_dimensions();
        switch (lhs_dims)
        {
        case 0:
            return equal0d(std::move(lhs), std::move(rhs), propagate_type);

        case 1:
            return equal1d(std::move(lhs), std::move(rhs), propagate_type);

        case 2:
            return equal2d(std::move(lhs), std::move(rhs), propagate_type);

        default:
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "equal::equal_all",
                util::generate_error_message(
                    "left hand side operand has unsupported number "
                        "of dimensions",
                    name_, codename_));
        }
    }

    struct equal::visit_equal
    {
        template <typename T1, typename T2>
        primitive_argument_type operator()(T1, T2) const
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "equal::eval",
                util::generate_error_message(
                    "left hand side and right hand side are "
                        "incompatible and can't be compared",
                    equal_.name_, equal_.codename_));
        }

        template <typename T>
        primitive_argument_type operator()(T && lhs, T && rhs) const
        {
            return primitive_argument_type(
                ir::node_data<std::uint8_t>{lhs == rhs});
        }

        primitive_argument_type operator()(ir::node_data<double>&& lhs,
            ir::node_data<std::int64_t>&& rhs) const
        {
            if (lhs.num_dimensions() != 0 || rhs.num_dimensions() != 0)
            {
                return equal_.equal_all(std::move(lhs),
                    ir::node_data<double>(std::move(rhs)), propagate_type_);
            }

            if (propagate_type_)
            {
                return primitive_argument_type(
                    ir::node_data<double>{(lhs[0] == rhs[0]) ? 1.0 : 0.0});
            }

            return primitive_argument_type(
                ir::node_data<std::uint8_t>{lhs[0] == rhs[0]});
        }

        primitive_argument_type operator()(ir::node_data<std::int64_t>&& lhs,
            ir::node_data<double>&& rhs) const
        {
            if (lhs.num_dimensions() != 0 || rhs.num_dimensions() != 0)
            {
                return equal_.equal_all(ir::node_data<double>(std::move(lhs)),
                    std::move(rhs), propagate_type_);
            }

            if (propagate_type_)
            {
                return primitive_argument_type(
                    ir::node_data<double>{(lhs[0] == rhs[0]) ? 1.0 : 0.0});
            }

            return primitive_argument_type(
                ir::node_data<std::uint8_t>{lhs[0] == rhs[0]});
        }

        primitive_argument_type operator()(ir::node_data<std::uint8_t>&& lhs,
            ir::node_data<std::int64_t>&& rhs) const
        {
            if (lhs.num_dimensions() != 0 || rhs.num_dimensions() != 0)
            {
                return equal_.equal_all(std::move(lhs),
                    ir::node_data<std::uint8_t>{
                        std::move(rhs) != ir::node_data<std::int64_t>(0) != 0},
                    propagate_type_);
            }

            if (propagate_type_)
            {
                return primitive_argument_type(ir::node_data<std::int64_t>{
                    (lhs[0] == rhs[0]) ? 1 : 0});
            }

            return primitive_argument_type(
                ir::node_data<std::uint8_t>{lhs[0] == rhs[0]});
        }

        primitive_argument_type operator()(ir::node_data<std::int64_t>&& lhs,
            ir::node_data<std::uint8_t>&& rhs) const
        {
            if (lhs.num_dimensions() != 0 || rhs.num_dimensions() != 0)
            {
                return equal_.equal_all(
                    ir::node_data<std::uint8_t>{
                        std::move(lhs) != ir::node_data<std::int64_t>(0)},
                    std::move(rhs), propagate_type_);
            }

            if (propagate_type_)
            {
                return primitive_argument_type(ir::node_data<std::int64_t>{
                    (lhs[0] == rhs[0]) ? 1 : 0});
            }

            return primitive_argument_type(
                ir::node_data<std::uint8_t>{lhs[0] == rhs[0]});
        }

        primitive_argument_type operator()(ir::node_data<std::uint8_t>&& lhs,
            ir::node_data<double>&& rhs) const
        {
            if (lhs.num_dimensions() != 0 || rhs.num_dimensions() != 0)
            {
                return equal_.equal_all(std::move(lhs),
                    ir::node_data<std::uint8_t>{
                        std::move(rhs) != ir::node_data<double>(0) != 0},
                    propagate_type_);
            }

            if (propagate_type_)
            {
                return primitive_argument_type(ir::node_data<double>{
                    (lhs[0] == rhs[0]) ? 1.0 : 0.0});
            }

            return primitive_argument_type(
                ir::node_data<std::uint8_t>{lhs[0] == rhs[0]});
        }

        primitive_argument_type operator()(ir::node_data<double>&& lhs,
            ir::node_data<std::uint8_t>&& rhs) const
        {
            if (lhs.num_dimensions() != 0 || rhs.num_dimensions() != 0)
            {
                return equal_.equal_all(
                    ir::node_data<std::uint8_t>{
                        std::move(lhs) != ir::node_data<double>(0)},
                    std::move(rhs), propagate_type_);
            }

            if (propagate_type_)
            {
                return primitive_argument_type(ir::node_data<double>{
                    (lhs[0] == rhs[0]) ? 1.0 : 0.0});
            }

            return primitive_argument_type(
                ir::node_data<std::uint8_t>{lhs[0] == rhs[0]});
        }

        primitive_argument_type operator()(
            ir::node_data<std::uint8_t>&& lhs,
            ir::node_data<std::uint8_t>&& rhs) const
        {
            return equal_.equal_all(
                std::move(lhs), std::move(rhs), propagate_type_);
        }

        primitive_argument_type operator()(
            ir::node_data<double>&& lhs, ir::node_data<double>&& rhs) const
        {
            return equal_.equal_all(
                std::move(lhs), std::move(rhs), propagate_type_);
        }

        primitive_argument_type operator()(ir::node_data<std::int64_t>&& lhs,
            ir::node_data<std::int64_t>&& rhs) const
        {
            return equal_.equal_all(
                std::move(lhs), std::move(rhs), propagate_type_);
        }

        equal const& equal_;
        bool propagate_type_;
    };

    hpx::future<primitive_argument_type> equal::eval(
        primitive_arguments_type const& operands,
        primitive_arguments_type const& args) const
    {
        if (operands.size() < 2 || operands.size() > 3)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "equal::eval",
                util::generate_error_message(
                    "the equal primitive requires exactly two operands",
                    name_, codename_));
        }

        if (!valid(operands[0]) || !valid(operands[1]) ||
            (operands.size() == 3 && !valid(operands[2])))
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "equal::eval",
                util::generate_error_message(
                    "the equal primitive requires that the arguments "
                        "given by the operands array are valid",
                    name_, codename_));
        }

        auto this_ = this->shared_from_this();
        bool return_double = (operands.size() == 3 &&
            phylanx::execution_tree::extract_boolean_value_scalar(operands[2]));

        return hpx::dataflow(hpx::launch::sync, hpx::util::unwrapping(
            [this_, return_double](primitive_argument_type&& op1,
                primitive_argument_type&& op2)
            -> primitive_argument_type
            {
                return primitive_argument_type(
                    util::visit(visit_equal{*this_, return_double},
                        std::move(op1.variant()),
                        std::move(op2.variant())));
            }),
            literal_operand(operands[0], args, name_, codename_),
            literal_operand(operands[1], args, name_, codename_));
    }

    // implement '==' for all possible combinations of lhs and rhs
    hpx::future<primitive_argument_type> equal::eval(
        primitive_arguments_type const& args, eval_mode) const
    {
        if (this->no_operands())
        {
            return eval(args, noargs);
        }
        return eval(this->operands(), args);
    }
}}}
