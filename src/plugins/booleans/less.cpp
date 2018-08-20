//  Copyright (c) 2017-2018 Hartmut Kaiser
//  Copyright (c) 2018 Tianyi Zhang
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>
#include <phylanx/ir/node_data.hpp>
#include <phylanx/plugins/booleans/less.hpp>
#include <phylanx/ir/ranges.hpp>

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
    match_pattern_type const less::match_data =
    {
        hpx::util::make_tuple("__lt",
            std::vector<std::string>{
                "_1 < _2", "__lt(_1, _2)", "__lt(_1, _2, _3)"},
            &create_less, &create_primitive<less>,
                "arg1,arg2,arg3\n"
                "Args:\n"
                "\n"
                "    arg1 (number) : A value to compare\n"
                "    arg2 (number) : Another value to compare\n"
                "    arg3 (boolean,optional) : whether to use a\n"
                "                           numeric return value.\n"
                "\n"
                "Returns:\n"
                "\n"
                "    if arg3 is true\n"
                "      return 1 if arg1 < arg2, 0 otherwise.\n"
                "    else\n"
                "      return True if arg1 < arg2, False otherwise.\n"
            )
    };

    ///////////////////////////////////////////////////////////////////////////
    less::less(std::vector<primitive_argument_type>&& operands,
            std::string const& name, std::string const& codename)
      : primitive_component_base(std::move(operands), name, codename)
    {}

    ///////////////////////////////////////////////////////////////////////////
    template <typename T>
    primitive_argument_type less::less0d0d(
        ir::node_data<T>&& lhs, ir::node_data<T>&& rhs, bool type_double) const
    {
        if (type_double)
        {
            return primitive_argument_type(ir::node_data<double>{
                (lhs.scalar() < rhs.scalar()) ? 1.0 : 0.0});
        }
        return primitive_argument_type(
            ir::node_data<std::uint8_t>{lhs.scalar() < rhs.scalar()});
    }

    template <typename T>
    primitive_argument_type less::less0d1d(
        ir::node_data<T>&& lhs, ir::node_data<T>&& rhs, bool type_double) const
    {
        // TODO: SIMD functionality should be added, blaze implementation
        // is not currently available
        if (rhs.is_ref())
        {
            rhs = blaze::map(rhs.vector(),
                [&](double x) { return (x < lhs.scalar()); });
        }
        else
        {
            rhs.vector() = blaze::map(rhs.vector(),
                [&](double x) { return (x < lhs.scalar()); });
        }

        if (type_double)
        {
            return primitive_argument_type(
                ir::node_data<double>{std::move(rhs)});
        }
        return primitive_argument_type(
            ir::node_data<std::uint8_t>{std::move(rhs)});
    }

    template <typename T>
    primitive_argument_type less::less0d2d(
        ir::node_data<T>&& lhs, ir::node_data<T>&& rhs, bool type_double) const
    {
        // TODO: SIMD functionality should be added, blaze implementation
        // is not currently available
        if (rhs.is_ref())
        {
            rhs = blaze::map(rhs.matrix(),
                [&](double x) { return (x < lhs.scalar()); });
        }
        else
        {
            rhs.matrix() = blaze::map(rhs.matrix(),
                [&](double x) { return (x < lhs.scalar()); });
        }

        if (type_double)
        {
            return primitive_argument_type(
                ir::node_data<double>{std::move(rhs)});
        }
        return primitive_argument_type(
            ir::node_data<std::uint8_t>{std::move(rhs)});
    }

    template <typename T>
    primitive_argument_type less::less0d(
        ir::node_data<T>&& lhs, ir::node_data<T>&& rhs, bool type_double) const
    {
        std::size_t rhs_dims = rhs.num_dimensions();
        switch(rhs_dims)
        {
        case 0:
            return less0d0d(std::move(lhs), std::move(rhs), type_double);

        case 1:
            return less0d1d(std::move(lhs), std::move(rhs), type_double);

        case 2:
            return less0d2d(std::move(lhs), std::move(rhs), type_double);

        default:
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "less::less0d",
                util::generate_error_message(
                    "the operands have incompatible number of "
                        "dimensions",
                    name_, codename_));
        }
    }

    template <typename T>
    primitive_argument_type less::less1d0d(
        ir::node_data<T>&& lhs, ir::node_data<T>&& rhs, bool type_double) const
    {
        // TODO: SIMD functionality should be added, blaze implementation
        // is not currently available
        if (lhs.is_ref())
        {
            lhs = blaze::map(lhs.vector(),
                [&](double x) { return (x < rhs.scalar()); });
        }
        else
        {
            lhs.vector() = blaze::map(lhs.vector(),
                [&](double x) { return (x < rhs.scalar()); });
        }

        if (type_double)
        {
            return primitive_argument_type(
                ir::node_data<double>{std::move(lhs)});
        }
        return primitive_argument_type(
            ir::node_data<std::uint8_t>{std::move(lhs)});
    }

    template <typename T>
    primitive_argument_type less::less1d1d(
        ir::node_data<T>&& lhs, ir::node_data<T>&& rhs, bool type_double) const
    {
        std::size_t lhs_size = lhs.dimension(0);
        std::size_t rhs_size = rhs.dimension(0);

        if (lhs_size != rhs_size)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "less::less1d1d",
                util::generate_error_message(
                    "the dimensions of the operands do not match",
                    name_, codename_));
        }

        // TODO: SIMD functionality should be added, blaze implementation
        // is not currently available
        if (lhs.is_ref())
        {
            lhs = blaze::map(lhs.vector(), rhs.vector(),
                [&](double x, double y) { return (x < y); });
        }
        else
        {
            lhs.vector() = blaze::map(lhs.vector(), rhs.vector(),
                [&](double x, double y) { return (x < y); });
        }

        if (type_double)
        {
            return primitive_argument_type(
                ir::node_data<double>{std::move(lhs)});
        }
        return primitive_argument_type(
            ir::node_data<std::uint8_t>{std::move(lhs)});
    }

    template <typename T>
    primitive_argument_type less::less1d2d(
        ir::node_data<T>&& lhs, ir::node_data<T>&& rhs, bool type_double) const
    {
        auto cv = lhs.vector();
        auto cm = rhs.matrix();

        if (cv.size() != cm.columns())
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "less::less1d2d",
                util::generate_error_message(
                    "the dimensions of the operands do not match",
                    name_, codename_));
        }

        // TODO: SIMD functionality should be added, blaze implementation
        // is not currently available
        if (rhs.is_ref())
        {
            blaze::DynamicMatrix<double> m{cm.rows(), cm.columns()};

            for (size_t i = 0UL; i != cm.rows(); i++)
                blaze::row(m, i) = blaze::map(blaze::row(cm, i),
                    blaze::trans(cv),
                    [](double x, double y) { return x < y; });
            if (type_double)
            {
                return primitive_argument_type(
                    ir::node_data<double>{std::move(m)});
            }
            return primitive_argument_type(
                ir::node_data<std::uint8_t>{std::move(m)});
        }

        for (size_t i = 0UL; i != cm.rows(); i++)
            blaze::row(cm, i) = blaze::map(blaze::row(cm, i),
                blaze::trans(cv),
                [](double x, double y) { return x < y; });
        if (type_double)
        {
            return primitive_argument_type(
                ir::node_data<double>{std::move(rhs)});
        }
        return primitive_argument_type(
            ir::node_data<std::uint8_t>{std::move(rhs)});
    }

    template <typename T>
    primitive_argument_type less::less1d(
        ir::node_data<T>&& lhs, ir::node_data<T>&& rhs, bool type_double) const
    {
        std::size_t rhs_dims = rhs.num_dimensions();
        switch(rhs_dims)
        {
        case 0:
            return less1d0d(std::move(lhs), std::move(rhs), type_double);

        case 1:
            return less1d1d(std::move(lhs), std::move(rhs), type_double);

        case 2:
            return less1d2d(std::move(lhs), std::move(rhs), type_double);

        default:
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "less::less1d",
                util::generate_error_message(
                    "the operands have incompatible number of "
                        "dimensions",
                    name_, codename_));
        }
    }

    template <typename T>
    primitive_argument_type less::less2d0d(
        ir::node_data<T>&& lhs, ir::node_data<T>&& rhs, bool type_double) const
    {
        std::size_t lhs_size = lhs.dimension(0);
        std::size_t rhs_size = rhs.dimension(0);

        // TODO: SIMD functionality should be added, blaze implementation
        // is not currently available
        if (lhs.is_ref())
        {
            lhs = blaze::map(lhs.matrix(),
                [&](double x) { return (x < rhs.scalar()); });
        }
        else
        {
            lhs.matrix() = blaze::map(lhs.matrix(),
                [&](double x) { return (x < rhs.scalar()); });
        }

        if (type_double)
        {
            return primitive_argument_type(
                ir::node_data<double>{std::move(lhs)});
        }
        return primitive_argument_type(
            ir::node_data<std::uint8_t>{std::move(lhs)});
    }

    template <typename T>
    primitive_argument_type less::less2d1d(
        ir::node_data<T>&& lhs, ir::node_data<T>&& rhs, bool type_double) const
    {
        auto cv = rhs.vector();
        auto cm = lhs.matrix();

        if (cv.size() != cm.columns())
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "less::less2d1d",
                util::generate_error_message(
                    "the dimensions of the operands do not match",
                    name_, codename_));
        }

        // TODO: SIMD functionality should be added, blaze implementation
        // is not currently available
        if (lhs.is_ref())
        {
            blaze::DynamicMatrix<double> m{cm.rows(), cm.columns()};

            for (size_t i = 0UL; i != cm.rows(); i++)
                blaze::row(m, i) = blaze::map(blaze::row(cm, i),
                    blaze::trans(cv),
                    [](double x, double y) { return x < y; });
            if (type_double)
            {
                return primitive_argument_type(
                    ir::node_data<double>{std::move(m)});
            }
            return primitive_argument_type(
                ir::node_data<std::uint8_t>{std::move(m)});
        }

        for (size_t i = 0UL; i != cm.rows(); i++)
            blaze::row(cm, i) = blaze::map(blaze::row(cm, i),
                blaze::trans(cv),
                [](double x, double y) { return x < y; });

        if (type_double)
        {
            return primitive_argument_type(
                ir::node_data<double>{std::move(rhs)});
        }
        return primitive_argument_type(
            ir::node_data<std::uint8_t>{std::move(lhs)});
    }

    template <typename T>
    primitive_argument_type less::less2d2d(
        ir::node_data<T>&& lhs, ir::node_data<T>&& rhs, bool type_double) const
    {
        auto lhs_size = lhs.dimensions();
        auto rhs_size = rhs.dimensions();

        if (lhs_size != rhs_size)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "less::less2d2d",
                util::generate_error_message(
                    "the dimensions of the operands do not match",
                    name_, codename_));
        }

        // TODO: SIMD functionality should be added, blaze implementation
        //       is not currently available
        if (lhs.is_ref())
        {
            lhs = blaze::map(lhs.matrix(), rhs.matrix(),
                [&](double x, double y) { return (x < y); });
        }
        else
        {
            lhs.matrix() = blaze::map(lhs.matrix(), rhs.matrix(),
                [&](double x, double y) { return (x < y); });
        }

        if (type_double)
        {
            return primitive_argument_type(
                ir::node_data<double>{std::move(lhs)});
        }
        return primitive_argument_type(
            ir::node_data<std::uint8_t>{std::move(lhs)});
    }

    template <typename T>
    primitive_argument_type less::less2d(
        ir::node_data<T>&& lhs, ir::node_data<T>&& rhs, bool type_double) const
    {
        std::size_t rhs_dims = rhs.num_dimensions();
        switch(rhs_dims)
        {
        case 0:
            return less2d0d(std::move(lhs), std::move(rhs), type_double);

        case 1:
            return less2d1d(std::move(lhs), std::move(rhs), type_double);

        case 2:
            return less2d2d(std::move(lhs), std::move(rhs), type_double);

        default:
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "less::less2d",
                util::generate_error_message(
                    "the operands have incompatible number of "
                        "dimensions",
                    name_, codename_));
        }
    }

    template <typename T>
    primitive_argument_type less::less_all(
        ir::node_data<T>&& lhs, ir::node_data<T>&& rhs, bool type_double) const
    {
        std::size_t lhs_dims = lhs.num_dimensions();
        switch (lhs_dims)
        {
        case 0:
            return less0d(std::move(lhs), std::move(rhs), type_double);

        case 1:
            return less1d(std::move(lhs), std::move(rhs), type_double);

        case 2:
            return less2d(std::move(lhs), std::move(rhs), type_double);

        default:
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "less::less_all",
                util::generate_error_message(
                    "left hand side operand has unsupported number of "
                        "dimensions",
                    name_, codename_));
        }
    }

    struct less::visit_less
    {
        template <typename T1, typename T2>
        primitive_argument_type operator()(T1, T2) const
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "less::eval",
                util::generate_error_message(
                    "left hand side and right hand side are incompatible "
                        "and can't be compared",
                    less_.name_, less_.codename_));
        }

        primitive_argument_type operator()(
            ir::node_data<primitive_argument_type>&&,
            ir::node_data<primitive_argument_type>&&) const
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "less::eval",
                util::generate_error_message(
                    "left hand side and right hand side are incompatible "
                        "and can't be compared",
                    less_.name_, less_.codename_));
        }

        primitive_argument_type operator()(std::vector<ast::expression>&&,
            std::vector<ast::expression>&&) const
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "less::eval",
                util::generate_error_message(
                    "left hand side and right hand side are incompatible "
                        "and can't be compared",
                    less_.name_, less_.codename_));
        }

        primitive_argument_type operator()(
            ast::expression&&, ast::expression&&) const
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "less::eval",
                util::generate_error_message(
                    "left hand side and right hand side are incompatible "
                        "and can't be compared",
                    less_.name_, less_.codename_));
        }

        primitive_argument_type operator()(primitive&&, primitive&&) const
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "less::eval",
                util::generate_error_message(
                    "left hand side and right hand side are incompatible "
                        "and can't be compared",
                    less_.name_, less_.codename_));
        }

        template <typename T>
        primitive_argument_type operator()(T && lhs, T && rhs) const
        {
            if (type_double_)
            {
                return primitive_argument_type(
                    ir::node_data<double>{(lhs < rhs) ? 1.0 : 0.0});
            }
            return primitive_argument_type(
                ir::node_data<std::uint8_t>{lhs < rhs});
        }

        primitive_argument_type operator()(ir::range&&, ir::range&&) const
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "less::eval",
                util::generate_error_message(
                    "left hand side and right hand side are incompatible "
                        "and can't be compared",
                    less_.name_, less_.codename_));
        }

        primitive_argument_type operator()(ir::dictionary&&, ir::dictionary&&) const
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "less::eval",
                util::generate_error_message(
                    "left hand side and right hand side are incompatible "
                        "and can't be compared",
                    less_.name_, less_.codename_));
        }

        primitive_argument_type operator()(
            ir::node_data<double>&& lhs, ir::node_data<std::int64_t>&& rhs) const
        {
            if (lhs.num_dimensions() != 0 || rhs.num_dimensions() != 0)
            {
                return less_.less_all(
                    std::move(lhs), operand_type(std::move(rhs)), type_double_);
            }
            if (type_double_)
            {
                return primitive_argument_type(
                    ir::node_data<double>{(lhs[0] < rhs[0]) ? 1.0 : 0.0});
            }
            return primitive_argument_type(
                ir::node_data<std::uint8_t>{lhs[0] < rhs[0]});
        }

        primitive_argument_type operator()(
            ir::node_data<std::int64_t>&& lhs, ir::node_data<double>&& rhs) const
        {
            if (lhs.num_dimensions() != 0 || rhs.num_dimensions() != 0)
            {
                return less_.less_all(
                    operand_type(std::move(lhs)), std::move(rhs), type_double_);
            }
            if (type_double_)
            {
                return primitive_argument_type(
                    ir::node_data<double>{(lhs[0] < rhs[0]) ? 1.0 : 0.0});
            }
            return primitive_argument_type(
                ir::node_data<std::uint8_t>{lhs[0] < rhs[0]});
        }

        primitive_argument_type operator()(ir::node_data<std::uint8_t>&& lhs,
            ir::node_data<std::int64_t>&& rhs) const
        {
            if (lhs.num_dimensions() != 0 || rhs.num_dimensions() != 0)
            {
                return less_.less_all(std::move(lhs),
                    ir::node_data<std::uint8_t>{
                        std::move(rhs) != ir::node_data<std::int64_t>(0)},
                    type_double_);
            }
            if (type_double_)
            {
                return primitive_argument_type(
                    ir::node_data<double>{(lhs[0] < rhs[0]) ? 1.0 : 0.0});
            }
            return primitive_argument_type(
                ir::node_data<std::uint8_t>{lhs[0] < rhs[0]});
        }

        primitive_argument_type operator()(ir::node_data<std::int64_t>&& lhs,
            ir::node_data<std::uint8_t>&& rhs) const
        {
            if (lhs.num_dimensions() != 0 || rhs.num_dimensions() != 0)
            {
                return less_.less_all(
                    ir::node_data<std::uint8_t>{
                        std::move(lhs) != ir::node_data<std::int64_t>(0)},
                    std::move(rhs), type_double_);
            }
            if (type_double_)
            {
                return primitive_argument_type(
                    ir::node_data<double>{(lhs[0] < rhs[0]) ? 1.0 : 0.0});
            }
            return primitive_argument_type(
                ir::node_data<std::uint8_t>{lhs[0] < rhs[0]});
        }

        primitive_argument_type operator()(
            operand_type&& lhs, operand_type&& rhs) const
        {
            return less_.less_all(std::move(lhs), std::move(rhs), type_double_);
        }

        primitive_argument_type operator()(ir::node_data<std::uint8_t>&& lhs,
            ir::node_data<std::uint8_t>&& rhs) const
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "less::eval",
                util::generate_error_message(
                    "left hand side and right hand side can't be compared",
                    less_.name_, less_.codename_));
        }

        primitive_argument_type operator()(ir::node_data<std::int64_t>&& lhs,
            ir::node_data<std::int64_t>&& rhs) const
        {
            return less_.less_all(std::move(lhs), std::move(rhs), type_double_);
        }

        less const& less_;
        bool type_double_ = false;
    };

    hpx::future<primitive_argument_type> less::eval(
        std::vector<primitive_argument_type> const& operands,
        std::vector<primitive_argument_type> const& args) const
    {
        if (operands.size() < 2 || operands.size() > 3)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "less::eval",
                util::generate_error_message(
                    "the less primitive requires exactly two or three "
                        "operands",
                    name_, codename_));
        }

        if (!valid(operands[0]) || !valid(operands[1]) ||
            (operands.size() == 3 && !valid(operands[2])))
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "less::eval",
                util::generate_error_message(
                    "the less primitive requires that the "
                        "arguments given by the operands array are valid",
                    name_, codename_));
        }

        auto this_ = this->shared_from_this();
        if (operands.size() == 3 &&
            phylanx::execution_tree::extract_scalar_boolean_value(operands[2]))
        {
            return hpx::dataflow(hpx::launch::sync,
                hpx::util::unwrapping([this_](primitive_argument_type&& op1,
                                          primitive_argument_type&& op2)
                                          -> primitive_argument_type {
                    return primitive_argument_type(
                        util::visit(visit_less{*this_, true},
                            std::move(op1.variant()),
                            std::move(op2.variant())));
                }),
                literal_operand(operands[0], args, name_, codename_),
                literal_operand(operands[1], args, name_, codename_));
        }
        return hpx::dataflow(hpx::launch::sync, hpx::util::unwrapping(
            [this_](primitive_argument_type&& op1,
                    primitive_argument_type&& op2)
            ->  primitive_argument_type
            {
                return primitive_argument_type(
                    util::visit(visit_less{*this_},
                        std::move(op1.variant()),
                        std::move(op2.variant())));
            }),
            literal_operand(operands[0], args, name_, codename_),
            literal_operand(operands[1], args, name_, codename_));
    }

    //////////////////////////////////////////////////////////////////////////
    // Implement '<' for all possible combinations of lhs and rhs
    hpx::future<primitive_argument_type> less::eval(
        std::vector<primitive_argument_type> const& args) const
    {
        if (this->no_operands())
        {
            return eval(args, noargs);
        }
        return eval(this->operands(), args);
    }
}}}
