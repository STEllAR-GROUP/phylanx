//   Copyright (c) 2017 Bibek Wagle
//
//   Distributed under the Boost Software License, Version 1.0. (See accompanying
//   file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>
#include <phylanx/ast/detail/is_literal_value.hpp>
#include <phylanx/execution_tree/primitives/exponential_operation.hpp>
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
    phylanx::execution_tree::primitives::exponential_operation>
    exponential_operation_type;
HPX_REGISTER_DERIVED_COMPONENT_FACTORY(
    exponential_operation_type, phylanx_exponential_operation_component,
    "phylanx_primitive_component", hpx::components::factory_enabled)
HPX_DEFINE_GET_COMPONENT_TYPE(exponential_operation_type::wrapped_type)

///////////////////////////////////////////////////////////////////////////////
namespace phylanx { namespace execution_tree { namespace primitives
{
    ///////////////////////////////////////////////////////////////////////////
    match_pattern_type const exponential_operation::match_data =
    {
        "exp(_1)", &create<exponential_operation>
    };

    ///////////////////////////////////////////////////////////////////////////
    exponential_operation::exponential_operation(
            std::vector<primitive_argument_type>&& operands)
      : operands_(std::move(operands))
    {
        if (operands_.size() != 1)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "exponential_operation::exponential_operation",
                "the exponential_operation primitive requires"
                "exactly one operand");
        }

        if (!valid(operands_[0]))
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "exponential_operation::exponential_operation",
                "the exponential_operation primitive requires "
                "that the arguments given by the operands array"
                " is valid");
        }
    }

    ///////////////////////////////////////////////////////////////////////////
    ir::node_data<double> exponential_operation::exponential0d(
        operands_type && ops) const
    {
        return operand_type(std::exp(ops[0][0]));
    }

    ir::node_data<double> exponential_operation::exponential1d(
        operands_type && ops) const
    {
        auto const& val = ops[0].matrix();
        if (val.rows() != val.cols())
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "exponential_operation::exponential1d",
                "matrix exponentiation requires quadratic matrices");
        }

        using matrix_type = Eigen::Matrix<double, Eigen::Dynamic, 1>;

        matrix_type result = ops[0].matrix().exp();
        return ir::node_data<double>(std::move(result));
    }

    ir::node_data<double> exponential_operation::exponentialxd(
        operands_type && ops) const
    {
        auto const& val = ops[0].matrix();
        if (val.rows() != val.cols())
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "exponential_operation::exponential1d",
                "matrix exponentiation requires quadratic matrices");
        }

        using matrix_type =
            Eigen::Matrix<double, Eigen::Dynamic, Eigen::Dynamic>;

        matrix_type result = ops[0].matrix().exp();
        return ir::node_data<double>(std::move(result));
    }

    hpx::future<primitive_result_type> exponential_operation::eval() const
    {
        return hpx::dataflow(hpx::util::unwrapping(
            [this](operands_type&& ops) -> primitive_result_type
            {
                std::size_t dims = ops[0].num_dimensions();
                switch (dims)
                {
                case 0:
                    return primitive_result_type(exponential0d(std::move(ops)));

                case 1:
                    return primitive_result_type(exponential1d(std::move(ops)));

                case 2:
                    return primitive_result_type(exponentialxd(std::move(ops)));

                default:
                    HPX_THROW_EXCEPTION(hpx::bad_parameter,
                        "exponential_operation::eval",
                        "left hand side operand has unsupported number of "
                            "dimensions");
                }
            }),
            detail::map_operands(operands_, numeric_operand)
        );
    }
}}}
