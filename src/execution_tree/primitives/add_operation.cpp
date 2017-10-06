//  Copyright (c) 2017 Hartmut Kaiser
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>
#include <phylanx/ast/detail/is_literal_value.hpp>
#include <phylanx/execution_tree/primitives/add_operation.hpp>
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
    phylanx::execution_tree::primitives::add_operation>
    add_operation_type;
HPX_REGISTER_DERIVED_COMPONENT_FACTORY(
    add_operation_type, phylanx_add_operation_component,
    "phylanx_primitive_component", hpx::components::factory_enabled)
HPX_DEFINE_GET_COMPONENT_TYPE(add_operation_type::wrapped_type)

///////////////////////////////////////////////////////////////////////////////
namespace phylanx { namespace execution_tree { namespace primitives
{
    ///////////////////////////////////////////////////////////////////////////
    add_operation::add_operation(std::vector<primitive_argument_type>&& operands)
      : operands_(std::move(operands))
    {
        if (operands_.size() < 2)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "add_operation::add_operation",
                "the add_operation primitive requires at least two operands");
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
                "add_operation::add_operation",
                "the add_operation primitive requires that the arguments given "
                    "by the operands array are valid");
        }
    }

    ///////////////////////////////////////////////////////////////////////////
    ir::node_data<double> add_operation::add0d(operands_type const& ops) const
    {
        auto const& lhs = ops[0].value();
        auto const& rhs = ops[1].value();

        std::size_t rhs_dims = rhs.num_dimensions();
        switch(rhs_dims)
        {
        case 0:
            {
                if (ops.size() == 2)
                {
                    return lhs[0] + rhs[0];
                }

                return ir::node_data<double>(
                    std::accumulate(ops.begin() + 1, ops.end(), lhs[0],
                        [](double result, operand_type const& curr)
                        {
                            return result + curr.value()[0];
                        }));
            }
            break;

        case 1: HPX_FALLTHROUGH;
        case 2: HPX_FALLTHROUGH;
        default:
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "add_operation::add0d",
                "the operands have incompatible number of dimensions");
        }
    }

    ///////////////////////////////////////////////////////////////////////////
    ir::node_data<double> add_operation::add1d1d(operands_type const& ops) const
    {
        auto const& lhs = ops[0].value();
        auto const& rhs = ops[1].value();

        std::size_t lhs_size = lhs.dimension(0);
        std::size_t rhs_size = rhs.dimension(0);

        if(lhs_size  != rhs_size)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "add_operation::add1d1d",
                "the dimensions of the operands do not match");
        }

        using array_type = Eigen::Array<double, Eigen::Dynamic, 1>;
        using matrix_type = Eigen::Matrix<double, Eigen::Dynamic, 1>;

        if (ops.size() == 2)
        {
            matrix_type result = lhs.matrix().array() + rhs.matrix().array();
            return ir::node_data<double>(std::move(result));
        }

        array_type first_term = ops.begin()->value().matrix().array();
        matrix_type result =
            std::accumulate(
                ops.begin() + 1, ops.end(), first_term,
                [](array_type& result, operand_type const& curr)
                ->  array_type
                {
                    return result += curr.value().matrix().array();
                });

        return ir::node_data<double>(std::move(result));
    }

    ir::node_data<double> add_operation::add1d(operands_type const& ops) const
    {
        std::size_t rhs_dims = ops[1].value().num_dimensions();

        switch(rhs_dims)
        {
        case 1:
            return add1d1d(ops);

        case 0: HPX_FALLTHROUGH;
        case 2: HPX_FALLTHROUGH;
        default:
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "add_operation::add1d",
                "the operands have incompatible number of dimensions");
        }
    }

    ///////////////////////////////////////////////////////////////////////////
    ir::node_data<double> add_operation::add2d2d(operands_type const& ops) const
    {
        auto const& lhs = ops[0].value();
        auto const& rhs = ops[1].value();

        auto lhs_size = lhs.dimensions();
        auto rhs_size = rhs.dimensions();

        if (lhs_size != rhs_size)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "add_operation::add2d2d",
                "the dimensions of the operands do not match");
        }

        using array_type = Eigen::Array<double, Eigen::Dynamic, Eigen::Dynamic>;
        using matrix_type = Eigen::Matrix<double, Eigen::Dynamic, Eigen::Dynamic>;

        if (ops.size() == 2)
        {
            matrix_type result = lhs.matrix().array() + rhs.matrix().array();
            return ir::node_data<double>(std::move(result));
        }

        array_type first_term = ops.begin()->value().matrix().array();
        matrix_type result =
            std::accumulate(
                ops.begin() + 1, ops.end(), first_term,
                [](array_type& result, operand_type const& curr)
                ->  array_type
                {
                    return result += curr.value().matrix().array();
                });

        return ir::node_data<double>(std::move(result));
    }

    ir::node_data<double> add_operation::add2d(operands_type const& ops) const
    {
        std::size_t rhs_dims = ops[1].value().num_dimensions();
        switch(rhs_dims)
        {
        case 2:
            return add2d2d(ops);

        case 0: HPX_FALLTHROUGH;
        case 1: HPX_FALLTHROUGH;
        default:
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "add_operation::add2d",
                "the operands have incompatible number of dimensions");
        }
    }

    // implement '+' for all possible combinations of lhs and rhs
    hpx::future<util::optional<ir::node_data<double>>> add_operation::eval() const
    {
        return hpx::dataflow(hpx::util::unwrapping(
            [this](operands_type && ops)
            {
                if (detail::verify_argument_values(ops))
                {
                    HPX_THROW_EXCEPTION(hpx::bad_parameter,
                        "add_operation::eval",
                        "the add_operation primitive requires that the argument"
                            " values given by the operands array are non-empty");
                }

                std::size_t lhs_dims = ops[0].value().num_dimensions();
                switch (lhs_dims)
                {
                case 0:
                    return operand_type(add0d(ops));

                case 1:
                    return operand_type(add1d(ops));

                case 2:
                    return operand_type(add2d(ops));

                default:
                    HPX_THROW_EXCEPTION(hpx::bad_parameter,
                        "add_operation::eval",
                        "left hand side operand has unsupported number of "
                        "dimensions");
                }
            }),
            detail::map_operands(operands_, detail::extract_node_data)
        );
    }
}}}
