//  Copyright (c) 2017-2018 Hartmut Kaiser
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>
#include <phylanx/execution_tree/primitives/not_equal.hpp>
#include <phylanx/ir/node_data.hpp>

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
    primitive create_not_equal(hpx::id_type const& locality,
        std::vector<primitive_argument_type>&& operands, std::string const& name)
    {
        static std::string type("__ne");
        return create_primitive_component(
            locality, type, std::move(operands), name);
    }

    match_pattern_type const not_equal::match_data =
    {
        hpx::util::make_tuple("__ne",
            std::vector<std::string>{"_1 != _2"},
            &create_not_equal, &create_primitive<not_equal>)
    };

    ///////////////////////////////////////////////////////////////////////////
    not_equal::not_equal(std::vector<primitive_argument_type>&& operands)
      : primitive_component_base(std::move(operands))
    {}

    ///////////////////////////////////////////////////////////////////////////
    namespace detail
    {
        struct not_equal : std::enable_shared_from_this<not_equal>
        {
            not_equal() = default;

        protected:
            using operand_type = ir::node_data<double>;
            using operands_type = std::vector<primitive_argument_type>;

            primitive_argument_type not_equal0d1d(
                operand_type&& lhs, operand_type&& rhs) const
            {
                // TODO: SIMD functionality should be added, blaze implementation
                // is not currently available
                rhs.vector() = blaze::map(rhs.vector(),
                    [&](double x) { return (x != lhs.scalar()); });

                return primitive_argument_type(
                    ir::node_data<bool>{std::move(rhs)});
            }

            primitive_argument_type not_equal0d2d(
                operand_type&& lhs, operand_type&& rhs) const
            {
                // TODO: SIMD functionality should be added, blaze implementation
                // is not currently available
                rhs.matrix() = blaze::map(rhs.matrix(),
                    [&](double x) { return (x != lhs.scalar()); });

                return primitive_argument_type(
                    ir::node_data<bool>{std::move(rhs)});
            }

            primitive_argument_type not_equal0d(
                operand_type&& lhs, operand_type&& rhs) const
            {
                std::size_t rhs_dims = rhs.num_dimensions();
                switch(rhs_dims)
                {
                case 0:
                    return primitive_argument_type(
                        ir::node_data<bool>{lhs.scalar() != rhs.scalar()});

                case 1:
                    return not_equal0d1d(std::move(lhs), std::move(rhs));

                case 2:
                    return not_equal0d2d(std::move(lhs), std::move(rhs));

                default:
                    HPX_THROW_EXCEPTION(hpx::bad_parameter,
                        "not_equal::not_equal0d",
                        "the operands have incompatible number of dimensions");
                }
            }

            primitive_argument_type not_equal1d0d(
                operand_type&& lhs, operand_type&& rhs) const
            {
                // TODO: SIMD functionality should be added, blaze implementation
                // is not currently available
                lhs.vector() = blaze::map(lhs.vector(),
                    [&](double x) { return (x != rhs.scalar()); });

                return primitive_argument_type(
                    ir::node_data<bool>{std::move(lhs)});
            }

            primitive_argument_type not_equal1d1d(
                operand_type&& lhs, operand_type&& rhs) const
            {
                std::size_t lhs_size = lhs.dimension(0);
                std::size_t rhs_size = rhs.dimension(0);

                if (lhs_size != rhs_size)
                {
                    HPX_THROW_EXCEPTION(hpx::bad_parameter,
                        "not_equal::not_equal1d1d",
                        "the dimensions of the operands do not match");
                }

                // TODO: SIMD functionality should be added, blaze implementation
                // is not currently available
                lhs.vector() = blaze::map(lhs.vector(), rhs.vector(),
                    [&](double x, double y) { return (x != y); });

                return primitive_argument_type(
                    ir::node_data<bool>{std::move(lhs)});
            }

            primitive_argument_type not_equal1d2d(
                operand_type&& lhs, operand_type&& rhs) const
            {
                std::size_t lhs_size = lhs.dimension(0);
                auto rhs_size = rhs.dimensions();

                if (lhs_size != rhs_size[1])
                {
                    HPX_THROW_EXCEPTION(hpx::bad_parameter,
                        "not_equal::not_equal1d2d",
                        "the dimensions of the operands do not match");
                }

                // TODO: SIMD functionality should be added, blaze implementation
                // is not currently available
                for (size_t i = 0UL; i < rhs.matrix().rows(); i++)
                    blaze::row(rhs.matrix(), i) =
                        blaze::map(blaze::row(rhs.matrix(), i),
                            blaze::trans(lhs.vector()),
                            [](double x, double y) { return x != y; });

                return primitive_argument_type(
                    ir::node_data<bool>{std::move(rhs)});
            }

            primitive_argument_type not_equal1d(
                operand_type&& lhs, operand_type&& rhs) const
            {
                std::size_t rhs_dims = rhs.num_dimensions();
                switch(rhs_dims)
                {
                case 0:
                    return not_equal1d0d(std::move(lhs), std::move(rhs));

                case 1:
                    return not_equal1d1d(std::move(lhs), std::move(rhs));

                case 2:
                    return not_equal1d2d(std::move(lhs), std::move(rhs));

                default:
                    HPX_THROW_EXCEPTION(hpx::bad_parameter,
                        "not_equal::not_equal1d",
                        "the operands have incompatible number of dimensions");
                }
            }

            primitive_argument_type not_equal2d0d(
                operand_type&& lhs, operand_type&& rhs) const
            {
                std::size_t lhs_size = lhs.dimension(0);
                std::size_t rhs_size = rhs.dimension(0);

                // TODO: SIMD functionality should be added, blaze implementation
                // is not currently available
                lhs.matrix() = blaze::map(lhs.matrix(),
                    [&](double x) { return (x != rhs.scalar()); });

                return primitive_argument_type(
                    ir::node_data<bool>{std::move(lhs)});
            }

            primitive_argument_type not_equal2d1d(
                operand_type&& lhs, operand_type&& rhs) const
            {
                std::size_t rhs_size = rhs.dimension(0);
                auto lhs_size = lhs.dimensions();

                if (rhs_size != lhs_size[1])
                {
                    HPX_THROW_EXCEPTION(hpx::bad_parameter,
                        "not_equal::not_equal2d1d",
                        "the dimensions of the operands do not match");
                }

                // TODO: SIMD functionality should be added, blaze implementation
                // is not currently available
                for (size_t i = 0UL; i < lhs.matrix().rows(); i++)
                    blaze::row(lhs.matrix(), i) =
                        blaze::map(blaze::row(lhs.matrix(), i),
                            blaze::trans(rhs.vector()),
                            [](double x, double y) { return x != y; });

                return primitive_argument_type(
                    ir::node_data<bool>{std::move(lhs)});
            }

            primitive_argument_type not_equal2d2d(
                operand_type&& lhs, operand_type&& rhs) const
            {
                auto lhs_size = lhs.dimensions();
                auto rhs_size = rhs.dimensions();

                if (lhs_size != rhs_size)
                {
                    HPX_THROW_EXCEPTION(hpx::bad_parameter,
                        "not_equal::not_equal2d2d",
                        "the dimensions of the operands do not match");
                }

                // TODO: SIMD functionality should be added, blaze implementation
                // is not currently available
                lhs.matrix() = blaze::map(lhs.matrix(), rhs.matrix(),
                    [&](double x, double y) { return (x != y); });

                return primitive_argument_type(
                    ir::node_data<bool>{std::move(lhs)});
            }

            primitive_argument_type not_equal2d(
                operand_type&& lhs, operand_type&& rhs) const
            {
                std::size_t rhs_dims = rhs.num_dimensions();
                switch(rhs_dims)
                {
                case 0:
                    return not_equal2d0d(std::move(lhs), std::move(rhs));

                case 1:
                    return not_equal2d1d(std::move(lhs), std::move(rhs));

                case 2:
                    return not_equal2d2d(std::move(lhs), std::move(rhs));

                default:
                    HPX_THROW_EXCEPTION(hpx::bad_parameter,
                        "not_equal::not_equal2d",
                        "the operands have incompatible number of dimensions");
                }
            }

        public:
            primitive_argument_type not_equal_all(
                operand_type&& lhs, operand_type&& rhs) const
            {
                std::size_t lhs_dims = lhs.num_dimensions();
                switch (lhs_dims)
                {
                case 0:
                    return not_equal0d(std::move(lhs), std::move(rhs));

                case 1:
                    return not_equal1d(std::move(lhs), std::move(rhs));

                case 2:
                    return not_equal2d(std::move(lhs), std::move(rhs));

                default:
                    HPX_THROW_EXCEPTION(hpx::bad_parameter,
                        "not_equal::not_equal_all",
                        "left hand side operand has unsupported number of "
                            "dimensions");
                }
            }

        protected:
            struct visit_not_equal
            {
                template <typename T1, typename T2>
                primitive_argument_type operator()(T1, T2) const
                {
                    HPX_THROW_EXCEPTION(hpx::bad_parameter,
                        "not_equal::eval",
                        "left hand side and right hand side are incompatible "
                            "and can't be compared");
                }

                primitive_argument_type operator()(primitive&&, primitive&&) const
                {
                    HPX_THROW_EXCEPTION(hpx::bad_parameter,
                        "not_equal::eval",
                        "left hand side and right hand side are incompatible "
                            "and can't be compared");
                }

                template <typename T>
                primitive_argument_type operator()(T && lhs, T && rhs) const
                {
                    return ir::node_data<bool>{lhs != rhs};
                }

                primitive_argument_type operator()(
                    ir::node_data<double>&& lhs, std::int64_t&& rhs) const
                {
                    if (lhs.num_dimensions() != 0)
                    {
                        return not_equal_.not_equal_all(
                            std::move(lhs), operand_type(std::move(rhs)));
                    }
                    return ir::node_data<bool>{lhs[0] != rhs};
                }

                primitive_argument_type operator()(
                    std::int64_t&& lhs, ir::node_data<double>&& rhs) const
                {
                    if (rhs.num_dimensions() != 0)
                    {
                        return not_equal_.not_equal_all(
                            operand_type(std::move(lhs)), std::move(rhs));
                    }
                    return ir::node_data<bool>{lhs != rhs[0]};
                }

                primitive_argument_type operator()(
                    operand_type&& lhs, operand_type&& rhs) const
                {
                    return not_equal_.not_equal_all(
                        std::move(lhs), std::move(rhs));
                }

                primitive_argument_type operator()(
                    ir::node_data<bool>&& lhs, ir::node_data<bool>&& rhs) const
                {
                    return not_equal_.not_equal_all(
                        ir::node_data<double>(std::move(lhs)),
                        ir::node_data<double>(std::move(rhs)));
                }

                not_equal const& not_equal_;
            };

        public:
            hpx::future<primitive_argument_type> eval(
                std::vector<primitive_argument_type> const& operands,
                std::vector<primitive_argument_type> const& args) const
            {
                if (operands.size() != 2)
                {
                    HPX_THROW_EXCEPTION(hpx::bad_parameter,
                        "not_equal::eval",
                        "the not_equal primitive requires exactly two operands");
                }

                if (!valid(operands[0]) || !valid(operands[1]))
                {
                    HPX_THROW_EXCEPTION(hpx::bad_parameter,
                        "not_equal::eval",
                        "the not_equal primitive requires that the arguments "
                            "given by the operands array are valid");
                }

                auto this_ = this->shared_from_this();
                return hpx::dataflow(hpx::util::unwrapping(
                    [this_](operands_type && ops) -> primitive_argument_type
                    {
                        return primitive_argument_type(
                            util::visit(visit_not_equal{*this_},
                                std::move(ops[0].variant()),
                                std::move(ops[1].variant())));
                    }),
                    detail::map_operands(
                        operands, functional::literal_operand{}, args));
            }
        };
    }

    // implement '!=' for all possible combinations of lhs and rhs
    hpx::future<primitive_argument_type> not_equal::eval(
        std::vector<primitive_argument_type> const& args) const
    {
        if (operands_.empty())
        {
            return std::make_shared<detail::not_equal>()->eval(args, noargs);
        }

        return std::make_shared<detail::not_equal>()->eval(operands_, args);
    }
}}}
