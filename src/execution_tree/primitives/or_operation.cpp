//  Copyright (c) 2017-2018 Hartmut Kaiser
//  Copyright (c) 2018 Shahrzad Shirzad
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>
#include <phylanx/execution_tree/primitives/or_operation.hpp>
#include <phylanx/ir/node_data.hpp>

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
    ///////////////////////////////////////////////////////////////////////////
    primitive create_or_operation(hpx::id_type const& locality,
        std::vector<primitive_argument_type>&& operands,
        std::string const& name, std::string const& codename)
    {
        static std::string type("__or");
        return create_primitive_component(
            locality, type, std::move(operands), name, codename);
    }

    match_pattern_type const or_operation::match_data =
    {
        hpx::util::make_tuple("__or",
            std::vector<std::string>{"_1 || __2"},
            &create_or_operation, &create_primitive<or_operation>)
    };

    ///////////////////////////////////////////////////////////////////////////
    or_operation::or_operation(std::vector<primitive_argument_type>&& operands,
            std::string const& name, std::string const& codename)
      : primitive_component_base(std::move(operands), name, codename)
    {}

    ///////////////////////////////////////////////////////////////////////////
    primitive_argument_type or_operation::or_operation0d1d(
        operand_type&& lhs, operand_type&& rhs) const
            {
                // TODO: SIMD functionality should be added, blaze implementation
                // is not currently available
                rhs.vector() = blaze::map(
                    rhs.vector(), [&](bool x) { return (x || lhs.scalar()); });

                return primitive_argument_type(
                    ir::node_data<std::uint8_t>{std::move(rhs)});
            }

    primitive_argument_type or_operation::or_operation0d2d(
        operand_type&& lhs, operand_type&& rhs) const
            {
                // TODO: SIMD functionality should be added, blaze implementation
                // is not currently available
                rhs.matrix() = blaze::map(
                    rhs.matrix(), [&](bool x) { return (x || lhs.scalar()); });

                return primitive_argument_type(
                    ir::node_data<std::uint8_t>{std::move(rhs)});
            }

    primitive_argument_type or_operation::or_operation0d(
        operand_type&& lhs, operand_type&& rhs) const
            {
                std::size_t rhs_dims = rhs.num_dimensions();
                switch (rhs_dims)
                {
                case 0:
                    return primitive_argument_type(
                        ir::node_data<std::uint8_t>{lhs.scalar() || rhs.scalar()});

                case 1:
                    return or_operation0d1d(std::move(lhs), std::move(rhs));

                case 2:
                    return or_operation0d2d(std::move(lhs), std::move(rhs));

                default:
                    HPX_THROW_EXCEPTION(hpx::bad_parameter,
                        "or_operation::or_operation0d",
                        execution_tree::generate_error_message(
                            "the operands have incompatible number of "
                                "dimensions",
                            name_, codename_));
                }
            }

    primitive_argument_type or_operation::or_operation1d0d(
        operand_type&& lhs, operand_type&& rhs) const
            {
                // TODO: SIMD functionality should be added, blaze implementation
                // is not currently available
                lhs.vector() = blaze::map(
                    lhs.vector(), [&](bool x) { return (x || rhs.scalar()); });

                return primitive_argument_type(
                    ir::node_data<std::uint8_t>{std::move(lhs)});
            }

    primitive_argument_type or_operation::or_operation1d1d(
        operand_type&& lhs, operand_type&& rhs) const
            {
                std::size_t lhs_size = lhs.dimension(0);
                std::size_t rhs_size = rhs.dimension(0);

                if (lhs_size != rhs_size)
                {
                    HPX_THROW_EXCEPTION(hpx::bad_parameter,
                        "or_operation::or_operation1d1d",
                        execution_tree::generate_error_message(
                            "the dimensions of the operands do not match",
                            name_, codename_));
                }

                // TODO: SIMD functionality should be added, blaze implementation
                // is not currently available
                lhs.vector() = blaze::map(lhs.vector(), rhs.vector(),
                    [&](bool x, bool y) { return (x || y); });

                return primitive_argument_type(
                    ir::node_data<std::uint8_t>{std::move(lhs)});
            }

    primitive_argument_type or_operation::or_operation1d2d(
        operand_type&& lhs, operand_type&& rhs) const
            {
                std::size_t lhs_size = lhs.dimension(0);
                auto rhs_size = rhs.dimensions();

                if (lhs_size != rhs_size[1])
                {
                    HPX_THROW_EXCEPTION(hpx::bad_parameter,
                        "or_operation::or_operation1d2d",
                        execution_tree::generate_error_message(
                            "the dimensions of the operands do not match",
                            name_, codename_));
                }

                // TODO: SIMD functionality should be added, blaze implementation
                // is not currently available
                for (size_t i = 0UL; i < rhs.matrix().rows(); i++)
                    blaze::row(rhs.matrix(), i) =
                        blaze::map(blaze::row(rhs.matrix(), i),
                            blaze::trans(lhs.vector()),
                            [](bool x, bool y) { return x || y; });

                return primitive_argument_type(
                    ir::node_data<std::uint8_t>{std::move(rhs)});
            }

    primitive_argument_type or_operation::or_operation1d(
        operand_type&& lhs, operand_type&& rhs) const
            {
                std::size_t rhs_dims = rhs.num_dimensions();
                switch (rhs_dims)
                {
                case 0:
                    return or_operation1d0d(std::move(lhs), std::move(rhs));

                case 1:
                    return or_operation1d1d(std::move(lhs), std::move(rhs));

                case 2:
                    return or_operation1d2d(std::move(lhs), std::move(rhs));

                default:
                    HPX_THROW_EXCEPTION(hpx::bad_parameter,
                        "or_operation::or_operation1d",
                        execution_tree::generate_error_message(
                            "the operands have incompatible number of "
                                "dimensions",
                            name_, codename_));
                }
            }

    primitive_argument_type or_operation::or_operation2d0d(
        operand_type&& lhs, operand_type&& rhs) const
            {
                std::size_t lhs_size = lhs.dimension(0);
                std::size_t rhs_size = rhs.dimension(0);

                // TODO: SIMD functionality should be added, blaze implementation
                // is not currently available
                lhs.matrix() = blaze::map(lhs.matrix(),
                    [&](double x) { return (x || rhs.scalar()); });

                return primitive_argument_type(
                    ir::node_data<std::uint8_t>{std::move(lhs)});
            }

    primitive_argument_type or_operation::or_operation2d1d(
        operand_type&& lhs, operand_type&& rhs) const
            {
                std::size_t rhs_size = rhs.dimension(0);
                auto lhs_size = lhs.dimensions();

                if (rhs_size != lhs_size[1])
                {
                    HPX_THROW_EXCEPTION(hpx::bad_parameter,
                        "or_operation::or_operation2d1d",
                        execution_tree::generate_error_message(
                            "the dimensions of the operands do not match",
                            name_, codename_));
                }

                // TODO: SIMD functionality should be added, blaze implementation
                // is not currently available
                for (size_t i = 0UL; i < lhs.matrix().rows(); i++)
                    blaze::row(lhs.matrix(), i) =
                        blaze::map(blaze::row(lhs.matrix(), i),
                            blaze::trans(rhs.vector()),
                            [](bool x, bool y) { return x || y; });

                return primitive_argument_type(
                    ir::node_data<std::uint8_t>{std::move(lhs)});
            }

    primitive_argument_type or_operation::or_operation2d2d(
        operand_type&& lhs, operand_type&& rhs) const
            {
                auto lhs_size = lhs.dimensions();
                auto rhs_size = rhs.dimensions();

                if (lhs_size != rhs_size)
                {
                    HPX_THROW_EXCEPTION(hpx::bad_parameter,
                        "or_operation::or_operation2d2d",
                        execution_tree::generate_error_message(
                            "the dimensions of the operands do not match",
                            name_, codename_));
                }

                // TODO: SIMD functionality should be added, blaze implementation
                // is not currently available
                lhs.matrix() = blaze::map(lhs.matrix(), rhs.matrix(),
                    [&](bool x, bool y) { return (x || y); });

                return primitive_argument_type(
                    ir::node_data<std::uint8_t>{std::move(lhs)});
            }

    primitive_argument_type or_operation::or_operation2d(
        operand_type&& lhs, operand_type&& rhs) const
            {
                std::size_t rhs_dims = rhs.num_dimensions();
                switch (rhs_dims)
                {
                case 0:
                    return or_operation2d0d(std::move(lhs), std::move(rhs));

                case 1:
                    return or_operation2d1d(std::move(lhs), std::move(rhs));

                case 2:
                    return or_operation2d2d(std::move(lhs), std::move(rhs));

                default:
                    HPX_THROW_EXCEPTION(hpx::bad_parameter,
                        "or_operation::or_operation2d",
                        execution_tree::generate_error_message(
                            "the operands have incompatible number of "
                                "dimensions",
                            name_, codename_));
                }
            }

    primitive_argument_type or_operation::or_all(
        operand_type&& lhs, operand_type&& rhs) const
            {
                std::size_t lhs_dims = lhs.num_dimensions();
                switch (lhs_dims)
                {
                case 0:
                    return or_operation0d(std::move(lhs), std::move(rhs));

                case 1:
                    return or_operation1d(std::move(lhs), std::move(rhs));

                case 2:
                    return or_operation2d(std::move(lhs), std::move(rhs));

                default:
                    HPX_THROW_EXCEPTION(hpx::bad_parameter,
                        "or_operation::or_all",
                        execution_tree::generate_error_message(
                            "left hand side operand has unsupported number of "
                                "dimensions",
                            name_, codename_));
                }
            }

    struct or_operation::visit_or_operation
    {
        template <typename T1, typename T2>
        primitive_argument_type operator()(T1, T2) const
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "or_operation::eval",
                execution_tree::generate_error_message(
                    "left hand side and right hand side are incompatible "
                        "and can't be compared",
                    or_.name_, or_.codename_));
        }

        primitive_argument_type operator()(
            ir::node_data<primitive_argument_type>&&,
            ir::node_data<primitive_argument_type>&&) const
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "or_operation::eval",
                execution_tree::generate_error_message(
                    "left hand side and right hand side can't be "
                        "compared",
                    or_.name_, or_.codename_));
        }

        primitive_argument_type operator()(
            std::vector<ast::expression>&&,
            std::vector<ast::expression>&&) const
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "or_operation::eval",
                execution_tree::generate_error_message(
                    "left hand side and right hand side can't be "
                        "compared",
                    or_.name_, or_.codename_));
        }

        primitive_argument_type operator()(
            ast::expression&&, ast::expression&&) const
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "or_operation::eval",
                execution_tree::generate_error_message(
                    "left hand side and right hand side can't be "
                        "compared",
                    or_.name_, or_.codename_));
        }

        primitive_argument_type operator()(
            primitive&&, primitive&&) const
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "or_operation::eval",
                execution_tree::generate_error_message(
                    "left hand side and right hand side can't be "
                        "compared",
                    or_.name_, or_.codename_));
        }

        template <typename T>
        primitive_argument_type operator()(T&& lhs, T&& rhs) const
        {
            return primitive_argument_type(
                ir::node_data<std::uint8_t>{lhs && rhs});
        }

        primitive_argument_type operator()(
            ast::nil lhs, ast::nil rhs) const
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "or_operation::eval",
                execution_tree::generate_error_message(
                    "left hand side and right hand side can't be "
                        "compared",
                    or_.name_, or_.codename_));
        }

        primitive_argument_type operator()(
            phylanx::util::recursive_wrapper<
                std::vector<primitive_argument_type>>
                lhs,
            phylanx::util::recursive_wrapper<
                std::vector<primitive_argument_type>>
                rhs) const
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "or_operation::eval",
                execution_tree::generate_error_message(
                    "left hand side and right hand side can't be "
                        "compared",
                    or_.name_, or_.codename_));
        }

        primitive_argument_type operator()(
            std::string lhs, std::string rhs) const
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "or_operation::eval",
                execution_tree::generate_error_message(
                    "left hand side and right hand side can't be "
                        "compared",
                    or_.name_, or_.codename_));
        }

        primitive_argument_type operator()(ir::node_data<double>&& lhs,
            std::int64_t&& rhs) const
        {
            if (lhs.num_dimensions() != 0)
            {
                return or_.or_all(ir::node_data<bool>{std::move(lhs)},
                    operand_type{std::move(rhs)});
            }
            return primitive_argument_type(
                ir::node_data<bool>{(lhs[0] != 0) | (rhs != 0)});
        }

        primitive_argument_type operator()(std::int64_t&& lhs,
            ir::node_data<double>&& rhs) const
        {
            if (rhs.num_dimensions() != 0)
            {
                return or_.or_all(operand_type{std::move(lhs)},
                    ir::node_data<bool>{std::move(rhs)});
            }
            return primitive_argument_type(
                ir::node_data<bool>{(rhs[0] != 0) | (lhs != 0)});
        }

        primitive_argument_type operator()(ir::node_data<double>&& lhs,
            ir::node_data<double>&& rhs) const
        {
            return or_.or_all(ir::node_data<std::uint8_t>{std::move(lhs)},
                ir::node_data<std::uint8_t>{std::move(rhs)});
        }

        primitive_argument_type operator()(
            operand_type&& lhs, operand_type&& rhs) const
        {
            return or_.or_all(std::move(lhs), std::move(rhs));
        }

        or_operation const& or_;
    };

    hpx::future<primitive_argument_type> or_operation::eval(
        std::vector<primitive_argument_type> const& operands,
        std::vector<primitive_argument_type> const& args) const
    {
        //TODO: support for operands.size()>2
        if (operands.size() != 2)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "or_operation::eval",
                execution_tree::generate_error_message(
                    "the or_operation primitive requires exactly "
                        "two operands",
                    name_, codename_));
        }

        if (!valid(operands[0]) || !valid(operands[1]))
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "or_operation::eval",
                execution_tree::generate_error_message(
                    "the or_operation primitive requires that the "
                        "arguments given by the operands array "
                        "are valid",
                    name_, codename_));
        }

        auto this_ = this->shared_from_this();
        return hpx::dataflow(
            hpx::util::unwrapping(
                [this_](
                    operands_type&& ops) -> primitive_argument_type
                {
                    return primitive_argument_type(
                        util::visit(visit_or_operation{*this_},
                            std::move(ops[0].variant()),
                            std::move(ops[1].variant())));
                }),
            detail::map_operands(
                operands, functional::literal_operand{}, args,
                name_, codename_));
    }

    //////////////////////////////////////////////////////////////////////////
    // Implement '||' for all possible combinations of lhs and rhs
    hpx::future<primitive_argument_type> or_operation::eval(
        std::vector<primitive_argument_type> const& args) const
    {
        if (operands_.empty())
        {
            return eval(args, noargs);
        }
        return eval(operands_, args);
    }
}}}
