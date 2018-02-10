//  Copyright (c) 2017-2018 Hartmut Kaiser
//  Copyright (c) 2017 Alireza Kheirkhahan
//  Copyright (c) 2017 Parsa Amini
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>
#include <phylanx/execution_tree/primitives/mul_operation.hpp>
#include <phylanx/ir/node_data.hpp>

#include <hpx/include/components.hpp>
#include <hpx/include/lcos.hpp>
#include <hpx/include/util.hpp>

#include <cstddef>
#include <memory>
#include <numeric>
#include <string>
#include <utility>
#include <vector>

#include <blaze/Math.h>

///////////////////////////////////////////////////////////////////////////////
typedef hpx::components::component<
    phylanx::execution_tree::primitives::mul_operation>
    mul_operation_type;
HPX_REGISTER_DERIVED_COMPONENT_FACTORY(mul_operation_type,
    phylanx_mul_operation_component, "phylanx_primitive_component",
    hpx::components::factory_enabled)
HPX_DEFINE_GET_COMPONENT_TYPE(mul_operation_type::wrapped_type)

///////////////////////////////////////////////////////////////////////////////
namespace phylanx { namespace execution_tree { namespace primitives
{
    ///////////////////////////////////////////////////////////////////////////
    match_pattern_type const mul_operation::match_data =
    {
        hpx::util::make_tuple("__mul",
            std::vector<std::string>{"_1 * __2"},
            &create<mul_operation>)
    };

    ///////////////////////////////////////////////////////////////////////////
    mul_operation::mul_operation(std::vector<primitive_argument_type>&& operands)
      : base_primitive(std::move(operands))
    {}

    ///////////////////////////////////////////////////////////////////////////
    namespace detail
    {
        struct mul : std::enable_shared_from_this<mul>
        {
            mul() = default;

        private:
            using operand_type = ir::node_data<double>;
            using operands_type = std::vector<operand_type>;

        private:
            primitive_argument_type mul0d(operands_type && ops) const
            {
                switch (ops[1].num_dimensions())
                {
                case 0:
                    return mul0d0d(std::move(ops));

                case 1:
                    return mul0d1d(std::move(ops));

                case 2:
                    return mul0d2d(std::move(ops));

                default:
                    HPX_THROW_EXCEPTION(hpx::bad_parameter,
                        "mul_operation::mul0d",
                        "the operands have incompatible number of dimensions");
                }
            }

            primitive_argument_type mul0d0d(operands_type && ops) const
            {
                operand_type& lhs = ops[0];
                operand_type& rhs = ops[1];

                if (ops.size() == 2)
                {
                    lhs.scalar() *= rhs.scalar();
                    return primitive_argument_type{ std::move(lhs) };
                }

                return primitive_argument_type{
                    std::accumulate(
                        ops.begin() + 1, ops.end(), std::move(lhs),
                        [](operand_type& result, operand_type const& curr) {

                            if (curr.num_dimensions() != 0)
                            {
                                HPX_THROW_EXCEPTION(hpx::bad_parameter,
                                    "mul_operation::mul0d0d",
                                    "all operands must be scalars");
                            }
                            result.scalar() *= curr.scalar();
                            return std::move(result);
                        })
                    };
            }

            primitive_argument_type mul0d1d(operands_type && ops) const
            {
                if (ops.size() > 2)
                {
                    HPX_THROW_EXCEPTION(hpx::bad_parameter,
                        "mul_operation::mul0d1d",
                        "can't handle more than 2 operands");
                }

                operand_type& lhs = ops[0];
                operand_type& rhs = ops[1];

                rhs = rhs.vector() * lhs.scalar();
                return primitive_argument_type{ std::move(rhs) };
            }

            primitive_argument_type mul0d2d(operands_type && ops) const
            {

                if (ops.size() > 2)
                {
                    HPX_THROW_EXCEPTION(hpx::bad_parameter,
                        "mul_operation::mul0d2d",
                        "can't handle more than 2 operands");
                }

                operand_type& lhs = ops[0];
                operand_type& rhs = ops[1];

                rhs = rhs.matrix() * lhs.scalar();
                return primitive_argument_type{ std::move(rhs) };
            }

            ///////////////////////////////////////////////////////////////////
            primitive_argument_type mul1d(operands_type && ops) const
            {
                switch (ops[1].num_dimensions())
                {
                case 0:
                    return mul1d0d(std::move(ops));

                case 1:
                    return mul1d1d(std::move(ops));

                case 2:
                    return mul1d2d(std::move(ops));

                default:
                    HPX_THROW_EXCEPTION(hpx::bad_parameter,
                        "mul_operation::mul1d",
                        "the operands have incompatible number of dimensions");
                }
            }

            primitive_argument_type mul1d0d(operands_type && ops) const
            {
                if (ops.size() > 2)
                {
                    HPX_THROW_EXCEPTION(hpx::bad_parameter,
                        "mul_operation::mul1d0d",
                        "can't handle more than 2 operands");
                }

                operand_type& lhs = ops[0];
                operand_type& rhs = ops[1];

                lhs = lhs.vector() * rhs.scalar();
                return primitive_argument_type{ std::move(lhs) };
            }

            primitive_argument_type mul1d1d(operands_type && ops) const
            {
                if (ops.size() == 2)
                {
                    ops[0] = ops[0].vector() * ops[1].vector();
                    return primitive_argument_type{ std::move(ops[0]) };
                }

                return primitive_argument_type{
                    std::accumulate(
                        ops.begin() + 1, ops.end(), std::move(ops[0]),
                        [](operand_type& result, operand_type const& curr)
                        ->  operand_type
                        {
                            if (curr.num_dimensions() != 1)
                            {
                                HPX_THROW_EXCEPTION(hpx::bad_parameter,
                                    "mul_operation::mul1d1d",
                                    "all operands must be vectors");
                            }
                            result = result.vector() * curr.vector();
                            return std::move(result);
                        })
                    };
            }

            primitive_argument_type mul1d2d(operands_type && ops) const
            {
                if (ops.size() > 2)
                {
                    HPX_THROW_EXCEPTION(hpx::bad_parameter,
                        "mul_operation::mul1d2d",
                        "can't handle more than 2 operands");
                }

                operand_type& lhs = ops[0];
                operand_type& rhs = ops[1];

                rhs = blaze::trans(
                    blaze::trans(lhs.vector()) * rhs.matrix());
                return primitive_argument_type{ std::move(rhs) };
            }

            primitive_argument_type mul2d(operands_type && ops) const
            {
                switch (ops[1].num_dimensions())
                {
                case 0:
                    return mul2d0d(std::move(ops));

                case 1:
                    return mul2d1d(std::move(ops));

                case 2:
                    return mul2d2d(std::move(ops));

                default:
                    HPX_THROW_EXCEPTION(hpx::bad_parameter,
                        "mul_operation::mul2d",
                        "the operands have incompatible number of dimensions");
                }
            }

            primitive_argument_type mul2d0d(operands_type && ops) const
            {
                if (ops.size() > 2)
                {
                    HPX_THROW_EXCEPTION(hpx::bad_parameter,
                        "mul_operation::mul2d0d",
                        "can't handle more than 2 operands");
                }

                operand_type& lhs = ops[0];
                operand_type& rhs = ops[1];

                lhs = lhs.matrix() * rhs.scalar();
                return primitive_argument_type{ std::move(lhs) };
            }

            primitive_argument_type mul2d1d(operands_type && ops) const
            {
                if (ops.size() > 2)
                {
                    HPX_THROW_EXCEPTION(hpx::bad_parameter,
                        "mul_operation::mul2d1d",
                        "can't handle more than 2 operands");
                }

                operand_type& lhs = ops[0];
                operand_type& rhs = ops[1];

                rhs = lhs.matrix() * rhs.vector();
                return primitive_argument_type{ std::move(rhs) };
            }

            primitive_argument_type mul2d2d(operands_type && ops) const
            {
                if (ops.size() == 2)
                {
                    ops[0] = ops[0].matrix() * ops[1].matrix();
                    return primitive_argument_type{ std::move(ops[0]) };
                }

                return primitive_argument_type{
                    std::accumulate(
                        ops.begin() + 1, ops.end(), std::move(ops[0]),
                        [](operand_type& result, operand_type const& curr)
                        ->  operand_type
                        {
                            if (curr.num_dimensions() != 2)
                            {
                                HPX_THROW_EXCEPTION(hpx::bad_parameter,
                                    "mul_operation::mul2d2d",
                                    "all operands must be matrices");
                            }

                            result = result.matrix() * curr.matrix();
                            return std::move(result);
                        })
                    };
            }

        public:
            hpx::future<primitive_argument_type> eval(
                std::vector<primitive_argument_type> const& operands,
                std::vector<primitive_argument_type> const& args) const
            {
                if (operands.size() < 2)
                {
                    HPX_THROW_EXCEPTION(hpx::bad_parameter,
                        "mul_operation::mul_operation",
                        "the mul_operation primitive requires at least two "
                        "operands");
                }

                bool arguments_valid = true;
                for (std::size_t i = 0; i != operands.size(); ++i)
                {
                    if (!valid(operands[i]))
                    {
                        arguments_valid = false;
                    }
                }

                if (!arguments_valid)
                {
                    HPX_THROW_EXCEPTION(hpx::bad_parameter,
                        "mul_operation::mul_operation",
                        "the mul_operation primitive requires that the "
                            "arguments given by the operands array are valid");
                }

                auto this_ = this->shared_from_this();
                return hpx::dataflow(hpx::util::unwrapping(
                    [this_](operands_type&& ops) -> primitive_argument_type
                    {
                        std::size_t lhs_dims = ops[0].num_dimensions();
                        switch (lhs_dims)
                        {
                        case 0:
                            return this_->mul0d(std::move(ops));

                        case 1:
                            return this_->mul1d(std::move(ops));

                        case 2:
                            return this_->mul2d(std::move(ops));

                        default:
                            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                                "mul_operation::eval",
                                "left hand side operand has unsupported "
                                    "number of dimensions");
                        }
                    }),
                    detail::map_operands(
                        operands, functional::numeric_operand{}, args));
            }
        };
    }

    // implement '*' for all possible combinations of lhs and rhs
    hpx::future<primitive_argument_type> mul_operation::eval(
        std::vector<primitive_argument_type> const& args) const
    {
        if (operands_.empty())
        {
            return std::make_shared<detail::mul>()->eval(args, noargs);
        }

        return std::make_shared<detail::mul>()->eval(operands_, args);
    }
}}}
