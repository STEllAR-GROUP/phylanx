// Copyright (c) 2017-2018 Hartmut Kaiser
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>
#include <phylanx/execution_tree/primitives/greater.hpp>
#include <phylanx/ir/node_data.hpp>
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
    primitive create_greater(hpx::id_type const& locality,
        std::vector<primitive_argument_type>&& operands,
        std::string const& name, std::string const& codename)
    {
        static std::string type("__gt");
        return create_primitive_component(
            locality, type, std::move(operands), name, codename);
    }

    match_pattern_type const greater::match_data =
    {
        hpx::util::make_tuple("__gt",
            std::vector<std::string>{"_1 > _2"},
            &create_greater, &create_primitive<greater>)
    };

    ///////////////////////////////////////////////////////////////////////////
    greater::greater(std::vector<primitive_argument_type>&& operands,
            std::string const& name, std::string const& codename)
      : primitive_component_base(std::move(operands), name, codename)
    {}

    ///////////////////////////////////////////////////////////////////////////
    primitive_argument_type greater::greater0d1d(
        operand_type&& lhs, operand_type&& rhs) const
    {
        // TODO: SIMD functionality should be added, blaze implementation
        // is not currently available
        if (rhs.is_ref())
        {
            rhs = blaze::map(rhs.vector(),
                [&](double x) { return (x > lhs.scalar()); });
        }
        else
        {
            rhs.vector() = blaze::map(rhs.vector(),
                [&](double x) { return (x > lhs.scalar()); });
        }

        return primitive_argument_type(
            ir::node_data<bool>{std::move(rhs)});
    }

    primitive_argument_type greater::greater0d2d(
        operand_type&& lhs, operand_type&& rhs) const
    {
        // TODO: SIMD functionality should be added, blaze implementation
        // is not currently available
        if (rhs.is_ref())
        {
            rhs = blaze::map(rhs.matrix(),
                [&](double x) { return (x > lhs.scalar()); });
        }
        else
        {
            rhs.matrix() = blaze::map(rhs.matrix(),
                [&](double x) { return (x > lhs.scalar()); });
        }

        return primitive_argument_type(
            ir::node_data<bool>{std::move(rhs)});
    }

    primitive_argument_type greater::greater0d(
        operand_type&& lhs, operand_type&& rhs) const
    {
        std::size_t rhs_dims = rhs.num_dimensions();
        switch(rhs_dims)
        {
        case 0:
            return primitive_argument_type(
                ir::node_data<bool>{lhs.scalar() > rhs.scalar()});

        case 1:
            return greater0d1d(std::move(lhs), std::move(rhs));

        case 2:
            return greater0d2d(std::move(lhs), std::move(rhs));

        default:
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "greater::greater0d",
                execution_tree::generate_error_message(
                    "the operands have incompatible number of "
                        "dimensions",
                    name_, codename_));
        }
    }

    primitive_argument_type greater::greater1d0d(
        operand_type&& lhs, operand_type&& rhs) const
    {
        // TODO: SIMD functionality should be added, blaze implementation
        // is not currently available
        if (lhs.is_ref())
        {
            lhs = blaze::map(lhs.vector(),
                [&](double x) { return (x > rhs.scalar()); });
        }
        else
        {
            lhs.vector() = blaze::map(lhs.vector(),
                [&](double x) { return (x > rhs.scalar()); });
        }

        return primitive_argument_type(
            ir::node_data<bool>{std::move(lhs)});
    }

    primitive_argument_type greater::greater1d1d(
        operand_type&& lhs, operand_type&& rhs) const
    {
        std::size_t lhs_size = lhs.dimension(0);
        std::size_t rhs_size = rhs.dimension(0);

        if (lhs_size != rhs_size)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "greater::greater1d1d",
                execution_tree::generate_error_message(
                    "the dimensions of the operands do not match",
                    name_, codename_));
        }

        // TODO: SIMD functionality should be added, blaze implementation
        // is not currently available
        if (lhs.is_ref())
        {
            lhs = blaze::map(lhs.vector(), rhs.vector(),
                [&](double x, double y) { return (x > y); });
        }
        else
        {
            lhs.vector() = blaze::map(lhs.vector(), rhs.vector(),
                [&](double x, double y) { return (x > y); });
        }

        return primitive_argument_type(
            ir::node_data<bool>{std::move(lhs)});
    }

    primitive_argument_type greater::greater1d2d(
        operand_type&& lhs, operand_type&& rhs) const
    {
        auto cv = lhs.vector();
        auto cm = rhs.matrix();

        if (cv.size() != cm.columns())
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "greater::greater1d2d",
                execution_tree::generate_error_message(
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
                    [](double x, double y) { return x > y; });
            return primitive_argument_type(
                ir::node_data<bool>{std::move(m)});
        }

        for (size_t i = 0UL; i != cm.rows(); i++)
            blaze::row(cm, i) = blaze::map(blaze::row(cm, i),
                blaze::trans(cv),
                [](double x, double y) { return x > y; });

        return primitive_argument_type(
            ir::node_data<bool>{std::move(rhs)});
    }

    primitive_argument_type greater::greater1d(
        operand_type&& lhs, operand_type&& rhs) const
    {
        std::size_t rhs_dims = rhs.num_dimensions();
        switch(rhs_dims)
        {
        case 0:
            return greater1d0d(std::move(lhs), std::move(rhs));

        case 1:
            return greater1d1d(std::move(lhs), std::move(rhs));

        case 2:
            return greater1d2d(std::move(lhs), std::move(rhs));

        default:
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "greater::greater1d",
                execution_tree::generate_error_message(
                    "the operands have incompatible number of "
                        "dimensions",
                    name_, codename_));
        }
    }

    primitive_argument_type greater::greater2d0d(
        operand_type&& lhs, operand_type&& rhs) const
    {
        std::size_t lhs_size = lhs.dimension(0);
        std::size_t rhs_size = rhs.dimension(0);

        // TODO: SIMD functionality should be added, blaze implementation
        // is not currently available
        if (lhs.is_ref())
        {
            lhs = blaze::map(lhs.matrix(),
                [&](double x) { return (x > rhs.scalar()); });
        }
        else
        {
            lhs.matrix() = blaze::map(lhs.matrix(),
                [&](double x) { return (x > rhs.scalar()); });
        }

        return primitive_argument_type(
            ir::node_data<bool>{std::move(lhs)});
    }

    primitive_argument_type greater::greater2d1d(
        operand_type&& lhs, operand_type&& rhs) const
    {
        auto cv = rhs.vector();
        auto cm = lhs.matrix();

        if (cv.size() != cm.columns())
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "greater::greater2d1d",
                execution_tree::generate_error_message(
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
                    [](double x, double y) { return x > y; });
            return primitive_argument_type(
                ir::node_data<bool>{std::move(m)});
        }

        for (size_t i = 0UL; i != cm.rows(); i++)
            blaze::row(cm, i) = blaze::map(blaze::row(cm, i),
                blaze::trans(cv),
                [](double x, double y) { return x > y; });

        return primitive_argument_type(
            ir::node_data<bool>{std::move(lhs)});
    }

    primitive_argument_type greater::greater2d2d(
        operand_type&& lhs, operand_type&& rhs) const
    {
        auto lhs_size = lhs.dimensions();
        auto rhs_size = rhs.dimensions();

        if (lhs_size != rhs_size)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "greater::greater2d2d",
                execution_tree::generate_error_message(
                    "the dimensions of the operands do not match",
                    name_, codename_));
        }

        // TODO: SIMD functionality should be added, blaze implementation
        //       is not currently available
        if (lhs.is_ref())
        {
            lhs = blaze::map(lhs.matrix(), rhs.matrix(),
                [&](double x, double y) { return (x > y); });
        }
        else
        {
            lhs.matrix() = blaze::map(lhs.matrix(), rhs.matrix(),
                [&](double x, double y) { return (x > y); });
        }

        return primitive_argument_type(
            ir::node_data<bool>{std::move(lhs)});
    }

    primitive_argument_type greater::greater2d(
        operand_type&& lhs, operand_type&& rhs) const
    {
        std::size_t rhs_dims = rhs.num_dimensions();
        switch(rhs_dims)
        {
        case 0:
            return greater2d0d(std::move(lhs), std::move(rhs));

        case 1:
            return greater2d1d(std::move(lhs), std::move(rhs));

        case 2:
            return greater2d2d(std::move(lhs), std::move(rhs));

        default:
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "greater::greater2d",
                execution_tree::generate_error_message(
                    "the operands have incompatible number of "
                        "dimensions",
                    name_, codename_));
        }
    }

    primitive_argument_type greater::greater_all(
        operand_type&& lhs, operand_type&& rhs) const
    {
        std::size_t lhs_dims = lhs.num_dimensions();
        switch (lhs_dims)
        {
        case 0:
            return greater0d(std::move(lhs), std::move(rhs));

        case 1:
            return greater1d(std::move(lhs), std::move(rhs));

        case 2:
            return greater2d(std::move(lhs), std::move(rhs));

        default:
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "greater::greater_all",
                execution_tree::generate_error_message(
                    "left hand side operand has unsupported number of "
                        "dimensions",
                    name_, codename_));
        }
    }

    struct greater::visit_greater
    {
        template <typename T1, typename T2>
        primitive_argument_type operator()(T1, T2) const
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "greater::eval",
                execution_tree::generate_error_message(
                    "left hand side and right hand side are incompatible "
                        "and can't be compared",
                    that_.name_, that_.codename_));
        }

        primitive_argument_type operator()(
            ir::node_data<primitive_argument_type>&&,
            ir::node_data<primitive_argument_type>&&) const
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "greater::eval",
                execution_tree::generate_error_message(
                    "left hand side and right hand side are incompatible "
                        "and can't be compared",
                    that_.name_, that_.codename_));
        }

        primitive_argument_type operator()(std::vector<ast::expression>&&,
            std::vector<ast::expression>&&) const
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "greater::eval",
                execution_tree::generate_error_message(
                    "left hand side and right hand side are incompatible "
                        "and can't be compared",
                    that_.name_, that_.codename_));
        }

        primitive_argument_type operator()(
            ast::expression&&, ast::expression&&) const
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "greater::eval",
                execution_tree::generate_error_message(
                    "left hand side and right hand side are incompatible "
                        "and can't be compared",
                    that_.name_, that_.codename_));
        }

        primitive_argument_type operator()(primitive&&, primitive&&) const
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "greater::eval",
                execution_tree::generate_error_message(
                    "left hand side and right hand side are incompatible "
                        "and can't be compared",
                    that_.name_, that_.codename_));
        }

        template <typename T>
        primitive_argument_type operator()(T && lhs, T && rhs) const
        {
            return primitive_argument_type(
                ir::node_data<bool>{lhs > rhs});
        }

        primitive_argument_type operator()(
            util::recursive_wrapper<ir::range>&&,
            util::recursive_wrapper<ir::range>&&) const
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "greater::eval",
                execution_tree::generate_error_message(
                    "left hand side and right hand side are incompatible "
                        "and can't be compared",
                    that_.name_, that_.codename_));
        }

        primitive_argument_type operator()(
            ir::node_data<double>&& lhs, std::int64_t&& rhs) const
        {
            if (lhs.num_dimensions() != 0)
            {
                return that_.greater_all(
                    std::move(lhs), operand_type(std::move(rhs)));
            }
            return primitive_argument_type(
                ir::node_data<bool>{lhs[0] > rhs});
        }

        primitive_argument_type operator()(
            std::int64_t&& lhs, ir::node_data<double>&& rhs) const
        {
            if (rhs.num_dimensions() != 0)
            {
                return that_.greater_all(
                    operand_type(std::move(lhs)), std::move(rhs));
            }
            return primitive_argument_type(
                ir::node_data<bool>{lhs > rhs[0]});
        }

        primitive_argument_type operator()(
            operand_type&& lhs, operand_type&& rhs) const
        {
            return that_.greater_all(
                std::move(lhs), std::move(rhs));
        }

        primitive_argument_type operator()(
            ir::node_data<bool>&& lhs, ir::node_data<bool>&& rhs) const
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "greater::eval",
                execution_tree::generate_error_message(
                    "left hand side and right hand side can't be compared",
                    that_.name_, that_.codename_));
        }

        greater const& that_;
    };

    hpx::future<primitive_argument_type> greater::eval(
        std::vector<primitive_argument_type> const& operands,
        std::vector<primitive_argument_type> const& args) const
    {
        if (operands.size() != 2)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "greater::eval",
                execution_tree::generate_error_message(
                    "the greater primitive requires exactly two "
                        "operands",
                    name_, codename_));
        }

        if (!valid(operands[0]) || !valid(operands[1]))
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "greater::eval",
                execution_tree::generate_error_message(
                    "the greater primitive requires that the "
                        "arguments given by the operands array are valid",
                    name_, codename_));
        }

        auto this_ = this->shared_from_this();
        return hpx::dataflow(hpx::util::unwrapping(
            [this_](operands_type && ops) -> primitive_argument_type
            {
                return primitive_argument_type(
                    util::visit(visit_greater{*this_},
                        std::move(ops[0].variant()),
                        std::move(ops[1].variant())));
            }),
            detail::map_operands(
                operands, functional::literal_operand{}, args,
                name_, codename_));
    }

    //////////////////////////////////////////////////////////////////////////
    // Implement '>' for all possible combinations of lhs and rhs
    hpx::future<primitive_argument_type> greater::eval(
        std::vector<primitive_argument_type> const& args) const
    {
        if (operands_.empty())
        {
            return eval(args, noargs);
        }
        return eval(operands_, args);
    }
}}}
