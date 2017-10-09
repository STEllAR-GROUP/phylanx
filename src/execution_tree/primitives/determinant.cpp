//   Copyright (c) 2017 Bibek Wagle
//
//   Distributed under the Boost Software License, Version 1.0. (See accompanying
//   file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>
#include <phylanx/ast/detail/is_literal_value.hpp>
#include <phylanx/execution_tree/primitives/determinant.hpp>
#include <phylanx/ir/node_data.hpp>
#include <phylanx/util/serialization/eigen.hpp>

#include <hpx/include/components.hpp>
#include <hpx/include/lcos.hpp>
#include <hpx/include/util.hpp>

#include <cmath>
#include <cstddef>
#include <utility>
#include <vector>

#include <unsupported/Eigen/MatrixFunctions>

///////////////////////////////////////////////////////////////////////////////
typedef hpx::components::component<
    phylanx::execution_tree::primitives::determinant>
    determinant_type;
HPX_REGISTER_DERIVED_COMPONENT_FACTORY(
    determinant_type, phylanx_determinant_component,
    "phylanx_primitive_component", hpx::components::factory_enabled)
HPX_DEFINE_GET_COMPONENT_TYPE(determinant_type::wrapped_type)

///////////////////////////////////////////////////////////////////////////////
namespace phylanx { namespace execution_tree { namespace primitives
{
    ///////////////////////////////////////////////////////////////////////////
    match_pattern_type const determinant::match_data =
    {
        "transpose(_1)", &create<determinant>
    };

    ///////////////////////////////////////////////////////////////////////////
    determinant::determinant(
            std::vector<primitive_argument_type>&& operands)
      : operands_(std::move(operands))
    {
        if (operands_.size() != 1)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "determinant::determinant",
                "the determinant primitive requires"
                "exactly one operand");
        }

        if (!valid(operands_[0]))
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "determinant::determinant",
                "the determinant primitive requires that the "
                    "arguments given by the operands array is valid");
        }
    }

    ///////////////////////////////////////////////////////////////////////////
    ir::node_data<double> determinant::determinant0d(operands_type && ops) const
    {
        return std::move(ops[0]);       // no-op
    }

    ir::node_data<double> determinant::determinantxd(operands_type&& ops) const
    {
        using matrix_type =
            Eigen::Matrix<double, Eigen::Dynamic, Eigen::Dynamic>;

        double result = ops[0].matrix().determinant();
        return ir::node_data<double>(result);
    }

    hpx::future<primitive_result_type> determinant::eval() const
    {
        return hpx::dataflow(hpx::util::unwrapping(
            [this](operands_type&& ops) -> primitive_result_type
            {
                std::size_t dims = ops[0].num_dimensions();
                switch (dims)
                {
                case 0:
                    return primitive_result_type(determinant0d(std::move(ops)));

                case 1: HPX_FALLTHROUGH;
                case 2:
                    return primitive_result_type(determinantxd(std::move(ops)));

                default:
                    HPX_THROW_EXCEPTION(hpx::bad_parameter,
                        "determinant::eval",
                        "left hand side operand has unsupported number of "
                            "dimensions");
                }
            }),
            detail::map_operands(operands_, numeric_operand)
        );
    }
}}}
