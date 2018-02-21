//  Copyright (c) 2017-2018 Hartmut Kaiser
//  Copyright (c) 2018 Shahrzad Shirzad
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>
#include <phylanx/execution_tree/primitives/and_operation.hpp>

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
    primitive create_and_operation(hpx::id_type const& locality,
        std::vector<primitive_argument_type>&& operands,
        std::string const& name, std::string const& codename)
    {
        static std::string type("__and");
        return create_primitive_component(
            locality, type, std::move(operands), name, codename);
    }

    match_pattern_type const and_operation::match_data =
    {
        hpx::util::make_tuple("__and",
            std::vector<std::string>{"_1 && __2"},
            &create_and_operation, &create_primitive<and_operation>)
    };

    ///////////////////////////////////////////////////////////////////////////
    and_operation::and_operation(std::vector<primitive_argument_type>&& operands,
            std::string const& name, std::string const& codename)
      : primitive_component_base(std::move(operands), name, codename)
    {}

    ///////////////////////////////////////////////////////////////////////////
    namespace detail
    {
        struct and_operation : std::enable_shared_from_this<and_operation>
        {
            and_operation(std::string const& name, std::string const& codename)
              : name_(name)
              , codename_(codename)
            {
            }

        protected:
            std::string name_;
            std::string codename_;

        protected:
            using operand_type = ir::node_data<bool>;
            using operands_type = std::vector<primitive_argument_type>;

            primitive_argument_type and0d1d(
                operand_type&& lhs, operand_type&& rhs) const
            {
                // TODO: SIMD functionality should be added, blaze implementation
                // is not currently available
                rhs.vector() = blaze::map(
                    rhs.vector(), [&](bool x) { return (x && lhs.scalar()); });

                return primitive_argument_type(
                    ir::node_data<bool>{std::move(rhs)});
            }

            primitive_argument_type and0d2d(
                operand_type&& lhs, operand_type&& rhs) const
            {
                // TODO: SIMD functionality should be added, blaze implementation
                // is not currently available
                rhs.matrix() = blaze::map(
                    rhs.matrix(), [&](bool x) { return (x && lhs.scalar()); });

                return primitive_argument_type(
                    ir::node_data<bool>{std::move(rhs)});
            }

            primitive_argument_type and0d(
                operand_type&& lhs, operand_type&& rhs) const
            {
                std::size_t rhs_dims = rhs.num_dimensions();
                switch (rhs_dims)
                {
                case 0:
                    return primitive_argument_type(
                        ir::node_data<bool>{lhs.scalar() && rhs.scalar()});

                case 1:
                    return and0d1d(std::move(lhs), std::move(rhs));

                case 2:
                    return and0d2d(std::move(lhs), std::move(rhs));

                default:
                    HPX_THROW_EXCEPTION(hpx::bad_parameter,
                        "and::and0d",
                        generate_error_message(
                            "the operands have incompatible number of dimensions",
                            name_, codename_));
                }
            }

            primitive_argument_type and1d0d(
                operand_type&& lhs, operand_type&& rhs) const
            {
                // TODO: SIMD functionality should be added, blaze implementation
                // is not currently available
                lhs.vector() = blaze::map(
                    lhs.vector(), [&](bool x) { return (x && rhs.scalar()); });

                return primitive_argument_type(
                    ir::node_data<bool>{std::move(lhs)});
            }

            primitive_argument_type and1d1d(
                operand_type&& lhs, operand_type&& rhs) const
            {
                std::size_t lhs_size = lhs.dimension(0);
                std::size_t rhs_size = rhs.dimension(0);

                if (lhs_size != rhs_size)
                {
                    HPX_THROW_EXCEPTION(hpx::bad_parameter,
                        "and::and1d1d",
                        generate_error_message(
                            "the dimensions of the operands do not match",
                            name_, codename_));
                }

                // TODO: SIMD functionality should be added, blaze implementation
                // is not currently available
                lhs.vector() = blaze::map(lhs.vector(), rhs.vector(),
                    [&](bool x, bool y) { return (x && y); });

                return primitive_argument_type(
                    ir::node_data<bool>{std::move(lhs)});
            }

            primitive_argument_type and1d2d(
                operand_type&& lhs, operand_type&& rhs) const
            {
                std::size_t lhs_size = lhs.dimension(0);
                auto rhs_size = rhs.dimensions();

                if (lhs_size != rhs_size[1])
                {
                    HPX_THROW_EXCEPTION(hpx::bad_parameter,
                        "and::and1d2d",
                        generate_error_message(
                            "the dimensions of the operands do not match",
                            name_, codename_));
                }

                // TODO: SIMD functionality should be added, blaze implementation
                // is not currently available
                for (size_t i = 0UL; i < rhs.matrix().rows(); i++)
                    blaze::row(rhs.matrix(), i) =
                        blaze::map(blaze::row(rhs.matrix(), i),
                            blaze::trans(lhs.vector()),
                            [](bool x, bool y) { return x && y; });

                return primitive_argument_type(
                    ir::node_data<bool>{std::move(rhs)});
            }

            primitive_argument_type and1d(
                operand_type&& lhs, operand_type&& rhs) const
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
                        generate_error_message(
                            "the operands have incompatible number of "
                                "dimensions",
                            name_, codename_));
                }
            }

            primitive_argument_type and2d0d(
                operand_type&& lhs, operand_type&& rhs) const
            {
                std::size_t lhs_size = lhs.dimension(0);
                std::size_t rhs_size = rhs.dimension(0);

                // TODO: SIMD functionality should be added, blaze implementation
                // is not currently available
                lhs.matrix() = blaze::map(lhs.matrix(),
                    [&](double x) { return (x && rhs.scalar()); });

                return primitive_argument_type(
                    ir::node_data<bool>{std::move(lhs)});
            }

            primitive_argument_type and2d1d(
                operand_type&& lhs, operand_type&& rhs) const
            {
                std::size_t rhs_size = rhs.dimension(0);
                auto lhs_size = lhs.dimensions();

                if (rhs_size != lhs_size[1])
                {
                    HPX_THROW_EXCEPTION(hpx::bad_parameter,
                        "and::and2d1d",
                        generate_error_message(
                            "the dimensions of the operands do not match",
                            name_, codename_));
                }

                // TODO: SIMD functionality should be added, blaze implementation
                // is not currently available
                for (size_t i = 0UL; i < lhs.matrix().rows(); i++)
                    blaze::row(lhs.matrix(), i) =
                        blaze::map(blaze::row(lhs.matrix(), i),
                            blaze::trans(rhs.vector()),
                            [](bool x, bool y) { return x && y; });

                return primitive_argument_type(
                    ir::node_data<bool>{std::move(lhs)});
            }

            primitive_argument_type and2d2d(
                operand_type&& lhs, operand_type&& rhs) const
            {
                auto lhs_size = lhs.dimensions();
                auto rhs_size = rhs.dimensions();

                if (lhs_size != rhs_size)
                {
                    HPX_THROW_EXCEPTION(hpx::bad_parameter,
                        "and::and2d2d",
                        generate_error_message(
                            "the dimensions of the operands do not match",
                            name_, codename_));
                }

                // TODO: SIMD functionality should be added, blaze implementation
                // is not currently available
                lhs.matrix() = blaze::map(lhs.matrix(), rhs.matrix(),
                    [&](bool x, bool y) { return (x && y); });

                return primitive_argument_type(
                    ir::node_data<bool>{std::move(lhs)});
            }

            primitive_argument_type and2d(
                operand_type&& lhs, operand_type&& rhs) const
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
                        generate_error_message(
                            "the operands have incompatible number of "
                                "dimensions",
                            name_, codename_));
                }
            }

        public:
            primitive_argument_type and_all(
                operand_type&& lhs, operand_type&& rhs) const
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
                        generate_error_message(
                            "left hand side operand has unsupported number of "
                                "dimensions",
                            name_, codename_));
                }
            }

        protected:
            struct visit_and
            {
                template <typename T1, typename T2>
                primitive_argument_type operator()(T1, T2) const
                {
                    HPX_THROW_EXCEPTION(hpx::bad_parameter,
                        "and::eval",
                        generate_error_message(
                            "left hand side and right hand side are "
                                "incompatible and can't be compared",
                            and_.name_, and_.codename_));
                }

                primitive_argument_type operator()(
                    ir::node_data<primitive_argument_type>&&,
                    ir::node_data<primitive_argument_type>&&) const
                {
                    HPX_THROW_EXCEPTION(hpx::bad_parameter,
                        "and::eval",
                        generate_error_message(
                            "left hand side and right hand side can't be "
                                "compared",
                            and_.name_, and_.codename_));
                }

                primitive_argument_type operator()(
                    std::vector<ast::expression>&&,
                    std::vector<ast::expression>&&) const
                {
                    HPX_THROW_EXCEPTION(hpx::bad_parameter,
                        "and::eval",
                        generate_error_message(
                            "left hand side and right hand side can't be "
                                "compared",
                            and_.name_, and_.codename_));
                }

                primitive_argument_type operator()(
                    ast::expression&&, ast::expression&&) const
                {
                    HPX_THROW_EXCEPTION(hpx::bad_parameter,
                        "and::eval",
                        generate_error_message(
                            "left hand side and right hand side can't be "
                                "compared",
                            and_.name_, and_.codename_));
                }

                primitive_argument_type operator()(
                    primitive&&, primitive&&) const
                {
                    HPX_THROW_EXCEPTION(hpx::bad_parameter,
                        "and::eval",
                        generate_error_message(
                            "left hand side and right hand side can't be "
                                "compared",
                            and_.name_, and_.codename_));
                }

                template <typename T>
                primitive_argument_type operator()(T&& lhs, T&& rhs) const
                {
                    return primitive_argument_type(
                        ir::node_data<bool>{lhs && rhs});
                }

                primitive_argument_type operator()(
                    ast::nil lhs, ast::nil rhs) const
                {
                    HPX_THROW_EXCEPTION(hpx::bad_parameter,
                        "and::eval",
                        generate_error_message(
                            "left hand side and right hand side can't be "
                                "compared",
                            and_.name_, and_.codename_));
                }

                primitive_argument_type operator()(
                    phylanx::util::recursive_wrapper<
                        std::vector<primitive_argument_type>> lhs,
                    phylanx::util::recursive_wrapper<
                        std::vector<primitive_argument_type>> rhs) const
                {
                    HPX_THROW_EXCEPTION(hpx::bad_parameter,
                        "and::eval",
                        generate_error_message(
                            "left hand side and right hand side can't be "
                                "compared",
                            and_.name_, and_.codename_));
                }

                primitive_argument_type operator()(
                    std::string lhs, std::string rhs) const
                {
                    HPX_THROW_EXCEPTION(hpx::bad_parameter,
                        "and::eval",
                        generate_error_message(
                            "left hand side and right hand side can't be "
                                "compared",
                            and_.name_, and_.codename_));
                }

                primitive_argument_type operator()(ir::node_data<double>&& lhs,
                    ir::node_data<double>&& rhs) const
                {
                    return and_.and_all(ir::node_data<bool>{std::move(lhs)},
                        ir::node_data<bool>{std::move(rhs)});
                }

                primitive_argument_type operator()(
                    operand_type&& lhs, operand_type&& rhs) const
                {
                    return and_.and_all(std::move(lhs), std::move(rhs));
                }

                and_operation const& and_;
            };

        public:
            hpx::future<primitive_argument_type> eval(
                std::vector<primitive_argument_type> const& operands,
                std::vector<primitive_argument_type> const& args) const
            {
                //TODO: support for operands.size()>2
                if (operands.size() != 2)
                {
                    HPX_THROW_EXCEPTION(hpx::bad_parameter,
                        "and::eval",
                        generate_error_message(
                            "the and primitive requires exactly two "
                                "operands",
                            name_, codename_));
                }

                if (!valid(operands[0]) || !valid(operands[1]))
                {
                    HPX_THROW_EXCEPTION(hpx::bad_parameter,
                        "and::eval",
                        generate_error_message(
                            "the and primitive requires that the "
                                "arguments given by the operands array "
                                "are valid",
                            name_, codename_));
                }

                auto this_ = this->shared_from_this();
                return hpx::dataflow(
                    hpx::util::unwrapping(
                        [this_](
                            operands_type&& ops) -> primitive_argument_type {
                            return primitive_argument_type(
                                util::visit(visit_and{*this_},
                                    std::move(ops[0].variant()),
                                    std::move(ops[1].variant())));
                        }),
                    detail::map_operands(
                        operands, functional::literal_operand{}, args,
                        name_, codename_));
            }
        };
    }

    // implement '&&' for all possible combinations of lhs and rhs
    hpx::future<primitive_argument_type> and_operation::eval(
        std::vector<primitive_argument_type> const& args) const
    {
        if (operands_.empty())
        {
            return std::make_shared<detail::and_operation>(name_, codename_)
                ->eval(args, noargs);
        }
        return std::make_shared<detail::and_operation>(name_, codename_)
            ->eval(operands_, args);
    }
}}}
