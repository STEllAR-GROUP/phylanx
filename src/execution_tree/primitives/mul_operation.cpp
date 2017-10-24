//  Copyright (c) 2017 Hartmut Kaiser
//  Copyright (c) 2017 Alireza Kheirkhahan
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
#include <utility>
#include <vector>

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
    std::vector<match_pattern_type> const mul_operation::match_data =
    {
        hpx::util::make_tuple("mul", "_1 * __2", &create<mul_operation>)
    };

    ///////////////////////////////////////////////////////////////////////////
    mul_operation::mul_operation(std::vector<primitive_argument_type>&& operands)
      : operands_(std::move(operands))
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
            primitive_result_type mul0d(operands_type && ops) const
            {
                operand_type& lhs = ops[0];
                operand_type& rhs = ops[1];

                std::size_t rhs_dims = rhs.num_dimensions();
                switch (rhs_dims)
                {
                case 0:
                    {
                        if (ops.size() == 2)
                        {
                            lhs[0] *= rhs[0];
                            return primitive_result_type{std::move(lhs)};
                        }

                        return primitive_result_type{std::accumulate(
                            ops.begin() + 1, ops.end(), std::move(lhs),
                            [](operand_type& result, operand_type const& curr)
                            {
                                result.matrix() *= curr[0];
                                return std::move(result);
                            })};
                    }
                    break;

                case 1: HPX_FALLTHROUGH;
                case 2:
                    {
                        if (ops.size() > 2)
                        {
                            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                                "mul_operation::mul0d",
                                "can't handle more than 2 operands when first "
                                 "operand is not a matrix");
                        }

                        rhs.matrix() = lhs[0] * rhs.matrix();
                        return primitive_result_type{std::move(rhs)};
                    }

                default:
                    HPX_THROW_EXCEPTION(hpx::bad_parameter,
                        "mul_operation::mul0d",
                        "the operands have incompatible number of dimensions");
                }
            }

            ///////////////////////////////////////////////////////////////////
            primitive_result_type mulxd(operands_type && ops) const
            {
                operand_type& lhs = ops[0];
                operand_type& rhs = ops[1];

                if (ops.size() == 2)
                {
                    if (rhs.num_dimensions() == 0)
                    {
                        lhs.matrix() *= rhs[0];
                        return std::move(lhs);
                    }

                    lhs.matrix() *= rhs.matrix();
                    return primitive_result_type{std::move(lhs)};
                }

                return primitive_result_type{std::accumulate(
                    ops.begin() + 1, ops.end(), std::move(lhs),
                    [](operand_type& result, operand_type const& curr)
                    ->  operand_type
                    {
                        if (curr.num_dimensions() == 0)
                        {
                            result.matrix() *= curr[0];
                        }
                        else
                        {
                            result.matrix() *= curr.matrix();
                        }
                        return std::move(result);
                    })};
            }

        public:
            hpx::future<primitive_result_type> eval(
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
                    [this_](operands_type&& ops) -> primitive_result_type
                    {
                        std::size_t lhs_dims = ops[0].num_dimensions();
                        switch (lhs_dims)
                        {
                        case 0:
                            return this_->mul0d(std::move(ops));

                        case 1: HPX_FALLTHROUGH;
                        case 2:
                            return this_->mulxd(std::move(ops));

                        default:
                            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                                "mul_operation::eval",
                                "left hand side operand has unsupported "
                                    "number of dimensions");
                        }
                    }),
                    detail::map_operands(operands, numeric_operand , args)
                );
            }
        };
    }

    // implement '*' for all possible combinations of lhs and rhs
    hpx::future<primitive_result_type> mul_operation::eval(
        std::vector<primitive_argument_type> const& args) const
    {
        if (operands_.empty())
        {
            static std::vector<primitive_argument_type> noargs;
            return std::make_shared<detail::mul>()->eval(args, noargs);
        }

        return std::make_shared<detail::mul>()->eval(operands_, args);
    }
}}}
