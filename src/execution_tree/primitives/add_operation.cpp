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
    match_pattern_type const add_operation::match_data =
    {
        "_1 + __2", &create<add_operation>
    };

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
    ir::node_data<double> add_operation::add0d0d(operands_type && ops) const
    {
        operand_type& lhs = ops[0];
        operand_type& rhs = ops[1];

        if (ops.size() == 2)
        {
            lhs[0] += rhs[0];
            return std::move(lhs);
        }

        return std::accumulate(
            ops.begin() + 1, ops.end(), std::move(lhs),
            [](operand_type& result, operand_type const& curr) -> operand_type
            {
                result[0] += curr[0];
                return std::move(result);
            });
}

    ir::node_data<double> add_operation::add0d1d(operands_type && ops) const
    {
        if (ops.size() != 2)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "add_operation::add0d1d",
                "the add_operation primitive can add a single value to a "
                    "vector only if there are exactly 2 operands");
        }

        ops[1].matrix().array() = ops[0][0] + ops[1].matrix().array();
        return std::move(ops[1]);
    }

    ir::node_data<double> add_operation::add0d2d(operands_type && ops) const
    {
        if (ops.size() != 2)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "add_operation::add0d2d",
                "the add_operation primitive can add a single value to a "
                    "matrix only if there are exactly 2 operands");
        }

        ops[1].matrix().array() = ops[0][0] + ops[1].matrix().array();
        return std::move(ops[1]);
    }

    ir::node_data<double> add_operation::add0d(operands_type && ops) const
    {
        std::size_t rhs_dims = ops[1].num_dimensions();
        switch(rhs_dims)
        {
        case 0:
            return add0d0d(std::move(ops));

        case 1:
            return add0d1d(std::move(ops));

        case 2:
            return add0d2d(std::move(ops));

        default:
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "add_operation::add0d",
                "the operands have incompatible number of dimensions");
        }
    }

    ///////////////////////////////////////////////////////////////////////////
    ir::node_data<double> add_operation::add1d0d(operands_type && ops) const
    {
        if (ops.size() != 2)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "add_operation::add0d1d",
                "the add_operation primitive can add a single value to a "
                    "vector only if there are exactly 2 operands");
        }

        ops[0].matrix().array() += ops[1][0];
        return std::move(ops[0]);
    }

    ir::node_data<double> add_operation::add1d1d(operands_type && ops) const
    {
        operand_type& lhs = ops[0];
        operand_type& rhs = ops[1];

        std::size_t lhs_size = lhs.dimension(0);
        std::size_t rhs_size = rhs.dimension(0);

        if (lhs_size != rhs_size)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "add_operation::add1d1d",
                "the dimensions of the operands do not match");
        }

        if (ops.size() == 2)
        {
            lhs.matrix().array() += rhs.matrix().array();
            return std::move(lhs);
        }

        operand_type& first_term = *ops.begin();
        return std::accumulate(
            ops.begin() + 1, ops.end(), std::move(first_term),
            [](operand_type& result, operand_type const& curr) -> operand_type
            {
                result.matrix().array() += curr.matrix().array();
                return std::move(result);
            });
    }

    ir::node_data<double> add_operation::add1d(operands_type && ops) const
    {
        std::size_t rhs_dims = ops[1].num_dimensions();

        switch(rhs_dims)
        {
        case 0:
            return add1d0d(std::move(ops));

        case 1:
            return add1d1d(std::move(ops));

        case 2: HPX_FALLTHROUGH;
        default:
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "add_operation::add1d",
                "the operands have incompatible number of dimensions");
        }
    }

    ///////////////////////////////////////////////////////////////////////////
    ir::node_data<double> add_operation::add2d0d(operands_type && ops) const
    {
        if (ops.size() != 2)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "add_operation::add0d2d",
                "the add_operation primitive can add a single value to a "
                    "matrix only if there are exactly 2 operands");
        }

        ops[0].matrix().array() += ops[1][0];
        return std::move(ops[0]);
    }

    ir::node_data<double> add_operation::add2d2d(operands_type && ops) const
    {
        operand_type& lhs = ops[0];
        operand_type& rhs = ops[1];

        auto lhs_size = lhs.dimensions();
        auto rhs_size = rhs.dimensions();

        if (lhs_size != rhs_size)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "add_operation::add2d2d",
                "the dimensions of the operands do not match");
        }

        if (ops.size() == 2)
        {
            lhs.matrix().array() += rhs.matrix().array();
            return std::move(lhs);
        }

        operand_type& first_term = *ops.begin();
        return std::accumulate(
            ops.begin() + 1, ops.end(), std::move(first_term),
            [](operand_type& result, operand_type const& curr) -> operand_type
            {
                result.matrix().array() += curr.matrix().array();
                return std::move(result);
            });
    }

    ir::node_data<double> add_operation::add2d(operands_type && ops) const
    {
        std::size_t rhs_dims = ops[1].num_dimensions();
        switch(rhs_dims)
        {
        case 0:
            return add2d0d(std::move(ops));

        case 2:
            return add2d2d(std::move(ops));

        case 1: HPX_FALLTHROUGH;
        default:
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "add_operation::add2d",
                "the operands have incompatible number of dimensions");
        }
    }

    // implement '+' for all possible combinations of lhs and rhs
    hpx::future<primitive_result_type> add_operation::eval() const
    {
        return hpx::dataflow(hpx::util::unwrapping(
            [this](operands_type && ops) -> primitive_result_type
            {
                std::size_t lhs_dims = ops[0].num_dimensions();
                switch (lhs_dims)
                {
                case 0:
                    return primitive_result_type(add0d(std::move(ops)));

                case 1:
                    return primitive_result_type(add1d(std::move(ops)));

                case 2:
                    return primitive_result_type(add2d(std::move(ops)));

                default:
                    HPX_THROW_EXCEPTION(hpx::bad_parameter,
                        "add_operation::eval",
                        "left hand side operand has unsupported number of "
                        "dimensions");
                }
            }),
            detail::map_operands(operands_, numeric_operand)
        );
    }
}}}
