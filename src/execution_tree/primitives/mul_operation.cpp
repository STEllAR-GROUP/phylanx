//  Copyright (c) 2017 Hartmut Kaiser
//  Copyright (c) 2017 Alireza Kheirkhahan
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>
#include <phylanx/ast/detail/is_literal_value.hpp>
#include <phylanx/execution_tree/primitives/mul_operation.hpp>
#include <phylanx/ir/node_data.hpp>
#include <phylanx/util/optional.hpp>
#include <phylanx/util/serialization/eigen.hpp>
#include <phylanx/util/serialization/optional.hpp>

#include <hpx/include/components.hpp>
#include <hpx/include/lcos.hpp>
#include <hpx/include/util.hpp>

#include <cstddef>
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
    mul_operation::mul_operation(std::vector<primitive_argument_type>&& operands)
      : operands_(std::move(operands))
    {
        if (operands_.size() < 2)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "mul_operation::mul_operation",
                "the mul_operation primitive requires at least two "
                "operands");
        }

        bool arguments_valid = true;
        for (std::size_t i = 0; i != operands_.size(); ++i)
        {
            if (!valid(operands_[i]))
            {
                arguments_valid = false;
            }
        }

        if (!arguments_valid)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "mul_operation::mul_operation",
                "the mul_operation primitive requires that the arguments given "
                    "by the operands array is valid");
        }
    }

    ///////////////////////////////////////////////////////////////////////////
    ir::node_data<double> mul_operation::mul0d(operands_type const& ops) const
    {
        using matrix_type = Eigen::Matrix<double, Eigen::Dynamic, Eigen::Dynamic>;

        auto const& lhs = ops[0].value();
        auto const& rhs = ops[1].value();

        std::size_t rhs_dims = rhs.num_dimensions();
        switch (rhs_dims)
        {
        case 0:
            {
                if (ops.size() == 2)
                {
                    return lhs[0] * rhs[0];
                }

                return ir::node_data<double>(
                    std::accumulate(ops.begin() + 1, ops.end(), lhs[0],
                        [](double result, operand_type const& curr)
                        {
                            return result * curr.value()[0];
                        }));
            }
            break;

        case 1:
        case 2:
            {
                if (ops.size() > 2)
                {
                    HPX_THROW_EXCEPTION(hpx::bad_parameter,
                        "mul_operation::mul0d",
                        "can't handle more than 2 operands when first operand "
                        "is not a matrix");
                }

                matrix_type result = lhs[0] * rhs.matrix();
                return ir::node_data<double>(std::move(result));
            }

        default:
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "mul_operation::mul0d",
                "the operands have incompatible number of dimensions");
        }
    }

    ///////////////////////////////////////////////////////////////////////////
    ir::node_data<double> mul_operation::mulxd(operands_type const& ops) const
    {
        using matrix_type = Eigen::Matrix<double, Eigen::Dynamic, Eigen::Dynamic>;

        auto const& lhs = ops[0].value();
        auto const& rhs = ops[1].value();

        if (ops.size() == 2)
        {
            if (rhs.num_dimensions() == 0)
            {
                matrix_type result = lhs.matrix() * rhs[0];
                return ir::node_data<double>(std::move(result));
            }

            matrix_type result = lhs.matrix() * rhs.matrix();
            return ir::node_data<double>(std::move(result));
        }

        matrix_type first_term = ops.begin()->value().matrix();
        matrix_type result =
            std::accumulate(ops.begin() + 1, ops.end(), first_term,
                [](matrix_type& result, operand_type const& curr)
                ->  matrix_type
                {
                    auto const& val = curr.value();
                    if (val.num_dimensions() == 0)
                        return result *= val[0];
                    return result *= val.matrix();
                });

        return ir::node_data<double>(std::move(result));
    }

    // implement '*' for all possible combinations of lhs and rhs
    hpx::future<util::optional<ir::node_data<double>>> mul_operation::eval() const
    {
        return hpx::dataflow(hpx::util::unwrapping(
            [this](operands_type&& ops)
            {
                if (!detail::verify_argument_values(ops))
                {
                    HPX_THROW_EXCEPTION(hpx::bad_parameter,
                        "mul_operation::eval",
                        "the mul_operation primitive requires that the argument"
                            " values given by the operands array are non-empty");
                }

                std::size_t lhs_dims = ops[0].value().num_dimensions();
                switch (lhs_dims)
                {
                case 0:
                    return operand_type(mul0d(ops));

                case 1:
                case 2:
                    return operand_type(mulxd(ops));

                default:
                    HPX_THROW_EXCEPTION(hpx::bad_parameter,
                        "mul_operation::eval",
                        "left hand side operand has unsupported number of "
                        "dimensions");
                }
            }),
            detail::map_operands(operands_, evaluate_operand)
        );
    }
}}}
