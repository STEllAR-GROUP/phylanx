//  Copyright (c) 2018 Hartmut Kaiser
//  Copyright (c) 2018 Shahrzad Shirzad
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>
#include <phylanx/execution_tree/primitives/equal_elementwise.hpp>
#include <phylanx/ir/node_data.hpp>

#include <hpx/include/components.hpp>
#include <hpx/include/lcos.hpp>
#include <hpx/include/util.hpp>

#include <cstddef>
#include <cstdint>
#include <memory>
#include <numeric>
#include <string>
#include <utility>
#include <vector>

///////////////////////////////////////////////////////////////////////////////
typedef hpx::components::component<
    phylanx::execution_tree::primitives::equal_elementwise>
    equal_elementwise_type;
HPX_REGISTER_DERIVED_COMPONENT_FACTORY(equal_elementwise_type,
    phylanx_equal_elementwise_component, "phylanx_primitive_component",
    hpx::components::factory_enabled)
HPX_DEFINE_GET_COMPONENT_TYPE(equal_elementwise_type::wrapped_type)

///////////////////////////////////////////////////////////////////////////////
namespace phylanx { namespace execution_tree { namespace primitives
{
    ///////////////////////////////////////////////////////////////////////////
    match_pattern_type const equal_elementwise::match_data = {
        hpx::util::make_tuple("eeq", std::vector<std::string>{"eeq(_1,_2)"},
            &create<equal_elementwise>)};

    ///////////////////////////////////////////////////////////////////////////
    equal_elementwise::equal_elementwise(
        std::vector<primitive_argument_type> && operands)
      : base_primitive(std::move(operands))
    {
    }

    ///////////////////////////////////////////////////////////////////////////
    namespace detail {
        struct equal_elementwise
          : std::enable_shared_from_this<equal_elementwise>
        {
            equal_elementwise() = default;

        protected:
            using operand_type = ir::node_data<double>;
            using operands_type = std::vector<primitive_result_type>;

            primitive_result_type equal0d(
                operand_type&& lhs, operand_type&& rhs) const
            {
                std::size_t rhs_dims = rhs.num_dimensions();
                switch (rhs_dims)
                {
                case 0:
                    return equal0d0d(std::move(lhs), std::move(rhs));

                case 1:
                    return equal0d1d(std::move(lhs), std::move(rhs));

                case 2:
                    return equal0d2d(std::move(lhs), std::move(rhs));
                default:
                    HPX_THROW_EXCEPTION(hpx::bad_parameter,
                        "equal::equal0d",
                        "the operands have incompatible number of "
                        "dimensions");
                }
            }

            primitive_result_type equal0d0d(
                operand_type&& lhs, operand_type&& rhs) const
            {
                lhs.scalar() = rhs.scalar() == lhs.scalar();

                return primitive_result_type(ir::node_data<bool>{lhs});
            }

            primitive_result_type equal0d1d(
                operand_type&& lhs, operand_type&& rhs) const
            {
                // TODO: SIMD functionality should be added, blaze implementation
                // is not currently available
                rhs.vector() = blaze::map(rhs.vector(),
                    [&](double x) { return (x == lhs.scalar()); });

                return primitive_result_type(ir::node_data<bool>{rhs});
            }

            primitive_result_type equal0d2d(
                operand_type&& lhs, operand_type&& rhs) const
            {
                // TODO: SIMD functionality should be added, blaze implementation
                // is not currently available
                rhs.matrix() = blaze::map(rhs.matrix(),
                    [&](double x) { return (x == lhs.scalar()); });

                return primitive_result_type(ir::node_data<bool>{rhs});
            }

            primitive_result_type equal1d0d(
                operand_type&& lhs, operand_type&& rhs) const
            {
                // TODO: SIMD functionality should be added, blaze implementation
                // is not currently available
                lhs.vector() = blaze::map(lhs.vector(),
                    [&](double x) { return (x == rhs.scalar()); });

                return primitive_result_type(ir::node_data<bool>{lhs});
            }

            primitive_result_type equal1d1d(
                operand_type&& lhs, operand_type&& rhs) const
            {
                std::size_t lhs_size = lhs.dimension(0);
                std::size_t rhs_size = rhs.dimension(0);

                if (lhs_size != rhs_size)
                {
                    HPX_THROW_EXCEPTION(hpx::bad_parameter,
                        "equal::equal1d1d",
                        "the dimensions of the operands do not match");
                }

                // TODO: SIMD functionality should be added, blaze implementation
                // is not currently available
                lhs.vector() = blaze::map(lhs.vector(), rhs.vector(),
                    [&](double x, double y) { return (x == y); });

                return primitive_result_type(ir::node_data<bool>{lhs});
            }

            primitive_result_type equal1d2d(
                operand_type&& lhs, operand_type&& rhs) const
            {
                std::size_t lhs_size = lhs.dimension(0);
                auto rhs_size = rhs.dimensions();

                if (lhs_size != rhs_size[1])
                {
                    HPX_THROW_EXCEPTION(hpx::bad_parameter,
                        "equal::equal1d2d",
                        "the dimensions of the operands do not match");
                }

                // TODO: SIMD functionality should be added, blaze implementation
                // is not currently available
                for (size_t i = 0UL; i < rhs.matrix().rows(); i++)
                    blaze::row(rhs.matrix(), i) =
                        blaze::map(blaze::row(rhs.matrix(), i),
                            blaze::trans(lhs.vector()),
                            [](double x, double y) { return x == y; });

                return primitive_result_type(ir::node_data<bool>{rhs});
            }

            primitive_result_type equal1d(
                operand_type&& lhs, operand_type&& rhs) const
            {
                std::size_t rhs_dims = rhs.num_dimensions();
                switch (rhs_dims)
                {
                case 1:
                    return equal1d1d(std::move(lhs), std::move(rhs));

                case 0:
                    return equal1d0d(std::move(lhs), std::move(rhs));

                case 2:
                    return equal1d2d(std::move(lhs), std::move(rhs));

                default:
                    HPX_THROW_EXCEPTION(hpx::bad_parameter,
                        "equal::equal1d",
                        "the operands have incompatible number of "
                        "dimensions");
                }
            }

            primitive_result_type equal2d2d(
                operand_type&& lhs, operand_type&& rhs) const
            {
                auto lhs_size = lhs.dimensions();
                auto rhs_size = rhs.dimensions();

                if (lhs_size != rhs_size)
                {
                    HPX_THROW_EXCEPTION(hpx::bad_parameter,
                        "equal::equal2d2d",
                        "the dimensions of the operands do not match");
                }

                // TODO: SIMD functionality should be added, blaze implementation
                // is not currently available
                lhs.matrix() = blaze::map(lhs.matrix(), rhs.matrix(),
                    [&](double x, double y) { return (x == y); });

                return primitive_result_type(ir::node_data<bool>{lhs});
            }

            primitive_result_type equal2d0d(
                operand_type&& lhs, operand_type&& rhs) const
            {
                std::size_t lhs_size = lhs.dimension(0);
                std::size_t rhs_size = rhs.dimension(0);

                // TODO: SIMD functionality should be added, blaze implementation
                // is not currently available
                lhs.matrix() = blaze::map(lhs.matrix(),
                    [&](double x) { return (x == rhs.scalar()); });

                return primitive_result_type(ir::node_data<bool>{lhs});
            }

            primitive_result_type equal2d(
                operand_type&& lhs, operand_type&& rhs) const
            {
                std::size_t rhs_dims = rhs.num_dimensions();
                switch (rhs_dims)
                {
                case 2:
                    return equal2d2d(std::move(lhs), std::move(rhs));

                case 0:
                    return equal2d0d(std::move(lhs), std::move(rhs));

                case 1:
                    return equal2d1d(std::move(lhs), std::move(rhs));

                default:
                    HPX_THROW_EXCEPTION(hpx::bad_parameter,
                        "equal::equal2d",
                        "the operands have incompatible number of "
                        "dimensions");
                }
            }

            primitive_result_type equal2d1d(
                operand_type&& lhs, operand_type&& rhs) const
            {
                std::size_t rhs_size = rhs.dimension(0);
                auto lhs_size = lhs.dimensions();

                if (rhs_size != lhs_size[1])
                {
                    HPX_THROW_EXCEPTION(hpx::bad_parameter,
                        "equal::equal2d1d",
                        "the dimensions of the operands do not match");
                }

                // TODO: SIMD functionality should be added, blaze implementation
                // is not currently available
                for (size_t i = 0UL; i < lhs.matrix().rows(); i++)
                    blaze::row(lhs.matrix(), i) =
                        blaze::map(blaze::row(lhs.matrix(), i),
                            blaze::trans(rhs.vector()),
                            [](double x, double y) { return x == y; });

                return primitive_result_type(ir::node_data<bool>{lhs});
            }

        public:
            primitive_result_type equal_all(
                operand_type&& lhs, operand_type&& rhs) const
            {
                std::size_t lhs_dims = lhs.num_dimensions();
                switch (lhs_dims)
                {
                case 0:
                    return equal0d(std::move(lhs), std::move(rhs));

                case 1:
                    return equal1d(std::move(lhs), std::move(rhs));

                case 2:
                    return equal2d(std::move(lhs), std::move(rhs));

                default:
                    HPX_THROW_EXCEPTION(hpx::bad_parameter,
                        "equal::equal_all",
                        "left hand side operand has unsupported number of "
                        "dimensions");
                }
            }

        protected:
            struct visit_equal_elementwise
            {
                template <typename T1, typename T2>
                primitive_result_type operator()(T1, T2) const
                {
                    HPX_THROW_EXCEPTION(hpx::bad_parameter,
                        "equal_elementwise::eval",
                        "left hand side and right hand side are "
                        "incompatible "
                        "and can't be compared");
                }

                template <typename T>
                primitive_result_type operator()(T&& lhs, T&& rhs) const
                {
                    return primitive_result_type(
                        ir::node_data<bool>{lhs == rhs});
                }

                primitive_result_type operator()(
                    ir::node_data<double>&& lhs, std::int64_t rhs) const
                {
                    if (lhs.num_dimensions() != 0)
                    {
                        return equal_elementwise_.equal_all(
                            std::move(lhs), operand_type(std::move(rhs)));
                    }
                    return primitive_result_type(
                        ir::node_data<bool>{lhs[0] == rhs});
                }

                primitive_result_type operator()(
                    std::int64_t&& lhs, ir::node_data<double> rhs) const
                {
                    if (rhs.num_dimensions() != 0)
                    {
                        return equal_elementwise_.equal_all(
                            operand_type(std::move(lhs)), std::move(rhs));
                    }
                    return primitive_result_type(
                        ir::node_data<bool>{lhs == rhs[0]});
                }

                primitive_result_type operator()(
                    operand_type&& lhs, operand_type&& rhs) const
                {
                    return equal_elementwise_.equal_all(
                        std::move(lhs), std::move(rhs));
                }

                equal_elementwise const& equal_elementwise_;
            };

        public:
            hpx::future<primitive_result_type> eval(
                std::vector<primitive_argument_type> const& operands,
                std::vector<primitive_argument_type> const& args) const
            {
                if (operands.size() != 2)
                {
                    HPX_THROW_EXCEPTION(hpx::bad_parameter,
                        "equal_elementwise::eval",
                        "the equal primitive requires exactly two "
                        "operands");
                }

                if (!valid(operands[0]) || !valid(operands[1]))
                {
                    HPX_THROW_EXCEPTION(hpx::bad_parameter,
                        "equal_elementwise::eval",
                        "the equal primitive requires that the arguments "
                        "given "
                        "by the operands array are valid");
                }

                auto this_ = this->shared_from_this();
                return hpx::dataflow(
                    hpx::util::unwrapping([this_](operands_type&& ops) {
                        return primitive_result_type(
                            util::visit(visit_equal_elementwise{*this_},
                                std::move(ops[0].variant()),
                                std::move(ops[1].variant())));
                    }),
                    detail::map_operands(
                        operands, functional::literal_operand{}, args));
            }
        };
    }

    // implement '==' for all possible combinations of lhs and rhs
    hpx::future<primitive_result_type> equal_elementwise::eval(
        std::vector<primitive_argument_type> const& args) const
    {
        if (operands_.empty())
        {
            return std::make_shared<detail::equal_elementwise>()->eval(
                args, noargs);
        }

        return std::make_shared<detail::equal_elementwise>()->eval(
            operands_, args);
    }
}}}
