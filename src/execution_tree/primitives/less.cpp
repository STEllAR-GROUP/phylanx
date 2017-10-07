//  Copyright (c) 2017 Hartmut Kaiser
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>
#include <phylanx/ast/detail/is_literal_value.hpp>
#include <phylanx/execution_tree/primitives/less.hpp>
#include <phylanx/ir/node_data.hpp>

#include <hpx/include/components.hpp>
#include <hpx/include/lcos.hpp>
#include <hpx/include/util.hpp>

#include <cstddef>
#include <numeric>
#include <utility>
#include <vector>

///////////////////////////////////////////////////////////////////////////////
typedef hpx::components::component<
    phylanx::execution_tree::primitives::less>
    less_type;
HPX_REGISTER_DERIVED_COMPONENT_FACTORY(
    less_type, phylanx_less_component,
    "phylanx_primitive_component", hpx::components::factory_enabled)
HPX_DEFINE_GET_COMPONENT_TYPE(less_type::wrapped_type)

///////////////////////////////////////////////////////////////////////////////
namespace phylanx { namespace execution_tree { namespace primitives
{
    ///////////////////////////////////////////////////////////////////////////
    match_pattern_type const less::match_data =
    {
        "_1 < _2", &create<less>
    };

    ///////////////////////////////////////////////////////////////////////////
    less::less(std::vector<primitive_argument_type>&& operands)
      : operands_(std::move(operands))
    {
        if (operands_.size() != 2)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "less::less",
                "the less primitive requires exactly two operands");
        }

        if (!valid(operands_[0]) || !valid(operands_[1]))
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "less::less",
                "the less primitive requires that the arguments given "
                    "by the operands array are valid");
        }
    }

    ///////////////////////////////////////////////////////////////////////////
    ir::node_data<double> less::less0d(operands_type const& ops) const
    {
        auto const& lhs = ops[0].value();
        auto const& rhs = ops[1].value();

        std::size_t rhs_dims = rhs.num_dimensions();
        switch(rhs_dims)
        {
        case 0:
            return lhs[0] < rhs[0];

        case 1: HPX_FALLTHROUGH;
        case 2: HPX_FALLTHROUGH;
        default:
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "less::less0d",
                "the operands have incompatible number of dimensions");
        }
    }

    ///////////////////////////////////////////////////////////////////////////
    ir::node_data<double> less::less1d1d(operands_type const& ops) const
    {
        auto const& lhs = ops[0].value();
        auto const& rhs = ops[1].value();

        std::size_t lhs_size = lhs.dimension(0);
        std::size_t rhs_size = rhs.dimension(0);

        if (lhs_size  != rhs_size)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "less::less1d1d",
                "the dimensions of the operands do not match");
        }

        using matrix_type = Eigen::Matrix<double, Eigen::Dynamic, 1>;

        matrix_type result =
            (lhs.matrix().array() < rhs.matrix().array()).cast<double>();
        return ir::node_data<double>(std::move(result));
    }

    ir::node_data<double> less::less1d(operands_type const& ops) const
    {
        std::size_t rhs_dims = ops[1].value().num_dimensions();

        switch(rhs_dims)
        {
        case 1:
            return less1d1d(ops);

        case 0: HPX_FALLTHROUGH;
        case 2: HPX_FALLTHROUGH;
        default:
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "less::less1d",
                "the operands have incompatible number of dimensions");
        }
    }

    ///////////////////////////////////////////////////////////////////////////
    ir::node_data<double> less::less2d2d(operands_type const& ops) const
    {
        auto const& lhs = ops[0].value();
        auto const& rhs = ops[1].value();

        auto lhs_size = lhs.dimensions();
        auto rhs_size = rhs.dimensions();

        if (lhs_size != rhs_size)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "less::less2d2d",
                "the dimensions of the operands do not match");
        }

        using matrix_type = Eigen::Matrix<double, Eigen::Dynamic, Eigen::Dynamic>;

        matrix_type result =
            (lhs.matrix().array() < rhs.matrix().array()).cast<double>();
        return ir::node_data<double>(std::move(result));
    }

    ir::node_data<double> less::less2d(operands_type const& ops) const
    {
        std::size_t rhs_dims = ops[1].value().num_dimensions();
        switch(rhs_dims)
        {
        case 2:
            return less2d2d(ops);

        case 0: HPX_FALLTHROUGH;
        case 1: HPX_FALLTHROUGH;
        default:
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "less::less2d",
                "the operands have incompatible number of dimensions");
        }
    }

    // implement '<' for all possible combinations of lhs and rhs
    hpx::future<util::optional<ir::node_data<double>>> less::eval() const
    {
        return hpx::dataflow(hpx::util::unwrapping(
            [this](operands_type && ops)
            {
                if (!detail::verify_argument_values(ops))
                {
                    HPX_THROW_EXCEPTION(hpx::bad_parameter,
                        "less::eval",
                        "the less primitive requires that the argument"
                            " values given by the operands array are non-empty");
                }

                std::size_t lhs_dims = ops[0].value().num_dimensions();
                switch (lhs_dims)
                {
                case 0:
                    return operand_type(less0d(ops));

                case 1:
                    return operand_type(less1d(ops));

                case 2:
                    return operand_type(less2d(ops));

                default:
                    HPX_THROW_EXCEPTION(hpx::bad_parameter,
                        "less::eval",
                        "left hand side operand has unsupported number of "
                        "dimensions");
                }
            }),
            detail::map_operands(operands_, evaluate_operand)
        );
    }
}}}
