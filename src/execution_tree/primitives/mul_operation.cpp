//  Copyright (c) 2017 Hartmut Kaiser
//  Copyright (c) 2017 Alireza Kheirkhahan
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>
#include <phylanx/ast/detail/is_literal_value.hpp>
#include <phylanx/execution_tree/primitives/mul_operation.hpp>
#include <phylanx/ir/node_data.hpp>
#include <phylanx/util/serialization/eigen.hpp>

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

        std::size_t rhs_dims = ops[1].num_dimensions();
        switch (rhs_dims)
        {
        case 0:
            {
                if (ops.size() == 2)
                {
                    return ops[0][0] * ops[1][0];
                }

                return ir::node_data<double>(
                    std::accumulate(ops.begin() + 1, ops.end(), ops[0][0],
                        [](double result, ir::node_data<double> const& curr)
                        {
                            return result * curr[0];
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

                matrix_type result = ops[0][0] * ops[1].matrix();
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

        if (ops.size() == 2)
        {
            if (ops[1].num_dimensions() == 0)
            {
                matrix_type result = ops[0].matrix() * ops[1][0];
                return ir::node_data<double>(std::move(result));
            }

            matrix_type result = ops[0].matrix() * ops[1].matrix();
            return ir::node_data<double>(std::move(result));
        }

        matrix_type first_term = ops.begin()->matrix();
        matrix_type result =
            std::accumulate(ops.begin() + 1, ops.end(), first_term,
                [](matrix_type& result, ir::node_data<double> const& curr)
                ->  matrix_type
                {
                    if (curr.num_dimensions() == 0)
                        return result *= curr[0];
                    return result *= curr.matrix();
                });

        return ir::node_data<double>(std::move(result));
    }

    // implement '*' for all possible combinations of lhs and rhs
    hpx::future<ir::node_data<double>> mul_operation::eval() const
    {
        return hpx::dataflow(hpx::util::unwrapping(
            [this](std::vector<ir::node_data<double>>&& ops)
            {
                std::size_t lhs_dims = ops[0].num_dimensions();
                switch (lhs_dims)
                {
                case 0:
                    return mul0d(ops);

                case 1:
                case 2:
                    return mulxd(ops);

                default:
                    HPX_THROW_EXCEPTION(hpx::bad_parameter,
                        "mul_operation::eval",
                        "left hand side operand has unsupported number of "
                        "dimensions");
                }
            }),
            detail::map_operands(operands_,
                [](primitive_argument_type const& val)
                ->  hpx::future<ir::node_data<double>>
                {
                    primitive const* p = util::get_if<primitive>(&val);
                    if (p != nullptr)
                        return p->eval();

                    HPX_ASSERT(valid(val));
                    return hpx::make_ready_future(extract_literal_value(val));
                }));
    }
}}}
