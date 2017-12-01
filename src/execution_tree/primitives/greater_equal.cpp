//  Copyright (c) 2017 Hartmut Kaiser
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>
#include <phylanx/execution_tree/primitives/greater_equal.hpp>
#include <phylanx/ir/node_data.hpp>

#include <hpx/include/components.hpp>
#include <hpx/include/lcos.hpp>
#include <hpx/include/util.hpp>

#include <cstddef>
#include <memory>
#include <numeric>
#include <utility>
#include <vector>

///////////////////////////////////////////////////////////////////////////////
typedef hpx::components::component<
    phylanx::execution_tree::primitives::greater_equal>
    greater_equal_type;
HPX_REGISTER_DERIVED_COMPONENT_FACTORY(
    greater_equal_type, phylanx_greater_equal_component,
    "phylanx_primitive_component", hpx::components::factory_enabled)
HPX_DEFINE_GET_COMPONENT_TYPE(greater_equal_type::wrapped_type)

///////////////////////////////////////////////////////////////////////////////
namespace phylanx { namespace execution_tree { namespace primitives
{
    ///////////////////////////////////////////////////////////////////////////
    std::vector<match_pattern_type> const greater_equal::match_data =
    {
        hpx::util::make_tuple("ge", "_1 >= _2", &create<greater_equal>)
    };

    ///////////////////////////////////////////////////////////////////////////
    greater_equal::greater_equal(std::vector<primitive_argument_type>&& operands)
      : base_primitive(std::move(operands))
    {}

    ///////////////////////////////////////////////////////////////////////////
    namespace detail
    {
        struct greater_equal : std::enable_shared_from_this<greater_equal>
        {
            greater_equal() = default;

        protected:
            using operand_type = ir::node_data<double>;
            using operands_type = std::vector<primitive_result_type>;

            bool greater_equal0d(operand_type&& lhs, operand_type&& rhs) const
            {
                std::size_t rhs_dims = rhs.num_dimensions();
                switch(rhs_dims)
                {
                case 0:
                    return lhs.scalar() >= rhs.scalar();

                case 1: HPX_FALLTHROUGH;
                case 2: HPX_FALLTHROUGH;
                default:
                    HPX_THROW_EXCEPTION(hpx::bad_parameter,
                        "greater_equal::greater_equal0d",
                        "the operands have incompatible number of dimensions");
                }
            }

            bool greater_equal1d1d(operand_type&& lhs, operand_type&& rhs) const
            {
                std::size_t lhs_size = lhs.dimension(0);
                std::size_t rhs_size = rhs.dimension(0);

                if (lhs_size != rhs_size)
                {
                    HPX_THROW_EXCEPTION(hpx::bad_parameter,
                        "greater_equal::greater_equal1d1d",
                        "the dimensions of the operands do not match");
                }

                // TODO: SIMD functionality should be added, blaze implementation
                //       is not currently available
                lhs = blaze::map(lhs.vector(), rhs.vector(),
                    [](double x1, double x2) { return x1 >= x2 ? 1.0 : 0.0; });

                return lhs.vector().nonZeros() > 0;
            }

            bool greater_equal1d(operand_type&& lhs, operand_type&& rhs) const
            {
                std::size_t rhs_dims = rhs.num_dimensions();
                switch(rhs_dims)
                {
                case 1:
                    return greater_equal1d1d(std::move(lhs), std::move(rhs));

                case 0: HPX_FALLTHROUGH;
                case 2: HPX_FALLTHROUGH;
                default:
                    HPX_THROW_EXCEPTION(hpx::bad_parameter,
                        "greater_equal::greater_equal1d",
                        "the operands have incompatible number of dimensions");
                }
            }

            bool greater_equal2d2d(operand_type&& lhs, operand_type&& rhs) const
            {
                auto lhs_size = lhs.dimensions();
                auto rhs_size = rhs.dimensions();

                if (lhs_size != rhs_size)
                {
                    HPX_THROW_EXCEPTION(hpx::bad_parameter,
                        "greater_equal::greater_equal2d2d",
                        "the dimensions of the operands do not match");
                }

                // TODO: SIMD functionality should be added, blaze implementation
                //       is not currently available
                lhs = blaze::map(lhs.matrix(), rhs.matrix(),
                    [](double x1, double x2) { return x1 >= x2 ? 1 : 0; });

                return lhs.matrix().nonZeros() > 0;
            }

            bool greater_equal2d(operand_type&& lhs, operand_type&& rhs) const
            {
                std::size_t rhs_dims = rhs.num_dimensions();
                switch(rhs_dims)
                {
                case 2:
                    return greater_equal2d2d(std::move(lhs), std::move(rhs));

                case 0: HPX_FALLTHROUGH;
                case 1: HPX_FALLTHROUGH;
                default:
                    HPX_THROW_EXCEPTION(hpx::bad_parameter,
                        "greater_equal::greater_equal2d",
                        "the operands have incompatible number of dimensions");
                }
            }

        public:
            bool greater_equal_all(operand_type&& lhs, operand_type&& rhs) const
            {
                std::size_t lhs_dims = lhs.num_dimensions();
                switch (lhs_dims)
                {
                case 0:
                    return greater_equal0d(std::move(lhs), std::move(rhs));

                case 1:
                    return greater_equal1d(std::move(lhs), std::move(rhs));

                case 2:
                    return greater_equal2d(std::move(lhs), std::move(rhs));

                default:
                    HPX_THROW_EXCEPTION(hpx::bad_parameter,
                        "greater_equal::greater_equal_all",
                        "left hand side operand has unsupported number of "
                            "dimensions");
                }
            }

        protected:
            struct visit_greater_equal
            {
                template <typename T1, typename T2>
                bool operator()(T1, T2) const
                {
                    HPX_THROW_EXCEPTION(hpx::bad_parameter,
                        "greater_equal::eval",
                        "left hand side and right hand side are incompatible "
                            "and can't be compared");
                }

                bool operator()(std::vector<ast::expression>&&,
                    std::vector<ast::expression>&&) const
                {
                    HPX_THROW_EXCEPTION(hpx::bad_parameter,
                        "greater_equal::eval",
                        "left hand side and right hand side are incompatible "
                            "and can't be compared");
                }

                bool operator()(ast::expression&&, ast::expression&&) const
                {
                    HPX_THROW_EXCEPTION(hpx::bad_parameter,
                        "greater_equal::eval",
                        "left hand side and right hand side are incompatible "
                            "and can't be compared");
                }

                bool operator()(primitive&&, primitive&&) const
                {
                    HPX_THROW_EXCEPTION(hpx::bad_parameter,
                        "greater_equal::eval",
                        "left hand side and right hand side are incompatible "
                            "and can't be compared");
                }

                template <typename T>
                bool operator()(T && lhs, T && rhs) const
                {
                    return lhs >= rhs;
                }

                bool operator()(
                    util::recursive_wrapper<
                        std::vector<primitive_result_type>>&&,
                    util::recursive_wrapper<
                        std::vector<primitive_result_type>>&&) const
                {
                    HPX_THROW_EXCEPTION(hpx::bad_parameter,
                        "less::eval",
                        "left hand side and right hand side are incompatible "
                            "and can't be compared");
                }

                bool operator()(
                    ir::node_data<double>&& lhs, std::int64_t&& rhs) const
                {
                    if (lhs.num_dimensions() != 0)
                    {
                        HPX_THROW_EXCEPTION(hpx::bad_parameter,
                            "greater_equal::eval",
                            "left hand side and right hand side are "
                                "incompatible and can't be compared");
                    }
                    return lhs[0] >= rhs;
                }

                bool operator()(
                    std::int64_t&& lhs, ir::node_data<double>&& rhs) const
                {
                    if (rhs.num_dimensions() != 0)
                    {
                        HPX_THROW_EXCEPTION(hpx::bad_parameter,
                            "greater_equal::eval",
                            "left hand side and right hand side are "
                                "incompatible and can't be compared");
                    }
                    return lhs >= rhs[0];
                }

                bool operator()(operand_type&& lhs, operand_type&& rhs) const
                {
                    return greater_equal_.greater_equal_all(
                        std::move(lhs), std::move(rhs));
                }

                greater_equal const& greater_equal_;
            };

        public:
            hpx::future<primitive_result_type> eval(
                std::vector<primitive_argument_type> const& operands,
                std::vector<primitive_argument_type> const& args) const
            {
                if (operands.size() != 2)
                {
                    HPX_THROW_EXCEPTION(hpx::bad_parameter,
                        "greater_equal::eval",
                        "the greater_equal primitive requires exactly two "
                            "operands");
                }

                if (!valid(operands[0]) || !valid(operands[1]))
                {
                    HPX_THROW_EXCEPTION(hpx::bad_parameter,
                        "greater_equal::eval",
                        "the greater_equal primitive requires that the "
                            "arguments given by the operands array are valid");
                }

                auto this_ = this->shared_from_this();
                return hpx::dataflow(hpx::util::unwrapping(
                    [this_](operands_type && ops)
                    {
                        return primitive_result_type(
                            util::visit(visit_greater_equal{*this_},
                                std::move(ops[0]), std::move(ops[1])));
                    }),
                    detail::map_operands(operands, literal_operand, args)
                );
            }
        };
    }

    // implement '>=' for all possible combinations of lhs and rhs
    hpx::future<primitive_result_type> greater_equal::eval(
        std::vector<primitive_argument_type> const& args) const
    {
        if (operands_.empty())
        {
            return std::make_shared<detail::greater_equal>()->eval(args, noargs);
        }

        return std::make_shared<detail::greater_equal>()->eval(operands_, args);
    }
}}}
