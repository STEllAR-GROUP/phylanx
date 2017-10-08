//  Copyright (c) 2017 Hartmut Kaiser
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>
#include <phylanx/ast/detail/is_literal_value.hpp>
#include <phylanx/execution_tree/primitives/div_operation.hpp>
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
    phylanx::execution_tree::primitives::div_operation>
    div_operation_type;
HPX_REGISTER_DERIVED_COMPONENT_FACTORY(
    div_operation_type, phylanx_div_operation_component,
    "phylanx_primitive_component", hpx::components::factory_enabled)
HPX_DEFINE_GET_COMPONENT_TYPE(div_operation_type::wrapped_type)

///////////////////////////////////////////////////////////////////////////////
namespace phylanx { namespace execution_tree { namespace primitives
{
    ///////////////////////////////////////////////////////////////////////////
    match_pattern_type const div_operation::match_data =
    {
        "_1 / __2", &create<div_operation>
    };

    ///////////////////////////////////////////////////////////////////////////
    div_operation::div_operation(std::vector<primitive_argument_type>&& operands)
      : operands_(std::move(operands))
    {
        if (operands_.size() < 2)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "div_operation::div_operation",
                "the div_operation primitive requires at least two operands");
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
                "div_operation::div_operation",
                "the div_operation primitive requires that the arguments given "
                    "by the operands array are valid");
        }
    }

    ///////////////////////////////////////////////////////////////////////////
    ir::node_data<double> div_operation::div0d0d(operands_type const& ops) const
    {
        auto const& lhs = ops[0];
        auto const& rhs = ops[1];

        if (ops.size() == 2)
        {
            return lhs[0] / rhs[0];
        }

        return ir::node_data<double>(
            std::accumulate(ops.begin() + 1, ops.end(), lhs[0],
                [](double result, operand_type const& curr)
                {
                    return result / curr[0];
                }));
    }

    ir::node_data<double> div_operation::div0d1d(operands_type const& ops) const
    {
        if (ops.size() != 2)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "div_operation::div0d1d",
                "the div_operation primitive can div a single value to a "
                    "vector only if there are exectly 2 operands");
        }

        using matrix_type = Eigen::Matrix<double, Eigen::Dynamic, 1>;
        matrix_type result = ops[0][0] / ops[1].matrix().array();
        return ir::node_data<double>(std::move(result));
    }

    ir::node_data<double> div_operation::div0d2d(operands_type const& ops) const
    {
        if (ops.size() != 2)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "div_operation::div0d2d",
                "the div_operation primitive can div a single value to a "
                    "matrix only if there are exectly 2 operands");
        }

        using matrix_type = Eigen::Matrix<double, Eigen::Dynamic, Eigen::Dynamic>;
        matrix_type result = ops[0][0] / ops[1].matrix().array();
        return ir::node_data<double>(std::move(result));
    }

    ir::node_data<double> div_operation::div0d(operands_type const& ops) const
    {
        std::size_t rhs_dims = ops[1].num_dimensions();
        switch(rhs_dims)
        {
        case 0:
            return div0d0d(ops);

        case 1:
            return div0d1d(ops);

        case 2:
            return div0d2d(ops);

        default:
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "div_operation::div0d",
                "the operands have incompatible number of dimensions");
        }
    }

    ///////////////////////////////////////////////////////////////////////////
    ir::node_data<double> div_operation::div1d0d(operands_type const& ops) const
    {
        if (ops.size() != 2)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "div_operation::div0d1d",
                "the div_operation primitive can div a single value to a "
                    "vector only if there are exactly 2 operands");
        }

        using matrix_type = Eigen::Matrix<double, Eigen::Dynamic, 1>;
        matrix_type result = ops[0].matrix().array() / ops[1][0];
        return ir::node_data<double>(std::move(result));
    }

    ir::node_data<double> div_operation::div1d1d(operands_type const& ops) const
    {
        auto const& lhs = ops[0];
        auto const& rhs = ops[1];

        std::size_t lhs_size = lhs.dimension(0);
        std::size_t rhs_size = rhs.dimension(0);

        if (lhs_size  != rhs_size)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "div_operation::div1d1d",
                "the dimensions of the operands do not match");
        }

        using array_type = Eigen::Array<double, Eigen::Dynamic, 1>;
        using matrix_type = Eigen::Matrix<double, Eigen::Dynamic, 1>;

        if (ops.size() == 2)
        {
            matrix_type result = lhs.matrix().array() / rhs.matrix().array();
            return ir::node_data<double>(std::move(result));
        }

        array_type first_term = ops.begin()->matrix().array();
        matrix_type result =
            std::accumulate(
                ops.begin() + 1, ops.end(), std::move(first_term),
                [](array_type& result, operand_type const& curr)
                ->  array_type
                {
                    return result /= curr.matrix().array();
                });

        return ir::node_data<double>(std::move(result));
    }

    ir::node_data<double> div_operation::div1d(operands_type const& ops) const
    {
        std::size_t rhs_dims = ops[1].num_dimensions();

        switch(rhs_dims)
        {
        case 0:
            return div1d0d(ops);

        case 1:
            return div1d1d(ops);

        case 2: HPX_FALLTHROUGH;
        default:
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "div_operation::div1d",
                "the operands have incompatible number of dimensions");
        }
    }

    ///////////////////////////////////////////////////////////////////////////
    ir::node_data<double> div_operation::div2d0d(operands_type const& ops) const
    {
        if (ops.size() != 2)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "div_operation::div0d2d",
                "the div_operation primitive can div a single value to a "
                    "matrix only if there are exactly 2 operands");
        }

        using matrix_type = Eigen::Matrix<double, Eigen::Dynamic, Eigen::Dynamic>;
        matrix_type result = ops[0].matrix().array() / ops[1][0];
        return ir::node_data<double>(std::move(result));
    }

    ir::node_data<double> div_operation::div2d2d(operands_type const& ops) const
    {
        auto const& lhs = ops[0];
        auto const& rhs = ops[1];

        auto lhs_size = lhs.dimensions();
        auto rhs_size = rhs.dimensions();

        if (lhs_size != rhs_size)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "div_operation::div2d2d",
                "the dimensions of the operands do not match");
        }

        using array_type = Eigen::Array<double, Eigen::Dynamic, Eigen::Dynamic>;
        using matrix_type = Eigen::Matrix<double, Eigen::Dynamic, Eigen::Dynamic>;

        if (ops.size() == 2)
        {
            matrix_type result = lhs.matrix().array() / rhs.matrix().array();
            return ir::node_data<double>(std::move(result));
        }

        array_type first_term = ops.begin()->matrix().array();
        matrix_type result =
            std::accumulate(
                ops.begin() + 1, ops.end(), std::move(first_term),
                [](array_type& result, operand_type const& curr)
                ->  array_type
                {
                    return result /= curr.matrix().array();
                });

        return ir::node_data<double>(std::move(result));
    }

    ir::node_data<double> div_operation::div2d(operands_type const& ops) const
    {
        std::size_t rhs_dims = ops[1].num_dimensions();
        switch(rhs_dims)
        {
        case 0:
            return div2d0d(ops);

        case 2:
            return div2d2d(ops);

        case 1: HPX_FALLTHROUGH;
        default:
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "div_operation::div2d",
                "the operands have incompatible number of dimensions");
        }
    }

    // implement '+' for all possible combinations of lhs and rhs
    hpx::future<primitive_result_type> div_operation::eval() const
    {
        return hpx::dataflow(hpx::util::unwrapping(
            [this](operands_type && ops) -> primitive_result_type
            {
                std::size_t lhs_dims = ops[0].num_dimensions();
                switch (lhs_dims)
                {
                case 0:
                    return primitive_result_type(div0d(ops));

                case 1:
                    return primitive_result_type(div1d(ops));

                case 2:
                    return primitive_result_type(div2d(ops));

                default:
                    HPX_THROW_EXCEPTION(hpx::bad_parameter,
                        "div_operation::eval",
                        "left hand side operand has unsupported number of "
                        "dimensions");
                }
            }),
            detail::map_operands(operands_, numeric_operand)
        );
    }
}}}
